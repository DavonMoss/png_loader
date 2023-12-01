// the holy resource:
//https://handmade.network/forums/articles/t/2822-tutorial_implementing_a_basic_png_reader_the_handmade_way 

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

#include "bits.h"

/********************************
*	VARIABLES AND SHIT
*********************************/

static const int CHUNK_SIZE_BYTES = 4;  // number of bytes used to represent the size of a chunk
static const int CHUNK_NAME_BYTES = 4;  // number of bytes used to represent the name of a chunk
static const int CHUNK_CRC_LENGTH = 4;  // length of CRC suffix for each chunk

static const std::string IEND = "IEND";
static const std::string IDAT = "IDAT";

struct chunk_type {
    std::uint32_t size;
    std::string name;
};

struct zlib_data {
	unsigned char cmf;
	unsigned char flg; 
};

struct png_data {
	// likely going to expand to hold tons of chunk header info
	zlib_data zlib;
	std::vector<unsigned char> buffer; 	// deflate buffer
};


/********************************
*	UTIL FUNCTIONS
*********************************/

/*
 * Converts an array of 4 bytes into an 32 bit integer (32 bit).
 */
std::uint32_t hex_bytes_to_int(unsigned char* bytes) {
    std::uint32_t length = 0;

    for(int i = 0; i < 4; i++) {
	length = length << 8;
        length = length | bytes[i];	
    }

    return length;  
}

/*
	Prints error message and exits program.
*/
void fail(const char * msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(0);
}

/********************************
*	PNG SPECIFIC STUFF
*********************************/

/*
 * Checks the first 8 bytes of a file against the expechunked PNG signature.
 * */
