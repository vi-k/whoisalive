#ifndef WHO_OBJ_CLASS_H
#define WHO_OBJ_CLASS_H

#include "ipgui.h"

#include "../pinger/host_state.h"

#include "../common/my_xml.h"
#include "../common/my_ptr.h"

#include <string>
#include <memory>

namespace who
{

class server;

typedef shared_ptr<Gdiplus::Bitmap> bitmap_ptr;

class obj_class
{
public:
	typedef shared_ptr<obj_class> ptr;

private:
	server &server_;
	std::wstring name_;
	xml::wptree config_;
	bitmap_ptr bitmap_[4];
	float w_;
	float h_;
	float xc_;
	float yc_;

public:
	obj_class(server &server, const xml::wptree &config);
	~obj_class() {}

	inline const std::wstring& name()
		{ return name_; }

	inline float w()
		{ return w_; }
	inline float h()
		{ return h_; }

	inline float xc()
		{ return xc_; }
	inline float yc()
		{ return yc_; }

	inline bitmap_ptr bitmap(pinger::host_state::state_t st)
		{ return bitmap_[st]; }

	//void paint( Gdiplus::Graphics *canvas, float offs_x, float offs_y,
	//		float scale, float x, float y, float w, float h);
};

}

#endif
