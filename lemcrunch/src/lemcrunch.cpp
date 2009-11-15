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
#include "lemcrunch.hpp"

using namespace std;

#if BYTE_ORDER == BIG_ENDIAN
#define be_swap32(x) (x)
#else
#define be_swap32(x) (swap32(x))
#endif

static inline uint32_t swap32( uint32_t x )
{
	return ((x<<24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x>>24));
}

namespace LemCrunch
{

static bool debug = false;

uint32_t Section::Header::compressed_size( void ) const
{
	return be_swap32(be_compressed_size) - 10;
}

void Section::compress( uint8_t * uncompressed_data, size_t uncompressed_data_len )
{
	m_bit_count = 0;
	m_checksum = 0;
	
	m_dsize = uncompressed_data_len;
	m_di = 0;
	m_csize = uncompressed_data_len;  // very rough guess, buffer will grow if necessary
	m_ci = 0;
	m_ddata = uncompressed_data;
	m_cdata = static_cast<uint8_t *>(malloc(m_csize));
	
	size_t raw = 0;
	
	while (m_di < m_dsize)
	{
		size_t ref, length;
		
		if ((ref = find_reference(length)) > 0)
		{
			if (raw > 0)
			{
				encode_raw_data(raw);
				raw = 0;
			}
			
			if (length > 4)
			{
				push_bits(12, ref - 1);
				push_bits(8, length - 1);
				push_bits(3, 6); // 110
			}
			else if (length == 4)
			{
				push_bits(10, ref - 1);
				push_bits(3, 5); // 101
			}
			else if (length == 3)
			{
				push_bits(9, ref - 1);
				push_bits(3, 4); // 100
			}
			else if (length == 2)
			{
				push_bits(8, ref - 1);
				push_bits(2, 1); // 01
			}
			
			debug && clog << "ref-" << ref << "-" << length << endl;
			
			m_di += length;
		}
		else
		{
			push_bits(8, m_ddata[m_di]);
			
			if (++raw == 264)
			{
				encode_raw_data(raw);
				raw = 0;
			}
			
			m_di++;
		}
	}
	encode_raw_data(raw);  // flush remaining raw bits
	
	// handle last partial byte
	m_checksum ^= m_cdata[m_ci];
	++m_ci;
	
	m_csize = m_ci;
	
	header.initial_bit_count = m_bit_count;
	header.checksum = m_checksum;
	header.be_decompressed_size = be_swap32(m_dsize);
	header.be_compressed_size = be_swap32(m_csize + 10);
	
	data = m_cdata;
	size = m_csize;
}

size_t Section::find_reference( size_t & length ) const
{
	const size_t max_length = (1 << 8) + 1;
	const size_t max_offset = (1 << 12) + 1;
	
	length = 4;  // throw away short references
	size_t offset = 0;
	
	size_t short_offset[3] = { 0 };
	
	for (size_t i = m_di + 1; i < m_di + max_offset; ++i)
	{
		size_t temp_len = 0;
		
		for (; temp_len < max_length && i + temp_len < m_dsize; ++temp_len)
		{
			// record short references
			if (temp_len >= 2 && temp_len <= 4)
				if (short_offset[temp_len - 2] == 0)
					short_offset[temp_len - 2] = i - m_di;
			
			if (m_ddata[m_di + temp_len] != m_ddata[i + temp_len])
				break;
		}
		if (temp_len == max_length)
			temp_len--;
		
		// largest reference so far? use it
		if (temp_len > length)
		{
			length = temp_len;
			offset = i - m_di;
		}
	}
	
	assert(length < max_length);
	assert(offset < max_offset);
	
	// no long references? try short
	if (offset == 0)
	{
		for (int i = 2; i >= 0; --i)
		{
			const size_t max_short_offset = (1 << (i + 8)) + 1;
			
			if (short_offset[i] > 0 && short_offset[i] < max_short_offset)
			{
				length = i + 2;
				offset = short_offset[i];
				break;
			}
		}
	}
	
	return offset;
}

void Section::encode_raw_data( size_t length )
{
	assert(length <= 255 + 9);
	
	if (length > 8)
	{
		push_bits(8, length - 9);
		push_bits(3, 7);  // 111
	}
	else if (length > 0)
	{
		push_bits(3, length - 1);
		push_bits(2, 0);  // 00
	}
	
	debug && clog << "raw-" << length << endl;
}

void Section::push_bits( unsigned int bit_count, uint16_t data )
{
	for (; bit_count > 0; --bit_count)
	{
		if (++m_bit_count > 8)
		{
			m_checksum ^= m_cdata[m_ci];  // keep a running checksum
			
			if (++m_ci >= m_csize)  // need to get more memory
			{
				m_csize += 100;
				m_cdata = static_cast<uint8_t *>(realloc(m_cdata, m_csize));
			}
			
			m_cdata[m_ci] = 0;  // not critical, but makes compression deterministic ;)
			
			m_bit_count = 1;
		}
		
		debug && clog << (data & 1);
		
		m_cdata[m_ci] <<= 1;
		m_cdata[m_ci] |= data & 1;
		data >>= 1;
	}
	
	debug && clog << " ";
}

void Section::decompress( uint8_t * compressed_data )
{
	m_bit_count = header.initial_bit_count;
	m_checksum = header.checksum;
	
	m_csize = m_ci = header.compressed_size();
	m_dsize = m_di = be_swap32(header.be_decompressed_size);
	m_cdata = compressed_data;
	m_ddata = static_cast<uint8_t *>(malloc(m_dsize));
	
	size = m_dsize;
	data = m_ddata;
	
	if (m_dsize == 0)
		return;
	
	if (m_ci == 0)
		throw DECOMPRESSION_ERROR;
	
	--m_ci;
	m_bits = m_cdata[m_ci];
	
	// handle last partial byte
	m_checksum ^= m_cdata[m_ci];
	
	do
	{
		if (get_bits(1) == 0)
		{
			switch (get_bits(1))
			{
			case 0:  // 00
				decode_raw_data(get_bits(3) + 1);
				break;
			case 1:  // 01
				dereference_data(2, 8);
				break;
			}
		}
		else
		{
			switch (get_bits(2))
			{
			case 0:  // 100
				dereference_data(3, 9);
				break;
			case 1:  // 101
				dereference_data(4, 10);
				break;
			case 2:  // 110
				dereference_data(get_bits(8) + 1, 12);
				break;
			case 3:  // 111
				decode_raw_data(get_bits(8) + 9);
				break;
			}
		}
	}
	while (m_di > 0);
	
	if (m_checksum != 0x00)
		throw CHECKSUM_MISMATCH;
}

uint16_t Section::get_bits( unsigned int bit_count )
{
	uint16_t result = 0;
	
	for (; bit_count > 0; --bit_count)
	{
		if (m_bit_count-- == 0)
		{
			if (m_ci == 0)
				throw DECOMPRESSION_ERROR;
			
			--m_ci;
			m_bits = m_cdata[m_ci];
			
			m_checksum ^= m_bits;  // keep a running checksum
			
			m_bit_count = 7;
		}
		
		debug && clog << (m_bits & 1);
		
		result <<= 1;
		result |= (m_bits & 1);
		m_bits >>= 1;
	}
	
	debug && clog << " ";
	
	return result;
}

void Section::dereference_data( size_t length, unsigned int offset_bit_count )
{
	size_t offset = get_bits(offset_bit_count) + 1;
	
	if (m_di - 1 + offset >= m_dsize && m_di < length)
		throw DECOMPRESSION_ERROR;
	
	debug && clog << "ref-" << offset << "-" << length << endl;
	
	for (; length > 0; --length)
	{
		--m_di;
		m_ddata[m_di] = m_ddata[m_di + offset];
	}
}

void Section::decode_raw_data( size_t length )
{
	if (m_di < length)
		throw DECOMPRESSION_ERROR;
	
	const size_t debug_length = length;
	
	for (; length > 0; --length)
	{
		--m_di;
		m_ddata[m_di] = get_bits(8);
	}
	
	debug && clog << "raw-" << debug_length << endl;
}

}
