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


#ifndef __XAP_WIDGET_H__
#define __XAP_WIDGET_H__

#include <string>

#include "ut_types.h"

class UT_UTF8String;


/** virtual widget accessor. This class is pure virtual and
    must be verry lightweight
*/
class ABI_EXPORT XAP_Widget
{
protected:
	XAP_Widget()
		{ }

public:
	/** The destructor just clean up. Must not destroy the real widget. */
	virtual ~XAP_Widget()
		{ }

	/** set the widget enabled/disabled state */
	virtual void setState(bool enabled) = 0;
	/** set the widget enabled/disabled state */
	virtual bool getState(void) = 0;

	/** set the widget visible state */
	virtual void setVisible(bool visible) = 0;
	/** get the widget visible state */
	virtual bool getVisible(void) = 0;

	/** set the widget int value */
	virtual void setValueInt(int val) = 0;
	/** get the widget int value */
	virtual int getValueInt(void) = 0;

	/** set the widget value as string */
	virtual void setValueString(const UT_UTF8String &val) = 0;
	/** get the widget value as string */
	virtual void getValueString(UT_UTF8String &val) = 0;

	/** set the widget value as float */
	virtual void setValueFloat(float val) = 0;
	/** get the widget value as float */
	virtual float getValueFloat(void) = 0;

	/** set the widget label */
	virtual void setLabel(const UT_UTF8String &val) = 0;
	virtual void setLabel(const std::string &val) = 0;
};


#endif
