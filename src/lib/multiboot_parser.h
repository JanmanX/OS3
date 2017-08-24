#pragma once

#include <stdint.h>
#include <multiboot2.h>

void multiboot_parser_init(uint8_t* mb_info_data_struct);
struct multiboot_tag *multiboot_parser_get_tag(uint32_t tag);
