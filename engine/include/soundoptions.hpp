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

#ifndef CE_SOUNDOPTIONS_HPP
#define CE_SOUNDOPTIONS_HPP

namespace cursedearth
{
    struct sound_options_t
    {
        static const size_t samples_in_block = 1024;
        static const size_t block_count = 16;
        static const size_t max_sample_size = 64;
        static const size_t max_block_size = max_sample_size * samples_in_block;
    };
}

#endif