bool verify_png_signature(std::ifstream* file) {
    const unsigned char PNG_SIGNATURE[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    int i = 0;

    while(file->good() && i < 8) {
        if(file->get() != PNG_SIGNATURE[i])
	    return false;

	i++;
    }
    
    return true;
}

/*
	Stores name and size of chunk. Must be called AFTER verify_png_signature.
 */
void determine_chunk_type(std::ifstream* file, chunk_type* chunk) {
    unsigned char chunk_size_bytes[CHUNK_SIZE_BYTES];
    unsigned char chunk_name_bytes[CHUNK_NAME_BYTES];

    // get size of chunk 
    for(int i = 0; i < CHUNK_SIZE_BYTES; i++) {
        chunk_size_bytes[i] = file->get();
    }
    chunk->size = hex_bytes_to_int(chunk_size_bytes);

    // get name of chunk
    for(int i = 0; i < CHUNK_NAME_BYTES; i++) {
        chunk_name_bytes[i] = file->get();
    }
    std::string chunk_name((const char*)chunk_name_bytes, CHUNK_NAME_BYTES);
    chunk->name = chunk_name;
}

/*
	Processes the chunk based on what type it is. Stores relevant data in 
	png_data object. Must be called AFTER determine_chunk_type.
*/
void process_chunk(std::ifstream* file, chunk_type* chunk, png_data* png) {
    //process chunk data
    for(int i = 0; i < chunk->size; i++) {
		//@CLEANUP: make this a nice robust function that can handle each chunk
		if(chunk->name == IDAT) {
        	png->buffer.push_back((unsigned char)file->get());
		} else {
        	file->get();
		}
    }

    //process CRC at end of chunk 
    for(int i = 0; i < CHUNK_CRC_LENGTH; i++) {
        file->get();
    } 
}

/*
	Parses through the PNG chunks and stores data.
	Decoding does NOT happen here. It just stores the IDAT bytes
	for later decoding via inflate_data.
*/
void parse_png(std::ifstream* file, png_data* png) {
    chunk_type chunk;

    if(!verify_png_signature(file)) {
		fail("Signature Invalid! Not a PNG.");
    } else {
        while(file->good() && chunk.name != IEND){
            determine_chunk_type(file, &chunk);

            std::cout << "Name: \"" << chunk.name << "\"\n";
            std::cout << "Length: " << chunk.size << '\n';

            process_chunk(file, &chunk, png);
        }
	}
}

/*
	Plucks out zlib headers from data buffer. Result is
	that data buffer only contains DEFLATE stream.
*/
void parse_zlib(png_data* png) {
	//@CLEANUP: this is hacky, need to do this cleaner and actually	handle checksum
    png->zlib.cmf = png->buffer[0];
    png->zlib.flg = png->buffer[1];
    std::vector<unsigned char> temp;

    for(int i = 2; i < png->buffer.size() - 4; i++) {
        temp.push_back(png->buffer[i]);
    }

	png->buffer = temp;
}

void get_huffman_tree(std::uint32_t number_of_codes, std::vector<std::uint32_t> alphabet, std::vector<std::uint32_t> code_lengths) {
	/*
	ASSUMPTIONS:
		- we are constructing the tree for some known alphabet [lit/len = 0 to 285, hclen = that weird ordered shit, etc]
		- the code lengths input is guaranteed to be the same size as the alphabet it represents

	STEPS:
		- count the occurrences of each length in 'code_lengths', store them in hashmap 'code_length_counts'. if n is a random length, this means 'code_length_counts[n]' is the number of times it appears
		- determine the base code of that given length
			- so for example, if code_length_counts[3] = 2, then we're determining the code for the first symbol encoded with a 3bit code, and then we'll derive the 3bit code for the second symbol later
	*/
	std::unordered_map<std::uint32_t, int> code_length_counts;
	std::vector<std::uint32_t> base_codes;
	std::uint32_t base_code, max_bits;

	// counting occurrences of each length
	for(int i = 0; i < code_lengths.size(); i++) {
		code_length_counts[code_lengths[i]]++;
	}

	// generating base codes from lengths
	max_bits = code_length_counts.size() - 1; //idea here is that map size == longest code length + 1
	base_code = 0;
	code_length_counts[0] = 0;
	base_codes.push_back(0);
	for(int bits = 1; bits <= max_bits; i++) {
		base_code = (base_code + code_length_counts[bits - 1]) << 1;
		base_codes.push_back(base_code); //@TODO: there is a problem here, the vector is not initialized to proper size. so we cant just do this.
	}
}

void inflate_data(std::vector<unsigned char> deflate_data) {
/*
note: data is organized in 'blocks' which do not strictly adhere to byte boundaries

read bits in from deflate_data
- first 3 bits are block header
	- first bit 	= BFINAL, 1 if this is last block, 0 if not
	- next 2	= BTYPE, data compression type, 00 = none, 01 = fixed huff, 10 = dynamic huff, 11 = error

*/
	enum BTYPE {
		NONE = 0,
		FIXED_HUFF = 1,
		DYN_HUFF = 2,
		ERROR = 3
	};

	// convert vector to a bit_stream so i can do stuff with bits
	bit_stream bits(deflate_data);
	std::uint32_t bit_buff, bfinal = 0, btype, len, nlen;
	std::vector<unsigned char> uncompressed_bytes;
	std::uint32_t hlit, hdist, hclen;

    while(bfinal) {
		/* processing header */
        bits.get_n_bits(1, &bfinal);	// get 1 bit from data
        bits.get_n_bits(2, &btype);		// get 2 bits from data

        if(btype == BTYPE::NONE) {
            /* move input stream to next byte boundary */
			bits.get_n_bits((bits.bit_count % 8), &bit_buff);

			/* grab len and nlen values*/
            bits.get_n_bits(16, &len);
            bits.get_n_bits(16, &nlen);
           
		   	/* just read the next 'len' bytes as char literals */
            for(int i = 0; i < len; i++) {
				bits.get_n_bits(8, &bit_buff)
                uncompressed_bytes.push_back((unsigned char)bit_buff);
            }
        } else {
            if(btype == BTYPE::DYN_HUFF) {
                /*@ TODO: store representation of code trees */
				/*
					Structure of block with dynamic huffman coding
					[3 bits] header
					[5 bits] hlit: this is the number of codes in the lit/len alphabet, minus 257. so we store this number and add 257 to it later to get the whole count
					[5 bits] hdist: this is the number of codes in the distance alphabet, minus 1. same idea as above
					[4 bits] hclen: this is the number of codes in the tree that encodes the two trees above, minus 4. same idea
					[(hclen + 4) x 3 bits] this is the actual AHHAHAHJSIKJDHAJKSHFAJIKSFGBAJIKSFB
					[hlit + 257 bits] the actual code lengths for the lit/len alphabet, but compressed using the hclen code
					[hdist + 1 bits] the actual code lengths for the distance alphabet, but compressed using the hclen code
					[n bits] the actual compressed data, encoded using the hdist and hlit codes
					[? bits] the symbol 256, encoded using the hlit code
				*/
				struct deflate_block {
					std::uint32_t hlit, hdist, hclen;
				};

				bits.get_n_bits(5, &hlit);
				bits.get_n_bits(5, &hdist);
				bits.get_n_bits(4, &hclen);

				hlit += 257;
				hdist += 1;
				hclen += 4;
            }

            for(;;) {
                /*decode literal/length val from deflate_data*/
                if(/*that value*/ < 256) {
                    uncompressed_bytes.push_back(/*that value a.k.a literal byte*/);
                } else {
                    if(/*that value*/ == 256) {
                        break;
                    } else /*implied here that value is from 257..285*/ {
                        distance = /*decode distance from deflate_data*/
                      
                        for(int i = 0; i < /*that value*/; i++) {
                            uncompressed_bytes.push_back(uncompressed_bytes[(/*current_pos*/ - distance) + i]);
                            /*maybe could do this cleaner by just copying sub string*/
                        }  
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fail("Invalid number of arguments. Give me a filename please!");
    }

    const char * filepath;
    std::ifstream file;
	png_data png;

	filepath = argv[1];
    file.open(filepath, std::ios::binary);

    parse_png(&file, &png);
    parse_zlib(&png);
	//inflate_data(png.buffer);

	///////////////////////////////////////
	///////	  TEST CODE CAN DELETE	///////
	///////////////////////////////////////
	bit_stream test(png.buffer);
	std::uint32_t bb;
	int spacer = 0;

	while(test.get_n_bits(1, &bb)) {
		std::cout << bb;
		spacer++;

		if((spacer % 8) == 0) {
			std::cout << " ";
		}
	}

	std::cout << "\n";
	///////////////////////////////////////
	///////////////////////////////////////
	///////////////////////////////////////

    file.close();

    return 0;
}
