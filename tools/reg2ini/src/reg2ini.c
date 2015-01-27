/*
 *  This file is part of Cursed Earth
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands
 *  Copyright (C) 2009-2010 Yanis Kurganov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static const uint32_t SIGNATURE = 0x45ab3efbu;

typedef struct {
    uint16_t unknown;
    uint32_t offset;
    uint8_t type;
    uint16_t option_name_length;
    char* option_name;
    uint16_t value_count;
} reg_option;

typedef struct {
    uint16_t unknown;
    uint32_t offset;
    uint16_t option_count;
    uint16_t section_name_length;
} reg_section;

typedef struct {
    uint32_t signature;
    uint16_t section_count;
} reg_header;

static void dump_int(FILE* reg_file, reg_option* option)
{
    int32_t values[option->value_count];
    fread(values, 4, option->value_count, reg_file);
    for (uint16_t i = 0; i < option->value_count; ++i) {
        printf("%s=%d\n", option->option_name, values[i]);
    }
}

static void dump_float(FILE* reg_file, reg_option* option)
{
    float values[option->value_count];
    fread(values, 4, option->value_count, reg_file);
    for (uint16_t i = 0; i < option->value_count; ++i) {
        printf("%s=%f\n", option->option_name, values[i]);
    }
}

static void dump_string(FILE* reg_file, reg_option* option)
{
    for (uint16_t i = 0; i < option->value_count; ++i) {
        uint16_t length;
        fread(&length, 2, 1, reg_file);

        char value[length + 1];
        value[length] = '\0';

        fread(value, 1, length, reg_file);
        printf("%s=%s\n", option->option_name, value);
    }
}

static void (*dump_procs[])(FILE*, reg_option*) = {
    [0] = dump_int,
    [1] = dump_float,
    [2] = dump_string,
};

int main(int argc, char* argv[])
{
    if (2 != argc) {
        fprintf(stderr, "reg2ini - REG to INI file converter\n"
                        "Copyright (C) 2009-2010 Yanis Kurganov\n\n"
                        "It's a small tool from reverse engineering "
                        "REG file format for project Cursed Earth.\n\n"
                        "usage: %s file.reg\n"
                        "       %s file.reg > file.ini\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    FILE* reg_file = fopen(argv[1], "rb");
    if (NULL == reg_file) {
        fprintf(stderr, "could not open file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    reg_header header;
    fread(&header.signature, 4, 1, reg_file);

    if (SIGNATURE != header.signature) {
        fprintf(stderr, "failed: wrong REG signature\n");
        fclose(reg_file);
        return EXIT_FAILURE;
    }

    fread(&header.section_count, 2, 1, reg_file);
    fprintf(stderr, "found %hu section(s)\n", header.section_count);

    reg_section sections[header.section_count];

    for (uint16_t i = 0; i < header.section_count; ++i) {
        fread(&sections[i].unknown, 2, 1, reg_file);
        fread(&sections[i].offset, 4, 1, reg_file);
    }

    for (uint16_t i = 0; i < header.section_count; ++i) {
        fseek(reg_file, sections[i].offset, SEEK_SET);

        fread(&sections[i].option_count, 2, 1, reg_file);
        fread(&sections[i].section_name_length, 2, 1, reg_file);

        char section_name[sections[i].section_name_length + 1];
        section_name[sections[i].section_name_length] = '\0';

        fread(section_name, 1, sections[i].section_name_length, reg_file);

        printf("[%s]\n", section_name);

        reg_option options[sections[i].option_count];

        for (uint16_t j = 0; j < sections[i].option_count; ++j) {
            fread(&options[j].unknown, 2, 1, reg_file);
            fread(&options[j].offset, 4, 1, reg_file);
        }

        for (uint16_t j = 0; j < sections[i].option_count; ++j) {
            fseek(reg_file, sections[i].offset + options[j].offset, SEEK_SET);

            fread(&options[j].type, 1, 1, reg_file);
            fread(&options[j].option_name_length, 2, 1, reg_file);

            char option_name[options[j].option_name_length + 1];
            option_name[options[j].option_name_length] = '\0';
            options[j].option_name = option_name;

            fread(option_name, 1, options[j].option_name_length, reg_file);

            if (options[j].type >= 128) {
                fread(&options[j].value_count, 2, 1, reg_file);
                options[j].type -= 128;
            } else {
                options[j].value_count = 1;
            }

            (*dump_procs[options[j].type])(reg_file, &options[j]);
        }
    }

    fprintf(stderr, "success\n");
    fclose(reg_file);

    return EXIT_SUCCESS;
}
