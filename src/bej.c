/**
 * @file bej.c
 * @brief Main BEJ decoder implementation
 */
#include "bej.h"

uint8_t
bej_read_nnint(uint8_t *data, size_t *offset, size_t size,
               uint32_t *value)
{
    if (*offset >= size)
        return FAILURE;

    uint8_t nnint_bytes_length = data[*offset];

    *value = 0U;
    for (uint8_t i = 0U; i < nnint_bytes_length; i++) {
        *value |= ((uint32_t)data[*offset + 1 + i]) << (8 * i);
    }

    dbgmsg("Read NNInt: %u (length %u)", *value, nnint_bytes_length);

    *offset += 1 + nnint_bytes_length;

    return SUCCESS;
}

uint8_t
bej_read_sequence_number(uint8_t *data, size_t *offset, size_t size,
                         uint32_t *seqnum, uint8_t *dictselector)
{
    uint32_t sequence = 0U;
    if (bej_read_nnint(data, offset, size, &sequence)) {
        return FAILURE;
    }
    *seqnum = sequence >> 1;
    *dictselector = (uint8_t)(sequence & 0x01);

    return SUCCESS;
}

uint8_t
bej_read_format(uint8_t *data, size_t *offset, size_t size, 
                uint8_t *format, uint8_t *flags)
{
    if (*offset >= size)
        return FAILURE;
    
    uint8_t format_byte = data[*offset];
    *format = (format_byte >> 4) & 0x0F;  // upper 4 as LE
    *flags = format_byte & 0x0F; // lower 4
    
    (*offset)++;
    
    dbgmsg("Read format: type=%u, flags=%#02x", *format, *flags);

    return SUCCESS;
}

uint8_t
bej_parse_dict(bej_dictionary_context_t *dict, uint8_t *data, size_t size)
{
    if (!dict || !data || size < 12UL)
        return FAILURE;

    size_t offset = 0UL;
    
    dict->version_tag = READ_U8_AND_INC(data, offset);
    dict->truncation_flag = READ_U8_AND_INC(data, offset);
    dict->entry_count = READ_U16_LE(data, offset);

    infomsg("Schema dictionary has %u entries", dict->entry_count);

    offset += 2;
    
    dict->schema_version = READ_U32_LE(data, offset);

    infomsg("Schema dictionary schema version: %#04x", dict->schema_version);

    offset += 4;
    
    dict->dictionary_size = size;
    dict->data = data;
    dict->data_size = size;
    
    return SUCCESS;
}

uint8_t
bej_find_dict_entry(bej_context_t *ctx, bej_dictionary_context_t *dict,
                    uint32_t sequence, bej_dict_entry_t *entry)
{
    if (!dict || !entry || !dict->data) {
        errmsg("Invalid parameters for dictionary lookup");
        return FAILURE;
    }

    /* start from global data offset just after the header
    *  unless parent entry has both entry->child_offset
    *  and entry->child_count specified; exception is for the root
    */
    size_t offset = ctx->parent_child_offset[ctx->indent_level];
    uint16_t count = ctx->parent_child_count[ctx->indent_level];

    for (uint16_t i = 0; i < count; i++) {
        if (offset + 12UL > dict->data_size) {
            errmsg("Dictionary entry exceeds bounds");
            return FAILURE;
        }
        
        uint8_t format = READ_U8_AND_INC(dict->data, offset);
        uint16_t seq = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint16_t child_off = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint16_t child_count = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint8_t name_length = READ_U8_AND_INC(dict->data, offset);
        uint16_t name_offset = READ_U16_LE(dict->data, offset);
        offset += 2;
        
        if (seq == sequence) {
            entry->format = format;
            entry->sequence = seq;
            entry->child_offset = child_off;
            entry->child_count = child_count;

            if (name_offset > 0 && name_offset < dict->data_size) {
                entry->name_length = name_length;
                entry->name_offset = name_offset;
            } else {
                entry->name_length = 0u;
                entry->name_offset = 0u;
            }
            
            dbgmsg("Found entry: seq=%u, format=%u, children=%u, name_len=%u",
                   seq, format, child_count, entry->name_length);
            
            return SUCCESS;
        }
    }
    
    dbgmsg("Entry with sequence %u not found", sequence);
    return FAILURE;
}

