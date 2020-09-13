/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2015 Emma Krantz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef _MSC_VER
	#define PACKED(structname) __pragma(pack()) \
		typedef struct structname
	#define PACKED_notypedef(structname) __pragma(pack()) \
		struct structname
#else
	#define PACKED(structname) typedef struct  __attribute__((__packed__)) structname
	#define PACKED_notypedef(structname) struct __attribute__((__packed__)) structname
#endif
