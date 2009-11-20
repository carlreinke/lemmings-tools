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
#include "lem3edit.hpp"
#include "level.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace std;

void Level::draw( SDL_Surface *surface, signed int x, signed int y, const Style &style ) const
{
	draw_objects(surface, x, y, PERM, 0, style);
	draw_objects(surface, x, y, TEMP, 0, style);
	draw_objects(surface, x, y, PERM, 5000, style);
}

void Level::draw_objects( SDL_Surface *surface, signed int x, signed int y, int type, unsigned int id_min, const Style &style ) const
{
	assert((unsigned)type < COUNTOF(this->object));
	
	for (vector<Object>::const_iterator i = object[type].begin(); i != object[type].end(); ++i)
	{
		const Object &o = *i;
		if (o.id < id_min)
			continue;
			
		int so = style.object_by_id(type, o.id);
		if (so == -1)
			continue;
		
		style.blit_object(surface, o.x - x, o.y - y, type, so, 0);
		if (style.object[type][so].frame.size() > 1)
			style.blit_object(surface, o.x - x, o.y - y, type, so, 1);
	}
}

Level::Object::Index Level::get_object_by_position( signed int x, signed int y, const Style &style ) const
{
	signed int i;
	
	i = get_object_by_position(x, y, PERM, 5000, style);
	if (i != -1)
		return Object::Index(PERM, i);
	
	i = get_object_by_position(x, y, TEMP, 0, style);
	if (i != -1)
		return Object::Index(TEMP, i);
	
	i = get_object_by_position(x, y, PERM, 0, style);
	if (i != -1)
		return Object::Index(PERM, i);
	
	return Object::Index(0, -1);
}

signed int Level::get_object_by_position( signed int x, signed int y, int type, unsigned int id_min, const Style &style ) const
{
	assert((unsigned)type < COUNTOF(this->object));
	
	for (vector<Object>::const_reverse_iterator i = object[type].rbegin(); i != object[type].rend(); ++i)
	{
		const Object &o = *i;
		if (o.id < id_min)
			continue;
		
		if (x < o.x || y < o.y)
			continue;
		
		int so = style.object_by_id(type, o.id);
		if (so == -1)
			continue;
		
		if (x < o.x + style.object[type][so].width * 8 && y < o.y + style.object[type][so].height * 2)
			return object[type].rend() - i - 1; // index
	}
	
	return -1;
}

bool Level::load( unsigned int n )
{
	const string path = "LEVELS/";
	const string level = "LEVEL";
	const string perm = "PERM", temp = "TEMP";
	
	return load_level(path, level, n) &&
	       load_objects(PERM, path, perm, n) &&
	       load_objects(TEMP, path, temp, n);
}

bool Level::load_level( const std::string &path, const std::string &name, unsigned int n )
{
	const string dat = ".DAT";
	
	return load_level(l3_filename(path, name, n, dat));
}

bool Level::load_level( const string &filename )
{
	ifstream f(filename.c_str(), ios::binary);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
		return false;
	}
	
	f.read((char *)&tribe,          sizeof(tribe));
	f.read((char *)&cave_map,       sizeof(cave_map));
	f.read((char *)&cave_raw,       sizeof(cave_raw));
	f.read((char *)&temp,           sizeof(temp));
	f.read((char *)&perm,           sizeof(perm));
	f.read((char *)&style,          sizeof(style));
	f.read((char *)&width,          sizeof(width));
	f.read((char *)&height,         sizeof(height));
	f.read((char *)&x,              sizeof(x));
	f.read((char *)&y,              sizeof(y));
	f.read((char *)&time,           sizeof(time));
	f.read((char *)&extra_lemmings, sizeof(extra_lemmings));
	f.read((char *)&unknown,        sizeof(unknown));
	f.read((char *)&release_rate,   sizeof(release_rate));
	f.read((char *)&release_delay,  sizeof(release_delay));
	f.read((char *)&enemies,        sizeof(enemies));
	
	cout << "loaded level from '" << filename << "'" << endl;
	return true;
}

bool Level::load_objects( int type, const string &path, const string &name, unsigned int n )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	const string obs = ".OBS";
	
	return load_objects(type, l3_filename(path, name, n, obs));
}

bool Level::load_objects( int type, const string &filename )
{
	assert((unsigned)type < COUNTOF(this->object));
	
	object[type].clear();
	
	ifstream f(filename.c_str(), ios::binary);
	if (!f)
	{
		cerr << "failed to open '" << filename << "'" << endl;
		return false;
	}
	
	while (true)
	{
		Object o;
		
		f.read((char *)&o.id, sizeof(o.id));
		f.read((char *)&o.x,  sizeof(o.x));
		f.read((char *)&o.y,  sizeof(o.y));
		
		if (!f)
			break;
		
		object[type].push_back(o);
	}
	
	cout << "loaded " << object[type].size() << " objects from '" << filename << "'" << endl;
	return true;
}
