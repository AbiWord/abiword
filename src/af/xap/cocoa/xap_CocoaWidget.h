/* AbiSource Application Framework
 * Copyright (C) 2005 Hubert Figuiere
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


#ifndef __XAP_COCOAWIDGET_H__
#define __XAP_COCOAWIDGET_H__

#import <Cocoa/Cocoa.h>

#include "xap_Widget.h"

class XAP_CocoaWidget : public XAP_Widget
{
public:
	XAP_CocoaWidget(id w);

	/** The destructor just clean up. Must not destroy the real widget. */
	virtual ~XAP_CocoaWidget();

	/** set the widget enabled/disabled state */
	virtual void setState(bool enabled);
	/** set the widget enabled/disabled state */
	virtual bool getState(void);

	/** set the widget visible state */
	virtual void setVisible(bool visible);
	/** get the widget visible state */
	virtual bool getVisible(void);

	/** set the widget int value */
	virtual void setValueInt(int val);
	/** get the widget int value */
	virtual int getValueInt(void);

	/** set the widget value as string */
	virtual void setValueString(const UT_UTF8String &val);
	/** get the widget value as string */
	virtual void getValueString(UT_UTF8String &val);

	/** set the widget value as float */
	virtual void setValueFloat(float val);
	/** get the widget value as float */
	virtual float getValueFloat(void);

	/** set the widget label */
	virtual void setLabel(const UT_UTF8String &val);
	virtual void setLabel(const std::string &val);
private:
	id	m_widget;
};

#endif
