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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */



#include <gtk/gtk.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "xap_UnixWidget.h"


/** set the widget enabled/disabled state */
void XAP_UnixWidget::setState(bool enabled)
{
	gtk_widget_set_sensitive(m_widget, enabled);
}


/** set the widget enabled/disabled state */
bool XAP_UnixWidget::getState(void)
{
	return GTK_WIDGET_SENSITIVE(m_widget);
}

/** set the widget visible state */
void XAP_UnixWidget::setVisible(bool visible)
{
	if (visible) {
		gtk_widget_show(m_widget);
	}
	else {
		gtk_widget_hide(m_widget);
	}
}
/** get the widget visible state */
bool XAP_UnixWidget::getVisible(void)
{
	return GTK_WIDGET_VISIBLE(m_widget);
}

/** set the widget int value */
void XAP_UnixWidget::setValueInt(int val)
{
	if (GTK_IS_TOGGLE_BUTTON(m_widget)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_widget), (val?TRUE:FALSE));
	}
	else if (GTK_IS_LABEL(m_widget)) {
	  UT_UTF8String str;
	  UT_UTF8String_sprintf(str,"%d",val);
	  gtk_label_set_text(GTK_LABEL(m_widget), str.utf8_str());
	}
	else if (GTK_IS_ENTRY(m_widget)) {
	  UT_UTF8String str;
	  UT_UTF8String_sprintf(str,"%d",val);
	  gtk_entry_set_text(GTK_ENTRY(m_widget), str.utf8_str());
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
}

/** get the widget int value */
int XAP_UnixWidget::getValueInt(void)
{
	if (GTK_IS_TOGGLE_BUTTON(m_widget)) {
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_widget));
	}
	else if (GTK_IS_ENTRY(m_widget)) {
		return atoi(gtk_entry_get_text(GTK_ENTRY(m_widget)));
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	return 0;
}

	
/** set the widget value as string */
void XAP_UnixWidget::setValueString(const UT_UTF8String &val)
{
	if (GTK_IS_ENTRY(m_widget)) {
		gtk_entry_set_text(GTK_ENTRY(m_widget), val.utf8_str());
	}
	else if (GTK_IS_LABEL(m_widget)) {
		gtk_label_set_text(GTK_LABEL(m_widget), val.utf8_str());
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
}

/** get the widget value as string */
void XAP_UnixWidget::getValueString(UT_UTF8String &val)
{
	if (GTK_IS_ENTRY(m_widget)) {
		val.assign(gtk_entry_get_text(GTK_ENTRY(m_widget)));
	}
	else if (GTK_IS_LABEL(m_widget)) {
		val.assign(gtk_label_get_text(GTK_LABEL(m_widget)));
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
}

/** set the widget value as float */
void XAP_UnixWidget::setValueFloat(float val)
{
	if (GTK_IS_ENTRY(m_widget)) {
        UT_UTF8String str;
	UT_UTF8String_sprintf(str,"%f",val);
	gtk_entry_set_text(GTK_ENTRY(m_widget), str.utf8_str());
	}
}

/** get the widget value as float */
float XAP_UnixWidget::getValueFloat(void)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return 0;
}


/** set the widget label */
void XAP_UnixWidget::setLabel(const UT_UTF8String &val)
{
	if (GTK_IS_BUTTON(m_widget)) {
		gtk_button_set_label(GTK_BUTTON(m_widget), val.utf8_str());
	}
	else if (GTK_IS_LABEL(m_widget)) {
		gtk_label_set_text(GTK_LABEL(m_widget), val.utf8_str());
	}
	else if (GTK_IS_WINDOW(m_widget)) {
		gtk_window_set_title(GTK_WINDOW(m_widget), val.utf8_str());
	}
	else {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
}
