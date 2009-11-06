#include "lemcrunch.hpp"

using namespace std;

int main( int argc, char * argv[] )
{
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " DATFILE FILE..." << endl;
		cout << "Compress FILEs into DATFILE." << endl << endl;
		cout << "http://lemmings-tools.googlecode.com/" << endl;
		exit(EXIT_FAILURE);
	}
	
	FILE *datf = fopen(argv[1], "wb");
	if (!datf)
	{
		cerr << argv[0] << ": failed to open '" << argv[1] << "'" << endl;
		exit(EXIT_FAILURE);
	}
	
	for (int i = 2; i < argc; ++i)
	{
		FILE* f = fopen(argv[i], "rb");
		if (!f)
		{
			cerr << argv[0] << ": failed to open '" << argv[i] << "'" << endl;
			continue;
		}
		
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		uint8_t *data = static_cast<uint8_t *>(malloc(size));
		fread(data, 1, size, f);
		
		fclose(f);
		
		LemCrunch::Section section;
		section.compress(data, size);
		
		free(data);
		
		cout << "section " << (i - 2) << ": " << argv[i] << endl
		     << "\tchecksum: 0x" << hex << setw(2) << setfill('0') << (int)section.header.checksum << dec << endl
		     << "\tcompressed size: " << section.size << " B (" << (size > 0 ? section.size * 100 / size : 0) << "%)" << endl;
		
		fwrite(&section.header.initial_bit_count,    sizeof(uint8_t),  1, datf);
		fwrite(&section.header.checksum,             sizeof(uint8_t),  1, datf);
		fwrite(&section.header.be_decompressed_size, sizeof(uint32_t), 1, datf);
		fwrite(&section.header.be_compressed_size,   sizeof(uint32_t), 1, datf);
		
		fwrite(section.data, 1, section.size, datf);
	}
	
	fclose(datf);
	
	return EXIT_SUCCESS;
}
