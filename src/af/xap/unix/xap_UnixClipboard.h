/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef XAP_UNIXCLIPBOARD_H
#define XAP_UNIXCLIPBOARD_H

#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Clipboard.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

class XAP_UnixClipboard : public XAP_Clipboard
{
public:
   XAP_UnixClipboard();
   XAP_UnixClipboard(const char * selection);

   virtual ~XAP_UnixClipboard();
	
   virtual UT_Bool		open(void);
   virtual UT_Bool		close(void);
   virtual UT_Bool		addData(const char* format, void* pData, UT_sint32 iNumBytes);
   virtual UT_sint32	getDataLen(const char* format);
   virtual UT_Bool		getData(const char* format, void* pData);
   virtual UT_Bool		hasFormat(const char* format);
   virtual UT_sint32	countFormats(void);
   virtual const char *	getNthFormat(UT_sint32 n);
   virtual UT_Bool		clear(void);
   
   virtual GR_Image*	getImage(void);
   virtual UT_Bool		addImage(GR_Image*);

   GdkAtom	m_received_format;
   void*	m_received_data;
   UT_uint32	m_received_length;

   UT_Vector	m_vecData;

   UT_Vector   	m_vecFormatAtoms;

   UT_Bool	m_waiting;
   UT_Bool	m_error;

   UT_Bool	m_ownClipboard;

   GdkAtom	m_selection;
   
protected:

   virtual UT_Bool _init(GdkAtom selection);
   
   GtkWidget*	m_targets_widget;
   GtkWidget*	m_data_widget;
   
   virtual UT_Bool	_getFormats(void);
   virtual UT_Bool	_getData(GdkAtom target);
   
   virtual UT_Bool	_getClipboard(void);
   virtual void		_releaseClipboard(void);
      
   virtual GdkAtom	_convertFormatString(const char * format);
   virtual const char * _convertToFormatString(GdkAtom fmt);
   UT_Vector	m_vecFormat;
   UT_Vector	m_vecCF;
};

#endif /* XAP_UNIXCLIPBOARD_H */

