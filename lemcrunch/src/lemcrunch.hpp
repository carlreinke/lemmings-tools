/*
 * lemcrunch
 * Copyright (C) 2006, 2009 Carl Reinke
 * Portions Copyright (C) 2004 ccexplore
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
#include <stdint.h>
#include <cstdlib>

namespace LemCrunch
{

enum Error
{
	DECOMPRESSION_ERROR,
	CHECKSUM_MISMATCH
};

class Section
{
public:
	Section( void ) : data(NULL), size(0) { /* nothing to do */ };
	~Section( void ) { free(data); };
	
	class Header
	{
	public:
		uint8_t  initial_bit_count;
		uint8_t  checksum;
		uint32_t be_decompressed_size;
		uint32_t be_compressed_size;  // includes the 10-byte header
		
		uint32_t compressed_size( void ) const;
	};
	
	Header    header;
	uint8_t * data;
	size_t    size;
	
	void compress( uint8_t * uncompressed_data, size_t uncompressed_data_len );
	void decompress( uint8_t * compressed_data );
	
private:
	int       m_bit_count;
	uint8_t   m_checksum;
	
	size_t    m_csize;
	size_t    m_ci;
	uint8_t * m_cdata;
	
	size_t    m_dsize;
	size_t    m_di;
	uint8_t * m_ddata;
	
	uint8_t   m_bits;
	
	void push_bits( unsigned int bit_count, uint16_t data );
	size_t find_reference( size_t & length ) const;
	void encode_raw_data( size_t length );
	
	uint16_t get_bits( unsigned int count );
	void dereference_data( size_t length, unsigned int offset_bit_count );
	void decode_raw_data( size_t length );
};

}
