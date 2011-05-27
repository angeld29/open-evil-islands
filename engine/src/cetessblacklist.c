/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2011 Anton Kurkin, Yanis Kurganov.
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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "cetessblacklist.h"
#include "cestr.h"
#include "cealloc.h"
#include "celogging.h"

ce_tess_blacklist* ce_blacklist;

/// cut comment after # and '\n' char
char* ce_strtrim_blacklistcomment(char* restrict str)
{
	size_t len = strlen(str);
	if (0 != len) {
		size_t commentpos;
		for (commentpos = 0; commentpos < len && str[commentpos] != '#' && str[commentpos] != '\n'; ++commentpos) {
		}
		len = commentpos;
	}
	str[len] = '\0';
	return str;
}

static bool ce_blacklist_parse(ce_tess_blacklist* blacklist,
									const char* path, FILE* file)
{
	enum {
		MAX_LINE_SIZE = 256,
	};
	const char INVERT[] = "ALL_EXCEPT";

	char line[MAX_LINE_SIZE], temp1[MAX_LINE_SIZE], temp2[MAX_LINE_SIZE];
	ce_tess_blacklist_section* section = NULL;

    ce_vector* nesting_list = ce_vector_new();
    bool leaf_lvl = false;
    char* searchedchr;

	for (int line_number = 1; NULL != fgets(temp1, MAX_LINE_SIZE, file); ++line_number) {
		size_t line_length = strlen(ce_strtrimblank(line,ce_strtrim_blacklistcomment(temp1)));

		if (line_length + 1 == MAX_LINE_SIZE) {
			ce_logging_warning("blacklist file: %s:%d: line is too long: "
								"'%s', skipped...", path, line_number, line);
			// FIXME: skip line tail until new line
			int ch;
			do {
				ch = fgetc(file);
			} while (EOF != ch && '\n' != ch);
			continue;
		}

		if (0 == line_length || '\n' == line[0]) {
			continue;
		}

//      blacklist reading debug output
//		if(!leaf_lvl)
//            printf("%d: %s",nesting_list->count, line);
//        else
//            printf("\t%s",line);

		if ('[' == line[0] || '{' == line[0]) {
			ce_logging_error("blacklist file: %s:%d: place brackets at parent "
								"section line", path, line_number);
            ce_vector_del(nesting_list);
			return false;
		}

        if ((']' == line[0] && !leaf_lvl) || ('}' == line[0] && 0 == nesting_list->count) ) {
			ce_logging_error("blacklist file: %s:%d: closing bracket doesn`t "
								"have opening one", path, line_number);
            ce_vector_del(nesting_list);
			return false;
        }

        if (leaf_lvl) {
            if (']' == line[0])  {
                leaf_lvl = false;
                continue;
            }
            if (searchedchr = strchr(line, '\n'))
                ce_vector_push_back(section->child_sections, ce_string_new_str(ce_strleft(temp1, line, searchedchr - line)));
            else
                ce_vector_push_back(section->child_sections, ce_string_new_str(line));
            continue;
        }

        if ('}' == line[0]) {
            ce_vector_pop_back(nesting_list);
			continue;
        }

        section = ce_alloc(sizeof(ce_tess_blacklist_section));

        if (0 == nesting_list->count)
            ce_vector_push_back (blacklist->sections, section);
        else
            ce_vector_push_back (((ce_tess_blacklist_section*) nesting_list->items[nesting_list->count - 1])->child_sections, section);

		section->isNameEnd = true;
        if (searchedchr = strchr(line, '*')) {
			section->name = ce_string_new_str(ce_strleft(temp1, line, searchedchr - line));
			section->isNameEnd = false;
        }
        else if (searchedchr = strchr(line, ' '))
			section->name = ce_string_new_str(ce_strleft(temp1, line, searchedchr - line));
        else if (searchedchr = strchr(line, '['))
			section->name = ce_string_new_str(ce_strleft(temp1, line, searchedchr - line));
        else if (searchedchr = strchr(line, '{'))
			section->name = ce_string_new_str(ce_strleft(temp1, line, searchedchr - line));
        else if (searchedchr = strchr(line, '\n'))
			section->name = ce_string_new_str(ce_strleft(temp1, line, searchedchr - line));
        else
			section->name = ce_string_new_str(line);

        if (NULL != strstr(line, INVERT))
            section->isAllExcept = true;
        else
            section->isAllExcept = false;

		section->child_sections = ce_vector_new();
        if (NULL == strrchr(line, '{')) {
            section->isLeafNode = true;
            if (NULL != strrchr(line, '['))
                leaf_lvl = true;
        } else {
            section->isLeafNode = false;
			section->isNameEnd = false;
            ce_vector_push_back(nesting_list, section);
        }
	}
	ce_vector_del(nesting_list);
	return true;
}

ce_tess_blacklist* ce_blacklist_open(const char* path)
{
	FILE* file = fopen(path, "rt");
	if (NULL == file) {
		ce_logging_error("blacklist file: could not open file '%s'", path);
		return NULL;
	}

	ce_tess_blacklist* blacklist = ce_alloc(sizeof(ce_tess_blacklist));
	blacklist->sections = ce_vector_new();

	if (!ce_blacklist_parse(blacklist, path, file)) {
		ce_logging_error("blacklist file: failed to parse '%s'", path);
		ce_blacklist_close(blacklist);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return blacklist;
}

void ce_blacklist_free_section(ce_tess_blacklist_section* section)
{
    if (section->isLeafNode)
		for (size_t i = 0; i < section->child_sections->count; ++i)
            ce_string_del(section->child_sections->items[i]);
    else
        for (size_t i = 0; i < section->child_sections->count; ++i)
            ce_blacklist_free_section(section->child_sections->items[i]);
	ce_vector_del(section->child_sections);
	ce_string_del(section->name);
    ce_free(section, sizeof(ce_tess_blacklist_section));
}

void ce_blacklist_close(ce_tess_blacklist* blacklist)
{
	if (NULL != blacklist) {
		for (size_t i = 0; i < blacklist->sections->count; ++i)
			ce_blacklist_free_section(blacklist->sections->items[i]);

		ce_vector_del(blacklist->sections);
		ce_free(blacklist, sizeof(ce_tess_blacklist));
	}
}
