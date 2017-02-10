/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2017 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

#ifndef CE_MEMFILE_HPP
#define CE_MEMFILE_HPP

#include "byteorder.hpp"

#include <boost/filesystem/path.hpp>

namespace cursedearth
{
    typedef struct ce_mem_file ce_mem_file;

    typedef struct {
        size_t size;
        int (*close)(ce_mem_file* mem_file);
        size_t (*read)(ce_mem_file* mem_file, void* ptr, size_t size, size_t n);
        int (*seek)(ce_mem_file* mem_file, long int offset, int whence);
        long int (*tell)(ce_mem_file* mem_file);
        int (*eof)(ce_mem_file* mem_file);
        int (*error)(ce_mem_file* mem_file);
    } ce_mem_file_vtable;

    struct ce_mem_file {
        ce_mem_file_vtable vtable;
        void* impl;
    };

    extern const int CE_MEM_FILE_SEEK_CUR;
    extern const int CE_MEM_FILE_SEEK_END;
    extern const int CE_MEM_FILE_SEEK_SET;

    /*
     *  You may to instruct mem file to either automatically close or not to close
     *  the resource in mem file_close. Automatic closure is disabled by passing
     *  NULL as the close callback.
    */
    ce_mem_file* ce_mem_file_new(ce_mem_file_vtable vtable);
    void ce_mem_file_del(ce_mem_file* mem_file);

    /*
     *  Implements in-memory files.
     *  NOTE: mem file takes ownership of the data.
    */
    ce_mem_file* ce_mem_file_new_data(void* data, size_t size);

    /*
     *  Implements a buffered interface for the FILE standard functions.
    */
    ce_mem_file* ce_mem_file_new_path(const boost::filesystem::path&);

    inline size_t ce_mem_file_read(ce_mem_file* mem_file, void* ptr, size_t size, size_t n)
    {
        return (mem_file->vtable.read)(mem_file, ptr, size, n);
    }

    inline int ce_mem_file_seek(ce_mem_file* mem_file, long int offset, int whence)
    {
        return (mem_file->vtable.seek)(mem_file, offset, whence);
    }

    inline long int ce_mem_file_tell(ce_mem_file* mem_file)
    {
        return (mem_file->vtable.tell)(mem_file);
    }

    inline void ce_mem_file_rewind(ce_mem_file* mem_file)
    {
        ce_mem_file_seek(mem_file, 0L, CE_MEM_FILE_SEEK_SET);
    }

    inline int ce_mem_file_eof(ce_mem_file* mem_file)
    {
        return (mem_file->vtable.eof)(mem_file);
    }

    inline int ce_mem_file_error(ce_mem_file* mem_file)
    {
        return (mem_file->vtable.error)(mem_file);
    }

    inline long int ce_mem_file_size(ce_mem_file* mem_file)
    {
        long int pos = ce_mem_file_tell(mem_file);
        ce_mem_file_seek(mem_file, 0, CE_MEM_FILE_SEEK_END);
        long int size = ce_mem_file_tell(mem_file);
        ce_mem_file_seek(mem_file, pos, CE_MEM_FILE_SEEK_SET);
        return size;
    }

    inline void ce_mem_file_skip(ce_mem_file* mem_file, size_t size)
    {
        ce_mem_file_seek(mem_file, size, CE_MEM_FILE_SEEK_CUR);
    }

    inline int8_t ce_mem_file_read_i8(ce_mem_file* mem_file)
    {
        int8_t value;
        ce_mem_file_read(mem_file, &value, 1, 1);
        return value;
    }

    inline int16_t ce_mem_file_read_i16le(ce_mem_file* mem_file)
    {
        int16_t value;
        ce_mem_file_read(mem_file, &value, 2, 1);
        return le2cpu(value);
    }

    inline int32_t ce_mem_file_read_i32le(ce_mem_file* mem_file)
    {
        int32_t value;
        ce_mem_file_read(mem_file, &value, 4, 1);
        return le2cpu(value);
    }

    inline int64_t ce_mem_file_read_i64le(ce_mem_file* mem_file)
    {
        int64_t value;
        ce_mem_file_read(mem_file, &value, 8, 1);
        return le2cpu(value);
    }

    inline uint8_t ce_mem_file_read_u8(ce_mem_file* mem_file)
    {
        uint8_t value;
        ce_mem_file_read(mem_file, &value, 1, 1);
        return value;
    }

    inline uint16_t ce_mem_file_read_u16le(ce_mem_file* mem_file)
    {
        uint16_t value;
        ce_mem_file_read(mem_file, &value, 2, 1);
        return le2cpu(value);
    }

    inline uint32_t ce_mem_file_read_u32le(ce_mem_file* mem_file)
    {
        uint32_t value;
        ce_mem_file_read(mem_file, &value, 4, 1);
        return le2cpu(value);
    }

    inline uint64_t ce_mem_file_read_u64le(ce_mem_file* mem_file)
    {
        uint64_t value;
        ce_mem_file_read(mem_file, &value, 8, 1);
        return le2cpu(value);
    }

    inline uint32_t ce_mem_file_read_fle(ce_mem_file* mem_file)
    {
        float value;
        ce_mem_file_read(mem_file, &value, 4, 1);
        return value;
    }
}

#endif
