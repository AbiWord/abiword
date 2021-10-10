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

#include "ut_assert.h"
#include "gr_Graphics.h"

#include "xap_CustomWidget.h"

void XAP_CustomWidget::queueDraw(const UT_Rect *clip)
{
	m_drawQueue.push(clip ? UT_Option<UT_Rect>(*clip) : UT_Option<UT_Rect>());
	getGraphics()->queueDraw(clip);
}

void XAP_CustomWidget::queueDrawLU(const UT_Rect *clip)
{
	GR_Graphics *gr = getGraphics();
	UT_ASSERT(gr);
	if (!gr) {
		return;
	}

	if (clip == nullptr) {
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

void XAP_CustomWidget::drawImmediate(const UT_Rect* clip)
{
	GR_Graphics *gr = getGraphics();
	UT_ASSERT(gr);

	if (clip == NULL) {
		drawImmediateLU(NULL);
	} else {
		UT_Rect r(
				gr->tlu(clip->left),
				gr->tlu(clip->top),
				gr->tlu(clip->width),
				gr->tlu(clip->height)
			);
		drawImmediateLU(&r);
	}
}

