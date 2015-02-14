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

#include <functional>

#include "alloc.hpp"
#include "logging.hpp"
#include "optionmanager.hpp"
#include "soundsystem.hpp"

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

    ce_sound_system_vtable ce_sound_system_null()
    {
        ce_sound_system_vtable vt = { 0, ce_sound_system_null_ctor, NULL, ce_sound_system_null_write };
        return vt;
    }

    void ce_sound_system_dtor()
    {
        if (NULL != sound_system_t::instance()) {
            if (NULL != sound_system_t::instance()->vtable.dtor) {
                (*sound_system_t::instance()->vtable.dtor)();
            }
            ce_free(sound_system_t::instance()->impl, sound_system_t::instance()->vtable.size);
        }
    }

    bool ce_sound_system_ctor(ce_sound_system_vtable vtable)
    {
        sound_system_t::instance()->impl = ce_alloc_zero(vtable.size);
        sound_system_t::instance()->vtable = vtable;

        if (!(*vtable.ctor)()) {
            ce_sound_system_dtor();
            return false;
        }

        return true;
    }

    sound_system_t::sound_system_t():
        singleton_t<sound_system_t>(this),
        m_buffer(std::make_shared<sound_buffer_t>(make_default_format())),
        m_done(false)
    {
        if (!ce_sound_system_ctor(option_manager_t::instance()->disable_sound() ? ce_sound_system_null() : ce_sound_system_platform())) {
            ce_sound_system_ctor(ce_sound_system_null());
        }
        m_thread = std::thread([this]{execute();});
    }

    sound_system_t::~sound_system_t()
    {
        m_done = true;
        m_thread.join();
        ce_sound_system_dtor();
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
        while (!m_done) {
            sound_block_ptr_t block = m_buffer->read();
            if (!(*vtable.write)(block)) {
                ce_logging_critical("sound system: could not write block");
            }
            m_buffer->release_block(block);
        }
    }

    sound_system_ptr_t make_sound_system()
    {
        return make_unique<sound_system_t>();
    }
}
