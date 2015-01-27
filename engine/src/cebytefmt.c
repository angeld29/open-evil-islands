/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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

#include "cestr.h"
#include "cebytefmt.h"

char* ce_bytefmt_size(char* buffer, size_t size, long long int bytes)
{
    long double KiB = 1024.0l;
    long double MiB = 1024.0l * KiB;
    long double GiB = 1024.0l * MiB;

    if (bytes > GiB) {
        snprintf(buffer, size, "%.1Lf GiB", bytes / GiB);
    } else if (bytes > MiB) {
        snprintf(buffer, size, "%.1Lf MiB", bytes / MiB);
    } else if (bytes > KiB) {
        snprintf(buffer, size, "%.1Lf KiB", bytes / KiB);
    } else {
        snprintf(buffer, size, "%llu B", bytes);
    }

    return buffer;
}

char* ce_bytefmt_dot(char* buffer, size_t size, long long int bytes)
{
    char triple[8];
    const char* triple_format = "%llu";

    long long int KiB = 1000ll;
    long long int MiB = 1000ll * KiB;
    long long int GiB = 1000ll * MiB;

    ce_strlcpy(buffer, "", size);

    if (bytes > GiB) {
        snprintf(triple, sizeof(triple), triple_format, bytes / GiB);
        ce_strlcat(buffer, triple, size);
        ce_strlcat(buffer, ".", size);
        triple_format = "%03llu";
        bytes %= GiB;
    }
    if (bytes > MiB) {
        snprintf(triple, sizeof(triple), triple_format, bytes / MiB);
        ce_strlcat(buffer, triple, size);
        ce_strlcat(buffer, ".", size);
        triple_format = "%03llu";
        bytes %= MiB;
    }
    if (bytes > KiB) {
        snprintf(triple, sizeof(triple), triple_format, bytes / KiB);
        ce_strlcat(buffer, triple, size);
        ce_strlcat(buffer, ".", size);
        triple_format = "%03llu";
        bytes %= KiB;
    }

    snprintf(triple, sizeof(triple), triple_format, bytes);
    ce_strlcat(buffer, triple, size);
    ce_strlcat(buffer, " B", size);

    return buffer;
}

char* ce_bytefmt_detail(char* buffer, size_t size, long long int bytes)
{
    char tmp1[16], tmp2[32];
    snprintf(buffer, size, "%s (%s)",
        ce_bytefmt_size(tmp1, sizeof(tmp1), bytes),
        ce_bytefmt_dot(tmp2, sizeof(tmp2), bytes));
    return buffer;
}
