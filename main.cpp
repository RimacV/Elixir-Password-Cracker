

#include "sha256.h"
#include <chrono>
#include <iostream>
#include "picosha2.h"
#include <string>
int main()
{
	char result[65];

	sha256_init(2048);
	auto start = std::chrono::high_resolution_clock::now();
	sha256_crypt("1", result);
	auto finish = std::chrono::high_resolution_clock::now();
	std::cout << "Total(GPU) :" << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";

	std::string src_str = "1";
	std::string hash_hex_str;
	std::chrono::high_resolution_clock::now();
	picosha2::hash256_hex_string(src_str, hash_hex_str);
	finish = std::chrono::high_resolution_clock::now();
	std::cout << "Total(CPU) :" << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";

	std::cout << hash_hex_str << std::endl;
	printf("%s\n", result);

	sha256_crypt("2", result);
	printf("%s\n", result);

	sha256_crypt("123", result);
	printf("%s\n", result);

	sha256_crypt("1234567890123456789012345678901234567890123456789012345678901234567890", result);
	printf("%s\n", result);
}