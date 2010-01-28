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

#ifndef XAP_CUSTOMWIDGET_H
#define XAP_CUSTOMWIDGET_H

#include "ut_misc.h"
#include "gr_Graphics.h"

class XAP_CustomWidget
{
public:
	XAP_CustomWidget() {}
	virtual ~XAP_CustomWidget() {}

	virtual void queueDraw(const UT_Rect *clip=NULL);

protected:
	/* derived classes should do their actual drawing here */
	virtual void draw(const UT_Rect *clip=NULL) = 0;
};

/* utility class for widgets drawing in layout units */
class XAP_CustomWidgetLU: virtual public XAP_CustomWidget
{
public:
	virtual GR_Graphics *getGraphics() const = 0;
	virtual void queueDrawLU(const UT_Rect *clip=NULL);

protected:
	virtual void draw(const UT_Rect *clip);
	virtual void drawLU(const UT_Rect *clip) = 0;
};

#endif
