#include "stdafx.h"

#include <string>
using namespace std;

#include "time_grid.h"

#include "../common/my_time.h"

namespace my {

time_grid::time_grid(wxWindow *window, bool animate, int top, int bottom,
		on_event on_before_paint, on_event on_after_paint,
		int anim_steps, int init_z, int min_z, int max_z)
	: window_(window)
	, animate_(animate)
	, on_before_paint_(on_before_paint)
	, on_after_paint_(on_after_paint)
	, anim_steps_(anim_steps)
	, top_(top)
	, bottom_(bottom)
	, z_( (double)init_z )
	, new_z_(z_)
	, z_step_(0)
	, min_z_( (double)min_z )
	, max_z_( (double)max_z )
	, paint_hash_(0)
	, move_x_(0)
{
	window->Connect(wxID_ANY,wxEVT_PAINT,(wxObjectEventFunction)&time_grid::on_paint,0,this);
	window->Connect(wxID_ANY,wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&time_grid::on_erase_background,0,this);
	window->Connect(wxID_ANY,wxEVT_LEFT_DOWN,(wxObjectEventFunction)&time_grid::on_left_down,0,this);
	window->Connect(wxID_ANY,wxEVT_LEFT_UP,(wxObjectEventFunction)&time_grid::on_left_up,0,this);
	window->Connect(wxID_ANY,wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&time_grid::on_right_down,0,this);
	window->Connect(wxID_ANY,wxEVT_RIGHT_UP,(wxObjectEventFunction)&time_grid::on_left_up,0,this);
	window->Connect(wxID_ANY,wxEVT_MOTION,(wxObjectEventFunction)&time_grid::on_mouse_move,0,this);
	window->Connect(window->GetId(),wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&time_grid::on_mouse_wheel,0,this);
}

void time_grid::prepare_buffer(wxBitmap *buffer, wxDouble *pw, wxDouble *ph)
{
	int iw, ih;
	window_->GetClientSize(&iw, &ih);
	*pw = (wxDouble)iw;
	*ph = (wxDouble)ih;

	if (buffer->GetWidth() != iw || buffer->GetHeight() != ih)
		buffer->Create(iw, ih);
}

void time_grid::on_erase_background(wxEraseEvent& event)
{
	event.Skip(false);
}

void time_grid::animate(wxDouble width)
{
	unique_lock<recursive_mutex> l(params_mutex_);

	if (z_step_)
	{
		/* Запоминаем состояние графика */
		posix_time::ptime _cursor = cursor();
		posix_time::ptime old_right_bound = right_bound();
		posix_time::time_duration old_resolution = resolution();
		wxDouble old_pos = static_time_to_x(_cursor,
			old_right_bound, old_resolution, width);

		/* Меняем масштаб */
		z_ += (new_z_ - z_) / z_step_;

		/* Новые (изменённые) параметры */
		posix_time::ptime new_right_bound;
		wxDouble delta;
		
		new_right_bound = right_bound_.is_special() ?
			old_right_bound : right_bound_;
		delta = (width / 2.0 - old_pos) / z_step_;

		posix_time::time_duration new_resolution = resolution();

		/* new_pos - точка, куда бы переместился курсор при увеличении */
		wxDouble new_pos = static_time_to_x(_cursor,
			new_right_bound, new_resolution, width);
		
		/* Сначала возвращаем курсор на место, а затем только
			смещаем его к центру */
		new_right_bound = old_right_bound - my::time::mul(
			new_resolution, old_pos - new_pos + delta);
			
		--z_step_;
		right_bound_ = new_right_bound > my::time::utc_now() - new_resolution
			? posix_time::not_a_date_time : new_right_bound;
	}
}

void time_grid::paint()
{
	/* Сетки для каждого из масштабов
		1-е число - основная сетка (яркие линии);
		2-е число - метки времени;
		3-е число - простая сетка (минимальная ячейка).
		
		Для недель, месяцев и лет отдельный расчёт
	*/
	static const long grids_for_z[][3] =
	{
		/* 0 */ { 60, 60, 10},
		/* 1 */ { 60, 60, 10},
		/* 2 */ { 10*60, 2*60, 60},
		/* 3 */ { 10*60, 10*60, 60},
		/* 4 */ { 10*60, 10*60, 60},
		/* 5 */ { 1*3600, 3600/2, 10*60},
		/* 6 */ { 1*3600, 1*3600, 10*60},
		/* 7 */ { 1*3600, 1*3600, 10*60},
		/* 8 */ { 24*3600, 3*3600, 1*3600},
		/* 9 */ { 24*3600, 6*3600, 1*3600},
		/* 10 */ { 24*3600, 24*3600, 1*3600},
		/* 11 */ { 24*3600, 24*3600, 12*3600},
		/* 12 */ { 0, 0, 24*3600}, /* неделя */
		/* 13 */ { 0, 0, 24*3600},
		/* 14 */ { 0, 0, 24*3600},
		/* 15 */ { 0, 0, 24*3600}, /* месяц */
		/* 16 */ { 0, 0, 24*3600},
		/* 17 */ { 0, 0, 24*3600}, /* год */
		/* 18 */ { 0, 0, 24*3600},
		/* 19 */ { 0, 0, 24*3600}
	};

	unique_lock<mutex> l_buf(buffer_mutex_);
	unique_lock<recursive_mutex> l(params_mutex_);

	/* Фон рисуем в отдельном буфере для ускорения,
		чтобы избежать ненужных прорисовок */
	wxDouble width, height;
	prepare_buffer(&background_, &width, &height);
	prepare_buffer(&buffer_, &width, &height);

	animate(width);

	/* Делаем копии основных параметров */
	posix_time::ptime _right_bound = right_bound();
	posix_time::time_duration _resolution = resolution();

	/* Узнаём, изменился ли фон */
	size_t hash = 0;
	boost::hash_combine(hash, boost::hash_value(
		my::time::floor(_right_bound, _resolution / 3) ));
	boost::hash_combine(hash, boost::hash_value(z_));
	boost::hash_combine(hash, boost::hash_value(width));
	boost::hash_combine(hash, boost::hash_value(height));
	boost::hash_combine(hash, boost::hash_value(cursor_));

	bool foreground_paint = animate_;

	if (hash != paint_hash_)
	{
		paint_hash_ = hash;
		foreground_paint = true;

		wxMemoryDC dc(background_);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

		/* Очищаем. Чёрный фон - для текущего времени, серый - если сдвинули */
		gc->SetBrush( wxBrush( right_bound_.is_special()
			? wxColour(0, 0, 0) : wxColour(48, 48, 48) ) );
		gc->DrawRectangle(0, 0, width, height);

		/* Направляющие */
		gc->SetPen( wxPen( wxColour(64, 64, 64) ) );
		gc->StrokeLine(0, top_, width, top_);
		gc->StrokeLine(0, bottom_, width, bottom_);

		if (on_before_paint_)
			on_before_paint_(gc.get(), width, height, _right_bound, _resolution);

		/* Шрифт для подписей */
		wxFont font(6, wxFONTFAMILY_DEFAULT,
			wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

		/* Сегодняшняя дата (для меток этого дня дату не ставим) */
		gregorian::date cur_date
			= posix_time::microsec_clock::local_time().date();

		/* Рисуем сетку дважды для плавного перехода между z */
		for (int i = 0; i < 2; ++i)
		{
			int iz = (int)z_;
			double za = z_ - iz;
			int z;

			if (i == 0)
				z = iz, za = 1.0 - za;
			else
				z = iz + 1;

			unsigned char zac = (unsigned char)(255.0 * za);

			if (z > 19)
				break;

			posix_time::time_duration grid_res[3];
			grid_res[0] = posix_time::seconds( grids_for_z[z][0] );
			grid_res[1] = posix_time::seconds( grids_for_z[z][1] );
			grid_res[2] = posix_time::seconds( grids_for_z[z][2] );

			posix_time::ptime grid_time
				= my::time::floor(_right_bound, grid_res[2]);

			wxDouble x;

			do /* while (x >= 0.0) */
			{
				posix_time::ptime t = my::time::utc_to_local(grid_time);
				gregorian::date d = t.date();
				
				x = static_time_to_x(grid_time, _right_bound, _resolution, width);

				/* Основная сетка (яркие линии) */
				if ( z <= 11 && my::time::floor(t, grid_res[0]) == t
					|| z >= 12 && z <= 14 && d.day_of_week() == 1
					|| z >= 15 && z <= 16 && d.day() == 1
					|| z >= 17 && d.day_of_year() == 1 )
				{
					wxDouble ext = 4.0 * za;
					gc->SetPen( wxPen( wxColour(192, 192, 192, zac) ) );
					gc->StrokeLine(x, top_ - ext, x, bottom_ + ext);
				}
	            /* Простая сетка (минимальная ячейка) */
				else if (z <= 14 || z >= 17 && d.day() == 1)
				{
					gc->SetPen( wxPen( wxColour(64, 64, 64, zac) ) );
					gc->StrokeLine(x, top_, x, bottom_);
				}

				/* Метки времени */
				if ( z <= 11 && my::time::floor(t, grid_res[1]) == t
					|| z >= 12 && z <= 14 && d.day_of_week() == 1
					|| z >= 15 && d.day() == 1 &&
						(z <= 16 || z == 17 && (d.month() & 1) == 1
						|| z == 18 && (d.month() == 1 || d.month() == 7)
						|| z == 19 && d.month() == 1) )
				{
					gc->SetFont( font, wxColour(192, 192, 192, zac) );

					wxDouble str_w, str_h, str_x, str_y, descent;
					wstring str;

					str_y = bottom_ + 5.0;
					str = my::time::to_wstring(t, L"%H:%M:%S");
					gc->GetTextExtent(str, &str_w, &str_h, &descent, 0);
					str_x = x - str_w / 2.0;

					if (i == 0 && z == 9)
						str_y -= (str_h - 2.0) * (1.0 - za);

					if (z < 10)
					{
						gc->DrawText(str, str_x, str_y);
						str_y += str_h - 2.0;
					}

					if (i == 1 && z == 10)
						str_y += (str_h - 2.0) * (1.0 - za);

					if (z >= 10 || d != cur_date)
					{
						str = my::time::to_wstring(t, L"%d.%m.%Y");
						gc->GetTextExtent(str, &str_w, &str_h, &descent, 0);
						str_x = x - str_w / 2.0;
						gc->DrawText(str, str_x, str_y);
					}
				}

				grid_time -= grid_res[2];

			} while (x >= 0.0);

		} /* for (int i = 0; i < 2; ++i) */
	
	} /* if (hash != paint_hash_) */

	if (foreground_paint)
	{
		/* Переносим фон в буфер и запускаем обработчик пользователя */
		wxMemoryDC dc(buffer_);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

		gc->DrawBitmap(background_, 0.0, 0.0, width, height);

		if (on_after_paint_)
			on_after_paint_(gc.get(), width, height, _right_bound, _resolution);

		/* Курсор */
		if (!cursor_.is_special())
		{
			wxDouble x = static_time_to_x( cursor_,
				_right_bound, _resolution, width);
			gc->SetPen( wxColour(255, 128, 0) );
			gc->StrokeLine(x, 0.0, x, height);
		}
	}

	scoped_ptr<wxGraphicsContext> gc_win( wxGraphicsContext::Create(window_) );
	gc_win->DrawBitmap(buffer_, 0.0, 0.0, width, height);
}

void time_grid::on_paint(wxPaintEvent& event)
{
	unique_lock<mutex> l(buffer_mutex_);

	if (buffer_.IsOk())
	{
		/* Создание wxPaintDC в обработчике wxPaintEvent обязательно! */
		wxPaintDC dc(window_);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

		int w, h;
		window_->GetClientSize(&w, &h);

		gc->DrawBitmap(buffer_, 0.0, 0.0, (double)w, (double)h);
	}

	event.Skip(false);
}

void time_grid::on_left_down(wxMouseEvent& event)
{
	unique_lock<recursive_mutex> l(params_mutex_);

	if (z_ == new_z_)
	{
		posix_time::ptime new_cursor = x_to_time( (double)event.GetX() );
		posix_time::time_duration dist = new_cursor - cursor();
	
		if (dist.is_negative())
			dist = -dist;

		if (dist < resolution())
			new_cursor = posix_time::not_a_date_time;

		cursor_ = new_cursor;

		on_right_down(event);
	}

	window_->SetFocus();
}

void time_grid::on_right_down(wxMouseEvent& event)
{
	unique_lock<recursive_mutex> l(params_mutex_);

	if (z_ == new_z_)
	{
		move_x_ = event.GetX();
		move_right_bound_ = right_bound();
		window_->CaptureMouse();
	}
}

void time_grid::on_left_up(wxMouseEvent& event)
{
	if (window_->HasCapture())
		window_->ReleaseMouse();
}

void time_grid::on_mouse_move(wxMouseEvent& event)
{
	unique_lock<recursive_mutex> l(params_mutex_);

	/* Сдвигаем график (если в это время не изменяется масштаб) */
	if (window_->HasCapture() && z_ == new_z_)
	{
		posix_time::ptime new_right_bound = move_right_bound_
			+ resolution() * (move_x_ - event.GetX());

		right_bound_ = new_right_bound >= my::time::utc_now() ?
			posix_time::not_a_date_time : new_right_bound;
	}

	/* Выводим время для текущей позиции мыши */
	posix_time::ptime mouse_time = x_to_time( (double)event.GetX() );

	window_->SetToolTip( my::time::to_wstring(
		my::time::utc_to_local(mouse_time), L"%H:%M:%S\n%d-%m-%Y" ) );
}

void time_grid::on_mouse_wheel(wxMouseEvent& event)
{
	unique_lock<recursive_mutex> l(params_mutex_);

	if (cursor().is_special())
		cursor_ = x_to_time( (double)event.GetX() );

	double z = new_z_ - event.GetWheelRotation() / event.GetWheelDelta();

	if (z >= min_z_ && z <= max_z_)
	{
		new_z_ = z;
		z_step_ = 4 * anim_steps_;
	}
}

}
