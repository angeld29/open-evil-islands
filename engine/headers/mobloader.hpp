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

#ifndef CE_MOBLOADER_HPP
#define CE_MOBLOADER_HPP

#include <string>

#include "vector.hpp"
#include "mobfile.hpp"

namespace cursedearth
{
    typedef struct {
        size_t processed_event_count;
        size_t posted_event_count;
        ce_string* name;
        ce_mob_file* mob_file;
    } ce_mob_task;

    ce_mob_task* ce_mob_task_new(const char* name);
    void ce_mob_task_del(ce_mob_task* mob_task);

    extern struct ce_mob_loader {
        size_t completed_job_count;
        size_t queued_job_count;
        ce_vector* mob_tasks;
    }* ce_mob_loader;

    void ce_mob_loader_init(void);
    void ce_mob_loader_term(void);

    void ce_mob_loader_load_mob(const std::string& name);
}

#endif
