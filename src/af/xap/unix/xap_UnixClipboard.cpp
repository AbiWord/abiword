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


static void targets_selection_received(GtkWidget*,
				       GtkSelectionData*,
#if (GTK_MINOR_VERSION > 0)
				       guint32,
#endif				      
				       gpointer);
static void selection_received(GtkWidget*, 
			       GtkSelectionData*,
#if (GTK_MINOR_VERSION > 0)
			       guint32, 
#endif
			       gpointer);
static gint selection_clear(GtkWidget*,
			    GdkEventSelection*
			    );
static void selection_handler(GtkWidget*,
			      GtkSelectionData*,
			      gpointer);

struct _ClipboardItem
{
   _ClipboardItem(GdkAtom _target, void* _pData, UT_uint32 _iLen);
   ~_ClipboardItem();
   
   GdkAtom target;
   void* pData;
   UT_uint32 iLen;
};

_ClipboardItem::_ClipboardItem(GdkAtom _target, void* _pData, UT_uint32 _iLen)
{
   target = _target;
   pData = new char[_iLen];
   memcpy(pData, _pData, _iLen);
   iLen = _iLen;
}

_ClipboardItem::~_ClipboardItem()
{
   delete pData;
}

XAP_UnixClipboard::XAP_UnixClipboard()
	: XAP_Clipboard()
{
   _init(gdk_atom_intern("CLIPBOARD", FALSE));
}

XAP_UnixClipboard::XAP_UnixClipboard(const char * selection)
	: XAP_Clipboard()
{
   _init(gdk_atom_intern(selection, FALSE));
}

UT_Bool XAP_UnixClipboard::_init(GdkAtom selection)
{
   UT_DEBUGMSG(("initializing clipboard\n"));
   
   m_selection = selection;
   UT_DEBUGMSG(("selection = %lu", m_selection));
   m_received_data = NULL;
   m_ownClipboard = UT_FALSE;
   
   // widgets for signal handling
   // for querying targets
   m_targets_widget = gtk_window_new( GTK_WINDOW_POPUP );
   gtk_widget_realize( m_targets_widget );
   gtk_object_set_data(GTK_OBJECT(m_targets_widget),
		       "clipboard", (gpointer)this);

   // for getting and offering data
   m_data_widget = gtk_window_new( GTK_WINDOW_POPUP );
   gtk_widget_realize( m_data_widget );
   gtk_object_set_data(GTK_OBJECT(m_data_widget),
		       "clipboard", (gpointer)this);
   
   // selection event handlers
   gtk_signal_connect(GTK_OBJECT(m_targets_widget),
		      "selection_received",
		      GTK_SIGNAL_FUNC(targets_selection_received),
		      (gpointer) this);

   gtk_signal_connect(GTK_OBJECT(m_data_widget),
		      "selection_received",
		      GTK_SIGNAL_FUNC(selection_received),
		      (gpointer) this);

   gtk_signal_connect(GTK_OBJECT(m_data_widget),
		      "selection_clear_event",
		      GTK_SIGNAL_FUNC(selection_clear),
		      (gpointer) this);

   return UT_TRUE;
}

XAP_UnixClipboard::~XAP_UnixClipboard()
{
   UT_DEBUGMSG(("destroying clipboard\n"));

   clear();
   if (m_received_data) delete m_received_data;
   if (m_targets_widget) gtk_widget_destroy(m_targets_widget);
   if (m_data_widget) gtk_widget_destroy(m_data_widget);
}

UT_Bool XAP_UnixClipboard::open(void)
{
   UT_DEBUGMSG(("opening clipboard\n"));
   
   if (m_bOpen) return UT_FALSE;
   return m_bOpen = UT_TRUE;
}

UT_Bool XAP_UnixClipboard::close(void)
{
   UT_DEBUGMSG(("closing clipboard\n"));
	       
   m_bOpen = UT_FALSE;
   return UT_TRUE;
}

UT_Bool XAP_UnixClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
   UT_DEBUGMSG(("adding data to clipboard\n"));

   // now we add the data to our local list.
   // with selections, the application owning the clipboard
   // holds onto the data until another application requests it...
   
   GdkAtom target = _convertFormatString(format);
   UT_DEBUGMSG(("format atom = %s / %lu\n", format, target));
   
   _ClipboardItem * pItem = new _ClipboardItem(target, pData, iNumBytes);
   
   if (m_vecData.addItem(pItem) < 0)
      return UT_FALSE;

