/* AbiSource Application Framework
 * Copyright (C) 2010 Patrik Fimml
 * Copyright (C) 2021 Hubert Figuière
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <gtk/gtk.h>

#include "ut_assert.h"
#include "ut_misc.h"
#include "xap_UnixCustomWidget.h"

void XAP_UnixCustomWidget::_fe::draw(XAP_UnixCustomWidget *self, cairo_t *cr)
{
	self->m_cr = cr;
	double x1, y1, x2, y2;
	cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

	UT_Rect r(x1, y1, x2 - x1, y2 - y1);
	self->drawImmediate(&r);
}
