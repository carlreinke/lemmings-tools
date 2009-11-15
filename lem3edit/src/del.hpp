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
#ifndef DEL_HPP
#define DEL_HPP

#include "SDL.h"
#include <string>
#include <vector>

class Del
{
public:
	class Frame
	{
	public:
		Uint16 size;
		Uint8 *frame;
		
		void blit( SDL_Surface *surface, signed int x, signed int y, unsigned int width, unsigned int height ) const;
		
		Frame( unsigned int size ) : size(size), frame(new Uint8[size]) { /* nothing to do */ }
		Frame( const Frame &that ) { copy(that); }
		~Frame() { destroy(); }
		
		void copy( const Frame & );
		void destroy( void );
		Frame & operator=( const Frame & );
	};
	
	std::vector<Frame> frame;
	
	void blit( SDL_Surface *surface, signed int x, signed int y, unsigned int frame, unsigned int width, unsigned int height ) const;
	void blit_text( SDL_Surface *surface, signed int x, signed int y, const std::string &text ) const;
	
	bool load( const std::string &name );
	bool load( const std::string &path, const std::string &name, unsigned int n );
	bool load( const std::string &din_filename, const std::string &del_filename );
	
	Del() {}
	
private:
	Del( const Del & );
	Del & operator=( const Del & );
};

#endif // DEL_HPP
