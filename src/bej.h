#pragma once
#include "common.h"

/*
* converts BEJ encoded data to JSON format.
* this function is main decoder interface.
*/
bool bej_decode(const uint8_t *bej_data, size_t bej_size,
                 const uint8_t *schema_dict_data, size_t schema_dict_size,
                 const uint8_t *anno_dict_data, size_t anno_dict_size,
                 char *output_buffer, size_t output_buffer_size,
                 size_t *output_length);

