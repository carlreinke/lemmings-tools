#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "del.hpp"
#include "level.hpp"
#include "tribe.hpp"

#include "SDL.h"

#include <set>

class Editor
{
public:
	bool redraw;
	
	void draw( SDL_Surface *surface );
	
	Del font;
	
	Level level;
	Tribe tribe;
	Style style;
	
	signed int scroll_x, scroll_y;
	signed int drag_snap_x, drag_snap_y;
	
	typedef std::set<Level::Object::Index> Selection;
	Selection selection;
	
	typedef std::vector< std::pair<Level::Object::Index, Level::Object> > Clipboard;
	Clipboard clipboard;
	
	bool select( signed int x, signed int y, bool modify );
	bool select_none( void );
	bool select_all( void );
	
	bool copy_selected( void );
	bool paste( void );
	bool delete_selected( void );
	bool move_selected( signed int delta_x, signed int delta_y );
	bool move_selected_z( signed int delta_z );
	
	bool scroll( signed int delta_x, signed int delta_y, bool drag );
	
	bool load( int n );
	
	Editor( void );
	~Editor( void ) { /* nothing to do */ }
	
private:
	Editor( const Editor & );
	Editor & operator=( const Editor & );
	
	signed int snap( signed int &value, unsigned int snap );
};

#endif // EDITOR_HPP
