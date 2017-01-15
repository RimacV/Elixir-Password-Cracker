#ifndef SHA256CRACKER_H
#define SHA256CRACKER_H
#endif
#ifndef uint32_t
#define uint32_t unsigned int
#endif

#define H0 0x6a09e667
#define H1 0xbb67ae85
#define H2 0x3c6ef372
#define H3 0xa54ff53a
#define H4 0x510e527f
#define H5 0x9b05688c
#define H6 0x1f83d9ab
#define H7 0x5be0cd19

//sha for 123456
#define R0 0x8d969eef
#define R1 0x6ecad3c2
#define R2 0x9a3a6292
#define R3 0x80e686cf
#define R4 0x0c3f5d5a
#define R5 0x86aff3ca
#define R6 0x12020c92
#define R7 0x3adc6c92

#ifdef __cplusplus
extern "C" {
#endif

	unsigned int rotr(unsigned int x, int n);
	unsigned int ch(unsigned int x, unsigned int y, unsigned int z);
	unsigned int maj(unsigned int x, unsigned int y, unsigned int z);
	unsigned int sigma0(unsigned int x);
	unsigned int sigma1(unsigned int x);
	unsigned int gamma0(unsigned int x);
	unsigned int gamma1(unsigned int x);
	int sha256Cracker(char* inputA[], int* inputB, int* inputC);

#ifdef __cplusplus
}
#endif