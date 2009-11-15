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
#include "lem3edit.hpp"
#include "level.hpp"
#include "raw.hpp"
#include "style.hpp"
#include "tribe.hpp"

#include "SDL.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

const char *prog_name = "lem2zip",
           *prog_ver = "SVN",
           *prog_date = "no date";

SDL_Surface *display_surface = NULL;

bool video_init( int width, int height, bool fullscreen );

Uint32 loop_timer( Uint32 interval, void *param );
void die();

signed int snap( signed int &value, unsigned int snap );

void box_dashed( SDL_Surface *surface, int x, int y, int width, int height, int dash );

void version();

int main( int argc, char *argv[] )
{
	version();
	
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO))
	{
		cerr << "failed to initialize SDL: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}
	
	SDL_TimerID loop_timer_id = SDL_AddTimer((100.0 / 30.0) * 10.0, &loop_timer, NULL);
	
	if (video_init(640, 480, false) == false)
	{
		return EXIT_FAILURE;
	}
	
	SDL_LockSurface(display_surface);
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	Uint8 *key_state = SDL_GetKeyState(NULL);
	
	int level_id = (argc > 1) ? atoi(argv[1]) : 1;
	
	Editor editor;
	editor.load(level_id);
	
	SDL_Event event;
	while (SDL_WaitEvent(&event) && event.type != SDL_QUIT)
	{
		unsigned int delta_multiplier = (SDL_GetModState() & KMOD_SHIFT) ? 4 : 2;
		
		switch (event.type)
		{
			case SDL_MOUSEBUTTONDOWN:
			{
				SDL_MouseButtonEvent &e = event.button;
				bool ctrl_down = SDL_GetModState() & KMOD_CTRL;
				
				if (e.button == SDL_BUTTON_LEFT)
					editor.drag_snap_x = editor.drag_snap_y = 0;
				
				if (e.button == SDL_BUTTON_LEFT || e.button == SDL_BUTTON_RIGHT)
					editor.select(e.x, e.y, ctrl_down);
				
				break;
			}
			case SDL_MOUSEMOTION:
			{
				SDL_MouseMotionEvent &e = event.motion;
				
				if (e.state & SDL_BUTTON(SDL_BUTTON_LEFT))
					editor.move_selected(e.xrel, e.yrel);
				
				break;
			}
			case SDL_KEYDOWN:
			{
				SDL_KeyboardEvent &e = event.key;
				bool alt_down = e.keysym.mod & KMOD_ALT;
				
				Uint8 mouse_state = SDL_GetMouseState(NULL, NULL);
				
				switch (e.keysym.sym)
				{
					case SDLK_ESCAPE:
						editor.select_none();
						break;
					case SDLK_a:
						editor.select_all();
						break;
					case SDLK_c:
						editor.copy_selected();
						break;
					case SDLK_p:
						editor.paste();
						break;
					case SDLK_UP:
					case SDLK_DOWN:
						if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
							editor.move_selected(0, (e.keysym.sym == SDLK_UP ? -1 : e.keysym.sym == SDLK_DOWN ? 1 : 0) * delta_multiplier);
						break;
					case SDLK_LEFT:
					case SDLK_RIGHT:
						if (!mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT))
							editor.move_selected((e.keysym.sym == SDLK_LEFT ? -4 : e.keysym.sym == SDLK_RIGHT ? 4 : 0) * delta_multiplier, 0);
						break;
					case SDLK_DELETE:
						editor.delete_selected();
						break;
					case SDLK_q:
						die();
						break;
					case SDLK_RETURN:
						if (alt_down)
						{
							static bool fullscreen = 0;
							fullscreen = !fullscreen;
							video_init(display_surface->w, display_surface->h, fullscreen);
							
							editor.redraw = true;
						}
						break;
					default:
						break;
				}
				
				break;
			}
			case SDL_USEREVENT:
			{
				
				int mouse_x, mouse_y;
				Uint8 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
				
				{ // scroll if ijkl or mouse at border
					const int mouse_scroll_trigger = min(display_surface->w, display_surface->h) / 32;
					
					const unsigned int left = (mouse_x < mouse_scroll_trigger) ? 2 : 0 + key_state[SDLK_j] ? 1 : 0;
					const unsigned int right = (mouse_x >= (signed)display_surface->w - mouse_scroll_trigger) ? 2 : 0 + key_state[SDLK_l] ? 1 : 0;
					signed int delta_x = (-left + right) * delta_multiplier;
					
					const unsigned int up = (mouse_y < mouse_scroll_trigger) ? 2 : 0 + key_state[SDLK_i] ? 1 : 0;
					const unsigned int down = (mouse_y >= (signed)display_surface->h - mouse_scroll_trigger) ? 2 : 0 + key_state[SDLK_l] ? 1 : 0;
					signed int delta_y = (-up + down) * delta_multiplier;
					
					editor.scroll(delta_x, delta_y, mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT));
				}
				
				editor.draw(display_surface);
				
				SDL_Flip(display_surface);
				
				break;
			}
			default:
				break;
		}
	}
	
	SDL_UnlockSurface(display_surface);
	
	SDL_RemoveTimer(loop_timer_id);
	
	SDL_Quit();
	
	return EXIT_SUCCESS;
}

Uint32 loop_timer( Uint32 interval, void *param )
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = NULL;
	event.user.data2 = NULL;
	
	SDL_PushEvent(&event);
	
	return interval;
}

void die() {
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}



string l3_filename( const string &path, const string &name, const string &ext )
{
	ostringstream filename;
	filename << path << name << ext;
	return filename.str();
}

string l3_filename( const string &path, const string &name, int n, const string &ext )
{
	ostringstream filename;
	filename << path << name << setfill('0') << setw(3) << n << ext;
	return filename.str();
}


bool video_init( int width, int height, bool fullscreen )
{
	SDL_Color palette_buffer[256];
	int was_init = display_surface != NULL;
	
	if (was_init)
		memcpy(palette_buffer, display_surface->format->palette->colors, sizeof(palette_buffer));
	
	display_surface = SDL_SetVideoMode(width, height, 8, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE | (fullscreen ? SDL_FULLSCREEN : 0));
	
	if (display_surface == NULL)
	{
		cerr << "failed to initialize video: " << SDL_GetError() << endl;
		return false;
	}
	
	SDL_WM_SetCaption("Lem3Edit", NULL);
	
	if (was_init)
		SDL_SetColors(display_surface, palette_buffer, 0, 256);
	
	return true;
}

void box_dashed( SDL_Surface *surface, int x, int y, int width, int height, int dash )
{
	const int dash_toggle = 4;
	
	--width;
	--height;
	
	if (y >= 0 && y < surface->h)
	{
		for (int i = x; i < (x + width); ++i)
		{
			dash %= dash_toggle * 2;
			if (i > 0 && i < surface->pitch)
				((Uint8 *)surface->pixels)[y * surface->pitch + i] = (dash < dash_toggle) ? 0xff : 0x00;
			++dash;
		}
	}
	else
	{
		dash += width;
	}
	
	if ((x + width) >= 0 && (x + width) < surface->w)
	{
		for (int i = y; i < (y + height); ++i)
		{
			dash %= dash_toggle * 2;
			if (i > 0 && i < surface->h)
				((Uint8 *)surface->pixels)[i * surface->pitch + (x + width)] = (dash < dash_toggle) ? 0xff : 0x00;
			++dash;
		}
	}
	else
	{
		dash += height;
	}
	
	if ((y + height) >= 0 && (y + height) < surface->h)
	{
		for (int i = (x + width); i > x; --i)
		{
			dash %= dash_toggle * 2;
			if (i > 0 && i < surface->pitch)
				((Uint8 *)surface->pixels)[(y + height) * surface->pitch + i] = (dash < dash_toggle) ? 0xff : 0x00;
			++dash;
		}
	}
	else
	{
		dash += width;
	}
	
	if (x >= 0 && x < surface->w)
	{
		for (int i = (y + height); i > y; --i)
		{
			dash %= dash_toggle * 2;
			if (i > 0 && i < surface->h)
				((Uint8 *)surface->pixels)[i * surface->pitch + x] = (dash < dash_toggle) ? 0xff : 0x00;
			++dash;
		}
	}
	else
	{
		dash += height;
	}
}


void version( void )
{
	cerr <<  prog_name << " " << prog_ver << " (" << prog_date << ")" << endl;
	cerr << "Copyright (C) 2008-2009 Carl Reinke" << endl << endl;
	cerr << "This is free software.  You may redistribute copies of it under the terms of" << endl
	     << "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>." << endl
	     << "There is NO WARRANTY, to the extent permitted by law." << endl << endl;
}
