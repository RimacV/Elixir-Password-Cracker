#include "sha256.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

//#include <memory.h>
//#include "sha256.h" //https://github.com/B-Con/crypto-algorithms/blob/master/sha256_test.c

static char* source_str;
static size_t source_size;

unsigned long counter2;


//static uint32_t *partial_hashes;
/*
static char *saved_plain;
static int *saved_start_index;
static int *saved_end_index;
*/
static int have_full_hashes;

static size_t kpc = 4;

static size_t string_len;

void load_source();
void read_words(FILE *f, struct WordsWithPositions *inputWords, enum Strategie strategie);
int crypt_all(struct WordsWithPositions *inputWords);




struct WordsWithPositions
{
	char words[NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD];
	int startValues[NUM_ELEMENTS_INT];
	int endValues[NUM_ELEMENTS_INT];
	int eofIsReached[1];
};

enum Strategie
{
	NoStrategie,
	ReplaceLettersWithNumbers,
	RemoveVocals,
	AddNumberAtBeginnig,
	AddNumberAtEnd
};

void remove_all_chars(char* str, char c)
{
	char *pr = str, *pw = str;
	while (*pr)
	{
		*pw = *pr++;
		pw += (*pw != c);
	}
	*pw = '\0';
}

void replace_all_chars(char* str, char from, char to)
{
	while (strchr(str, from))
	{
		char *c1 = strchr(str, from);
		*c1 = to;
	}
}
void read_words(FILE *f, struct WordsWithPositions *inputWords, enum Strategie strategie)
{
	char x[MAX_LENGTH_ONE_WORD];
	static int numberToAdd = 0;
	static int pos = 0;

	inputWords->eofIsReached[0] = 0;
	int len = 0;
	fseek(f, pos, 0);
	for (size_t i = 0; i < NUM_ELEMENTS_INT; i++)
	{
		if (fscanf_s(f, "%s", x, _countof(x)) == EOF)
		{
			inputWords->eofIsReached[0] = 1;
			pos = 0;
			if (strategie == AddNumberAtBeginnig || strategie == AddNumberAtEnd)
			{
				if (numberToAdd == MAX_NUMBER_TO_ADD - 1)
				{
					numberToAdd = 0;
				}
				else
				{
					numberToAdd++;
				}
			}
			return;
		}

		if (strategie == ReplaceLettersWithNumbers)
		{
			replace_all_chars(x, 'e', '3');
			replace_all_chars(x, 'E', '3');
			replace_all_chars(x, 'l', '1');
			replace_all_chars(x, 'L', '1');
			replace_all_chars(x, 'o', '0');
			replace_all_chars(x, 'O', '0');
		}
		else if (strategie == RemoveVocals)
		{
			remove_all_chars(x, 'a');
			remove_all_chars(x, 'A');
			remove_all_chars(x, 'e');
			remove_all_chars(x, 'E');
			remove_all_chars(x, 'i');
			remove_all_chars(x, 'I');
			remove_all_chars(x, 'o');
			remove_all_chars(x, 'O');
			remove_all_chars(x, 'u');
			remove_all_chars(x, 'U');
		}
		else if (strategie == AddNumberAtBeginnig)
		{
			char str[MAX_LENGTH_ONE_WORD];
			sprintf_s(str, MAX_LENGTH_ONE_WORD, "%d", numberToAdd);
			strcat_s(str, MAX_LENGTH_ONE_WORD, x);
			strcpy_s(x, MAX_LENGTH_ONE_WORD, str);
		}
		else if (strategie == AddNumberAtEnd)
		{
			char str[MAX_LENGTH_ONE_WORD];
			sprintf_s(str, MAX_LENGTH_ONE_WORD, "%d", numberToAdd);
			strcat_s(x, MAX_LENGTH_ONE_WORD, str);
		}

		//printf("%s\n", x);

		if (i == 0)
		{
			inputWords->startValues[i] = len;
			strcpy_s((char*)inputWords->words, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, x);
			len = strlen(x);
		}
		else
		{
			inputWords->startValues[i] = len + 1;
			//strcat_s(inputWords.words, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, x); //geht nicht weil \0 zwischen den wörtern entfernt wird
			len += sprintf_s((char*)inputWords->words + len, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD - len, "%c%s", '\0', x);
		}
		inputWords->endValues[i] = len + 1;
	}
	pos = ftell(f);
	return;
}
/*
void print_calculated_sha_values() {
	char outpoutHex[65];
	int shaIndex = 0;
	int shaEnd = 8;
	for (int j = 0; j < NUM_ELEMENTS_INT; j++)
	{
		int k = 0;
		for (int i = shaIndex; i<shaEnd; i++)
		{
			printf(outpoutHex + k * 8, "%08x", partial_hashes[i]);
			k++;
		}
		shaIndex += 8;
		shaEnd += 8;
		printf("Number:%i | %s\n", j, outpoutHex);
	}
}*/

void sha256_init(size_t user_kpc)
{
	kpc = user_kpc;
}

int calculate_sha(struct WordsWithPositions *wordsToTest) {
	
	int index=crypt_all(wordsToTest);
	/*printf("results[0] %i\n", partial_hashes[0]);*/

	
	if (index!=-1) {
		char password[MAX_LENGTH_ONE_WORD];
		int j = 0;
		for (int i = wordsToTest->startValues[index]; i < wordsToTest->endValues[index]; i++) {
			password[j] = wordsToTest->words[i];
			j++;
		}
		printf("Password found!: %s\n", password);
		return TRUE;
	}
	return FALSE;
}

int crypt_all(struct WordsWithPositions *wordsToTest)
{
	int index = -1;
	int temp = FALSE;
	unsigned int i;
	// i<NUM_ELEMENTS_INT falls datei weniger wörter hat, als NUM_ELMENTS zulässt
	for (i = 0; i<NUM_ELEMENTS_INT && wordsToTest->startValues[i] >= 0; i++){
		
		temp = sha256Cracker(wordsToTest->words, wordsToTest->startValues[i], wordsToTest->endValues[i] );
		if (temp == TRUE){
			index = i;
		}
	}
	counter2 += counter2 + i;
	//printf("%lu words checked \n", i);
	return index;
}


void start_brute_force_sha256(char* pathToDict, int printingIsEnabled)
{
	FILE *dict;
	errno_t err;
	struct WordsWithPositions wordsToTest;
	counter2 = 0;
	//global_work_size = NUM_ELEMENTS_INT;
	err = fopen_s(&dict, pathToDict, "r");
	if (err == 0)
	{
		printf("The file '%s' was opened\n", pathToDict);
		unsigned long counter = 0;

		printf("Applying Strategie: NoStrategie\n");
		while (wordsToTest.eofIsReached[0] != 1 )
		{
			read_words(dict, &wordsToTest, NoStrategie);
			counter += NUM_ELEMENTS_INT;
			if (calculate_sha(&wordsToTest)) {
				printf("%lu words checked \n", counter2);
				printf("%lu words checked \n", counter);
				return;
			}
			/*if (printingIsEnabled)
			{
				print_calculated_sha_values();
			}*/
			counter += NUM_ELEMENTS_INT;
			//if (counter % 1000) {
			//	/*printf("%lu words checked \n", counter);*/
			//}
		}
		//printf("%lu words checked \n", counter);
		/*
		printf("Applying Strategie: ReplaceLettersWithNumbers\n");
		wordsToTest.eofIsReached[0] = 0;
		while (wordsToTest.eofIsReached[0] != 1)
		{
			read_words(dict, &wordsToTest, ReplaceLettersWithNumbers);
			if (calculate_sha(&wordsToTest, &counter)) {
				printf("%lu words checked \n", counter);
				return;
			}
			if (printingIsEnabled)
			{
				print_calculated_sha_values();
			}
			counter += NUM_ELEMENTS_INT;
			//if (counter % 1000) {
			//	printf("%lu words checked \n", counter);
			//}
		}
		printf("%lu words checked \n", counter);
		printf("Applying Strategie: RemoveVocals\n");
		wordsToTest.eofIsReached[0] = 0;
		while (wordsToTest.eofIsReached[0] != 1)
		{
			read_words(dict, &wordsToTest, RemoveVocals);
			if (calculate_sha(&wordsToTest, &counter)) {
				printf("%lu words checked \n", counter);
				return;
			}
			if (printingIsEnabled)
			{
				print_calculated_sha_values();
			}
			counter += NUM_ELEMENTS_INT;
			//if (counter % 1000) {
			//	printf("%lu words checked \n", counter);
			//}
		}
		printf("%lu words checked \n", counter);
		printf("Applying Strategie: AddNumberAtBeginnig\n");
		wordsToTest.eofIsReached[0] = 0;
		while (wordsToTest.eofIsReached[0] != 1)
		{
			read_words(dict, &wordsToTest, AddNumberAtBeginnig);
			if (calculate_sha(&wordsToTest, &counter)) {
				printf("%lu words checked \n", counter);
				return;
			}
			if (printingIsEnabled)
			{
				print_calculated_sha_values();
			}
			counter += NUM_ELEMENTS_INT;
			//if (counter % 1000) {
			//	printf("%lu words checked \n", counter);
			//}
		}
		printf("%lu words checked \n", counter);
		printf("Applying Strategie: AddNumberAtEnd\n");
		wordsToTest.eofIsReached[0] = 0;
		while (wordsToTest.eofIsReached[0] != 1)
		{
			read_words(dict, &wordsToTest, AddNumberAtEnd);
			if (calculate_sha(&wordsToTest, &counter)) {
				printf("%lu words checked \n", counter);
				return;
			}
			if (printingIsEnabled)
			{
				print_calculated_sha_values();
			}
			counter += NUM_ELEMENTS_INT;
			//if (counter % 1000) {
			//	/*printf("%lu words checked \n", counter);
			//}
		}
		printf("%lu words checked \n", counter);
		*/
		printf("%lu words checked \n", counter2);
		printf("%lu words checked \n", counter);
	}
}