#if (GTK_MINOR_VERSION > 0)
   
   gtk_selection_add_target(m_data_widget, m_selection, target, 0);

   gtk_signal_connect(GTK_OBJECT(m_data_widget),
		      "selection_get",
		      GTK_SIGNAL_FUNC(selection_handler),
		      (gpointer)this);
   
#else
   
   gtk_selection_add_handler(m_data_widget,
			     m_selection,
			     target,
			     selection_handler,
			     (gpointer)this);
   
#endif
   
   // take ownership of the clipboard
   if (!m_ownClipboard && !_getClipboard()) return UT_FALSE;
   
   return UT_TRUE;
}

UT_Bool XAP_UnixClipboard::hasFormat(const char* format)
{
   UT_DEBUGMSG(("clipboard: querying format\n"));
   
   // map internal Abi format names to a GdkAtom.
   GdkAtom format_atom = _convertFormatString(format);

   if (!m_ownClipboard) {
      // if we do not have the clipboard, we have to request the current 
      // formats, and wait for the callback to occur, which the following 
      // function does for us.
      _getFormats();

      // now search vector for matching format
      UT_sint32 iCount = m_vecFormatAtoms.getItemCount();
      for (int i = 0; i < iCount; i++) {
	 if ((GdkAtom)m_vecFormatAtoms.getNthItem(i) == format_atom) 
	   return UT_TRUE;
      }
      return UT_FALSE;
   }

   // otherwise, we can just look at our local clipboard...
   UT_sint32 iCount = m_vecData.getItemCount();
   for (int i = 0; i < iCount; i++) {
      _ClipboardItem* pItem = (_ClipboardItem*)m_vecData.getNthItem(i);
      if (pItem->target == format_atom)
	return UT_TRUE;
   }
   return UT_FALSE;
}

UT_sint32 XAP_UnixClipboard::getDataLen(const char * format)
{
   UT_DEBUGMSG(("clipboard: getDataLen\n"));
   
   // map internal Abi format names to a GdkAtom.
   GdkAtom format_atom = _convertFormatString(format);

   if (!m_ownClipboard) {
      // if we don't own the clipboard, we have to request the data first
      if (!_getData(format_atom)) {
	 // error, probably allocating memory
	 return -1;
      }
      // no matcing format
      if (m_received_data == NULL) return -1;
      // return length
      return m_received_length;
   }

   // otherwise, we can just look at our local clipboard...
   UT_sint32 iCount = m_vecData.getItemCount();
   for (int i = 0; i < iCount; i++) {
      _ClipboardItem* pItem = (_ClipboardItem*)m_vecData.getNthItem(i);
      if (pItem->target == format_atom)
	// return length
	return pItem->iLen;
   }
   // no matcing format
   return -1;
}

UT_Bool XAP_UnixClipboard::getData(const char * format, void* pData)
{
   UT_DEBUGMSG(("clipboard: getData\n"));
   
   // map internal Abi format names to a GdkAtom.
   GdkAtom format_atom = _convertFormatString(format);

   if (!m_ownClipboard) {
      // if we don't own the clipboard, we have to request the data first
      if (!_getData(format_atom)) {
	 // error, probably allocating memory
	 return UT_FALSE;
      }
      // no matcing format
      if (m_received_data == NULL) return UT_FALSE;
      // copy data
      memcpy(pData, m_received_data, m_received_length);
      return UT_TRUE;
   }

   // otherwise, we can just look at our local clipboard...
   UT_sint32 iCount = m_vecData.getItemCount();
   for (int i = 0; i < iCount; i++) {
      _ClipboardItem* pItem = (_ClipboardItem*)m_vecData.getNthItem(i);
      if (pItem->target == format_atom) {
	 memcpy(pData, pItem->pData, pItem->iLen);
	 return UT_TRUE;
      }
   }
   return UT_FALSE;
}

UT_sint32 XAP_UnixClipboard::countFormats(void)
{
   UT_DEBUGMSG(("clipboard: format count\n"));
   
   if (!m_ownClipboard) {
      // if we do not have the clipboard, we have to request the current 
      // formats, and wait for the callback to occur, which the following 
      // function does for us.
      _getFormats();
      return m_vecFormatAtoms.getItemCount();
   }
   
   return m_vecData.getItemCount();
}

