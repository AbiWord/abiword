/* AbiSource Application Framework
 * Copyright (C) 2010 Patrik Fimml
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "xap_UnixCustomWidget.h"

void XAP_UnixCustomWidget::queueDraw(const UT_Rect *clip)
{
	GtkWidget *widget = getWidget();
	UT_ASSERT(widget);

	if (!clip)
		gtk_widget_queue_draw(widget);
	else
	{
		gtk_widget_queue_draw_area(
				widget,
				clip->left,
				clip->top,
				clip->width,
				clip->height
			);
	}
}

void XAP_UnixCustomWidget::_fe::draw(XAP_UnixCustomWidget *self, cairo_t *cr)
{
	GdkEventExpose *ev = reinterpret_cast<GdkEventExpose *>(gtk_get_current_event());
	UT_Rect r(
			ev->area.x,
			ev->area.y,
			ev->area.width,
			ev->area.height
		);
	self->m_cr = cr;
	self->draw(&r);
}
