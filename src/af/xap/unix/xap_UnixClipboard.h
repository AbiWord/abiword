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

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_bytebuf.h"
#include "xap_FakeClipboard.h"
#include "xap_UnixApp.h"

//////////////////////////////////////////////////////////////////

class XAP_UnixClipboard
{
public:
	typedef enum _T_AllowGet { TAG_ClipboardOnly, TAG_PrimaryOnly, TAG_MostRecent } T_AllowGet;

	XAP_UnixClipboard(XAP_UnixApp * pUnixApp);
	virtual ~XAP_UnixClipboard();

	void				initialize(void);

	UT_Bool				addData(const char* format, void* pData, UT_sint32 iNumBytes);
	void				clearData(UT_Bool bClipboard, UT_Bool bPrimary);
	UT_Bool				getData(T_AllowGet tFrom, const char** formatList,
								void ** ppData, UT_uint32 * pLen,
								const char **pszFormatFound);
	UT_Bool				assertSelection(void);
	
	// the following are callbacks
	
	void					_selrcv(GtkSelectionData *selectionData, guint32 time, gpointer data);
	gint					_selclr(GdkEventSelection * event);
	void					_selsnd(GtkSelectionData * selectionData, guint info, guint32 time, gpointer data);

protected:
	virtual GdkAtom			_convertFormatString(const char * format);
	virtual const char *	_convertToFormatString(GdkAtom fmt) const;

	void					_releaseOwnership(GdkAtom atom, guint32 timeOfRelease);
	UT_Bool					_testOwnership(GdkAtom atom) const;

	UT_Bool					_getDataFromServerInFormat(GdkAtom atom, GdkAtom atomFormat,
													   void ** ppData, UT_uint32 * pLen,
													   const char **pszFormatFound);
	void					_getFormats(GdkAtom atom);
	UT_Bool					_getDataFromServer(GdkAtom atom, const char** formatList,
											   void ** ppData, UT_uint32 * pLen,
											   const char **pszFormatFound);
	UT_Bool					_getDataFromFakeClipboard(const char** formatList,
													  void ** ppData, UT_uint32 * pLen,
													  const char **pszFormatFound);
	guint32					_getTimeFromServer(GdkAtom atom);
	UT_Bool					_getCurrentSelection(const char** formatList,
												 void ** ppData, UT_uint32 * pLen,
												 const char **pszFormatFound);

	GtkWidget *			m_myWidget;				// private widget to sync selection/clipboard communication with XServer.
   
	UT_Bool				m_waiting;				// sync flag between top-half and bottom-half (callbacks)
	UT_Bool				m_bOwnClipboard;		// do we own CLIPBOARD property (ie the clipboard)
	UT_Bool				m_bOwnPrimary;			// do we own PRIMARY property (ie the X selection)
	UT_Bool				m_bWaitingForDataFromServer;	// transient used to guard against stray SELRCVs

	guint32				m_timeClipboard;		// eventTime when we took ownership of CLIPBOARD property
	guint32				m_timePrimary;			// eventTime when we took ownership of PRIMARY property
	guint32				m_timeOnServer;			// transient we use to request server time on a property
	
	GdkAtom				m_atomClipboard;		// intern("CLIPBOARD")
	GdkAtom				m_atomPrimary;			// intern("PRIMARY")
	GdkAtom				m_atomTargets;			// intern("TARGETS")
	GdkAtom				m_atomTimestamp;		// intern("TIMESTAMP")
	GdkAtom				m_databuftype;			// transient atom describing current contents of m_databuf
   
	UT_Vector			m_vecFormat_AP_Name;	// our internal list of (pseudo-mime types)
	UT_Vector			m_vecFormat_GdkAtom;	// atoms for ...AP_Name
	UT_Vector			m_vecFormatsOnServer;	// transient list of atoms from server
	UT_ByteBuf			m_databuf;				// transient buffer to receive selection data from server

	XAP_UnixApp *		m_pUnixApp;
	XAP_FakeClipboard	m_fakeClipboard;		// internal clipboard to short-circut the XServer.
};

#endif /* XAP_UNIXCLIPBOARD_H */

