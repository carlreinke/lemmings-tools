#include <stdint.h>
#include <cstdlib>

namespace LemCrunch
{

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
	};
	
	Header    header;
	uint8_t * data;
	size_t    size;
	
	void compress( uint8_t * uncompressed_data, size_t uncompressed_data_len );
	
private:
	int       m_bit_count;
	uint8_t   m_checksum;
	
	size_t    m_csize;
	size_t    m_ci;
	uint8_t * m_cdata;
	
	size_t    m_dsize;
	size_t    m_di;
	uint8_t * m_ddata;
	
	void push_bits( size_t count, uint16_t data );
	size_t find_reference( size_t & length ) const;
	void encode_raw_data( size_t length );
};

}
