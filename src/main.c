#include "bej.h"
#include <getopt.h>

/*
 * Prints help information.
 */
static void
print_usage(const char *program_name)
{
	fprintf(stdout,
		"Overview: A Redfish binary encoded json decoder to UTF-8 json format. Copyleft 🄯 2025 aeaeo.\n\n"
		"Usage: %s "/*[-a <annotation_dictionary_file>]*/"-s <schema_dictionary_file> -b <bej_file> [-o <output_file>]\n\n"
		"Options:\n\n"
			//"\t-a\tSpecify the annotation dictionary file (optional)\n"
			"\t-b\tSpecify the BEJ binary file to decode. Required.\n"
			"\t-h\tShow help message.\n"
			"\t-o\tSpecify output JSON file. Optional, default is stdout\n"
			"\t-s\tSpecify the schema dictionary file. Required.\n",
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
		errmsg("Failed to open file %s\n", filename);
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size < 0 || file_size > max_size) {
        errmsg("File %s is too large or invalid\n", filename);
        fclose(f);
        return 0;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != file_size) {
        errmsg("Error: Failed to read complete file %s\n", filename);
        return 0;
    }
    
    return bytes_read;
}

int
main(int argc, char** argv)
{
	uint8_t schema_dict_data[65536] = {0};
	//uint8_t anno_dict_data[65536] = {0};
	uint8_t bej_data[65536] = {0};
	size_t schema_dict_size = 0UL;
	//size_t anno_dict_size = 0UL;
	size_t bej_size = 0UL;
	char* output_file = NULL;
	FILE *output = stdout;

	int option = EOF;
	while ((option = getopt(argc, argv, "h"/*a:*/"b:s:o:")) != EOF) {
		switch (option) {
		//case 'a':
			/* todo: annotation dict */
		//	anno_file = optarg;
		//	break;
		case 'b':
			bej_size = read_file(optarg, bej_data, sizeof(bej_data));
			if (!bej_size)
				return FAILURE;
			break;
		case 's':
			schema_dict_size = read_file(optarg, schema_dict_data, sizeof(schema_dict_data));
			if (!schema_dict_size)
				return FAILURE;
			break;
		case 'o':
			output_file = optarg;
			if (output_file) {
				output = fopen(output_file, "w");
				if (!output) {
					errmsg("Failed to open output file %s\n", output_file);
					return FAILURE;
				}
			}
			break;
		case 'h':
		default:
			print_usage(argv[0]);
			return SUCCESS;
		}
	}

	if (!bej_size || !schema_dict_size) {
		errmsg("Both -s and -b options are required\n");
		print_usage(argv[0]);
		return FAILURE;
	}

	bej_context_t ctx;
    if (bej_init_context(&ctx,
						 schema_dict_data, schema_dict_size,
						 //anno_dict_data, anno_dict_size,
						 bej_data, bej_size,
						 output)) {
		errmsg("Failed to initialize BEJ context\n");
        if (output != stdout)
            fclose(output);
        return FAILURE;
    }
    
    uint8_t result = bej_decode(&ctx);
    if (result) {
		errmsg("Failed to decode BEJ data\n");
        if (output != stdout)
            fclose(output);
        return FAILURE;
    }
    
    fprintf(output, "\n");

    if (output != stdout) {
        fclose(output);
        printf("Successfully decoded BEJ to %s\n", output_file);
    }

	return 0;
}