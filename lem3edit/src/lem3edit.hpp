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
#ifndef LEM3EDIT_HPP
#define LEM3EDIT_HPP

#include "SDL.h"
#include <string>

#define BETWEEN(min_, val, max_) (std::max((int)(min_), std::min((int)(max_), (int)(val))))

#define COUNTOF( x ) (sizeof(x) / sizeof(*(x)))

static const int PERM = 0, TEMP = 1;

std::string l3_filename( const std::string &path, const std::string &name, const std::string &ext );
std::string l3_filename( const std::string &path, const std::string &name, int n, const std::string &ext);

void box_dashed( SDL_Surface *dest, int x, int y, int width, int height, int dash );

#endif // LEM3EDIT_HPP
