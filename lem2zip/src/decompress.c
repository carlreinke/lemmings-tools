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
#include "decompress.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const Uint8 valid_fourcc[4] = { 0x47, 0x53, 0x43, 0x4d };

bool read_header( FILE *f, Uint32 *d_size )
{
	Uint8 fourcc[4];
	
	return (fread(fourcc, 1, 4, f)
	     && memcmp(fourcc, valid_fourcc, 4) == 0
	     && fread(d_size, sizeof(*d_size), 1, f));
}

bool read_chunk( FILE *f, chunk_type *chunk )
{
	if (fread(&chunk->flags, sizeof(chunk->flags), 1, f) == 0
	 || fread(&chunk->ref_count, sizeof(chunk->ref_count), 1, f) == 0)
		return false;
	
	if ((chunk->ref = (struct ref_struct *)malloc(chunk->ref_count * sizeof(*chunk->ref))) == NULL)
		exit(EXIT_FAILURE);
	
	for (int i = 0; i < chunk->ref_count; i++)
		if (fread(&chunk->ref[i].byte, sizeof(chunk->ref->byte), 1, f) == 0)
			return false;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < chunk->ref_count; i++)
			if (fread(&chunk->ref[i].byte_pair[j], sizeof(*chunk->ref->byte_pair), 1, f) == 0)
				return false;
	
	if (fread(&chunk->size, sizeof(chunk->size), 1, f) == 0)
		return false;
	
	if ((chunk->data = (Uint8 *)malloc(chunk->size)) == NULL)
		exit(EXIT_FAILURE);
	
	if (fread(chunk->data, 1, chunk->size, f) == 0)
		return false;
	
	return true;
}

bool decompress_chunk( const chunk_type *chunk, Uint8 *data, Uint32 *data_size )
{
	*data_size = chunk->size;
	if (*data_size > CHUNK_SIZE)
		return false;
	
	memcpy(data, chunk->data, *data_size);
	
	for (int i = chunk->ref_count - 1; i >= 0; i--) {
		int count = 0;
		for (int j = *data_size - 1; j >= 0; j--)
			if (data[j] == chunk->ref[i].byte)
				count++;
		
		if (*data_size + count > CHUNK_SIZE)
			return false;
		
		int d = *data_size - 1 + count;
		for (int s = *data_size - 1; s >= 0; s--) {
			if (data[s] == chunk->ref[i].byte) {
				for (int j = 1; j >= 0; j--)
					data[d--] = chunk->ref[i].byte_pair[j];
			} else {
				data[d--] = data[s];
			}
			assert(d >= -1);
		}
		
		*data_size += count;
	}
	
	return true;
}

// kate: tab-width 4;
