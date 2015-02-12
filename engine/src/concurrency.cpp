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

#include "alloc.hpp"
#include "concurrency.hpp"

namespace cursedearth
{
    semaphore_t* ce_semaphore_new(size_t n)
    {
        semaphore_t* semaphore = (semaphore_t*)ce_alloc_zero(sizeof(semaphore_t));
        semaphore->available = n;
        semaphore->mutex = ce_mutex_new();
        semaphore->wait_condition = ce_wait_condition_new();
        return semaphore;
    }

    void ce_semaphore_del(semaphore_t* semaphore)
    {
        if (NULL != semaphore) {
            ce_wait_condition_del(semaphore->wait_condition);
            ce_mutex_del(semaphore->mutex);
            ce_free(semaphore, sizeof(semaphore_t));
        }
    }

    size_t ce_semaphore_available(const semaphore_t* semaphore)
    {
        ce_mutex_lock(semaphore->mutex);
        size_t n = semaphore->available;
        ce_mutex_unlock(semaphore->mutex);
        return n;
    }

    void ce_semaphore_acquire(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        while (n > semaphore->available) {
            ce_wait_condition_wait(semaphore->wait_condition, semaphore->mutex);
        }
        semaphore->available -= n;
        ce_mutex_unlock(semaphore->mutex);
    }

    void ce_semaphore_release(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        semaphore->available += n;
        ce_wait_condition_wake_all(semaphore->wait_condition);
        ce_mutex_unlock(semaphore->mutex);
    }

    bool ce_semaphore_try_acquire(semaphore_t* semaphore, size_t n)
    {
        ce_mutex_lock(semaphore->mutex);
        bool result = true;
        if (n > semaphore->available) {
            result = false;
        } else {
            semaphore->available -= n;
        }
        ce_mutex_unlock(semaphore->mutex);
        return result;
    }
}
