#include <iostream>

#include "buffer.h"

/* CONSTRUCTOR */
encoded_data_buffer::encoded_data_buffer() {
    buffer = new unsigned char[INITIAL_BUFFER_SIZE];
    current_buffer_size = INITIAL_BUFFER_SIZE;
    next_free_space = 0;
}

/* Overload [] operator to check bounds, provides a mem safe way to get bytes.
Only returns data if its in bounds and we wrote it, otherwise we get null char '\0'.
For an encoded_data_buffer buff, this lets you do 'buff[5]' instead of 'buff.buffer[5]'. */
unsigned char encoded_data_buffer::operator[] (int index) {
    if(index >= current_buffer_size) {
        printf("Index %d is out of bounds of this buffer.", index);
        return '\0';
    } 
    
    if (index >= next_free_space) {
        printf("Index %d is in bounds, but we didn't write this data, it's garbage.", index);
        return '\0';
    }

    return buffer[index];   
}

/* Add bytes to the buffer, grow it if we're out of space. */
void encoded_data_buffer::load(unsigned char byte) {
    if(next_free_space >= current_buffer_size) {
        int new_buffer_size = current_buffer_size * 2;
        unsigned char* new_buffer = new unsigned char[new_buffer_size];
    
        for(int i = 0; i < current_buffer_size; i++) {
            new_buffer[i] = buffer[i];
        }

        delete(buffer);
        buffer = new_buffer;
        next_free_space = current_buffer_size;
        current_buffer_size = new_buffer_size;
        load(byte); 
    } else {
        buffer[next_free_space] = byte;
        next_free_space++; 
    }
}

/* DEBUG FUNCTIONS */
void encoded_data_buffer::print_buffer() {
    for(int i = 0; i < current_buffer_size; i++) {
        printf("[%02d: %02X]\n", i, buffer[i]);
    }
}
