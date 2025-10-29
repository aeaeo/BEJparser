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
    BEJ_FORMAT_PROPERTY_ANNOTATION = 0xA,
    BEJ_FORMAT_RESOURCE_LINK = 0x0E,
    BEJ_FORMAT_RESOURCE_LINK_EXPANSION = 0x0F,
    BEJ_FORMAT_UNKNOWN = 0xFF
};

enum eBEJflags {
    BEJ_FLAG_DEFERRED = 1 << 0,
    BEJ_FLAG_READONLY = 1 << 1,
    BEJ_FLAG_NULLABLE = 1 << 2,
    BEJ_FLAG_NESTED_TOP_LEVEL_ANNOTATION = 1 << 1
};

/**
 * This one is a helper struct to store both header & data information as regards dictionary 
 */
typedef struct {
    uint8_t version_tag;
    uint8_t truncation_flag;    // not a flag but flags, only the named one is used though
    uint16_t entry_count;
    uint32_t schema_version;
    uint32_t dictionary_size;
    uint8_t *data;
    size_t data_size;
} bej_dictionary_context_t;

/**
 * Represents single dictionary entry
 */
typedef struct {
    uint8_t format;
    uint16_t sequence;
    uint16_t child_offset;
    uint16_t child_count;
    uint8_t name_length;
    uint16_t name_offset;
} bej_dict_entry_t;

/**
 * This one is a helper struct to maintain decoder context and mainly reduce verbosity
 * @todo Implement and integrate annotation dictionary logic
 */
typedef struct {
    bej_dictionary_context_t schema_dict;
    //bej_dictionary_t anno_dict;
    uint8_t *bej_data;
    size_t bej_size;
    size_t offset;
    FILE *output;
    int indent_level;
    uint16_t parent_child_offset[BEJ_CONTEXT_STACK_MAX_DEPTH];
    uint16_t parent_child_count[BEJ_CONTEXT_STACK_MAX_DEPTH];
} bej_context_t;


/**
 * @brief Converts BEJ encoded data to JSON format. This function is main decoder interface.
 * 
 * @param ctx BEJ decoder context
 * @return SUCCESS or FAILURE
 */
uint8_t bej_decode(bej_context_t *ctx);


/**
 * @brief Initialize BEJ decoder context
 * 
 * @param ctx Context to initialize
 * @param schema_data Schema dictionary binary data
 * @param schema_size Size of schema dictionary
 * @param bej_data BEJ encoded data
 * @param bej_size Size of BEJ data
 * @param output Output stream for JSON
 * @return SUCCESS or FAILURE
 */
uint8_t bej_init_context(bej_context_t *ctx,
                         uint8_t *schema_data, size_t schema_size,
                         uint8_t *bej_data, size_t bej_size,
                         FILE *output);


/**
 * @brief Parse BEJ dictionary
 * 
 * @param dict Dictionary structure to populate
 * @param data Dictionary binary data
 * @param size Size of dictionary data
 * @return SUCCESS or FAILURE
 */
uint8_t bej_parse_dict(bej_dictionary_context_t *dict, uint8_t *data, size_t size);


/**
 * @brief Read NNINT (Non-Negative Integer) from BEJ stream
 * 
 * NNINT format: First byte is length, followed by value in little-endian
 * 
 * @param data Input data buffer
 * @param offset Current offset (will be updated)
 * @param size Total size of data
 * @param value Output value
 * @return SUCCESS or FAILURE
 */
uint8_t bej_read_nnint(uint8_t *data, size_t *offset, size_t size, uint32_t *value);


/**
 * @brief Read and decode sequence number
 * 
 * @param data Input data buffer
 * @param offset Current offset (will be updated)
 * @param size Total size of data
 * @param seqnum Output sequence number (bits 1-31)
 * @param dictselector Output dictionary selector (bit 0)
 * @return SUCCESS or FAILURE
 */
