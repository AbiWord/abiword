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

#ifndef XAP_UNIXCUSTOMWIDGET_H
#define XAP_UNIXCUSTOMWIDGET_H

#include "xap_CustomWidget.h"
#include <gtk/gtkwidget.h>

class XAP_UnixCustomWidget: virtual public XAP_CustomWidget
{
public:
	virtual GtkWidget *getWidget() = 0;
	virtual void queueDraw(const UT_Rect *clip=NULL);

protected:
	class _fe
	{
	public:
		static void expose(XAP_UnixCustomWidget *self, GdkEventExpose *ev);
	};
	friend class _fe;
};

#endif