uint8_t
bej_get_entry_name(bej_dictionary_context_t *dict, bej_dict_entry_t *entry,
                   char *name, size_t name_size)
{
    if (!dict || !entry || !name || entry->name_length == 0)
        return FAILURE;

    uint8_t copy_len = entry->name_length;
    if (copy_len >= name_size)
        copy_len = name_size - 1;
    
    if (entry->name_offset + copy_len > dict->data_size) {
        errmsg("Name offset exceeds dictionary bounds");
        return FAILURE;
    }
    
    memcpy(name, &dict->data[entry->name_offset], copy_len);
    name[copy_len] = '\0';
    
    return SUCCESS;
}

static void
write_indent(bej_context_t *ctx)
{
    for (int i = 0; i < ctx->indent_level; i++) {
        fputc('\t', ctx->output);
    }
}

uint8_t
decode_integer(bej_context_t *ctx, uint8_t *value, uint32_t length)
{
    if (length == 0 || length > 8) {
        errmsg("Invalid integer length: %u", length);
        return FAILURE;
    }

    int64_t result = 0l; // le signed format
    
    for (uint32_t i = 0; i < length; i++) {
        result |= ((int64_t)value[i]) << (8 * i);
    }
    
    // sign extend if negative
    if (length < 8 && (value[length - 1] & 0x80)) {
        // fill upper bits with 1s
        for (uint32_t i = length; i < 8; i++) {
            result |= ((int64_t)0xFF) << (8 * i);
        }
    }
    
    fprintf(ctx->output, "%ld", result);
    return SUCCESS;
}

uint8_t
decode_string(bej_context_t *ctx, uint8_t *value, uint32_t length)
{
    if (length == 0) {
        fprintf(ctx->output, "\"\"");
        return SUCCESS;
    }
    /* last byte should be null terminator
       we don't need it */
    if (length > 0 && value[length - 1] == '\0') {
        length--;
    }
    
    fprintf(ctx->output, "\"");
    for (uint32_t i = 0; i < length; i++) {
        char c = value[i];
        switch (c) {
            case '\"': fprintf(ctx->output, "\\\""); break;
            case '\\': fprintf(ctx->output, "\\\\"); break;
            case '\b': fprintf(ctx->output, "\\b"); break;
            case '\f': fprintf(ctx->output, "\\f"); break;
            case '\n': fprintf(ctx->output, "\\n"); break;
            case '\r': fprintf(ctx->output, "\\r"); break;
            case '\t': fprintf(ctx->output, "\\t"); break;
            default:
                if (c >= 32 && c <= 126) {
                    fputc(c, ctx->output);
                } else {
                    fprintf(ctx->output, "\\u%04x", (unsigned char)c);
                }
        }
    }
    fprintf(ctx->output, "\"");
    return SUCCESS;
}

uint8_t
decode_enum(bej_context_t *ctx, uint8_t *value, uint32_t length,
            bej_dictionary_context_t *dict)
{
    size_t offset = 0UL;
    uint32_t enum_value = 0U;
    ctx->indent_level++;
    
    if (bej_read_nnint(value, &offset, length, &enum_value)) {
        ctx->indent_level--;
        errmsg("Failed to read enum value");
        return FAILURE;
    }
    
    // find enum string in child entries
    bej_dict_entry_t enum_entry;
    if (!bej_find_dict_entry(ctx, dict, enum_value, &enum_entry)) {
        char enum_name[BEJ_DICT_ENTRY_NAME_LENGTH+1];
        if (!bej_get_entry_name(dict, &enum_entry, enum_name, sizeof(enum_name))) {
            fprintf(ctx->output, "\"%s\"", enum_name);
            ctx->indent_level--;
            return SUCCESS;
        }
    }

    // print numeric then
    fprintf(ctx->output, "%u", enum_value);
    ctx->indent_level--;

    return SUCCESS;
}

