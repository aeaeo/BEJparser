#include <getopt.h>
#include "bej.h"

int
main(int argc, char** argv)
{
	uint8_t schema_dict_data[65536] = {0};
	size_t schema_dict_size = 0UL;
	uint8_t anno_dict_data[65536] = {0};
	size_t anno_dict_size = 0UL;
	uint8_t bej_data[65536] = {0};
	size_t bej_size = 0UL;
	FILE *f = NULL;

	const char* desc =
		"Overview: binary encoded json decoder to json format\n\n"
		"Usage: <program_name> [-a <annotation_dictionary_file>] [-s <schema_dictionary_file>] [-b <bjson_file>]\n\n"
		"Options:\n\n"
			"\t-a\tSpecify the annotation dictionary file\n"
			"\t-b\tSpecify the BEJ binary file to decode\n"
			"\t-h\tShow this message\n"
			"\t-s\tSpecify the schema dictionary file\n";

	int option = getopt(argc, argv, "ha:b:s:");
	while (option != EOF) {
		switch (option) {
		case 'a':
			f = fopen(optarg, "rb");
			if (!f) {
				fprintf(stderr, "Failed to open annotation dictionary file\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			anno_dict_size = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(anno_dict_data, 1, anno_dict_size, f);
			fclose(f);
			f = NULL;
			break;
		case 'b':
			f = fopen(optarg, "rb");
			if (!f) {
				fprintf(stderr, "Failed to open binary json file\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			bej_size = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(bej_data, 1, bej_size, f);
			fclose(f);
			f = NULL;
			break;
		case 's':
			f = fopen(optarg, "rb");
			if (!f) {
				fprintf(stderr, "Failed to open schema dictionary file\n");
				return -1;
			}
			fseek(f, 0, SEEK_END);
			schema_dict_size = ftell(f);
			fseek(f, 0, SEEK_SET);
			fread(schema_dict_data, 1, schema_dict_size, f);
			fclose(f);
			f = NULL;
			break;
		case 'h':
		default:
			fprintf(stderr, "%s", desc);
			return -1;
		}
	}

	if (argc <= 3 || argc > 4 || bej_size == 0UL || schema_dict_size == 0UL || anno_dict_size == 0UL) {
		fprintf(stderr, desc);
		return -1;
	}


	//

	return 0;
}