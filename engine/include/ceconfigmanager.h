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

#ifndef CE_CONFIGMANAGER_H
#define CE_CONFIGMANAGER_H

#include "cevector.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CE_CONFIG_MOVIE_START,
	CE_CONFIG_MOVIE_CRDTFIN,
	CE_CONFIG_MOVIE_CRDTFOUT,
	CE_CONFIG_MOVIE_TITLESFIN,
	CE_CONFIG_MOVIE_TITLESFOUT,
	CE_CONFIG_MOVIE_COUNT
};

extern struct ce_config_manager {
	ce_vector* movies[CE_CONFIG_MOVIE_COUNT];
}* ce_config_manager;

extern void ce_config_manager_init(void);
extern void ce_config_manager_term(void);

#ifdef __cplusplus
}
#endif

#endif /* CE_CONFIGMANAGER_H */