uint8_t
decode_set(bej_context_t *ctx, uint32_t length,
           bej_dictionary_context_t *dict, uint8_t add_name)
{
    fprintf(ctx->output, "{\n");
    ctx->indent_level++;
    
    size_t set_end = ctx->offset + length;
    
    uint32_t count = 0U;    // reading elements count first
    if (bej_read_nnint(ctx->bej_data, &ctx->offset, ctx->bej_size, &count)) {
        ctx->indent_level--;
        errmsg("Failed to read set count");
        return FAILURE;
    }
    
    dbgmsg("Decoding set with %u elements", count);
    // now decoding each element
    for (uint32_t i = 0U; i < count && ctx->offset < set_end; i++) {
        write_indent(ctx);
        
        // set elements have names from dictionary
        if (decode_bej_sflv(ctx, dict, 1U)) {
            ctx->indent_level--;
            return FAILURE;
        }
        
        if (i < count - 1) {
            fprintf(ctx->output, ",");
        }
        fprintf(ctx->output, "\n");
    }
    
    ctx->indent_level--;
    write_indent(ctx);
    fprintf(ctx->output, "}");
    
    // check if length matches expectations
    if (ctx->offset != set_end) {
        warnmsg("Set length mismatch: expected %zu, got %zu", 
                set_end, ctx->offset);
        ctx->offset = set_end;
    }
    
    return SUCCESS;
}

uint8_t
decode_array(bej_context_t *ctx, uint32_t length,
             bej_dictionary_context_t *dict, uint8_t add_names)
{   // same things as for set here except for no names
    fprintf(ctx->output, "[\n");
    ctx->indent_level++;
    
    size_t array_end = ctx->offset + length;
    
    uint32_t count = 0U; 
    if (bej_read_nnint(ctx->bej_data, &ctx->offset, ctx->bej_size, &count)) {
        ctx->indent_level--;
        errmsg("Failed to read array count");
        return FAILURE;
    }
    
    dbgmsg("Decoding array with %u elements", count);
    
    for (uint32_t i = 0; i < count && ctx->offset < array_end; i++) {
        write_indent(ctx);
        
        // ...whereas arrays doesn't
        if (decode_bej_sflv(ctx, dict, 0U)) {
            ctx->indent_level--;
            return FAILURE;
        }
        
        if (i < count - 1) {
            fprintf(ctx->output, ",");
        }
        fprintf(ctx->output, "\n");
    }
    
    ctx->indent_level--;
    write_indent(ctx);
    fprintf(ctx->output, "]");
    
    if (ctx->offset != array_end) {
        warnmsg("Array length mismatch: expected %zu, got %zu", 
                array_end, ctx->offset);
        ctx->offset = array_end;
    }
    
    return SUCCESS;
}

