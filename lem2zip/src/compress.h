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
#ifndef COMPRESS_H
#define COMPRESS_H

#include "lem2zip.h"
#include "decompress.h"

#include <stdbool.h>
#include <stdio.h>

bool write_header( FILE *f, Uint32 d_size );
bool compress_chunk( chunk_type *chunk, Uint8 *data, Uint32 data_size );
bool write_chunk( FILE *f, const chunk_type *chunk );

#endif // COMPRESS_H

// kate: tab-width 4;
