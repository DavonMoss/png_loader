#include <iostream>
#include <fstream>
#include <sys/stat.h>

int main(int argc, char** argv) {

    // take filename as input to CLI
    if (argc != 2) {
        std::cout << "Invalid number of arguments. Give me a filename please!" << std::endl;
    }

    const char * filepath = argv[1];

    // get the filesize in bytes so we know how large to make our buffer for reading the PNG
    struct stat stat_buffer;
    stat(filepath, &stat_buffer);

    // creates file stream, opens that file, we specify to read the binary data, then we get every byte one by one
    // and store it in 'buffer' which has the same size as the file
    std::ifstream file;
    file.open(filepath, std::ios::binary);
    unsigned char buffer[stat_buffer.st_size];
    unsigned char c = file.get();
    buffer[0] = c;
    int count = 1;

    while(file.good()) {
	c = file.get();
	buffer[count];
	count++;
    }

    file.close();

    // Now at this point, we have buffer stored in memory

    return 0;
}
