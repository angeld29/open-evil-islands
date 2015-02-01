/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
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

/*
 *  Abstraction layer for read-only binary files based on FILE interface.
 *  All implementations signal an EOF of file condition synchronously
 *  with the transmission of the last bytes of a file.
 */

#ifndef CE_MEMORYFILE_HPP
#define CE_MEMORYFILE_HPP

#include <memory>

#include <boost/noncopyable.hpp>

#include "byteorder.hpp"

namespace cursedearth
{
    typedef std::shared_ptr<class memory_file_t> memory_file_ptr_t;
    typedef std::shared_ptr<const class memory_file_t> memory_file_const_ptr_t;

    class memory_file_t: boost::noncopyable
    {
    public:
        enum seek_mode_t
        {
            SEEK_MODE_CUR,
            SEEK_MODE_END,
            SEEK_MODE_SET
        };

    public:
        virtual ~memory_file_t() = 0;

        virtual int close() = 0;
        virtual size_t read(void* ptr, size_t size, size_t n) = 0;
        virtual int seek(long int offset, seek_mode_t whence) = 0;
        virtual long int tell() = 0;
        virtual int eof() = 0;
        virtual int error() = 0;
    };

    // in-memory files
    // takes ownership of the data
    memory_file_ptr_t make_memory_file(void* data, size_t size);

    // buffered interface for the FILE standard functions
    memory_file_ptr_t make_memory_file(const char* path);

    inline void rewind(memory_file_t* memory_file)
    {
        memory_file->seek(0L, memory_file_t::SEEK_MODE_SET);
    }

    inline long int file_size(memory_file_t* memory_file)
    {
        long int pos = memory_file->tell();
        memory_file->seek(0, memory_file_t::SEEK_MODE_END);
        long int size = memory_file->tell();
        memory_file->seek(pos, memory_file_t::SEEK_MODE_SET);
        return size;
    }

    inline void skip(memory_file_t* memory_file, size_t size)
    {
        memory_file->seek(size, memory_file_t::SEEK_MODE_CUR);
    }

    inline int8_t read_i8(memory_file_t* memory_file)
    {
        int8_t value;
        memory_file->read(&value, 1, 1);
        return value;
    }

    inline int16_t read_i16le(memory_file_t* memory_file)
    {
        int16_t value;
        memory_file->read(&value, 2, 1);
        ce_le2cpu16s((uint16_t*)&value);
        return value;
    }

    inline int32_t read_i32le(memory_file_t* memory_file)
    {
        int32_t value;
        memory_file->read(&value, 4, 1);
        ce_le2cpu32s((uint32_t*)&value);
        return value;
    }

    inline int64_t read_i64le(memory_file_t* memory_file)
    {
        int64_t value;
        memory_file->read(&value, 8, 1);
        ce_le2cpu64s((uint64_t*)&value);
        return value;
    }

    inline uint8_t read_u8(memory_file_t* memory_file)
    {
        uint8_t value;
        memory_file->read(&value, 1, 1);
        return value;
    }

    inline uint16_t read_u16le(memory_file_t* memory_file)
    {
        uint16_t value;
        memory_file->read(&value, 2, 1);
        return ce_le2cpu16(value);
    }

    inline uint32_t read_u32le(memory_file_t* memory_file)
    {
        uint32_t value;
        memory_file->read(&value, 4, 1);
        return ce_le2cpu32(value);
    }

    inline uint64_t read_u64le(memory_file_t* memory_file)
    {
        uint64_t value;
        memory_file->read(&value, 8, 1);
        return ce_le2cpu64(value);
    }

    inline uint32_t read_fle(memory_file_t* memory_file)
    {
        float value;
        memory_file->read(&value, 4, 1);
        return value;
    }
}

#endif /* CE_MEMORYFILE_HPP */
