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
#include "compress.h"
#include "decompress.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool write_header( FILE *f, Uint32 d_size )
{
	return (fwrite(valid_fourcc, 1, 4, f)
	     && fwrite(&d_size, sizeof(d_size), 1, f));
}

bool compress_chunk( chunk_type *chunk, Uint8 *data, Uint32 data_size )
{
	if (data_size > CHUNK_SIZE)
		return false;
	
	for (; ; ) {
		Uint16 byte_count[256] = { 0 };
		Uint16 byte_pair_count[256][256] = { { 0 } };
		
		for (int i = data_size - 1; i >= 0; i--) {
			byte_count[data[i]]++;
			if (i > 0)
				byte_pair_count[data[i - 1]][data[i]]++;
			if (i > 1 && data[i] == data[i - 1] && data[i - 1] == data[i - 2])
				byte_count[data[--i]]++; // skip 
		}
		
		Uint8 unused_byte;
		for (int i = 256 - 1; i >= 0; i--)
			if (byte_count[i] == 0)
				unused_byte = i;
		if (byte_count[unused_byte] != 0) // no unused bytes
			break;
		
		int count = 0;
		Uint8 most_used_byte_pair[2];
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
				if (byte_pair_count[i][j] > count) {
					count = byte_pair_count[i][j];
					most_used_byte_pair[0] = i;
					most_used_byte_pair[1] = j;
				}
		if (count <= 2) // not worth compressing
			break;
		
		int d = 0;
		for (int s = 0; s < data_size; s++) {
			if (s < data_size - 1
			 && data[s] == most_used_byte_pair[0]
			 && data[s + 1] == most_used_byte_pair[1]) {
				data[d++] = unused_byte;
				s++;
			} else {
				data[d++] = data[s];
			}
			assert(d < CHUNK_SIZE);
		}
		
		data_size -= count;
		assert(d == data_size);
		
		chunk->ref_count++;
		if ((chunk->ref = (struct ref_struct *)realloc((void *)chunk->ref, chunk->ref_count * sizeof(*chunk->ref))) == NULL)
			exit(EXIT_FAILURE);
		chunk->ref[chunk->ref_count - 1].byte = unused_byte;
		for (int i = 0; i < 2; i++)
			chunk->ref[chunk->ref_count - 1].byte_pair[i] = most_used_byte_pair[i];
	}
	
	chunk->size = data_size;
	if ((chunk->data = (Uint8 *)malloc(chunk->size)) == NULL)
		exit(EXIT_FAILURE);
	memcpy(chunk->data, data, chunk->size);
	
	return true;
}

bool write_chunk( FILE *f, const chunk_type *chunk )
{
	if (fwrite(&chunk->flags, sizeof(chunk->flags), 1, f) == 0
	 || fwrite(&chunk->ref_count, sizeof(chunk->ref_count), 1, f) == 0)
		return false;
	
	for (int i = 0; i < chunk->ref_count; i++)
		if (fwrite(&chunk->ref[i].byte, sizeof(chunk->ref->byte), 1, f) == 0)
			return false;
	for (int j = 0; j < 2; j++)
		for (int i = 0; i < chunk->ref_count; i++)
			if (fwrite(&chunk->ref[i].byte_pair[j], sizeof(*chunk->ref->byte_pair), 1, f) == 0)
				return false;
	
	if (fwrite(&chunk->size, sizeof(chunk->size), 1, f) == 0
	 || fwrite(chunk->data, 1, chunk->size, f) == 0)
		return false;
	
	return true;
}

// kate: tab-width 4;
