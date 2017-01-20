#include "sha256Cracker.h"

unsigned int rotr(unsigned int x, int n) {
	if (n < 32) return (x >> n) | (x << (32 - n));
	return x;
}

unsigned int ch(unsigned int x, unsigned int y, unsigned int z) {
	return (x & y) ^ (~x & z);
}

unsigned int maj(unsigned int x, unsigned int y, unsigned int z) {
	return (x & y) ^ (x & z) ^ (y & z);
}

unsigned int sigma0(unsigned int x) {
	return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

unsigned int sigma1(unsigned int x) {
	return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

unsigned int gamma0(unsigned int x) {
	return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

unsigned int gamma1(unsigned int x) {
	return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

int sha256Cracker(char inputA[],
	int inputB,
	int inputC
	) {
	int result = FOUND_FALSE;
	int digest[8];
	//printf("globalId %s\n", inputA);
	int start = inputB;
	int end = inputC;
	int t, gid, msg_pad;
	int stop, mmod;
	uint32_t i, ulen, item, total;
	uint32_t W[80], temp, A, B, C, D, E, F, G, H, T1, T2;
	//uint num_keys = data_info[1];
	int current_pad;
	ulen = (end - start) - 1;
	char plain_key[100];
	int j = 0;
	
	for (int i = start; i < end; i++) {
		plain_key[j] = inputA[i];
			//printf("%c", plain_key[j]);
		j++;
	}
	
	//printf("start %i\n", start);
	//printf("end %i\n", end);

	uint32_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	msg_pad = 0;


	total = ulen % 64 >= 56 ? 2 : 1 + ulen / 64;
	/*printf("ulen: %u total:%u\n", ulen, total);*/
	
	/*printf("globalId mod %i\n", id);*/
	digest[0] = H0;
	digest[1] = H1;
	digest[2] = H2;
	digest[3] = H3;
	digest[4] = H4;
	digest[5] = H5;
	digest[6] = H6;
	digest[7] = H7;
	//printf("total %i",total);
	for (item = 0; item < total; item++)
	{

		A = digest[0];
		B = digest[1];
		C = digest[2];
		D = digest[3];
		E = digest[4];
		F = digest[5];
		G = digest[6];
		H = digest[7];

#pragma unroll
		for (t = 0; t < 80; t++) {
			W[t] = 0x00000000;
		}
		msg_pad = item * 64;
		if (ulen > msg_pad)
		{
			current_pad = (ulen - msg_pad) > 64 ? 64 : (ulen - msg_pad);
		}
		else
		{
			current_pad = -1;
		}

		//printf("current_pad: %d\n",current_pad);
		if (current_pad > 0)
		{
			i = current_pad;

			stop = i / 4;
			//printf("i:%d, stop: %d msg_pad:%d\n",i,stop, msg_pad);
			for (t = 0; t < stop; t++) {
				W[t] = ((unsigned char)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((unsigned char)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= ((unsigned char)plain_key[msg_pad + t * 4 + 2]) << 8;
				W[t] |= (unsigned char)plain_key[msg_pad + t * 4 + 3];
				/*printf("W[%u]: %u\n",t,W[t]);*/
			}
			mmod = i % 4;
			if (mmod == 3) {
				W[t] = ((unsigned char)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((unsigned char)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= ((unsigned char)plain_key[msg_pad + t * 4 + 2]) << 8;
				W[t] |= ((unsigned char)0x80);
			}
			else if (mmod == 2) {
				W[t] = ((unsigned char)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((unsigned char)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= 0x8000;
			}
			else if (mmod == 1) {
				W[t] = ((unsigned char)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= 0x800000;
			}
			else /*if (mmod == 0)*/ {
				W[t] = 0x80000000;
			}

			if (current_pad < 56)
			{
				W[15] = ulen * 8;
				/*printf("ulen avlue 2 :w[15] :%u\n", W[15]);*/
			}
		}
		else if (current_pad < 0)
		{
			if (ulen % 64 == 0)
				W[0] = 0x80000000;
			W[15] = ulen * 8;
			/*printf("ulen avlue 3 :w[15] :%u\n", W[15]);*/
		}

		for (t = 0; t < 64; t++) {
			if (t >= 16)
				W[t] = gamma1(W[t - 2]) + W[t - 7] + gamma0(W[t - 15]) + W[t - 16];
			T1 = H + sigma1(E) + ch(E, F, G) + K[t] + W[t];
			T2 = sigma0(A) + maj(A, B, C);
			H = G; G = F; F = E; E = D + T1; D = C; C = B; B = A; A = T1 + T2;
		}
		digest[0] += A;
		//if (0xd4735e3a == digest[id +1]) 
		//printf("digest[0]: %08x\n", digest[0]);
		digest[1] += B;
		//printf("digest[1]: %i\n", digest[1]);
		digest[2] += C;
		//printf("digest[2]: %i\n", digest[2]);
		digest[3] += D;
		//printf("digest[3]: %i\n", digest[3]);
		digest[4] += E;
		//printf("digest[4]: %i\n", digest[4]);
		digest[5] += F;
		//printf("digest[5]: %i\n", digest[5]);
		digest[6] += G;
		//printf("digest[6]: %i\n", digest[6]);
		digest[7] += H;
		//printf("digest[7]: %i\n", digest[7]);

		if (digest[0] == R0 && digest[1] == R1 && digest[2] == R2 && digest[3] == R3 && digest[4] == R4 && digest[5] == R5 && digest[6] == R6 && digest[7] == R7) {
			result= FOUND_TRUE;
		}
		
		//for (t = 0; t < 80; t++)
		//{
			//printf("W[%d]: %u\n", t, W[t]);
		//}
	}
	return result;
}
//