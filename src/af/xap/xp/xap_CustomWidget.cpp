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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "xap_CustomWidget.h"

void XAP_CustomWidget::queueDraw(const UT_Rect *clip)
{
	/* We provide a generic implementation here, calling draw() directly.  On
	 * some platforms this may not be practical, so don't rely on this
	 * behaviour. In future this might default to a generic mechanism that
	 * combines multiple drawing requests.
	 */
	draw(clip);
}

void XAP_CustomWidgetLU::queueDrawLU(const UT_Rect *clip)
{
	GR_Graphics *gr = getGraphics();
	UT_ASSERT(gr);
	if (!gr) {
		return;
	}

	if (clip == NULL) {
		queueDraw();
	}
	else {
		UT_Rect r(
				gr->tdu(clip->left),
				gr->tdu(clip->top),
				gr->tdu(clip->width),
				gr->tdu(clip->height)
			);
		queueDraw(&r);
	}
}

void XAP_CustomWidgetLU::draw(const UT_Rect *clip)
{
	GR_Graphics *gr = getGraphics();
	UT_ASSERT(gr);

	if (clip == NULL)
		drawLU(NULL);
	else
	{
		UT_Rect r(
				gr->tlu(clip->left),
				gr->tlu(clip->top),
				gr->tlu(clip->width),
				gr->tlu(clip->height)
			);
		drawLU(&r);
	}
}

