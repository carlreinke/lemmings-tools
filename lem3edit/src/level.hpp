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
#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "style.hpp"
#include "SDL.h"
#include <vector>
#include <string>

class Level
{
public:
	Uint16 tribe;
	Uint16 cave_map, cave_raw;
	Uint16 temp, perm;
	Uint16 style;
	Uint16 width, height;
	Uint16 x, y;
	Uint16 time;
	Uint8  extra_lemmings;
	Uint8  unknown;
	Uint16 release_rate, release_delay;
	Uint16 enemies;
	
	class Object
	{
	public:
		class Index
		{
		public:
			int type;
			signed int i;
			
			Index( int type, signed int i ) : type(type), i(i) { }
			
			inline bool operator<( const Index &that ) const { return this->type != that.type ? this->type < that.type : this->i < that.i; }
		};
		
		Uint16 id;
		Sint16 x, y;
	};
	
	std::vector<Object> object[2];
	
	void draw( SDL_Surface *surface, signed int x, signed int y, const Style &style ) const;
	void draw_objects( SDL_Surface *surface, signed int x, signed int y, int type, unsigned int id_min, const Style &style ) const;
	
	Object::Index get_object_by_position( signed int x, signed int y, const Style &style ) const;
	signed int get_object_by_position( signed int x, signed int y, int type, unsigned int id_min, const Style &style ) const;
	
	bool load( unsigned int n );
	bool load_level( const std::string &path, const std::string &name, unsigned int n );
	bool load_level( const std::string &filename );
	bool load_objects( int type, const std::string &path, const std::string &name, unsigned int n);
	bool load_objects( int type, const std::string &filename );
	
	bool validate(); // TODO remove objects with non-existant IDs, remove objects outside the level boundaries
	
	bool save( int n );
	
	Level() {}
	
private:
	Level( const Level & );
	Level & operator=( const Level & );
};

#endif // LEVEL_HPP
