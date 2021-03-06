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

//#define R0 0x229a99a7
//#define R1 0x94919912
//#define R2 0x1a3b5b5c
//#define R3 0xd648cb70
//#define R4 0x37fd4b8a
//#define R5 0x7bd6d613
//#define R6 0x248ed130
//#define R7 0x09fcdccd








uint rotr(uint x, int n) {
	if (n < 32) return (x >> n) | (x << (32 - n));
	return x;
}

uint ch(uint x, uint y, uint z) {
	return (x & y) ^ (~x & z);
}

uint maj(uint x, uint y, uint z) {
	return (x & y) ^ (x & z) ^ (y & z);
}

uint sigma0(uint x) {
	return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

uint sigma1(uint x) {
	return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

uint gamma0(uint x) {
	return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

uint gamma1(uint x) {
	return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

__kernel void sha256Cracker(__global const char* inputA,
	__global int* inputB,
	__global int* inputC,
	__global int *digest
	) {

	int id = get_global_id(0);
	//printf("globalId %s\n", inputA);
	int start = inputB[id];
	int end = inputC[id];
	int t, gid, msg_pad;
	int stop, mmod;
	uint i, ulen, item, total;
	uint W[80], temp, A, B, C, D, E, F, G, H, T1, T2;
	//uint num_keys = data_info[1];
	int current_pad;
	ulen = (end - start) - 1;
	char plain_key[100];
	int j = 0;
	//printf("start %i\n", start);
	//printf("end %i\n", end);
	for (int i = start; i < end; i++) {
		plain_key[j] = inputA[i];
	/*	printf("%c\n", plain_key[j]);*/
		j++;
	}
	//
	uint K[64] = {
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
	if (id != 0) {
		id = id * 8;
	}
	/*printf("globalId mod %i\n", id);*/
	digest[id] = H0;
	digest[id + 1] = H1;
	digest[id + 2] = H2;
	digest[id + 3] = H3;
	digest[id + 4] = H4;
	digest[id + 5] = H5;
	digest[id + 6] = H6;
	digest[id + 7] = H7;
	for (item = 0; item < total; item++)
	{

		A = digest[id];
		B = digest[id + 1];
		C = digest[id + 2];
		D = digest[id + 3];
		E = digest[id + 4];
		F = digest[id + 5];
		G = digest[id + 6];
		H = digest[id + 7];

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

		/*printf("current_pad: %d\n",current_pad);*/
		if (current_pad > 0)
		{
			i = current_pad;

			stop = i / 4;
			/*printf("i:%d, stop: %d msg_pad:%d\n",i,stop, msg_pad);*/
			for (t = 0; t < stop; t++) {
				W[t] = ((uchar)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((uchar)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= ((uchar)plain_key[msg_pad + t * 4 + 2]) << 8;
				W[t] |= (uchar)plain_key[msg_pad + t * 4 + 3];
				/*printf("W[%u]: %u\n",t,W[t]);*/
			}
			mmod = i % 4;
			if (mmod == 3) {
				W[t] = ((uchar)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((uchar)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= ((uchar)plain_key[msg_pad + t * 4 + 2]) << 8;
				W[t] |= ((uchar)0x80);
			}
			else if (mmod == 2) {
				W[t] = ((uchar)plain_key[msg_pad + t * 4]) << 24;
				W[t] |= ((uchar)plain_key[msg_pad + t * 4 + 1]) << 16;
				W[t] |= 0x8000;
			}
			else if (mmod == 1) {
				W[t] = ((uchar)plain_key[msg_pad + t * 4]) << 24;
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
		digest[id] += A;
		//if (0xd4735e3a == digest[id +1]) 
		//printf("digest[0]: %08x\n", digest[id]);
		digest[id + 1] += B;
		/*printf("digest[1]: %i\n", digest[id + 1]);*/
		digest[id + 2] += C;
		/*printf("digest[2]: %i\n", digest[id + 2]);*/
		digest[id + 3] += D;
		/*printf("digest[3]: %i\n", digest[id + 3 ]);*/
		digest[id + 4] += E;
		/*printf("digest[4]: %i\n", digest[id +4 ]);*/
		digest[id + 5] += F;
		/*printf("digest[5]: %i\n", digest[id+5]);*/
		digest[id + 6] += G;
		/*printf("digest[6]: %i\n", digest[id+6]);*/
		digest[id + 7] += H;
		/*printf("digest[7]: %i\n", digest[id+7]);*/

		if (digest[id] == R0 && digest[id + 1] == R1 && digest[id + 2] == R2 && digest[id + 3] == R3 && digest[id + 4] == R4 && digest[id + 5] == R5 && digest[id + 6] == R6 && digest[id + 7] == R7) {
			digest[0] = get_global_id(0);
		}
		//for (t = 0; t < 80; t++)
		//{
		//	printf("W[%d]: %u\n", t, W[t]);
		//}
	}
}
//