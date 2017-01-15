

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
	start_brute_force_sha256("realhuman_phill.txt", FALSE);
	auto finish = std::chrono::high_resolution_clock::now();
	std::cout << "Total(GPU) :" << std::chrono::duration_cast<std::chrono::seconds>(finish - start).count() << "seconds\n";

	std::string src_str = "1";
	//std::string hash_hex_str;
	//std::chrono::high_resolution_clock::now();
	//picosha2::hash256_hex_string(src_str, hash_hex_str);
	//finish = std::chrono::high_resolution_clock::now();
	//std::cout << "Total(CPU) :" << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";

	//std::cout << hash_hex_str << std::endl;
	//printf("%s\n", result);


}