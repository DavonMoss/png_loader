#ifndef _BIT_STREAM_H
#define _BIT_STREAM_H

#include <queue>
#include <cstdint>
#include <vector>

struct bit_stream {
    std::queue<unsigned char> bytes;		// actual bytes we read from
    std::uint32_t buffer;					// buffer we load bits into
    int bit_count;							// count of valid bits in the buffer, all bits are left justified (EX: [bit_count amount of bits][32 - bit_count amount of trash])

    bit_stream();
    bit_stream(std::queue<unsigned char>);
    bit_stream(std::vector<unsigned char>);
    void init_buffer();
    int load_bytes_into_buffer();
    bool get_n_bits(int, std::uint32_t*);
};

#endif
