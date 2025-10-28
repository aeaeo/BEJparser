#pragma once
#include "common.h"
/*
* Some possible BEJ types as specified in DSP0218 Section 7.2.4
*/
enum eBEJtype {
    BEJ_FORMAT_SET = 0x00,
    BEJ_FORMAT_ARRAY = 0x01,
    BEJ_FORMAT_NULL = 0x02,
    BEJ_FORMAT_INTEGER = 0x03,
    BEJ_FORMAT_ENUM = 0x04,
    BEJ_FORMAT_STRING = 0x05,
    BEJ_FORMAT_REAL = 0x06,
    BEJ_FORMAT_BOOLEAN = 0x07,
    BEJ_FORMAT_BYTE_STRING = 0x08,
    BEJ_FORMAT_CHOICE = 0x09,
    BEJ_FORMAT_PROPERTY_ANNOTATION = 0x10
};

typedef struct {
    uint8_t version_tag;
    uint8_t truncation_flag;    // not a flag but flags, only the named one is used though
    uint16_t entry_count;
    uint32_t schema_version;
    uint32_t dictionary_size;
    uint8_t *data;
    size_t data_size;
} bej_dictionary_context_t;

typedef struct {
    uint8_t format;
    uint16_t sequence;
    uint16_t child_offset;
    uint16_t child_count;
    uint8_t name_length;
    uint16_t name_offset;
} bej_dict_entry_t;

/*
* @brief This one is a helper struct to maintain decoder context and mainly reduce verbosity
*/
typedef struct {
    bej_dictionary_context_t schema_dict;
    //bej_dictionary_t anno_dict;
    uint8_t *bej_data;
    size_t bej_size;
    size_t offset;
    FILE *output;
    int indent_level;
    uint16_t parent_child_offset;
    uint16_t parent_child_count;
} bej_context_t;

/*
* Converts BEJ encoded data to JSON format.
* This function is main decoder interface.
*/
uint8_t bej_decode(bej_context_t *context);

/*
* Initializes the BEJ decoder context
*/
uint8_t bej_init_context(bej_context_t *ctx,
                     uint8_t *schema_data, size_t schema_size,
                     uint8_t *bej_data, size_t bej_size,
                     FILE *output);

/*
* Parses a BEJ dictionary header
*/
uint8_t bej_parse_dict(bej_dictionary_context_t *dict, uint8_t *data, size_t size);

/*
* Decodes a BEJ Non-Negative integer value
*/
uint8_t bej_read_nnint(uint8_t *data, size_t *offset, size_t size, uint32_t *value);

/*
* Decodes a BEJ sequence value and dictionary selector
*/
uint8_t bej_read_sequence_number(uint8_t *data, size_t *offset, size_t size, uint32_t *seqnum, uint8_t *dictselector);

/*
* Find dictionary entry by sequence number
*/
uint8_t bej_find_dict_entry(bej_context_t *ctx, bej_dictionary_context_t *dict, uint32_t sequence, bej_dict_entry_t *entry);

/*
* Get dictionary entry name
*/
uint8_t bej_get_entry_name(bej_dictionary_context_t *dict, bej_dict_entry_t *entry, 
                       char *name, size_t name_size);

uint8_t bej_read_format(uint8_t *data, size_t *offset, size_t size, 
                              uint8_t *format, uint8_t *flags);
uint8_t decode_bej_sflv(bej_context_t *ctx, bej_dictionary_context_t *dict, 
                        uint8_t add_name);
uint8_t decode_enum(bej_context_t *ctx, uint8_t *value, uint32_t length,
                    bej_dictionary_context_t *dict);
uint8_t decode_set(bej_context_t *ctx, uint32_t length, 
                   bej_dictionary_context_t *dict, uint8_t add_name);
uint8_t decode_array(bej_context_t *ctx, uint32_t length,
                     bej_dictionary_context_t *dict, uint8_t add_name);
void write_indent(bej_context_t *ctx);

void bej_dump_dictionary(bej_dictionary_context_t *dict, uint16_t max_entries);