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
#include "editor.hpp"

#include <cassert>
using namespace std;

Editor::Editor( void )
 : scroll_x(0), scroll_y(0), drag_snap_x(0), drag_snap_y(0)
{
	font.load("FONT");
}

bool Editor::load( int n )
{
	if (level.load(n) == false ||
	    tribe.load(level.tribe) == false ||
	    style.load(level.style) == false)
	{
		return false;
	}
	
	return redraw = true;
}

bool Editor::select( signed int x, signed int y, bool modify_selection )
{
	Level::Object::Index temp = level.get_object_by_position(scroll_x + x, scroll_y + y, style);
	
	if (temp.i == -1)  // selected nothing
	{
		selection.clear();
	}
	else
	{
		bool already_selected = selection.find(temp) != selection.end();
		
		if (!modify_selection && !already_selected)
			selection.clear();
		if (modify_selection && already_selected)
			selection.erase(temp);
		else
			selection.insert(temp);
	}
	
	return redraw = true;
}

bool Editor::select_none( void )
{
	if (selection.empty())
		return false;
	
	selection.clear();
	
	return redraw = true;
}

bool Editor::select_all( void )
{
	for (unsigned int type = 0; type < COUNTOF(level.object); ++type)
	{
		for (unsigned int i = 0; i < level.object[type].size(); ++i)
		{
			selection.insert(Level::Object::Index(type, i));
		}
	}
	
	return redraw = !selection.empty();
}

bool Editor::copy_selected( void )
{
	clipboard.clear();
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		clipboard.push_back(pair<Level::Object::Index, Level::Object>(*i, level.object[i->type][i->i]));
	}
	
	return !clipboard.empty();
}

bool Editor::paste( void )
{
	selection.clear(); // maybe delete selection instead?
	
	for (Clipboard::const_iterator i = clipboard.begin(); i != clipboard.end(); ++i)
	{
		const Level::Object::Index &index = i->first;
		
		level.object[index.type].push_back(i->second);
		
		selection.insert(Level::Object::Index(index.type, level.object[index.type].size() - 1));
	}
	
	return redraw = !clipboard.empty();
}

bool Editor::delete_selected( void )
{
	if (selection.empty())
		return false;
	
	for (Selection::const_reverse_iterator i = selection.rbegin(); i != selection.rend(); ++i)
		level.object[i->type].erase(level.object[i->type].begin() + i->i);
	
	selection.clear();
	
	return redraw = true;
}

bool Editor::move_selected( signed int delta_x, signed int delta_y )
{
	drag_snap_x += delta_x;
	drag_snap_y += delta_y;
	
	delta_x = snap(drag_snap_x, 8);
	delta_y = snap(drag_snap_y, 2);
	
	if (selection.empty() || (delta_x == 0 && delta_y == 0))
		return false;
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		o.x += delta_x;
		o.y += delta_y;
	}
	
	return redraw = true;
}

bool Editor::move_selected_z( signed int delta_z )
{
	if (selection.empty())
		return false;
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		assert(false); // TODO
		(void)delta_z;
	}
	
	return redraw = true;
}

bool Editor::scroll( signed int delta_x, signed int delta_y, bool drag )
{
	signed int old_scroll_x = scroll_x, old_scroll_y = scroll_y;
	
	scroll_x = BETWEEN(0, scroll_x + delta_x, level.width);
	delta_x = scroll_x - old_scroll_x;
	
	scroll_y = BETWEEN(0, scroll_y + delta_y, level.height);
	delta_y = scroll_y - old_scroll_y;
	
	if (delta_x != 0 || delta_y != 0)
	{
		if (drag)
			move_selected(delta_x, delta_y);
		
		return redraw = true;
	}
	
	return false;
}

void Editor::draw( SDL_Surface *surface )
{
	{
		SDL_SetColors(surface, tribe.palette, 0, 32);
		SDL_SetColors(surface, style.palette, 32, 208);
	}
	
	if (redraw)
	{
		SDL_FillRect(surface, NULL, 0x0);
		
		level.draw(surface, scroll_x, scroll_y, style);
		
		redraw = false;
	}
	
	static unsigned int selection_dash = 0;
	++selection_dash;
	
	for (Selection::const_iterator i = selection.begin(); i != selection.end(); ++i)
	{
		Level::Object &o = level.object[i->type][i->i];
		int so = style.object_by_id(i->type, o.id);
		box_dashed(surface, -scroll_x + o.x, -scroll_y + o.y, style.object[i->type][so].width * 8, style.object[i->type][so].height * 2, selection_dash);
	}
}

signed int Editor::snap( signed int &value, unsigned int snap )
{
	signed int delta = value - (value % snap);
	value %= snap;
	
	return delta;
}

