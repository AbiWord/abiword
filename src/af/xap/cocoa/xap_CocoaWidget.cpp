/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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


#import <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "ut_string_class.h"

#include "xap_CocoaWidget.h"
#include "xap_CocoaDialog_Utilities.h"

XAP_CocoaWidget::XAP_CocoaWidget(id w) :
	XAP_Widget(),
	m_widget(w)
{
	// 
}

XAP_CocoaWidget::~XAP_CocoaWidget()
{
	// 
}

/** set the widget enabled/disabled state */
void XAP_CocoaWidget::setState(bool enabled)
{
	[m_widget setEnabled:enabled];
}


/** set the widget enabled/disabled state */
bool XAP_CocoaWidget::getState(void)
{
	return [m_widget isEnabled];
}

/** set the widget visible state */
void XAP_CocoaWidget::setVisible(bool /*visible*/)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
/** get the widget visible state */
bool XAP_CocoaWidget::getVisible(void)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return true;
}

/** set the widget int value */
void XAP_CocoaWidget::setValueInt(int val)
{
	[m_widget setIntValue:val];
}

/** get the widget int value */
int XAP_CocoaWidget::getValueInt(void)
{
	return [m_widget intValue];
}

	
/** set the widget value as string */
void XAP_CocoaWidget::setValueString(const UT_UTF8String &val)
{
	NSString *str = [[NSString alloc ]initWithUTF8String:val.utf8_str()];

	[m_widget setStringValue:str];
	[str release];
}

/** get the widget value as string */
void XAP_CocoaWidget::getValueString(UT_UTF8String &val)
{
	NSString * str = [m_widget stringValue];
	val.assign([str UTF8String]);
}

/** set the widget value as float */
void XAP_CocoaWidget::setValueFloat(float val)
{
	[m_widget setFloatValue:val];

}

/** get the widget value as float */
float XAP_CocoaWidget::getValueFloat(void)
{
	return [m_widget floatValue];
}


/** set the widget label */
void XAP_CocoaWidget::setLabel(const UT_UTF8String &val)
{
	SetNSControlLabel(m_widget, val);
}

void XAP_CocoaWidget::setLabel(const std::string &val)
{
	SetNSControlLabel(m_widget, val);
}
