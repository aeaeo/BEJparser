#pragma once
#include <stddef.h> // size_t
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1

#define BEJ_CONTEXT_STACK_MAX_DEPTH ((uint8_t)16)
#define BEJ_DICT_ENTRY_NAME_LENGTH ((uint8_t)255)

#define READ_U8_AND_INC(ptr, off) ((uint8_t)(ptr[off++]))
#define READ_U16_LE(ptr, off) ((uint16_t)(ptr[off + 1] << 8) | (uint16_t)(ptr[off]))
#define READ_U32_LE(ptr, off) ((uint32_t)(ptr[off + 3] << 24) | ((uint32_t)(ptr[off + 2] << 16)) | \
                              ((uint32_t)(ptr[off + 1] << 8)) | (uint32_t)(ptr[off]))

#define errmsg(fmt, ...) \
    fprintf(stderr, "Error at %s():%d What: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#define warnmsg(fmt, ...) \
    fprintf(stdout, "Warning at %s():%d What: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
    
#ifdef NDEBUG
#define dbgmsg(fmt, ...) \
    fprintf(stdout, "[DEBUG] %s():%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define dbgmsg(fmt, ...) ((void)0)
#endif /* NDEBUG */

#define infomsg(fmt, ...) \
    fprintf(stdout, fmt "\n", ##__VA_ARGS__)