uint8_t bej_read_sequence_number(uint8_t *data, size_t *offset, size_t size, uint32_t *seqnum, uint8_t *dictselector);


/**
 * @brief Find dictionary entry by sequence number
 * 
 * @param ctx BEJ decoder context
 * @param dict Dictionary to search
 * @param sequence Sequence number to find
 * @param entry Output entry structure
 * @return SUCCESS or FAILURE
 */
uint8_t bej_find_dict_entry(bej_context_t *ctx, bej_dictionary_context_t *dict, uint32_t sequence, bej_dict_entry_t *entry);


/**
 * @brief Get dictionary entry name
 * 
 * @param dict Dictionary
 * @param entry Dictionary entry
 * @param name Output buffer for name
 * @param name_size Size of output buffer
 * @return SUCCESS or FAILURE
 */
uint8_t bej_get_entry_name(bej_dictionary_context_t *dict, bej_dict_entry_t *entry, 
                           char *name, size_t name_size);


/**
 * @brief Read and decode sequence number
 * 
 * @param data Input data buffer
 * @param offset Current offset (will be updated)
 * @param size Total size of data
 * @param format Entry property format
 * @param flags Lower nibble of format: [3] - reserved flag. [2] - nullable_property flag.
 * [1] - read_only_property_and_top_level_annotation flag. [0] - deferred_binding flag.
 * @return SUCCESS or FAILURE
 */                       
uint8_t bej_read_format(uint8_t *data, size_t *offset, size_t size, 
                        uint8_t *format, uint8_t *flags);


/**
 * @brief Recursively decode SFLV blocks starting from root
 * 
 * @param ctx BEJ decoder context
 * @param dict Dictionary to search
 * @param add_name Whether entry name must be written to ctx->output. 1U - Write name; 0 - Don't
 * @return SUCCESS or FAILURE
 */
uint8_t decode_bej_sflv(bej_context_t *ctx, bej_dictionary_context_t *dict, 
                        uint8_t add_name);


/**
 * @brief Decode Integer enum object
 * 
 * @param ctx BEJ decoder context
 * @param value Integer value pointer
 * @param length Value length
 * @return SUCCESS or FAILURE
 */
uint8_t decode_integer(bej_context_t *ctx, uint8_t *value, uint32_t length);


/**
 * @brief Decode String enum object
 * 
 * @param ctx BEJ decoder context
 * @param value String value pointer
 * @param length Value length
 * @return SUCCESS or FAILURE
 */
uint8_t decode_string(bej_context_t *ctx, uint8_t *value, uint32_t length);


/**
 * @brief Decode SFLV enum object
 * 
 * @param ctx BEJ decoder context
 * @param value Integer value pointer of the sequence number for the enumeration option selected
 * @param length Value length
 * @return SUCCESS or FAILURE
 */
uint8_t decode_enum(bej_context_t *ctx, uint8_t *value, uint32_t length,
                    bej_dictionary_context_t *dict);


/**
 * @brief Decode SFLV set object recursively
 * 
 * @param ctx BEJ decoder context
 * @param length Value length
 * @param dict Dictionary to search
 * @return SUCCESS or FAILURE
 */
uint8_t decode_set(bej_context_t *ctx, uint32_t length, 
                   bej_dictionary_context_t *dict);


/**
 * @brief Decode SFLV array object recursively
 * 
 * @param ctx BEJ decoder context
 * @param length Value length
 * @param dict Dictionary to search
 * @return SUCCESS or FAILURE
 */
uint8_t decode_array(bej_context_t *ctx, uint32_t length,
                     bej_dictionary_context_t *dict);

#ifdef NDEBUG
/**
 * @brief Dump dictionary contents for debugging
 * 
 * @param dict Dictionary to dump
 * @param max_entries Maximum entries to dump (0 = all)
 * @return nothing
 */
void bej_dump_dictionary(bej_dictionary_context_t *dict, uint16_t max_entries);
#endif