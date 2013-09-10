/*
 * lem3edit
 * Copyright (C) 2008-2009 Carl Reinke
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
#ifndef STYLE_HPP
#define STYLE_HPP

#include "cmp.hpp"
#include "lem3edit.hpp"

#include "SDL.h"

#include <string>
#include <vector>

class Style
{
public:
	class Object
	{
	public:
		Uint16 id;
		Uint8 width, height;
		
		Uint16 frl, unknown[4];
		
		std::vector<Uint16 *> frame;
		
		Object( void ) { /* nothing to do */ }
		Object( const Object &that ) { copy(that); }
		~Object( void ) { destroy(); }
		
		void copy( const Object & );
		void destroy( void );
		Object & operator=( const Object & );
	};
	
	class Block
	{
	public:
		Uint8 data[16];
		
		void blit( SDL_Surface *dest, signed int x, signed int y ) const;
		
		Block( Uint8 encoded_data[16] );
		
		static void decode( Uint8 *dest, const Uint8 *src, unsigned int size );
	};
	
	std::vector<Object> object[2];
	
	std::vector<Block> block[2];
	
	Cmp skill;
	
	SDL_Color palette[208];
	
	signed int object_by_id( int type, unsigned int id ) const;
	
	void blit_object( SDL_Surface *dest, signed int x, signed int y, int type, unsigned int object, unsigned int frame ) const;
	
	bool load( unsigned int n );
	bool load_palette( std::string path, std::string name, unsigned int n );
	bool load_palette( std::string pal_filename );
	bool load_objects( int type, const std::string &path, const std::string &name, unsigned int n);
	bool load_objects( int type, const std::string &obj_filename, const std::string &frl_filename );
	bool load_blocks( int type, const std::string &path, const std::string &name, unsigned int n );
	bool load_blocks( int type, const std::string &blk_filename );
	
	Style( void ) { /* nothing to do */ }
	
private:
	Style( const Style & );
	Style & operator=( const Style & );
};

#endif // STYLE_HPP
