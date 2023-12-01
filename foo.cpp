#include <iostream>
#include <cstdint>

int main() {
	std::uint32_t test;
	bool flag;
	char output;

	test = 0;
	flag = test;
	output = (flag) ? 'T' : 'F';
	std::cout << output << "\n";

	test = 1;
	flag = test;
	output = (flag) ? 'T' : 'F';
	std::cout << output << "\n";
}
