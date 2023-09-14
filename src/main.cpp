#include <iostream>
#include <fstream>

#include "buffer.h"

static const int CHUNK_SIZE_BYTES = 4;  // number of bytes used to represent the size of a chunk
static const int CHUNK_NAME_BYTES = 4;  // number of bytes used to represent the name of a chunk
static const int CHUNK_CRC_LENGTH = 4;  // length of CRC suffix for each chunk

static const std::string IEND = "IEND";
static const std::string IDAT = "IDAT";

/*
 * Struct to describe the type of chunk we're dealing with.
 */
struct chunk_type {
    unsigned long int size;
    std::string name;
};

/*
 * HEX BYTES TO INT
 *
 * Converts an array of 4 bytes into an unsigned long integer (32 bit).
 *
 * PARAMS:
 * unsigned char* bytes - the array of bytes to be converted
 *
 * RETURNS:
 * Value of those bytes as a 32 bit unsigned long integer.  
 */
unsigned long int hex_bytes_to_int(unsigned char* bytes) {
    unsigned long int length = 0;

    for(int i = 0; i < 4; i++) {
	length = length << 8;		// shift bits up and make space to flash our char bits on
        length = length | bytes[i];	// write the bits of our char onto the bottom 8 bits of the final answer
    }

    return length;  
}

/*
 * VERIFY PNG SIGNATURE
 *
 * Checks the first 8 bytes of a file against the expechunked PNG signature.
 *
 * PARAMS:
 * std::ifstream* file 	- Pointer to Input File Stream objechunk from C++ fstream. Requires file to have already been opened.
 * 
 * RETURNS (bool):
 * TRUE if the signature matches, FALSE if it doesn't.
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
 * DETERMINE CHUNK TYPE
 *
 * Operates on the first CHUNK_SIZE_BYTES + CHUNK_NAME_BYTES bytes of a chunk to determine the size and name.
 * This must be called in the correchunk position in the file stream.
 *
 * PARAMS: 
 * std::ifstream* file - Pointer to Input File Stream objechunk from C++ fstream. Requires file to hae already been opened.
 * chunk_type* chunk - Pointer to the chunk_type struchunk where we will store what we find.
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
    std::string chunk_name((const char*)chunk_name_bytes);
    chunk->name = chunk_name;
}

//@PLACEHOLDER: this just iterates through the length of the chunk so that we can test back to back chunk processing 
void process_chunk(std::ifstream* file, chunk_type* chunk) {
    //process chunk data
    for(int i = 0; i < chunk->size; i++) {
        file->get();
    }

    //process CRC at end of chunk 
    for(int i = 0; i < CHUNK_CRC_LENGTH; i++) {
        file->get();
    } 
}

// extract bytes from IDAT into buffer
void pull_idat_bytes(std::ifstream* file, chunk_type* chunk, encoded_data_buffer* buffer) {
    for(int i = 0; i < chunk->size; i++) {
        buffer->load((unsigned char)file->get());
    }

    //process CRC at end of chunk 
    for(int i = 0; i < CHUNK_CRC_LENGTH; i++) {
        file->get();
    } 
}

int main(int argc, char** argv) {

    // take filename as input to CLI
    if (argc != 2) {
        std::cout << "Invalid number of arguments. Give me a filename please!" << std::endl;
	return -1;
    }

    const char * filepath = argv[1];

    std::ifstream file;
    file.open(filepath, std::ios::binary);

    if(!verify_png_signature(&file)) {
        std::cout << "Signature Invalid! Not a PNG.\n";
    } else {
        chunk_type chunk;
        encoded_data_buffer encoded_bytes;

        while(file.good() && chunk.name != IEND){
            determine_chunk_type(&file, &chunk);

            if(chunk.name == IDAT) {
                pull_idat_bytes(&file, &chunk, &encoded_bytes);
            } else {
                process_chunk(&file, &chunk);
            }

            std::cout << "Name: \"" << chunk.name << "\"\n";
            std::cout << "Length: " << chunk.size << '\n';
        }

        encoded_bytes.print_buffer();
    }

    /* PNG chunks are in the following format:
*	- 4 byte size (a number that tells us how many bytes of data are in the chunk, we'll call it 'n'
*	- 4 byte name (name of the chunk)
*	- 'n' bytes of data, specific to that type of chunk
*	- 4 byte CRC info, used to verify chunk validity
*
       When we extract and concat all the data bytes from all the PNG's IDAT chunks, that resulting collection of bytes is a zlib stream as such:
        - 1 byte (CMF) compression method and flags 
          - bottom 4 bits (0-3)  CM == '8h' (a.k.a 1000) for 'deflate' compression method  
          - top 4 bits (4-7) CINFO == the base 2 log of the LZ77 window size, minus eight. CINFO=7 means window=32K. 7 is the max possible value.
        - 1 byte (FLG) additional flags and check bits
          - bits 0 to 4 FCHECK == 
          - bit 5 FDICT == indicates whether or not there's a preset dictionary. for a standard PNG, this should always be 0.
          - bits 6 and 7 FLEVEL == indicates what method the compressor used [0 = fastest, 1 = fast, 2 = default, 3 = maximum compress]. 
            this is mostly informational, not needed to decode.
        - n bytes of the compressed data to process, stored in the DEFLATE compressed data format spec
        - 4 bytes adler-32 checksum

       When we extract the DEFLATE compressed data (RFC 1951), this is the format (really got these bits onioned up good lord)
         - 
*/


    file.close();

    return 0;
}
