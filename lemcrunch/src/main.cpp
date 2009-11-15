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
#include "arg_parse.hpp"
#include "lemcrunch.hpp"

using namespace std;

void help( char *argv[] )
{
	cout << "Usage: " << argv[0] << " [OPTION]... DATFILE FILE..." << endl << endl
	     << "Options:" << endl
	     << "  -d, --decrunch  Decrunch" << endl
	     << "  -h, --help      Show this help" << endl
	     << "  -V, --version   Display version number" << endl
	     << endl << "http://lemmings-tools.googlecode.com/" << endl;
}

void usage( char *argv[] )
{
	cout << "Usage: " << argv[0] << " [OPTION]... DATFILE FILE..." << endl
	     << "Crunches or decrunches FILEs into/from DATFILE." << endl
	     << endl << "http://lemmings-tools.googlecode.com/" << endl;
}

void version( void )
{
	cout << "lemcrunch 1.0.0" << endl
	     << "Copyright (C) 2006, 2009 Carl Reinke" << endl
	     << "Portions Copyright (C) 2004 ccexplore" << endl
	     << "This is free software.  You may redistribute copies of it under the terms of" << endl
	     << "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>." << endl
	     << "There is NO WARRANTY, to the extent permitted by law." << endl;
}

int main( int argc, char * argv[] )
{
	bool decrunch = false;
	
	const Options options[] =
	{
		{ 'd', 'd', "decrunch", false },
		{ 'h', 'h', "help",     false },
		{ 'V', 'V', "version",  false },
		{ 0, 0, NULL, false}
	};
	Option option = { 0, NULL, 0 };
	
	for (; ; )
	{
		option = parse_args(argc, (const char **)argv, options);
		
		if (option.value == NOT_OPTION)
			break;
		
		switch (option.value)
		{
		case INVALID_OPTION:
		case AMBIGUOUS_OPTION:
		case OPTION_MISSING_ARG:
			fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
			exit(EXIT_FAILURE);
			break;
			
		case 'd':
			decrunch = true;
			break;
			
		case 'h':
			help(argv);
			exit(EXIT_SUCCESS);
			break;
		case 'V':
			version();
			exit(EXIT_SUCCESS);
			break;
		}
	}
	
	int argi = option.argn;
	if (argc - argi < 2)
	{
		usage(argv);
		exit(EXIT_FAILURE);
	}
	
	FILE *datf = fopen(argv[argi++], decrunch ? "rb" : "wb");
	if (!datf)
	{
		cerr << argv[0] << ": failed to open '" << argv[1] << "'" << endl;
		exit(EXIT_FAILURE);
	}
	
	if (decrunch)
	{
		fseek(datf, 0, SEEK_END);
		long size = ftell(datf);
		fseek(datf, 0, SEEK_SET);
		
		for (; argi < argc && ftell(datf) < size; ++argi)
		{
			LemCrunch::Section section;
			
			fread(&section.header.initial_bit_count,    sizeof(uint8_t),  1, datf);
			fread(&section.header.checksum,             sizeof(uint8_t),  1, datf);
			fread(&section.header.be_decompressed_size, sizeof(uint32_t), 1, datf);
			fread(&section.header.be_compressed_size,   sizeof(uint32_t), 1, datf);
			
			size_t size = section.header.compressed_size();
			uint8_t *data = static_cast<uint8_t *>(malloc(size));
			fread(data, 1, size, datf);
			
			cout << "section: " << argv[argi] << endl
			     << "\tchecksum: 0x" << hex << setw(2) << setfill('0') << (int)section.header.checksum << dec << endl
			     << "\tcompressed size: " << section.header.compressed_size() << " B" << endl;
			
			try
			{
				section.decompress(data);
			}
			catch (LemCrunch::Error e)
			{
				switch (e)
				{
				case LemCrunch::CHECKSUM_MISMATCH:
					cerr << argv[0] << ": checksum mismatch" << endl;
					break;
				case LemCrunch::DECOMPRESSION_ERROR:
					cerr << argv[0] << ": decompression error" << endl;
					break;
				}
				exit(EXIT_FAILURE);
			}
			
			FILE *f = fopen(argv[argi], "wb");
			if (!f)
			{
				cerr << argv[0] << ": failed to open '" << argv[argi] << "'" << endl;
				continue;
			}
			
			fwrite(section.data, 1, section.size, f);
			
			fclose(f);
			
			free(data);
		}
	}
	else
		for (; argi < argc; ++argi)
		{
			FILE* f = fopen(argv[argi], "rb");
			if (!f)
			{
				cerr << argv[0] << ": failed to open '" << argv[argi] << "'" << endl;
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
			
			cout << "section: " << argv[argi] << endl
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