const char * XAP_UnixClipboard::getNthFormat(UT_sint32 n)
{
   UT_DEBUGMSG(("clipboard: getting %ld format\n", n));
   
   if (!m_ownClipboard) {
      // if we do not have the clipboard, we have to request the current 
      // formats, and wait for the callback to occur, which the following 
      // function does for us.
      _getFormats();
      return _convertToFormatString((GdkAtom)m_vecFormatAtoms.getNthItem(n));
   }
   
   return _convertToFormatString((GdkAtom)m_vecData.getNthItem(n));
}

UT_Bool XAP_UnixClipboard::clear(void)
{
   UT_DEBUGMSG(("clipboard: clearing clipboard.\n"));
   
   UT_sint32 iCount = m_vecData.getItemCount();
   for (int i=0; i<iCount; i++) {
      _ClipboardItem* pItem = (_ClipboardItem*) m_vecData.getNthItem(i);
      UT_ASSERT(pItem);
      delete pItem;
   }
   m_vecData.clear();
   
   if (m_ownClipboard) _releaseClipboard();
   
   return UT_TRUE;
}

GR_Image * XAP_UnixClipboard::getImage(void)
{
   UT_ASSERT(UT_TODO);
   return NULL;
}

UT_Bool XAP_UnixClipboard::addImage(GR_Image*)
{
   UT_ASSERT(UT_TODO);
   return UT_FALSE;
}

GdkAtom XAP_UnixClipboard::_convertFormatString(const char * format)
{
   int kLimit = m_vecFormat.getItemCount();
   int k;
   
   for (k = 0; k < kLimit; k++)
     if (UT_stricmp(format,(const char *)m_vecFormat.getNthItem(k)) == 0)
       return (GdkAtom)m_vecCF.getNthItem(k);

   // no matches, so we'll create this new one for them...
   GdkAtom new_atom = gdk_atom_intern(format, FALSE);
   m_vecFormat.addItem((void*)format);
   m_vecCF.addItem((void*)new_atom);
   return new_atom;
}

const char * XAP_UnixClipboard::_convertToFormatString(GdkAtom fmt)
{ 
   int kLimit = m_vecFormat.getItemCount();
   int k;
   
   for (k = 0; k < kLimit; k++)
     if (fmt == (GdkAtom)m_vecCF.getNthItem(k))
       return (const char *)m_vecFormat.getNthItem(k);
   
   return NULL;
}

UT_Bool XAP_UnixClipboard::_getClipboard(void)
{
   UT_DEBUGMSG(("getting clipboard\n"));
   
   if (gdk_selection_owner_get(m_selection) != m_data_widget->window) {
      if (!gtk_selection_owner_set(m_data_widget, m_selection,
				   GDK_CURRENT_TIME))
	{
	   return UT_FALSE;
	}
   }
   return m_ownClipboard = UT_TRUE;
}

void XAP_UnixClipboard::_releaseClipboard(void)
{
   UT_DEBUGMSG(("releasing clipboard\n", m_selection));

   m_ownClipboard = UT_FALSE;
   if (gdk_selection_owner_get(m_selection) == m_data_widget->window) {
      m_waiting = UT_TRUE;
      gtk_selection_owner_set(NULL, m_selection, GDK_CURRENT_TIME);
      while (m_waiting) gtk_main_iteration();
   }
}

UT_Bool XAP_UnixClipboard::_getFormats(void)
{
   UT_DEBUGMSG(("getting formats...\n"));

   // get TARGETS atom (which contains the format atoms).
   // this bit of code is to avoid looking this up every time.
   static GdkAtom targets_atom = GDK_NONE;
   if (targets_atom == GDK_NONE) 
     targets_atom = gdk_atom_intern("TARGETS", FALSE);
   
   // selections in X are asynchronous, so we'll have to block
   
   m_waiting = UT_TRUE;
   m_error = UT_FALSE;

   gtk_selection_convert(m_targets_widget, m_selection, targets_atom, 
			 GDK_CURRENT_TIME);

   while (m_waiting) gtk_main_iteration();

   if (m_error) return UT_FALSE;
   else return UT_TRUE;
}

UT_Bool XAP_UnixClipboard::_getData(GdkAtom target)
{
   UT_DEBUGMSG(("getting data...\n"));

   // selections in X are asynchronous, so we'll have to block
   m_waiting = UT_TRUE;
   m_error = UT_FALSE;
   
   gtk_selection_convert(m_data_widget, m_selection, target, GDK_CURRENT_TIME);
   
   while (m_waiting) gtk_main_iteration();
   
   if (m_error) return UT_FALSE;
   else return UT_TRUE;
}

static void 
targets_selection_received(GtkWidget *widget,
			   GtkSelectionData *selection_data,
#if (GTK_MINOR_VERSION > 0)
			   guint32 time,
#endif			   
			   gpointer data)
{
   UT_DEBUGMSG(("formats received.\n"));
   
   // get a reference to our clipboard out of the widget
   XAP_UnixClipboard *clipboard = (XAP_UnixClipboard*)
     gtk_object_get_data(GTK_OBJECT(widget), "clipboard");

   clipboard->m_vecFormatAtoms.clear();
   
   if (selection_data->length <= 0) {
      clipboard->m_waiting = UT_FALSE;
      return;
   }
   
   if (selection_data->type != GDK_SELECTION_TYPE_ATOM) {
      clipboard->m_waiting = UT_FALSE;
      return;
   }
   
   GdkAtom *atoms = (GdkAtom *)selection_data->data;

   for (unsigned int i=0; i<selection_data->length/sizeof(GdkAtom); i++)
     {
	// push each atom onto our current format list
	if (clipboard->m_vecFormatAtoms.addItem((void*)(atoms[i])) < 0) {
	   clipboard->m_error = UT_TRUE;
	   clipboard->m_waiting = UT_FALSE;
	   return;
	}
     }
   
   clipboard->m_waiting = UT_FALSE;
   return;
}

static void selection_received(GtkWidget *widget,
			       GtkSelectionData *selection_data,
#if (GTK_MINOR_VERSION > 0)
			       guint32 time,
#endif			       
			       gpointer data)
{
   UT_DEBUGMSG(("selection received.\n"));

   // get a reference to our clipboard out of the widget
   XAP_UnixClipboard *clipboard = (XAP_UnixClipboard*)
     gtk_object_get_data(GTK_OBJECT(widget), "clipboard");

   if (clipboard->m_received_data) {
      delete clipboard->m_received_data;
      clipboard->m_received_data = NULL;
   }
   
   if (selection_data->length <= 0) {
      clipboard->m_waiting = FALSE;
      return;
   }
 
   clipboard->m_received_format = selection_data->type;
   clipboard->m_received_data = new char[selection_data->length];
   memcpy(clipboard->m_received_data, selection_data->data,
	  selection_data->length);
   clipboard->m_received_length = selection_data->length;
   
   clipboard->m_waiting = FALSE;
}

static gint selection_clear(GtkWidget *widget,
			    GdkEventSelection *event)
{
   UT_DEBUGMSG(("clipboard: lost ownership.\n"));
   
   // get a reference to our clipboard out of the widget
   XAP_UnixClipboard *clipboard = (XAP_UnixClipboard*)
     gtk_object_get_data(GTK_OBJECT(widget), "clipboard");
   
   if (event->selection == clipboard->m_selection) {

      if (clipboard->m_ownClipboard) {
	 clipboard->m_ownClipboard = UT_FALSE;
	 clipboard->clear();
      }
      clipboard->m_waiting = UT_FALSE;
      return TRUE;

   } else {
      
      clipboard->m_waiting = UT_FALSE;
      return FALSE;
      
   }
}

static void selection_handler(GtkWidget *widget,
			      GtkSelectionData *selection_data,
			      gpointer data)
{
   UT_DEBUGMSG(("selection requested.\n"));
   
   // get a reference to our clipboard out of the widget
   XAP_UnixClipboard *clipboard = (XAP_UnixClipboard*)
     gtk_object_get_data(GTK_OBJECT(widget), "clipboard");
   
   _ClipboardItem *pItem;
   UT_sint32 iCount = clipboard->m_vecData.getItemCount();
   for (int i = 0; i < iCount; i++) {
      pItem = (_ClipboardItem*)clipboard->m_vecData.getNthItem(i);
      if (pItem->target == selection_data->target) {
	 // do I need to copy the data and pass it?
	 gtk_selection_data_set(selection_data,
				pItem->target,
				8, (guchar*)pItem->pData, pItem->iLen);
	 return;
      }
   }
}

