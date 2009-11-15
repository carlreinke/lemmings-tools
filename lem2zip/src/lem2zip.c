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
#include "lem2zip.h"
#include "compress.h"
#include "decompress.h"

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage( void );
void version( void );
void help( void );

const char *prog_name = "lem2zip",
           *prog_ver = "0.2.0",
           *prog_date = "2008 Sept 14";

bool decompress, test;

struct option longopts[] =
{
    { "decompress", 0, NULL, 'd' }, /* decompress */
    { "uncompress", 0, NULL, 'd' }, /* decompress */
    { "help",       0, NULL, 'h' }, /* give help */
    { "license",    0, NULL, 'L' }, /* display software license */
    { "test",       0, NULL, 't' }, /* test compressed file integrity */
    { "version",    0, NULL, 'V' }, /* display version number */
    { NULL, 0, NULL, 0 }
};

int main( int argc, char *argv[] )
{
	int optc;
	while ((optc = getopt_long(argc, argv, "dhLtV", longopts, NULL)) != -1) {
		switch (optc) {
		case 'd':
			decompress = true; break;
		case 'h':
			help(); return EXIT_SUCCESS;
		case 'L':
			version(); return EXIT_SUCCESS;
		case 't':
			test = decompress = true; break;
		case 'V':
			version(); return EXIT_SUCCESS;
		default:
			/* error message already emitted by getopt_long. */
			usage();
			return EXIT_FAILURE;
		}
	}
	
	if (optind == argc)
		usage();
	
	for (int i = optind; i < argc; i++) {
		FILE *fi = fopen(argv[i], "rb"),
		     *fo = NULL;
		
		char temp_name[] = "lem2zip-XXXXXX";
		if (!test) {
			if ((fo = fopen(mktemp(temp_name), "wb")) == NULL) {
				fprintf(stderr, "%s: could not create temporary file\n", prog_name);
				continue;
			}
		}
		
		if (fi && (test || fo)) {
			static Uint8 d_buffer[CHUNK_SIZE]; // decompressed chunk
			Uint32 d_buffer_size;
			Uint32 d_size, d_size_test = 0;
			
			if (decompress) {
				
				if (!read_header(fi, &d_size)) {
					fprintf(stderr, "%s: %s: not in lem2zip format\n", prog_name, argv[i]);
					goto clean_continue;
				}
				
				while (!feof(fi)) {
					chunk_type chunk = { 0 };
					
					if (!read_chunk(fi, &chunk)) {
						free(chunk.ref);
						free(chunk.data);
						break;
					}
					
					if (!decompress_chunk(&chunk, d_buffer, &d_buffer_size)) {
						fprintf(stderr, "%s: %s: invalid compressed data\n", prog_name, argv[i]);
						goto clean_continue;
					}
					
					d_size_test += d_buffer_size;
					if (!test && fwrite(d_buffer, 1, d_buffer_size, fo) == 0) {
						fprintf(stderr, "%s: %s: disk write failed\n", prog_name, temp_name);
						goto clean_continue;
					}
					
					free(chunk.ref);
					free(chunk.data);
					
					if (chunk.flags)
						break;
				}
				
				if (d_size_test != d_size) {
					fprintf(stderr, "%s: %s: unexpected end of file\n", prog_name, argv[i]);
					goto clean_continue;
				} else if (!feof(fi)) {
					int temp = ftell(fi);
					fseek(fi, 0, SEEK_END);
					if (temp - ftell(fi) > 0)
						fprintf(stderr, "%s: %s: decompression okay, trailing garbage ignored\n", prog_name, argv[i]);
				}
				
			} else {
				fseek(fi, 0, SEEK_END);
				d_size = ftell(fi);
				fseek(fi, 0, SEEK_SET);
				
				write_header(fo, d_size);
				
				while (!feof(fi)) {
					chunk_type chunk = { 0 };
					
					d_buffer_size = d_size > CHUNK_SIZE ? CHUNK_SIZE : d_size;
					
					if (fread(d_buffer, d_buffer_size, 1, fi) == 0) {
						fprintf(stderr, "%s: %s: disk read failed\n", prog_name, argv[i]);
						goto clean_continue;
					}
					
					if (!compress_chunk(&chunk, d_buffer, d_buffer_size)) {
						fprintf(stderr, "%s: %s: compression failed (this should never happen)\n", prog_name, argv[i]);
						goto clean_continue;
					}
					
					d_size -= d_buffer_size;
					
					chunk.flags = d_size == 0 ? 0xff : 0x00;
					
					if (!write_chunk(fo, &chunk)) {
						free(chunk.ref);
						free(chunk.data);
						break;
					}
					
					free(chunk.ref);
					free(chunk.data);
					
					assert(d_size >= 0);
					if (d_size == 0)
						break;
				}
				
				if (!feof(fi)) {
					int temp = ftell(fi);
					fseek(fi, 0, SEEK_END);
					if (temp - ftell(fi) > 0) {
						fprintf(stderr, "%s: %s: unexpected termination of compression\n", prog_name, argv[i]);
						goto clean_continue;
					}
				}
				
			}
			
			// success! replace the original
			if (!test) {
				fclose(fi); fi = NULL;
				fclose(fo); fo = NULL;
				
				unlink(argv[i]);
				if (rename(temp_name, argv[i]))
					fprintf(stderr, "%s: %s could not be overwritten\n", prog_name, argv[i]);
			}
		} else {
			fprintf(stderr, "%s: %s could not be %s\n", prog_name, !fi ? argv[i] : temp_name, !fi ? "read from" : "written to");
		}
		
clean_continue:
		if (fi) fclose(fi);
		if (fo) fclose(fo);
		
		// delete temp file on fail
		if (!test)
			unlink(temp_name);
		
	}
}

void usage( void )
{
    fprintf(stderr, "usage: %s [-dhLtV] [file ...]\n", prog_name);
}

void version( void )
{
	fprintf(stderr, "%s %s (%s)\n", prog_name, prog_ver, prog_date);
	fprintf(stderr, "Copyright (C) 2008 Carl Reinke\n");
	fprintf(stderr, "This is free software.  You may redistribute copies of it under the terms of\n"
	                "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
	                "There is NO WARRANTY, to the extent permitted by law.\n");
}

void help( void )
{
    static char  *help_msg[] =
	{
		" -d --decompress  decompress",
		" -h --help        give this help",
		" -L --license     display software license",
		" -t --test        test compressed file integrity",
		" -V --version     display version number",
		" file ...         files to (de)compress",
		0
	};
    char **temp = help_msg;

    fprintf(stderr, "%s %s (%s)\n", prog_name, prog_ver, prog_date);
    usage();
    while (*temp)
		fprintf(stderr, "%s\n", *temp++);
}

// kate: tab-width 4;