uint8_t
decode_bej_sflv(bej_context_t *ctx, bej_dictionary_context_t *dict, 
                uint8_t add_name)
{
    uint32_t sequence = 0U;
    uint8_t dict_selector = 0U;

    uint8_t format = 0U;
    uint8_t flags = 0U;
    uint32_t length = 0U;

    if (bej_read_sequence_number(ctx->bej_data, &ctx->offset, ctx->bej_size,
                                 &sequence, &dict_selector)) {
        errmsg("Failed to read sequence number at offset %zu", ctx->offset);
        return FAILURE;
    }
    if (bej_read_format(ctx->bej_data, &ctx->offset, ctx->bej_size,
                        &format, &flags)) {
        errmsg("Failed to read format at offset %zu", ctx->offset);
        return FAILURE;
    }
    if (bej_read_nnint(ctx->bej_data, &ctx->offset, ctx->bej_size, &length)) {
        errmsg("Failed to read length at offset %zu", ctx->offset);
        return FAILURE;
    }
    if (ctx->offset + length > ctx->bej_size) {
        errmsg("Value length %u exceeds buffer at offset %zu", length, ctx->offset);
        return FAILURE;
    }
    
    uint8_t *value = &ctx->bej_data[ctx->offset];
    
    // performing dict lookup
    bej_dict_entry_t entry;
    uint8_t found_entry = !bej_find_dict_entry(ctx, dict, sequence, &entry);
    
    if (found_entry) {
        char name[BEJ_DICT_ENTRY_NAME_LENGTH+1] = {0};
        bej_get_entry_name(dict, &entry, name, sizeof(name));
        
        dbgmsg("Decoding entry: seq=%u, format=%u, name=\"%s\"", 
            sequence, format, name);

        if (add_name && name[0] != '\0') {
            fprintf(ctx->output, "\"%s\": ", name);
        }
    } else {    // this one unlikely, but let's the name based of seq
        dbgmsg("Decoding unknown entry: seq=%u, format=%u", sequence, format);
        if (add_name) {
            fprintf(ctx->output, "\"unknown_%u\": ", sequence);
        }
    }

    switch (format) {
        case BEJ_FORMAT_SET:
        case BEJ_FORMAT_ARRAY: {
            if (ctx->indent_level + 1 >= BEJ_CONTEXT_STACK_MAX_DEPTH) {
                errmsg("BEJ nesting too deep");
                return FAILURE;
            }
            ctx->parent_child_offset[ctx->indent_level+1] = entry.child_offset;
            ctx->parent_child_count[ctx->indent_level+1] = entry.child_count;

            return (!format) ? decode_set(ctx, length, dict, add_name) :
                               decode_array(ctx, length, dict, add_name); }
        case BEJ_FORMAT_INTEGER:
            ctx->offset += length;  // move past value for recursive call
            return decode_integer(ctx, value, length);
        case BEJ_FORMAT_STRING:
            ctx->offset += length;
            return decode_string(ctx, value, length);
        case BEJ_FORMAT_ENUM:
            ctx->offset += length;
            ctx->parent_child_offset[ctx->indent_level+1] = entry.child_offset;
            ctx->parent_child_count[ctx->indent_level+1] = entry.child_count;
            return decode_enum(ctx, value, length, dict);
        case BEJ_FORMAT_BOOLEAN:
            ctx->offset += length;
            fprintf(ctx->output, (length > 0 && value[0]) ? "true" : "false");
            return SUCCESS;
        case BEJ_FORMAT_NULL:
            ctx->offset += length;
            fprintf(ctx->output, "null");
            return SUCCESS;
        // todo: more types
        default:
            warnmsg("Unknown format type: %u", format);
            ctx->offset += length;
            fprintf(ctx->output, "null");
    }
    return SUCCESS;
}

uint8_t
bej_decode(bej_context_t *ctx)
{
    if (!ctx || !ctx->bej_data || !ctx->output){
        errmsg("Invalid context");
        return FAILURE;
    }

    // check if we need to actually check anything here
    if (ctx->bej_size < 7UL) {
        errmsg("Not a valid BEJ data");
        return FAILURE;
    }

    // either 1.1.0 or 1.1.0 is supported
    if (!(ctx->bej_data[0] == 0x00 && ctx->bej_data[1] == 0xF0
        && (ctx->bej_data[2] == 0xF0 || ctx->bej_data[2] == 0xF1)
        && ctx->bej_data[3] == 0xF1)) {
        errmsg("Unsupported BEJ version field: %#x",
               READ_U32_LE(ctx->bej_data, ctx->offset));
        return FAILURE;
    }

    if (!(ctx->bej_data[4] == 0x00 && ctx->bej_data[5] == 0x00)) {
        warnmsg("Non-zero BEJ flags: %#x",
                READ_U16_LE(ctx->bej_data, ctx->offset));
    }

    if (!(ctx->bej_data[6] == 0x00 || ctx->bej_data[5] == 0x01)) {
        if (ctx->bej_data[6] == 0x04) {
            warnmsg("The \"ERROR\" schema class (0x04) is not supported");
        }
        errmsg("Invalid BEJ schemaClass");
        return FAILURE;
    }
    
    ctx->offset += 7;   // unevenly skipping both version and flags bytes

    dbgmsg("Header is valid. Starting BEJ decode, data size: %zu bytes", ctx->bej_size);

    // decoding the root SFLV
    return decode_bej_sflv(ctx, &ctx->schema_dict, 0U);
}

#ifdef NDEBUG
void
bej_dump_dictionary(bej_dictionary_context_t *dict, uint16_t max_entries)
{
    if (!dict || !dict->data) {
        errmsg("Invalid dictionary\n");
        return;
    }
    
    infomsg("\n=== Dictionary Dump ===\n"
            "Version: %u, Truncation: %u\n"
            "Entry Count: %u\n"
            "Schema Version: %#08x\n"
            "Dictionary Size: %zu bytes",
            dict->version_tag, dict->truncation_flag, dict->entry_count,
            dict->schema_version, dict->data_size);
    
    size_t offset = 12L; // skip header
    uint16_t entries_to_show = (max_entries == 0 || max_entries > dict->entry_count) 
                               ? dict->entry_count : max_entries;
    
    for (uint16_t i = 0; i < entries_to_show && offset < dict->data_size; i++) {
        infomsg("\n--- Entry %u (offset %#zx) ---", i, offset);
        
        uint8_t format = READ_U8_AND_INC(dict->data, offset);
        uint16_t seq = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint16_t child_offset_ptr = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint16_t child_count = READ_U16_LE(dict->data, offset);
        offset += 2;
        uint8_t name_length = READ_U8_AND_INC(dict->data, offset);
        uint16_t name_off = READ_U16_LE(dict->data, offset);
        offset += 2;

        infomsg("\n\tFormat: %u"
                "\n\tSequence: %u"
                "\n\tChild Pointer Offset: %#x"
                "\n\tChild Count: %u"
                "\n\tName length: %u"
                "\n\tName offset: %#x",
                format, seq, child_offset_ptr, child_count, name_length, name_off);
        
        if (name_off > 0 && name_off < dict->data_size) {            
            if (name_length > 0 && name_off + 1 + name_length <= dict->data_size) {
                char name_buf[256];
                size_t copy_len = (name_length < sizeof(name_buf) - 1) ? name_length : sizeof(name_buf) - 1;
                memcpy(name_buf, &dict->data[name_off], copy_len);
                name_buf[copy_len] = '\0';
                infomsg("\tName: '%s'", name_buf);
            } else {
                infomsg("\tName: <invalid length or offset>");
            }
        } else {
            infomsg("\tName: <no name>");
        }
    }
    
    infomsg("\n=== End Dictionary Dump ===\n");
}
#endif /* NDEBUG */

uint8_t
bej_init_context(bej_context_t *ctx,
                 uint8_t *schema_data, size_t schema_size,
                 //uint8_t *anno_data, size_t anno_size,
                 uint8_t *bej_data, size_t bej_size,
                 FILE *output)
{
    if (!ctx || !schema_data || !bej_data || !output) {
        errmsg("Invalid parameters");
        return FAILURE;
    }

    memset(ctx, 0, sizeof(bej_context_t));

    if (bej_parse_dict(&ctx->schema_dict, schema_data, schema_size) != 0) {
        errmsg("Failed to parse schema dictionary");
        return FAILURE;
    }

#ifdef NDEBUG
    bej_dump_dictionary(&ctx->schema_dict, bej_size);
#endif /* NDEBUG */
    
    /*if (anno_data && anno_size > 0) {
        bej_parse_dict(&ctx->anno_dict, anno_data, anno_size);
    }*/
    
    ctx->bej_data = bej_data;
    ctx->bej_size = bej_size;
    ctx->offset = 0UL;
    ctx->output = output;
    ctx->indent_level = 0;
    ctx->parent_child_offset[0] = 12L;
    ctx->parent_child_count[0] = ctx->schema_dict.entry_count;
    
    return SUCCESS;
}