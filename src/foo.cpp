#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>

//TODO: before incorporating this into main, need to figure out how we're gonna represent the huffman trees (symbols, codes, etc)
//		its probably not that complicated but my brain fried rn. gonna need to clean this up too because its very funky

struct huffman_table {
	std::unordered_map<unsigned char, std::uint32_t> codes;
};

void get_huffman_tree(std::vector<unsigned char> symbols, std::vector<std::uint32_t> code_lengths) {
	std::unordered_map<std::uint32_t, int> code_length_counts;
	std::vector<std::uint32_t> base_codes, generated_codes;
	std::uint32_t base_code, max_bits, len;
	huffman_table huff;

	// counting occurrences of each length
	for(int i = 0; i < code_lengths.size(); i++) {
		code_length_counts[code_lengths[i]]++;
	}

	// generating base codes from lengths
	max_bits = code_length_counts.size() - 1; //idea here is that map size == longest code length + 1
	base_code = 0;
	code_length_counts[0] = 0;
	base_codes.push_back(0);
	for(int bits = 1; bits <= max_bits; bits++) {
		base_code = (base_code + code_length_counts[bits - 1]) << 1;
		base_codes.push_back(base_code); //@TODO: there is a problem here, the vector is not initialized to proper size. so we cant just do this.
	}

	// generating all the actual codes
	for(int i = 0; i <= code_lengths.size(); i++) {
		len = code_lengths[i];
		if(len != 0) {
			generated_codes.push_back(base_codes[len]);
			base_codes[len]++;
		}
	}

	// build huffman table
	for(int i = 0; i < symbols.size(); i++) {
		huff.codes[generated_codes[i]] = symbols[i];
	}
}

int main() {
	// params
	std::vector<unsigned char> symbols = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
	std::vector<std::uint32_t> code_lengths = {3, 3, 3, 3, 3, 2, 4, 4};

	get_huffman_tree(symbols, code_lengths);
}
