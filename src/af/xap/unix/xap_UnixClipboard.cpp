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

#include <string.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"

#include "xap_UnixClipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
	// for an explicit cut/copy operation, application-specific code
	// actually makes a copy of the current selection and hands us
	// a buffer in a named format (such as RTF).
	//
	// this should not be confused with the X Selection mechanism
	// (left-mouse select and middle-mouse paste) which

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

static void s_selsnd(GtkWidget * widget, GtkSelectionData * selectionData, guint info, guint32 time, gpointer data)
{
	// callback
	XAP_UnixClipboard * pThis = (XAP_UnixClipboard *)gtk_object_get_data(GTK_OBJECT(widget), "clipboard");
	pThis->_selsnd(selectionData,info,time,data);
	return;
}

static gint s_selclr(GtkWidget * widget, GdkEventSelection * event)
{
	// callback
	XAP_UnixClipboard * pThis = (XAP_UnixClipboard *)gtk_object_get_data(GTK_OBJECT(widget), "clipboard");
	return pThis->_selclr(event);
}

static void s_selrcv(GtkWidget * widget, GtkSelectionData *selectionData, guint32 time, gpointer data)
{
	// callback
	XAP_UnixClipboard * pThis = (XAP_UnixClipboard *)gtk_object_get_data(GTK_OBJECT(widget), "clipboard");
	return pThis->_selrcv(selectionData,time,data);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_UnixClipboard::XAP_UnixClipboard(XAP_UnixApp * pUnixApp)
{
	// caller must call initialize()

	m_pUnixApp = pUnixApp;
}

XAP_UnixClipboard::~XAP_UnixClipboard()
{
	UT_DEBUGMSG(("Clipboard: destroying [ownPrimary %d][ownClipboard %d]\n",
				 m_bOwnPrimary, m_bOwnClipboard));

	if (m_bOwnClipboard || m_bOwnPrimary)
		clearData(m_bOwnClipboard,m_bOwnPrimary);

	if (m_myWidget)
		gtk_widget_destroy(m_myWidget);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::initialize(void)
{
   UT_DEBUGMSG(("Clipboard: initializing\n"));

   m_waiting = UT_FALSE;
   m_bOwnClipboard = UT_FALSE;
   m_bOwnPrimary = UT_FALSE;
   m_timeClipboard = 0;
   m_timePrimary = 0;
   m_timeOnServer = 0;
   m_databuftype = GDK_NONE;

   m_atomClipboard = gdk_atom_intern("CLIPBOARD", FALSE);
   UT_DEBUGMSG(("Clipboard: property [%s atom %lu]\n","CLIPBOARD", m_atomClipboard));
   m_atomPrimary = gdk_atom_intern("PRIMARY", FALSE);
   UT_DEBUGMSG(("Clipboard: property [%s atom %lu]\n","PRIMARY", m_atomPrimary));

   m_atomTargets = gdk_atom_intern("TARGETS", FALSE);
   UT_DEBUGMSG(("Clipboard: target [%s atom %lu]\n","TARGETS",m_atomTargets));
   m_atomTimestamp = gdk_atom_intern("TIMESTAMP", FALSE);
   UT_DEBUGMSG(("Clipboard: target [%s atom %lu]\n","TIMESTAMP",m_atomTimestamp));
   

#ifdef DEBUG
   for (int j=0, jLimit=m_vecFormat_AP_Name.getItemCount(); (j<jLimit); j++)
	   UT_DEBUGMSG(("Clipboard: target [%s atom %lu]\n",
					(char *)m_vecFormat_AP_Name.getNthItem(j),
					(GdkAtom)m_vecFormat_GdkAtom.getNthItem(j)));
#endif
   
   // create hidden/private window to use with the clipboard
   
   m_myWidget = gtk_window_new(GTK_WINDOW_POPUP);
   gtk_widget_realize(m_myWidget);
   gtk_object_set_data(GTK_OBJECT(m_myWidget), "clipboard", (gpointer)this);

   // register static callbacks to listen to selection-related events
   // on this window.
   
   gtk_signal_connect(GTK_OBJECT(m_myWidget), "selection_received",    GTK_SIGNAL_FUNC(s_selrcv), (gpointer)this);
   gtk_signal_connect(GTK_OBJECT(m_myWidget), "selection_clear_event", GTK_SIGNAL_FUNC(s_selclr), (gpointer)this);
   gtk_signal_connect(GTK_OBJECT(m_myWidget), "selection_get",         GTK_SIGNAL_FUNC(s_selsnd), (gpointer)this);

   // register targets (formats) for each format that we support
   // on both the CLIPBOARD property and the PRIMARY property.
   
   for (int k=0, kLimit=m_vecFormat_AP_Name.getItemCount(); (k<kLimit); k++)
   {
	   gtk_selection_add_target(m_myWidget,m_atomClipboard,(GdkAtom)m_vecFormat_GdkAtom.getNthItem(k), 0);
	   gtk_selection_add_target(m_myWidget,m_atomPrimary,  (GdkAtom)m_vecFormat_GdkAtom.getNthItem(k), 0);
   }

   return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_UnixClipboard::assertSelection(void)
{
	// assert the X-Selection.  user must have started a
	// mouse click and drag or somehow started a selection.
	// unlike the CLIPBOARD (which requires an explicit
	// Cut or Copy), we must assert the X-Selection anytime
	// that something is selected.  we don't have to record
	// what it is that is selected, just that there is one.

	UT_ASSERT( (m_bOwnPrimary == _testOwnership(m_atomPrimary)) );

	if (m_bOwnPrimary)					// already asserted it and still own it
		return UT_TRUE;
	
	m_timePrimary = m_pUnixApp->getTimeOfLastEvent();

	UT_DEBUGMSG(("Clipboard: asserting PRIMARY property [timestamp %08lx]\n",m_timePrimary));

	m_bOwnPrimary = gtk_selection_owner_set(m_myWidget,m_atomPrimary,m_timePrimary);
	UT_DEBUGMSG(("Clipboard: took ownership of PRIMARY property [%s][timestamp %08lx]\n",
				 ((m_bOwnPrimary) ? "successful" : "failed"),m_timePrimary));
	return m_bOwnPrimary;
}
	
UT_Bool XAP_UnixClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	// This is an EXPLICIT Cut or Copy from the User.
	// First, we stick a copy of the data onto our internal clipboard.
	// Then, we assert ownership of the CLIPBOARD property.
	// NOTE: Do not call this for the PRIMARY property (use assertSelection()).
	
	m_timeClipboard = m_pUnixApp->getTimeOfLastEvent();
	
	UT_DEBUGMSG(("Clipboard: explicit cut/copy to CLIPBOARD property [format %s][len %d][timestamp %08lx]\n",
				 format,iNumBytes,m_timeClipboard));

	if (!m_fakeClipboard.addData(format,pData,iNumBytes))
		return UT_FALSE;

	m_bOwnClipboard = _testOwnership(m_atomClipboard);
	if (m_bOwnClipboard)
	{
		// There's probably a benign race-condition here.
		UT_DEBUGMSG(("Clipboard: refreshed ownership of CLIPBOARD property at [timestamp %08lx]\n",
					 m_timeClipboard));
	}
	else
	{
		m_bOwnClipboard = gtk_selection_owner_set(m_myWidget,m_atomClipboard,m_timeClipboard);
		UT_DEBUGMSG(("Clipboard: took ownership of CLIPBOARD property [%s][timestamp %08lx]\n",
					 ((m_bOwnClipboard) ? "successful" : "failed"),m_timeClipboard));
	}

	return m_bOwnClipboard;
}

void XAP_UnixClipboard::clearData(UT_Bool bClipboard, UT_Bool bPrimary)
{
	// User requested us to clear the clipboard.

	UT_DEBUGMSG(("Clipboard: explicit clear [CLIPBOARD %d][PRIMARY %d]\n",bClipboard,bPrimary));
	
	if (bClipboard)
	{
		m_timeClipboard = m_pUnixApp->getTimeOfLastEvent();
		_releaseOwnership(m_atomClipboard,m_timeClipboard);
		// TODO decide if we need to call fakeClipboard -- the callback
		// TODO should have taken care of this.
		m_fakeClipboard.clearClipboard();
	}
	
	if (bPrimary)
	{
		m_timePrimary = m_pUnixApp->getTimeOfLastEvent();
		_releaseOwnership(m_atomPrimary,m_timePrimary);
	}
	
	return;
}

UT_Bool XAP_UnixClipboard::getData(T_AllowGet tFrom, const char** formatList,
								   void ** ppData, UT_uint32 * pLen,
								   const char **pszFormatFound)
{
	// Fetch data from the clipboard (using the allowable source(s))
	// in one of the prioritized list of formats.  Return pointer
	// to clipboard's buffer.  

	UT_DEBUGMSG(("Clipboard:: getData called [bOwnClipboard %d][bOwnPrimary %d]\n",m_bOwnClipboard,m_bOwnPrimary));

	// The following asserts are probaby not true in an absolute
	// sense -- that is, there is a race condition here that
	// could happen, but I don't really care about.  Right now, I'm
	// more concerned about the general correctness of the code.
	
	UT_ASSERT( (m_bOwnClipboard == _testOwnership(m_atomClipboard)) );
	UT_ASSERT( (m_bOwnPrimary == _testOwnership(m_atomPrimary)) );
	
	if (tFrom == TAG_MostRecent)
	{
		guint32 timePrimary = ((m_bOwnPrimary) ? m_timePrimary : _getTimeFromServer(m_atomPrimary));
		guint32 timeClipboard = ((m_bOwnClipboard) ? m_timeClipboard : _getTimeFromServer(m_atomClipboard));
		tFrom = ((timePrimary > timeClipboard) ? TAG_PrimaryOnly : TAG_ClipboardOnly);

		UT_DEBUGMSG(("Clipboard::getData: Most recent is [%s]\n",
					 ((tFrom==TAG_PrimaryOnly) ? gdk_atom_name(m_atomPrimary) : gdk_atom_name(m_atomClipboard))));
	}

	switch (tFrom)
	{
	case TAG_ClipboardOnly:
		if (m_bOwnClipboard)
			return _getDataFromFakeClipboard(formatList,ppData,pLen,pszFormatFound);
		return _getDataFromServer(m_atomClipboard,formatList,ppData,pLen,pszFormatFound);
		
	case TAG_PrimaryOnly:
		if (m_bOwnPrimary)
			return _getCurrentSelection(formatList,ppData,pLen,pszFormatFound);
		return _getDataFromServer(m_atomPrimary,formatList,ppData,pLen,pszFormatFound);
		
	case TAG_MostRecent:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
		
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return UT_FALSE;
	}
	/*NOTREACHED*/
}
	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_UnixClipboard::_getDataFromFakeClipboard(const char** formatList,
													 void ** ppData, UT_uint32 * pLen,
													 const char **pszFormatFound)
{
	// fetch best match from our internal clipboard.

	UT_ASSERT(m_bOwnClipboard);
	
	for (int k=0; (formatList[k]); k++)
		if (m_fakeClipboard.getClipboardData(formatList[k],ppData,pLen))
		{
			*pszFormatFound = formatList[k];
			UT_DEBUGMSG(("Clipboard::_getDataFromFakeClipboard: found format [%s][len %d]\n",formatList[k],*pLen));
			return UT_TRUE;
		}

	UT_DEBUGMSG(("Clipboard::_getDataFromFakeClipboard: no matching format found ??\n"));
	return UT_FALSE;
}

UT_Bool XAP_UnixClipboard::_getDataFromServer(GdkAtom atom, const char** formatList,
											  void ** ppData, UT_uint32 * pLen,
											  const char **pszFormatFound)
{
	// Fetch selection data from the named atom from the XServer
	// in one of the prioritized list of formats.  Return pointer
	// to clipboard's buffer.

	UT_DEBUGMSG(("Clipboard::_getDataFromServer called\n"));

	// ask XServer for all formats on this property; fill m_vecFormatsOnServer.

	_getFormats(atom);
#if 1
	// TODO This is an experimental hack to try to get a "TEXT" from
	// TODO the current owner of the clipboard -- if they refused to
	// TODO respond (or returned a bogus reply) to the "TARGETS" request.
	// TODO Remove this later.
	if (m_vecFormatsOnServer.getItemCount() == 0)
		m_vecFormatsOnServer.addItem((void*)gdk_atom_intern("TEXT",FALSE));
#endif

	// walk desired formats list and find first one that server also has.
	
	int kLimit = m_vecFormatsOnServer.getItemCount();
	for (int j=0; (formatList[j]); j++)
	{
		GdkAtom atomJ = _convertFormatString(formatList[j]);
		for (int k=0; (k<kLimit); k++)
		{
			GdkAtom atomK = (GdkAtom)m_vecFormatsOnServer.getNthItem(k);
			if (atomJ == atomK)
			{
				UT_DEBUGMSG(("Clipboard::_getDataFromServer: [property %s] attempting to fetch [format %s]\n",
							 gdk_atom_name(atom),gdk_atom_name(atomK)));
				return _getDataFromServerInFormat(atom,atomK,ppData,pLen,pszFormatFound);
			}
		}
	}

	UT_DEBUGMSG(("Clipboard::_getDataFromServer: [property %s] didn't contain anything in format requested.\n",
				 gdk_atom_name(atom)));
	return UT_FALSE;
}

void XAP_UnixClipboard::_getFormats(GdkAtom atom)
{
	// populate m_vecFormatsOnServer with the set of formats
	// currently set on the named property.

	UT_DEBUGMSG(("Clipboard::_getFormats: requesting formats for [property %s]\n",
				 gdk_atom_name(atom)));

	m_waiting = UT_TRUE;
	//gtk_selection_convert(m_myWidget,atom,m_atomTargets,m_pUnixApp->getTimeOfLastEvent());
	gtk_selection_convert(m_myWidget,atom,m_atomTargets,GDK_CURRENT_TIME);
	while (m_waiting)
		gtk_main_iteration();

	return;
}

guint32 XAP_UnixClipboard::_getTimeFromServer(GdkAtom atom)
{
	// populate m_vecFormatsOnServer with the set of formats
	// currently set on the named property.

	UT_DEBUGMSG(("Clipboard::_getTimeFromServer: requesting timestamp for [property %s]\n",
				 gdk_atom_name(atom)));

	m_waiting = UT_TRUE;
	gtk_selection_convert(m_myWidget,atom,m_atomTimestamp,m_pUnixApp->getTimeOfLastEvent());
	while (m_waiting)
		gtk_main_iteration();

	return m_timeOnServer;
}

UT_Bool XAP_UnixClipboard::_getDataFromServerInFormat(GdkAtom atom, GdkAtom atomFormat,
													  void ** ppData, UT_uint32 * pLen,
													  const char **pszFormatFound)
{
	// request contents of selection/clipboard from the server
	// in the named format.

	UT_DEBUGMSG(("Clipboard::_getDataFromServerInFormat: [property %s][formst %s]\n",
				 gdk_atom_name(atom),gdk_atom_name(atomFormat)));

	m_bWaitingForDataFromServer = UT_TRUE;	// safety check to guard against stray messages
	m_waiting = UT_TRUE;
	gtk_selection_convert(m_myWidget,atom,atomFormat,m_pUnixApp->getTimeOfLastEvent());
	while (m_waiting)
		gtk_main_iteration();

	if (m_databuftype==GDK_NONE)
	{
		*pszFormatFound = NULL;
		*ppData = NULL;
		*pLen = 0;
		return UT_FALSE;
	}
	else
	{
		*pszFormatFound = _convertToFormatString(m_databuftype);
		*ppData = (void *)m_databuf.getPointer(0);
		*pLen = m_databuf.getLength();
		return UT_TRUE;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_UnixClipboard::_testOwnership(GdkAtom atom) const
{
	// return TRUE iff the XServer currently thinks
	// that we still own the given atom.
	
	GdkWindow * w = gdk_selection_owner_get(atom);
	return (w == m_myWidget->window);
}

void XAP_UnixClipboard::_releaseOwnership(GdkAtom atom, guint32 timeOfRelease)
{
	if (_testOwnership(atom))
	{
		// the following call will send a message to the XServer.
		// upon updating the property, the XServer will send us
		// a selection_clear.  we spin here until it comes in.
		// TODO investigate if this is necessary and/or if we
		// TODO need a timeout on this loop.

		m_waiting = UT_TRUE;
		gtk_selection_owner_set(NULL,atom,timeOfRelease);
		while (m_waiting)
			gtk_main_iteration();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::_selsnd(GtkSelectionData * selectionData, guint /*info*/, guint32 /*time*/, gpointer /*data*/)
{
	// callback
	// someone asked us for the contents of the selection
	// (CLIPBOARD or PRIMARY) that we asserted.
	// send them the data.
	
	UT_DEBUGMSG(("Clipboard: SELSND: [property %s][target %s]\n",
				 gdk_atom_name(selectionData->selection),
				 gdk_atom_name(selectionData->target)));

	UT_ASSERT(!m_waiting);

	const char * szApFormatDesired = _convertToFormatString(selectionData->target);
	if (!szApFormatDesired)
	{
		UT_DEBUGMSG(("Clipboard: SELSND failed -- unknown target format\n"));
		return;
	}
	if (UT_stricmp(szApFormatDesired,"STRING") == 0)
	{
		// silently map "STRING" into "TEXT" for lookup purposes
		szApFormatDesired = "TEXT";
		UT_DEBUGMSG(("Clipboard: silently folding target STRING onto TEXT\n"));
	}

	if (selectionData->selection == m_atomClipboard)
	{
//		UT_ASSERT(m_bOwnClipboard);

		void * pData = NULL;
		UT_uint32 iLen = 0;
		UT_Bool bHaveDataInRequestedFormat = m_fakeClipboard.getClipboardData(szApFormatDesired,&pData,&iLen);
		if (!bHaveDataInRequestedFormat)
		{
			UT_DEBUGMSG(("Clipboard: SELSND CLIPBOARD failed -- no data on internal clipboard in requested format [%s]\n",
						 szApFormatDesired));
			return;
		}

		UT_DEBUGMSG(("Clipboard: SELSND CLIPBOARD posting %d bytes [format %s]\n",iLen,szApFormatDesired));
		gtk_selection_data_set(selectionData,selectionData->target,8,(guchar*)pData,iLen);
		return;
	}

	if (selectionData->selection == m_atomPrimary)
	{
		UT_ASSERT(m_bOwnPrimary);

		void * pData = NULL;
		UT_uint32 iLen = 0;
		const char * aszFormats[2];
		const char * szApFormatFound = NULL;

		aszFormats[0] = szApFormatDesired;
		aszFormats[1] = NULL;

		UT_Bool bHaveDataInRequestedFormat = _getCurrentSelection(aszFormats,&pData,&iLen,&szApFormatFound);
		if (!bHaveDataInRequestedFormat)
		{
			UT_DEBUGMSG(("Clipboard:: SELSND PRIMARY failed -- no data on current X selection in requested format [%s]\n",
						 szApFormatDesired));
			return;
		}
		
		UT_DEBUGMSG(("Clipboard: SELSND PRIMARY posting %d bytes [format %s]\n",iLen,szApFormatFound));
		gtk_selection_data_set(selectionData,selectionData->target,8,(guchar*)pData,iLen);
		return;
	}

	UT_DEBUGMSG(("Clipboard: SELSND failed -- unknown property\n"));
	return;
}

gint XAP_UnixClipboard::_selclr(GdkEventSelection * event)
{
	// callback
	// someone else now owns the indicated property.
	// we should release our resources and probably
	// unhighlight the selection on screen.
	
	UT_DEBUGMSG(("Clipboard: SELCLR: [property %s]\n",
				 gdk_atom_name(event->selection)));

	// clear the waiting flag in case we are being called
	// because we did a user-clear and released the
	// property (set the owner to null).
	
	m_waiting = UT_FALSE;

	if (event->selection == m_atomClipboard)
	{
		m_fakeClipboard.clearClipboard();
		// assert that we are not the owner of the CLIPBOARD property.
		UT_ASSERT( !_testOwnership(m_atomClipboard) );
		m_bOwnClipboard = UT_FALSE;
		// TODO consider clearing the highlighted selection on screen
		return UT_TRUE;
	}

	if (event->selection == m_atomPrimary)
	{
		m_pUnixApp->clearSelection();
		m_bOwnPrimary = UT_FALSE;
		return UT_TRUE;
	}

	UT_DEBUGMSG(("Clipboard: SELCLR failed -- unknown property\n"));
	return UT_FALSE;
}

void XAP_UnixClipboard::_selrcv(GtkSelectionData *selectionData, guint32 /*time*/, gpointer /*data*/)
{
	// callback
	// The XServer is telling us the answer to a question
	// or request that we made.  Our top-half is sitting
	// in a "while (m_waiting) gtk_main_iteration()" loop.
	// here we need to collect the answer and let the top-half
	// proceed.

	UT_DEBUGMSG(("Clipboard: SELRCV: [property %s][format %s][type %s][pData %p][lenData %d]\n",
				 gdk_atom_name(selectionData->selection),
				 gdk_atom_name(selectionData->target),
				 gdk_atom_name(selectionData->type),
				 selectionData->data,selectionData->length));

	// no matter what we do, we must release the top-half.

	UT_ASSERT(m_waiting);
	m_waiting = UT_FALSE;

	// our processing depends upon the target.
	
	if (selectionData->target == m_atomTargets)
	{
		// we are receiving the answer to the TARGETS request (in _getFormats())

		m_vecFormatsOnServer.clear();

		if ((selectionData->length <= 0) || (!selectionData->data))
			return;
		if (selectionData->type != GDK_SELECTION_TYPE_ATOM)
			return;

		GdkAtom * aAtoms = (GdkAtom *)selectionData->data;
		UT_uint32 count = selectionData->length / sizeof(GdkAtom);

		for (UT_uint32 k=0; k<count; k++)
		{
			UT_DEBUGMSG(("Clipboard: SELRCV: [property %s] has [format %s]\n",
						 gdk_atom_name(selectionData->selection),
						 gdk_atom_name(aAtoms[k])));
			m_vecFormatsOnServer.addItem((void *)(aAtoms[k]));
		}

		return;
	}

	if (selectionData->target == m_atomTimestamp)
	{
		// we are receiving the answer to the TIMESTAMP request (in _getTimeFromServer())

		m_timeOnServer = 0;

		if ((selectionData->length <= 0) || (!selectionData->data))
			return;
		if (selectionData->type != GDK_SELECTION_TYPE_INTEGER)
			return;

		guint32 * p = (guint32 *)selectionData->data;
		m_timeOnServer = *p;

		UT_DEBUGMSG(("Clipboard: SELRCV: [property %s] has timestamp [%08lx]\n",
					 gdk_atom_name(selectionData->selection),m_timeOnServer));
		return;
	}
	
	if (m_bWaitingForDataFromServer)
	{
		UT_DEBUGMSG(("Clipboard: SELRCV: assuming data buffer [length %d]\n",selectionData->length));

		m_bWaitingForDataFromServer = UT_FALSE;
		
		m_databuf.truncate(0);
		m_databuftype = GDK_NONE;
	
		if ((selectionData->length <= 0) || (!selectionData->data))
			return;
		m_databuf.append((UT_Byte *)(selectionData->data),selectionData->length);
		m_databuftype = selectionData->target;

		return;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

GdkAtom XAP_UnixClipboard::_convertFormatString(const char * format)
{
   int kLimit = m_vecFormat_AP_Name.getItemCount();
   int k;
   
   for (k = 0; k < kLimit; k++)
     if (UT_stricmp(format,(const char *)m_vecFormat_AP_Name.getNthItem(k)) == 0)
       return (GdkAtom)m_vecFormat_GdkAtom.getNthItem(k);

   // no matches, so we'll create this new one for them...
   // TODO this is probably unnecessary -- jeff

   GdkAtom new_atom = gdk_atom_intern(format, FALSE);
   m_vecFormat_AP_Name.addItem((void*)format);
   m_vecFormat_GdkAtom.addItem((void*)new_atom);
   return new_atom;
}

const char * XAP_UnixClipboard::_convertToFormatString(GdkAtom fmt) const
{ 
   int kLimit = m_vecFormat_GdkAtom.getItemCount();
   int k;
   
   for (k = 0; k < kLimit; k++)
     if (fmt == (GdkAtom)m_vecFormat_GdkAtom.getNthItem(k))
       return (const char *)m_vecFormat_AP_Name.getNthItem(k);
   
   return NULL;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_UnixClipboard::_getCurrentSelection(const char** formatList,
												void ** ppData, UT_uint32 * pLen,
												const char **pszFormatFound)
{
	// get the contents of the current selection from the view.
	// this implements the copy-on-demand nature of X-Selections.

	UT_DEBUGMSG(("Clipboard::_getCurrentSelection: need current X selection.\n"));

	return m_pUnixApp->getCurrentSelection(formatList,ppData,pLen,pszFormatFound);
}

