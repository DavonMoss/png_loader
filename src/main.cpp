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
	std::vector<unsigned char> buffer;
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

//void inflate_data(std::vector<unsigned char> deflate_data) {
///*
//note: data is organized in 'blocks' which do not strictly adhere to byte boundaries
//
//read bits in from deflate_data
//- first 3 bits are block header
//	- first bit 	= BFINAL, 1 if this is last block, 0 if not
//	- next 2	= BTYPE, data compression type, 00 = none, 01 = fixed huff, 10 = dynamic huff, 11 = error
//
//*/
//    bool bfinal = 0;
//    bool btype[2] = {0, 0}; // LOL
//    std::uint16_t len, nlen;
//    std::queue<unsigned char> uncompressed_bytes;
//    int distance;
//    int bsp = 0; // 'byte_stream_position', tracks current byte from deflate stream
//    int bp = 0; // 'bit_position', tracks current bit in current byte
//
//    while(!bfinal) {
//        bfinal = deflate_data[bsp];
//        btype = /* next two bits */
//
//        if(btype == /*no compression*/) {
//            /* move input stream to next byte boundary */
//            len = /* next two bytes */
//            nlen = /* next two bytes after that */
//            
//            for(int i = 0; i < len; i++) {
//                uncompressed_bytes.push_back(deflate_data[i]);
//            }
//        } else {
//            if(btype == /*dynamic huff*/) {
//                /*store representation of code trees*/
//            }
//
//            for(;;) {
//                /*decode literal/length val from deflate_data*/
//                if(/*that value*/ < 256) {
//                    uncompressed_bytes.push_back(/*that value a.k.a literal byte*/);
//                } else {
//                    if(/*that value*/ == 256) {
//                        break;
//                    } else /*implied here that value is from 257..285*/ {
//                        distance = /*decode distance from deflate_data*/
//                      
//                        for(int i = 0; i < /*that value*/; i++) {
//                            uncompressed_bytes.push_back(uncompressed_bytes[(/*current_pos*/ - distance) + i]);
//                            /*maybe could do this cleaner by just copying sub string*/
//                        }  
//                    }
//                }
//            }
//        }
//    }
//
//}

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
