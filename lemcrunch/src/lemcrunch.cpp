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
			debug && clog << raw << "-" << ref << "-" << length << (length > 4 ? 'l' : 's') << endl;
			
			encode_raw_chunk(raw);
			raw = 0;
			
			if (length > 4)
			{
				push_next_bits(12, ref - 1);
				push_next_bits(8, length - 1);
				push_next_bits(3, 6); // 110
			}
			else if (length == 4)
			{
				push_next_bits(10, ref - 1);
				push_next_bits(3, 5); // 101
			}
			else if (length == 3)
			{
				push_next_bits(9, ref - 1);
				push_next_bits(3, 4); // 100
			}
			else if (length == 2)
			{
				push_next_bits(8, ref - 1);
				push_next_bits(2, 1); // 01
			}
			
			m_di += length;
		}
		else
		{
			push_next_bits(8, m_ddata[m_di]);
			
			if (++raw == 264)
			{
				debug && clog << raw << endl;
				
				encode_raw_chunk(raw);
				raw = 0;
			}
			
			m_di++;
		}
	}
	encode_raw_chunk(raw);  // flush remaining raw bits
	
	if (m_bit_count > 0)  // handle last partial byte
	{
		m_checksum ^= m_cdata[m_ci];
		++m_ci;
	}
	
	m_csize = m_ci;
	
	header.initial_bit_count = m_bit_count;
	header.checksum = m_checksum;
	header.be_decompressed_size = be_swap32(m_dsize);
	header.be_compressed_size = be_swap32(m_csize + 10);
	
	data = m_cdata;
	len = m_csize;
}

size_t Section::find_reference( size_t & length ) const
{
	const size_t max_len = (1 << 8) + 1;
	const size_t max_dist = (1 << 12) + 1;
	
	length = 4;  // throw away short references
	size_t distance = 0;
	
	size_t short_distance[3] = { 0 };
	
	for (size_t i = m_di + 1; i < m_di + max_dist; ++i)
	{
		size_t temp_len = 0;
		
		for (; temp_len < max_len && i + temp_len < m_dsize; ++temp_len)
		{
			// record short references
			if (temp_len >= 2 && temp_len <= 4)
				if (short_distance[temp_len - 2] == 0)
					short_distance[temp_len - 2] = i - m_di;
			
			if (m_ddata[m_di + temp_len] != m_ddata[i + temp_len])
				break;
		}
		if (temp_len == max_len)
			temp_len--;
		
		// largest reference so far? use it
		if (temp_len > length)
		{
			length = temp_len;
			distance = i - m_di;
		}
	}
	
	assert(length < max_len);
	assert(distance < max_dist);
	
	// no long references? try short
	if (distance == 0)
	{
		for (int i = 2; i >= 0; --i)
		{
			const size_t max_short_distance = (1 << (i + 8)) + 1;
			
			if (short_distance[i] > 0 && short_distance[i] < max_short_distance)
			{
				length = i + 2;
				distance = short_distance[i];
				break;
			}
		}
	}
	
	return distance;
}

void Section::encode_raw_chunk( size_t length )
{
	assert(length <= 255 + 9);
	
	if (length > 8)
	{
		push_next_bits(8, length - 9);
		push_next_bits(3, 7);  // 111
	}
	else if (length > 0)
	{
		push_next_bits(3, length - 1);
		push_next_bits(2, 0);  // 00
	}
}

void Section::push_next_bits( size_t count, uint16_t data )
{
	for (; count > 0; --count)
	{
		debug &&  printf("%d", data & 1);
		
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
		
		m_cdata[m_ci] <<= 1;
		m_cdata[m_ci] |= data & 1;
		data >>= 1;
	}
	
	debug && printf(" ");
}

}
