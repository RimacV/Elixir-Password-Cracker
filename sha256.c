#include "sha256.h"


static cl_platform_id platform_id = NULL;
static cl_device_id device_id = NULL;
static cl_uint ret_num_devices;
static cl_uint ret_num_platforms;
static cl_context context;

static cl_int ret;

static char* source_str;
static size_t source_size;

static cl_program program;
static cl_kernel kernel;
static cl_command_queue command_queue;


static cl_mem pinned_saved_keys, pinned_partial_hashes, pinned_start_indexes, pinned_end_indexes,
buffer_out, buffer_keys, buffer_start_index, buffer_end_index;
static cl_uint *partial_hashes;

static char *saved_plain;
static int *saved_start_index;
static int *saved_end_index;

static unsigned int datai[3];
static int have_full_hashes;

static size_t kpc = 4;

static size_t global_work_size = 1;
static size_t local_work_size = 1;
static size_t string_len;

void load_source();
void createDevice();
void createkernel();
void create_clobj();
void read_words(FILE *f, struct WordsWithPositions *inputWords, enum Strategie strategie);
void crypt_all();




struct WordsWithPositions
{
	cl_char words[NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD];
	cl_int startValues[NUM_ELEMENTS_INT];
	cl_int endValues[NUM_ELEMENTS_INT];
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


void sha256_init(size_t user_kpc)
{
	kpc = user_kpc;
	load_source();
	createDevice();
	createkernel();
	create_clobj();
}

void sha256_crypt(char* input, char* output)
{
	FILE *dict;
	errno_t err;
	struct WordsWithPositions wordsToTest;
	global_work_size = NUM_ELEMENTS_INT;
	err = fopen_s(&dict, "dict.txt", "r");
	if (err == 0)
	{
		printf("The file 'dict.txt' was opened\n");
		while (wordsToTest.eofIsReached[0] != 1)
		{
			read_words(dict, &wordsToTest, NoStrategie);
			memcpy(saved_plain, wordsToTest.words, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD);
			memcpy(saved_start_index, wordsToTest.startValues, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD);
			memcpy(saved_end_index, wordsToTest.endValues, NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD);
			crypt_all();
			printf("results[0] %i\n", partial_hashes[0]);
			char outpoutHex[65];
			int shaIndex = 0;
			int shaEnd = 8;
			for (int j = 0; j < NUM_ELEMENTS_INT; j++) {
				int k = 0;		
				for (int i = shaIndex; i<shaEnd; i++)
				{
					sprintf(outpoutHex + k * 8, "%08x", partial_hashes[i]);
					k++;
				}
				shaIndex += 8;
				shaEnd += 8;
				printf("Number:%i | %s\n", j, outpoutHex);
			}
		}

	}

	//int i;
	//string_len = strlen(input);
	//memcpy(saved_plain, input, string_len + 1);

	//crypt_all();

	//for (i = 0; i<SHA256_RESULT_SIZE; i++)
	//{
	//	sprintf(output + i * 8, "%08x", partial_hashes[i]);

	//}
}


void crypt_all()
{
	//printf("%s\n",saved_plain);
	ret = clEnqueueWriteBuffer(command_queue, buffer_keys, CL_TRUE, 0, sizeof(cl_char) * NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, saved_plain, 0, NULL, NULL);
	printf("%s\n", buffer_keys);
	ret = clEnqueueWriteBuffer(command_queue, buffer_start_index, CL_TRUE, 0, sizeof(cl_int) * NUM_ELEMENTS_INT, saved_start_index, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, buffer_end_index, CL_TRUE, 0, sizeof(cl_int) * NUM_ELEMENTS_INT, saved_end_index, 0, NULL, NULL);

	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	ret = clFinish(command_queue);
	// read back partial hashes
	ret = clEnqueueReadBuffer(command_queue, buffer_out, CL_TRUE, 0, sizeof(cl_int) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT, partial_hashes, 0, NULL, NULL);
	have_full_hashes = 0;
}

void load_source()
{
	FILE *fp;

	fp = fopen("sha256Cracker.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
}

void create_clobj() {
	pinned_saved_keys = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_char) * NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, NULL, &ret);
	saved_plain = (char*)clEnqueueMapBuffer(command_queue, pinned_saved_keys, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(cl_char) * NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, 0, NULL, NULL, &ret);
	memset(saved_plain, 0, sizeof(cl_char) * NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD);

	pinned_start_indexes = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int) * NUM_ELEMENTS_INT, NULL, &ret);
	saved_start_index = (int*)clEnqueueMapBuffer(command_queue, pinned_start_indexes, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(cl_int) * NUM_ELEMENTS_INT, 0, NULL, NULL, &ret);
	memset(saved_start_index, 0, sizeof(cl_int) * NUM_ELEMENTS_INT);

	pinned_end_indexes = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int) * NUM_ELEMENTS_INT, NULL, &ret);
	saved_end_index = (int*)clEnqueueMapBuffer(command_queue, pinned_end_indexes, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(cl_int) * NUM_ELEMENTS_INT, 0, NULL, NULL, &ret);
	memset(saved_end_index, 0, sizeof(cl_int) * NUM_ELEMENTS_INT);

	pinned_partial_hashes = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_int) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT, NULL, &ret);
	partial_hashes = (cl_int *)clEnqueueMapBuffer(command_queue, pinned_partial_hashes, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT, 0, NULL, NULL, &ret);
	memset(partial_hashes, 0, sizeof(cl_int) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT);

	buffer_keys = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_char) * NUM_ELEMENTS_INT * MAX_LENGTH_ONE_WORD, NULL, &ret);
	buffer_start_index = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int) * NUM_ELEMENTS_INT, NULL, &ret);
	buffer_end_index = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int) * NUM_ELEMENTS_INT, NULL, &ret);
	buffer_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * SHA256_RESULT_SIZE *NUM_ELEMENTS_INT, NULL, &ret);


	clSetKernelArg(kernel, 0, sizeof(buffer_keys), (void *)&buffer_keys);
	clSetKernelArg(kernel, 1, sizeof(buffer_start_index), (void *)&buffer_start_index);
	clSetKernelArg(kernel, 2, sizeof(buffer_end_index), (void *)&buffer_end_index);
	clSetKernelArg(kernel, 3, sizeof(buffer_out), (void *)&buffer_out);
}

void createDevice()
{
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 1, &device_id, &ret_num_devices);

	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
}

void createkernel()
{
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "sha256Cracker", &ret);
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
}
