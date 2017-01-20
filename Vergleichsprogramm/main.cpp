
#include "sha256.h"
#include <chrono>
#include <iostream>
#include <string>


int main()
{


	char result[65];

	sha256_init(2048);
	auto start = std::chrono::high_resolution_clock::now();
	start_brute_force_sha256("realhuman_phill.txt", TRUE);
	auto finish = std::chrono::high_resolution_clock::now();
	std::cout << "Total(CPU) :" << std::chrono::duration_cast<std::chrono::seconds>(finish - start).count() << "seconds\n";

	std::string src_str = "1";

}