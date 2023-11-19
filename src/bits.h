#ifndef _BIT_STREAM_H
#define _BIT_STREAM_H

#include <queue>
#include <cstdint>
#include <vector>

struct bit_stream {
    std::queue<unsigned char> bytes;
    std::uint32_t buffer;
    int bit_count;

    bit_stream();
    bit_stream(std::queue<unsigned char>);
    bit_stream(std::vector<unsigned char>);
    void init_buffer();
    int load_bytes_into_buffer();
    bool get_n_bits(int, std::uint32_t*);
};

#endif
