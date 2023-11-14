#include <iostream>

int str_to_int(const char* str) {
  int sum = 0;

  for(int i = 0; i < 3; i++) {
    std::cout << str[i];
    //sum += (str[i] - '0');
  }

  std::cout << '\n';

  return sum;
}

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cout << "Usage: bits [number 0 - 255]\n";
    exit(0);
  }
  
  std::cout << "INPUT: " << argv[1] << '\n';
  std::cout << "OUTPUT: " << str_to_int(argv[1]) << '\n';

/*
  int byte = atoi(argv[1]);

  std::cout << "atoi(\"D\") = " << atoi("D") << "\n";

  if(byte < 0 || byte > 255) {
    std::cout << "Enter a valid 8-bit value. [0 - 255]\n";
    exit(0);
  }

  for(int i = 0; i < 8; i++) {
    std::cout << (((unsigned char) byte >> i) & 1 ) << "\n";
  }
*/
  return 0;
}
