#ifndef _BUFFER_H
#define _BUFFER_H

/* Implementaton of a dynamic array to store bytes extracted from IDAT chunks.*/

/* Will hold data extracted from multiple IDAT chunks. We can load bytes in, and we have an operator to access buffer elements
   in a memory safe way. Can't delete stuff though but do we really need to? We don't wanna ever change this stuff it's just a
   collection of data to read and decode. */
struct encoded_data_buffer {
    const int INITIAL_BUFFER_SIZE = 32;		// size we initialize dynamic array to
    int current_buffer_size;			// current size of dynamic array
    int next_free_space;			// index of next free space
    unsigned char* buffer;			// dynamic array

    encoded_data_buffer();			// constructor, just inits vars and initially sized buffer
    unsigned char operator[] (int);		// makes it so we can say 'my_buffer[i]' instead of 'my_buffer.buffer[i]'. Also mem-safe.
    void load(unsigned char); 			// slaps a byte on the end of the buffer
    void print_buffer();			// outputs buffer contents for debug purposes
};

#endif
