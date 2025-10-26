#include <getopt.h>
#include "bej.h"

/*
 * Actually just prints help information.
 */
static void
print_usage(const char *program_name)
{
	fprintf(stderr,
		"Overview: binary encoded json decoder to json format\n\n"
		"Usage: %s [-a <annotation_dictionary_file>] -s <schema_dictionary_file> -b <bej_file> [-o <output_file>]\n\n"
		"Options:\n\n"
			"\t-a\tSpecify the annotation dictionary file (optional)\n"
			"\t-b\tSpecify the BEJ binary file to decode (required)\n"
			"\t-h\tShow this message\n"
			"\t-o\tSpecify output JSON file (default: stdout)\n"
			"\t-s\tSpecify the schema dictionary file (required)\n",
		program_name);
}

/*
 * General function to read the entire content of a file into a buffer.
 */
static size_t
read_file(const char *filename, uint8_t *buffer, size_t max_size)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Failed to open file %s\n", filename);
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size < 0 || file_size > max_size) {
        fprintf(stderr, "Error: File %s is too large or invalid\n", filename);
        fclose(f);
        return 0;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != file_size) {
        fprintf(stderr, "Error: Failed to read complete file '%s'\n", filename);
        return 0;
    }
    
    return bytes_read;
}

int
main(int argc, char** argv)
{
	uint8_t schema_dict_data[65536] = {0};
	size_t schema_dict_size = 0UL;
	uint8_t anno_dict_data[65536] = {0};
	size_t anno_dict_size = 0UL;
	uint8_t bej_data[65536] = {0};
	size_t bej_size = 0UL;

	char* anno_file = NULL;
	char* schema_file = NULL;	// TODO: make optional; also possibly no need of this due to simplification

	char* bej_file = NULL;
	char* output_file = NULL;

	int option = getopt(argc, argv, "ha:b:s:o:");

	while (option != EOF) {
		switch (option) {
		case 'a':
			anno_file = optarg;
			break;
		case 'b':
			bej_file = optarg;
			break;
		case 's':
			schema_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'h':
		default:
			print_usage(argv[0]);
			return -1;
		}
	}

	if (!bej_file || !schema_file) {
		fprintf(stderr, "Error: missed required arguments\n\n");
		print_usage(argv[0]);
		return -1;
	}


	//

	return 0;
}