#include <getopt.h>
#include "bej.h"

int main(int argc, char** argv)
{
	uint8_t schema_dict_data[65536] = {0};
	size_t schema_dict_size = 0UL;
	uint8_t anno_dict_data[65536] = {0};
	size_t anno_dict_size = 0UL;
	uint8_t bej_data[65536] = {0};
	size_t bej_size = 0UL;
	FILE *f = NULL;

	int option = getopt(argc, argv, "a:s:h");
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
			break;
		case 'h':
		default:
			//fprintf(stderr, desc);	// TODO: add help message
			return 0;
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
			break;
		}
	}

	return 0;
}