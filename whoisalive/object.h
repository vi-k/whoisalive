#ifndef WHO_OBJECT_H
#define WHO_OBJECT_H

#include "ipgui.h"
#include "widget.h"
#include "obj_class.h"
#include "hosts_state.h"

#include "../common/my_xml.h"

#include <string>
#include <list>

namespace who {

namespace link_type { enum t {wire, optics, air}; }

class object : public widget
{
	typedef std::list<std::wstring> hosts_list;

private:
	std::wstring name_;
	hosts_list hosts_;
	who::obj_class::ptr class_;
	pinger::hosts_state state_;
	Gdiplus::ColorMatrix state_matrix_;
	Gdiplus::ColorMatrix new_state_matrix_;
	int state_step_;
	int flash_step_;
	bool flash_pause_;
	float flash_alpha_;
	float flash_new_alpha_;
	bool show_name_;
	float offs_x_;
	float offs_y_;
	std::wstring host_link_;
	link_type::t link_type_;

public:
	object(server &server, const xml::wptree *pt = NULL);
	virtual ~object();

	virtual Gdiplus::RectF own_rect();
	Gdiplus::RectF rect_norm();
	virtual bool animate_calc();
	virtual void paint_self( Gdiplus::Graphics *canvas);
	virtual widget* hittest(float x, float y);

	inline const std::wstring& name()
		{ return name_; }

	virtual inline float alpha()
		{ return widget::alpha() * state_matrix_.m[3][3]; }

	inline float full_alpha()
		{ return alpha() * flash_alpha_; }

	inline pinger::host_state::state_t state()
		{ return state_.state(); }

	virtual void do_check_state();

	inline bool acknowledged()
		{ return state_.acknowledged(); }

	void acknowledge();
	void unacknowledge();

	inline const hosts_list& hosts()
		{ return hosts_; };

	inline std::wstring host_link()
		{ return host_link_; }

	inline bool show_name()
		{ return show_name_; }
	inline void set_show_name(bool show_name)
	{
		show_name_ = show_name;
		animate();
	}

	inline float offs_x()
		{ return offs_x_; }
	inline float offs_y()
		{ return offs_y_; }
		
	inline link_type::t link_type()
		{ return link_type_; }
};

}

#endif
