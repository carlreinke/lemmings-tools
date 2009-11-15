/*
 * lem2zip
 * Copyright (C) 2007-2008 Carl Reinke
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "lem2zip.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct {
	Uint8 flags;
	Uint16 ref_count;
	struct ref_struct {
		Uint8 byte;
		Uint8 byte_pair[2];
	} *ref;
	Uint16 size;
	Uint8 *data;
} chunk_type;

bool read_header( FILE *f, Uint32 *d_size );
bool read_chunk( FILE *f, chunk_type *chunk );
bool decompress_chunk( const chunk_type *chunk, Uint8 *data, Uint32 *data_size );

extern const Uint8 valid_fourcc[4];
#define CHUNK_SIZE 2048

#endif // DECOMPRESS_H

// kate: tab-width 4;
