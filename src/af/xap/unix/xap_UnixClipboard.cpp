/* AbiSource Application Framework
 * Copyright (c) 1998 AbiSource, Inc.
 * Copyright (c) 2002 Dom Lachowicz
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
#include "xap_UnixClipboard.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

static GtkClipboard* gtkClipboardForTarget(XAP_UnixClipboard::_T_AllowGet get)
{
  switch(get)
    {
    case XAP_UnixClipboard::TAG_ClipboardOnly:
      return gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      break;

    case XAP_UnixClipboard::TAG_PrimaryOnly:
      return gtk_clipboard_get(GDK_SELECTION_PRIMARY);
      break;
    }

  return 0;
}

static AV_View * viewFromApp(XAP_App * pApp)
{
  XAP_Frame * pFrame = pApp->getLastFocussedFrame();
  if ( !pFrame ) 
    return 0 ;

  return pFrame->getCurrentView () ;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_UnixClipboard::XAP_UnixClipboard(XAP_UnixApp * pUnixApp)
  : m_pUnixApp(pUnixApp), m_Targets(0), m_nTargets(0)
{
}

XAP_UnixClipboard::~XAP_UnixClipboard()
{
  // TODO: see if clearData is really needed
  clearData(true,true);  
  g_free(m_Targets);
}

void XAP_UnixClipboard::AddFmt(const char * szFormat)
{
  UT_return_if_fail(szFormat && strlen(szFormat));
  m_vecFormat_AP_Name.addItem((void *) szFormat);
  m_vecFormat_GdkAtom.addItem((void *) gdk_atom_intern(szFormat,FALSE));
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::initialize()
{
  // now setup the primary stuff
  m_nTargets = m_vecFormat_AP_Name.getItemCount();
  m_Targets  = g_new0(GtkTargetEntry, m_nTargets);
  
  for (int k = 0, kLimit = m_nTargets; (k < kLimit); k++)
    {
      GtkTargetEntry * target = &(m_Targets[k]);
      target->target = (char*)m_vecFormat_AP_Name.getNthItem(k);
      target->info = k;
    }
  
  // setup clipboard selection, delay primary until assertSelection()
  gtk_clipboard_set_with_data  (gtkClipboardForTarget(TAG_ClipboardOnly),
				m_Targets,
				m_nTargets,
				s_clipboard_get_func,
				s_clipboard_clear_func,
				this); 
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::common_get_func(GtkClipboard *clipboard,
					GtkSelectionData *selection_data,
					guint info, T_AllowGet which)
{
  AV_View * pView = viewFromApp(m_pUnixApp);
  if (!pView) return; // race condition - have request for data but no view

  XAP_FakeClipboard & which_clip = ( which == TAG_ClipboardOnly ? m_fakeClipboard : m_fakePrimaryClipboard );
  pView->cmdCopy(which == TAG_ClipboardOnly);

  guint ntargets = m_vecFormat_AP_Name.getItemCount();

  if (info < ntargets)
    {
      const gchar * data = 0;
      UT_uint32 data_len = 0 ;
      const gchar * format_name = (const gchar*)m_vecFormat_AP_Name.getNthItem(info);

      if(which_clip.hasFormat(format_name))
	{
	  which_clip.getClipboardData(format_name,(void**)&data,&data_len);	 
	  GdkAtom atom = (GdkAtom)m_vecFormat_GdkAtom.getNthItem(info);
	  gtk_selection_data_set(selection_data,atom,8,(const guchar*)data,data_len);
	}
    }
}

void  XAP_UnixClipboard::primary_clear_func (GtkClipboard *clipboard)
{
}

void  XAP_UnixClipboard::clipboard_clear_func (GtkClipboard *clipboard)
{
}

void XAP_UnixClipboard::primary_get_func(GtkClipboard *clipboard,
					 GtkSelectionData *selection_data,
					 guint info)
{
  common_get_func(clipboard, selection_data, info, TAG_PrimaryOnly);
}

void XAP_UnixClipboard::clipboard_get_func(GtkClipboard *clipboard,
					   GtkSelectionData *selection_data,
					   guint info)
{
  common_get_func(clipboard, selection_data, info, TAG_ClipboardOnly);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_UnixClipboard::assertSelection()
{
  // assert the X-Selection for PRIMARY.
  return ( gtk_clipboard_set_with_data (gtkClipboardForTarget(TAG_PrimaryOnly),
					m_Targets,
					m_nTargets,
					s_primary_get_func,
					s_primary_clear_func,
					this) == TRUE ); 
}

bool XAP_UnixClipboard::addData(T_AllowGet tFrom, const char* format, void* pData, UT_sint32 iNumBytes)
{
  // add this data to our data store
  if( tFrom == TAG_PrimaryOnly )
    {
      if(!m_fakePrimaryClipboard.addData(format,pData,iNumBytes))
	return false;
      
      xxx_UT_DEBUGMSG(("Added %d bytes of '%s' data to PRIMARY selection\n", iNumBytes, format));
      return true;
    }
  else 
    {
      if(!m_fakeClipboard.addData(format,pData,iNumBytes))
	return false;
      
      xxx_UT_DEBUGMSG(("Added %d bytes of '%s' data to CLIPBOARD selection\n", iNumBytes, format));
      return true;
    }
}

void XAP_UnixClipboard::clearData(bool bClipboard, bool bPrimary)
{
  UT_DEBUGMSG(("DOM: clearData: [CLIPBOARD: %d] [PRIMARY: %d]\n", bClipboard, bPrimary));

  // User requested us to clear the clipboard
  if (bClipboard)
    {
      gtk_clipboard_clear (gtkClipboardForTarget (TAG_ClipboardOnly));
      m_fakeClipboard.clearClipboard();
    }
  
  if (bPrimary)
    {
      gtk_clipboard_clear(gtkClipboardForTarget (TAG_PrimaryOnly));
      m_fakePrimaryClipboard.clearClipboard();
    }
}

bool XAP_UnixClipboard::getData(T_AllowGet tFrom, const char** formatList,
				void ** ppData, UT_uint32 * pLen,
				const char **pszFormatFound)
{
  // Fetch data from the clipboard (using the allowable source(s)) in one of
  // the prioritized list of formats.  Return pointe to clipboard's buffer. 

  UT_DEBUGMSG(("DOM: getData attempt\n"));

  *pszFormatFound = NULL;
  *ppData = NULL;
  *pLen = 0;
  
  switch (tFrom)
    {
    case TAG_ClipboardOnly:
      if (ownsClipboard(tFrom))
	return _getDataFromFakeClipboard(tFrom,formatList,ppData,pLen,pszFormatFound);
      else
	return _getDataFromServer(tFrom,formatList,ppData,pLen,pszFormatFound);
      
    case TAG_PrimaryOnly:
      if (ownsClipboard(tFrom))
	return _getDataFromFakeClipboard(tFrom,formatList,ppData,pLen,pszFormatFound);
      else
	return _getDataFromServer(tFrom,formatList,ppData,pLen,pszFormatFound);
      
    default:
      UT_ASSERT_NOT_REACHED();
      return false;
    }
}
	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_UnixClipboard::ownsClipboard(T_AllowGet which)
{
  // TODO: Possibly noticable speedups iff we own the clipboard. Determine how to determine this...
  return false;
}

bool XAP_UnixClipboard::_getDataFromFakeClipboard(T_AllowGet tFrom, const char** formatList,
						  void ** ppData, UT_uint32 * pLen,
						  const char **pszFormatFound)
{
  XAP_FakeClipboard & which_clip = ( tFrom == TAG_ClipboardOnly ? m_fakeClipboard : m_fakePrimaryClipboard );

  for (int k=0; (formatList[k]); k++)
    if (which_clip.getClipboardData(formatList[k],ppData,pLen))
      {
	*pszFormatFound = formatList[k];
	return true;
      }
  
  // should always happen since this is our internal buffer
  UT_ASSERT_NOT_REACHED();
  return false;
}

bool XAP_UnixClipboard::_getDataFromServer(T_AllowGet tFrom, const char** formatList,
					   void ** ppData, UT_uint32 * pLen,
					   const char **pszFormatFound)
{
  // walk desired formats list and find first one that server also has.

  GtkClipboard * clipboard = gtkClipboardForTarget (tFrom);

  UT_Vector atoms ;
  for(int atomCounter = 0; formatList[atomCounter]; atomCounter++)
    {
      atoms.addItem((void *) gdk_atom_intern(formatList[atomCounter],FALSE));
    }
  int len = atoms.size () ;

  for(int i = 0; i < len; i++)
    {
      GdkAtom atom = (GdkAtom)atoms.getNthItem(i);
      GtkSelectionData* selection = gtk_clipboard_wait_for_contents (clipboard, atom);
      if(selection)
	{
	  m_databuf.truncate(0);
	  m_databuf.append((UT_Byte *)selection->data, (UT_uint32)selection->length );
	  *pLen = selection->length;
	  *ppData = (void*)m_databuf.getPointer(0);
	  *pszFormatFound = formatList[i];
	  gtk_selection_data_free(selection);
	  return true;
	}
    }

  xxx_UT_DEBUGMSG(("Clipboard::_getDataFromServer: didn't contain anything in format requested.\n"));
  return false;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
