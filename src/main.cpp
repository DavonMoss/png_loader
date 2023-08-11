#include <iostream>
#include <fstream>

/*
 * VERIFY PNG SIGNATURE
 *
 * Checks the first 8 bytes of a file against the expected PNG signature.
 *
 * PARAMS:
 * std::ifstream* file 	- Pointer to Input File Stream object from C++ fstream. Requires file to have already been opened.
 * 
 * RETURNS (bool):
 * TRUE if the signature matches, FALSE if it doesn't.
 *
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

unsigned long int hex_bytes_to_int(unsigned char* bytes) {
    // get length of chunk 
    unsigned long int length = 0;

    for(int i = 0; i < 4; i++) {
        length = length | bytes[i];	// write the bits of our char onto the bottom 8 bits of the final answer
	length << 8;			// shift those bits up and make space for the next 8
    }

    return length;  
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

    std::cout << "Signature Validity: " << verify_png_signature(&file) << std::endl;
    
    unsigned char char_data[] = {0x00, 0x00, 0x00, 0x0D};
    unsigned long int hex_data = 0x0000000D;
    unsigned long int dec_data = 13;
    
    std::cout << "Hex == Dec?: " << (hex_data == dec_data) << std::endl;
    std::cout << "Hex == parse(Char): " << (hex_data == hex_bytes_to_int(char_data)) << std::endl;
    std::cout << "Hex: " << hex_data << std::endl;
    std::cout << "hex_bytes_to_int(Char): " << hex_bytes_to_int(char_data) << std::endl;

    file.close();

    return 0;
}
