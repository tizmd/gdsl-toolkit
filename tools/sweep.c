/* vim:cindent:ts=2:sw=2:expandtab */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <gdsl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv) {
	const rlim_t kStackSize = 64L * 1024L * 1024L; // min stack size = 64 Mb
	struct rlimit rl;
	int result;

	result = getrlimit(RLIMIT_STACK, &rl);
	if(result == 0) {
		if(rl.rlim_cur < kStackSize) {
			rl.rlim_cur = kStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if(result != 0) {
				fprintf(stderr, "setrlimit returned result = %d\n", result);
			}
		}
	}

	size_t offset;
	size_t length;
	char *file;

	switch(argc) {
		case 2: {
			file = argv[1];
			offset = 0;

			struct stat buf;
			stat(file, &buf);
			length = buf.st_size;
			break;
		}
		case 3: {
			file = argv[1];
			sscanf(argv[2], "%zu", &offset);

			struct stat buf;
			stat(file, &buf);
			length = buf.st_size;
			break;
		}
		case 4: {
			file = argv[1];
			sscanf(argv[2], "%zu", &offset);
			sscanf(argv[3], "%zu", &length);
			break;
		}
		default: {
			printf("Usage: sweep file offset length\n");
			return 1;
		}
	}

	FILE *f = fopen(file, "r");
	if(!f) {
		printf("Unable to open file.\n");
		return 1;
	}
	fseek(f, offset, SEEK_SET);

	size_t buffer_size = length + 15;
	char *buffer = (char*)malloc(buffer_size);
	size_t buffer_length = fread(buffer, 1, buffer_size, f);

	state_t state = gdsl_init();
	gdsl_set_code(state, buffer, buffer_length, 0);

	//uint64_t consumed = 0;
	while(1) {
		printf("++++++++++++ DECODING NEXT INSTRUCTION ++++++++++++\n");

		if(setjmp(*gdsl_err_tgt(state))) {
			fprintf(stderr, "decode failed: %s\n", gdsl_get_error_message(state));
			break;
		}
		obj_t insn = gdsl_decode(state);

		string_t fmt = gdsl_merge_rope(state, gdsl_pretty(state, insn));
		puts(fmt);

		printf("---------------------------\n");

		if(setjmp(*gdsl_err_tgt(state))) {
			fprintf(stderr, "translate failed: %s\n", gdsl_get_error_message(state));
			break;
		}
		obj_t rreil = gdsl_translate(state, insn);
//		__obj r = __runMonadicOneArg(__translate__, &state, insn);
		//__obj r = __translate(__translate__,insn);

		fmt = gdsl_merge_rope(state, gdsl_rreil_pretty(state, rreil));
		puts(fmt);

		gdsl_reset_heap(state);
	}

	gdsl_destroy(state);

	return 1;
}

