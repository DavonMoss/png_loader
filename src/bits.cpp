#include <iostream>
#include <cstdio>
#include <cstdint>
#include <queue>
#include <vector>

#include "bits.h"

bit_stream::bit_stream() {
    init_buffer();
}

bit_stream::bit_stream(std::queue<unsigned char> data) {
    bytes = data;
    init_buffer(); 
}

bit_stream::bit_stream(std::vector<unsigned char> data) {
    for(int i = 0; i < data.size(); i++) {
		bytes.push(data[i]);
	}
    init_buffer(); 
}

void bit_stream::init_buffer() {
    buffer = 0;
    bit_count = 0;
    load_bytes_into_buffer();
}

/*
Loads 32 bit buffer with as many bytes as it can. Returns number of bytes loaded.
*/
int bit_stream::load_bytes_into_buffer() {
    int n = 0;

    if(bit_count <= 24) {
        // determine how many bytes to load
        n = (32 - bit_count) / 8;
        n = (n < bytes.size()) ? n : bytes.size();

        // preserve current bits
        buffer = (buffer >> (32 - bit_count));

        // load bytes, update queue, update bit_count
        for(int i = 0; i < n; i++) {
            buffer = buffer << 8;
            buffer = buffer | bytes.front();
            bytes.pop();
            bit_count += 8;
        }
        
        // push bits to top of buffer
        buffer = (buffer << (32 - bit_count));
    }

    return n;
}

/*
Gets n bits from data. Returns true/false based on success. Stores answer in buffer parameter.
*/
bool bit_stream::get_n_bits(int n, std::uint32_t* bit_buffer) {
    std::uint32_t bits = 0;
    int bytes_loaded = 0;
    bool success = false;

    if(bit_count < n) {
        bytes_loaded = load_bytes_into_buffer();
        
        if(bytes_loaded > 0) {
            return get_n_bits(n, bit_buffer); 
        }
    } else {
        // get bits, update bit count, update buffer
        bits = (buffer >> (32 - n));
        buffer = buffer << n;
        bit_count -= n;
		success = true;
    }
    
	*bit_buffer = bits;
    return success; 
}
