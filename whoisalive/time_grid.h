#ifndef TIME_GRID_H
#define TIME_GRID_H

#include "stdafx.h"

#include "../common/my_time.h"
#include "../common/my_thread.h"
#include "../common/my_ptr.h"

namespace my {

class time_grid : public wxFrame
{
public:
	typedef boost::function<void (wxGraphicsContext *gc,
		wxDouble width, wxDouble height, posix_time::ptime right_bound,
		posix_time::time_duration resolution)> on_event;

private:
	wxWindow *window_;
	bool animate_;
	on_event on_before_paint_;
	on_event on_after_paint_;
	int anim_steps_;
	posix_time::ptime right_bound_;
	posix_time::ptime new_right_bound_;
	posix_time::ptime cursor_;
	double top_;
	double bottom_;
	double z_;
	double new_z_;
	int z_step_;
	double min_z_;
	double max_z_;
	wxBitmap buffer_;
	wxBitmap background_;
	mutex buffer_mutex_;
	std::size_t paint_hash_;
	wxCoord move_x_;
	posix_time::ptime move_right_bound_;
	recursive_mutex params_mutex_;

	inline static posix_time::time_duration calc_resolution(double z)
	{
		int iz = (int)z; /* Отбрасываем дробную часть */

		/* 250ms, 500ms, 1s, 2s, 4s, 8s, 16s, 32s, ... */
		posix_time::time_duration res
			= posix_time::milliseconds(250) * (2 << iz);

		return res + my::time::mul(res, z - (double)iz);
	}

	/* Подготовка буфера под размеры окна */
	void prepare_buffer(wxBitmap *buffer, wxDouble *pw, wxDouble *ph);

	void animate(wxDouble width);
    
    /* Обработчики событий окна */
	void on_paint(wxPaintEvent& event);
	void on_erase_background(wxEraseEvent& event);
	void on_left_down(wxMouseEvent& event);
	void on_right_down(wxMouseEvent& event);
	void on_left_up(wxMouseEvent& event);
	void on_mouse_move(wxMouseEvent& event);
	void on_mouse_wheel(wxMouseEvent& event);

public:
	time_grid(wxWindow *window, bool animate, int top, int bottom,
		on_event on_before_paint, on_event on_after_paint,
		int anim_steps, int init_z, int min_z = 0, int max_z = 19);

	/* Прорисовка (вызывается пользователем) */
	void paint();

    /* Правая граница */
	inline posix_time::ptime right_bound()
	{
		unique_lock<recursive_mutex> l(params_mutex_);
		return right_bound_.is_special() ? my::time::utc_now() : right_bound_;
	}

	/* Время под курсором */
	inline posix_time::ptime cursor()
	{
		unique_lock<recursive_mutex> l(params_mutex_);
		return cursor_;
	}

	inline posix_time::time_duration resolution()
	{
		unique_lock<recursive_mutex> l(params_mutex_);
		return calc_resolution(z_);
	}

	static inline double static_time_to_x( const posix_time::ptime &time,
		const posix_time::ptime &right_bound,
		const posix_time::time_duration &resolution,
		double width )
	{
		return width - 1.0 - my::time::div(right_bound - time, resolution);
	}

	static inline posix_time::ptime static_x_to_time( double x,
		const posix_time::ptime &right_bound,
		const posix_time::time_duration &resolution,
		double width)
	{
		return right_bound - my::time::mul(resolution, width - 1.0 - x);
	}

	inline double time_to_x(const posix_time::ptime &time)
	{
		int w, h;
		window_->GetClientSize(&w, &h);
		return static_time_to_x(time, right_bound(), resolution(), (double)w);
	}

	inline posix_time::ptime x_to_time(double x)
	{
		int w, h;
		window_->GetClientSize(&w, &h);
		return static_x_to_time(x, right_bound(), resolution(), (double)w);
	}

};

}

#endif
