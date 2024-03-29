﻿#ifndef WHO_SCHEME_H
#define WHO_SCHEME_H

#include "ipgui.h"
#include "widget.h"
#include "ipimage.h"
#include "object.h"

#include "../common/my_xml.h"
#include "../common/my_thread.h"
#include "../common/my_ptr.h"

#include <cstddef>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>

#include <boost/ptr_container/ptr_list.hpp>

namespace who {

class window;

class scheme : public widget
{
private:
	window *window_;
	std::wstring name_;
	bool first_activation_;
	recursive_mutex scheme_mutex_;
	float min_scale_;
	float max_scale_;
	scoped_ptr<Gdiplus::Bitmap> bitmap_;
	scoped_ptr<Gdiplus::Bitmap> background_;
	bool show_names_;
	bool show_map_;
	std::size_t background_hash_;

public:
	scheme(server &server, const xml::wptree *pt = NULL);
	virtual ~scheme() {}

	virtual unique_lock<recursive_mutex> create_lock()
		{ return unique_lock<recursive_mutex>(scheme_mutex_); }

	inline const wchar_t* get_name()
		{ return name_.c_str(); }

	inline bool first_activation()
		{ return first_activation_; }
	inline void set_first_activation(bool f)
		{ first_activation_ = f; }

	virtual Gdiplus::RectF own_rect();
	virtual Gdiplus::RectF client_rect();
	virtual void paint_self(Gdiplus::Graphics *canvas);
	virtual bool animate_calc();
	virtual widget* hittest(float x, float y);

	virtual void set_parent(widget *parent) {} /* Блокируем изменение parent'а */

	virtual window* get_window()
		{ return window_; }
	inline void set_window(who::window *window)
		{ window_ = window; }
	virtual scheme* get_scheme()
		{ return this; }

	virtual void animate();
	void repaint()
		{ background_hash_ = 0; }

	inline void zoom(float ds, float fix_x, float fix_y, int steps = 2)
		{ scale__(new_scale_ * ds, fix_x, fix_y, steps); }

	void scale__(float new_scale, float fix_x, float fix_y, int steps = 2);

	void align(float scr_w, float scr_h, int steps = 2);

	static inline int z(float scale)
	{
		int _z = 0;
		int zi = (int)scale;
		while (zi)
		{
			zi = zi >> 1;
			_z++;
		}
		return _z ? _z : 1;
	}

	inline bool show_names()
		{ return show_names_; }
};

}

#endif
