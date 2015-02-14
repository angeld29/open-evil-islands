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

#include "soundsystem.hpp"
#include "optionmanager.hpp"

namespace cursedearth
{
    bool ce_sound_system_null_ctor()
    {
        ce_logging_info("sound system: using null output");
        return true;
    }

    bool ce_sound_system_null_write(const sound_block_ptr_t&)
    {
        return true;
    }

    sound_system_t::sound_system_t():
        singleton_t<sound_system_t>(this),
        m_format(make_native_format()),
        m_buffer(std::make_shared<sound_buffer_t>(m_format)),
        m_thread([this]{execute();})
    {
    }

    sound_system_t::~sound_system_t()
    {
        m_thread.interrupt();
        m_thread.join();
    }

    sound_block_ptr_t sound_system_t::map()
    {
        return m_buffer->acquire_block();
    }

    void sound_system_t::unmap(const sound_block_ptr_t& block)
    {
        m_buffer->write(block);
    }

    void sound_system_t::execute()
    {
        try {
            while (true) {
                sound_block_ptr_t block = m_buffer->read();
                write(block);
                m_buffer->release_block(block);
            }
        } catch (const thread_interrupted_t&) {
            ce_logging_info("sound system: interrupted");
        } catch (const std::exception& error) {
            ce_logging_fatal("sound system: %s", error.what());
        } catch (...) {
            ce_logging_fatal("sound system: unknown error");
        }
    }
}
