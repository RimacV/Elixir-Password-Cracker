#define SHA256_FORMAT_NAME			"SHA-256-Vergleichsprogramm"
#define SHA256_ALGORITHM_NAME		"sha256-vergleichsprogramm develop only"

#define SHA256_PLAINTEXT_LENGTH		64

#define SHA256_BINARY_SIZE			32
#define SHA256_RESULT_SIZE			8

#define NUM_ELEMENTS_INT (9600)
#define MAX_LENGTH_ONE_WORD (10)
#define MAX_NUMBER_TO_ADD (1)

const int  TRUE = 1;
const int  FALSE = 0;

#define MAX_SOURCE_SIZE 0x10000000
#ifdef __cplusplus
extern "C" {
#endif

	void sha256_init(size_t);
	void start_brute_force_sha256(char*, int);


#ifdef __cplusplus
}
#endif