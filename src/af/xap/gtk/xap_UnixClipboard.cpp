/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <string.h>
#include "xap_UnixClipboard.h"
#include "xap_Frame.h"
#include "xav_View.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

GtkClipboard * XAP_UnixClipboard::gtkClipboardForTarget(XAP_UnixClipboard::_T_AllowGet get)
{
	if (XAP_UnixClipboard::TAG_ClipboardOnly == get)
		return m_clip;
	else if (XAP_UnixClipboard::TAG_PrimaryOnly == get)
		return m_primary;
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
	m_clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	m_primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
}

XAP_UnixClipboard::~XAP_UnixClipboard()
{
	clearData(true,true);
	g_free(m_Targets);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::AddFmt(const char * szFormat)
{
	UT_return_if_fail(szFormat && strlen(szFormat));
	m_vecFormat_AP_Name.addItem(szFormat);
	m_vecFormat_GdkAtom.addItem(gdk_atom_intern(szFormat,FALSE));
}

void XAP_UnixClipboard::deleteFmt(const char * szFormat)
{
	UT_return_if_fail(szFormat && strlen(szFormat));
	UT_sint32 item = m_vecFormat_AP_Name.findItem(szFormat);
	m_vecFormat_AP_Name.deleteNthItem(item);
	m_vecFormat_GdkAtom.findItem(gdk_atom_intern(szFormat,FALSE));
	m_vecFormat_GdkAtom.deleteNthItem(item);
}

void XAP_UnixClipboard::initialize()
{
	m_nTargets = m_vecFormat_AP_Name.getItemCount();
	m_Targets  = g_new0(GtkTargetEntry, m_nTargets);
	
	for (int k = 0, kLimit = m_nTargets; (k < kLimit); k++)
    {
		GtkTargetEntry * target = &(m_Targets[k]);
		target->target = (gchar*)m_vecFormat_AP_Name.getNthItem(k);
		target->info = k;
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixClipboard::common_get_func(GtkClipboard * /*clipboard*/,
										GtkSelectionData *selection_data,
										guint /*info*/, T_AllowGet which)
{
	XAP_FakeClipboard & which_clip = ( which == TAG_ClipboardOnly ? m_fakeClipboard : m_fakePrimaryClipboard );
	
	// if this is for PRIMARY, we need to copy the current selection
	// else this is for CLIPBOARD and the data is already copied; do nothing
	if (which == TAG_PrimaryOnly)
    {
		// will only get the view from the last focussed frame, and not some offscreen
		// (print, format painter) view. this is fine, since we're operating on PRIMARY
		AV_View * pView = viewFromApp(m_pUnixApp);
		if (!pView) 
			return; // race condition - have request for data but no view. fail harmlessly
		pView->cmdCopy(false);
    }
	
	guint ntargets = m_vecFormat_GdkAtom.getItemCount();
	
	GdkAtom needle = gtk_selection_data_get_target(selection_data);
	for (guint i = 0 ; i < ntargets ; i++)
	{
		if (needle == m_vecFormat_GdkAtom.getNthItem(i))
		{
			const gchar * format_name = m_vecFormat_AP_Name.getNthItem(i);
			
			if(which_clip.hasFormat(format_name))
            {
				guchar * data = 0;
				UT_uint32 data_len = 0;

				guchar **pdata = &data;
                which_clip.getClipboardData(format_name,reinterpret_cast<void**>(pdata),&data_len);	 
                gtk_selection_data_set(selection_data,needle,8, data, data_len);
            }
			break; // success or failure, we've found the needle in the haystack so exit the loop
		}  
	}
}

void  XAP_UnixClipboard::primary_clear_func (GtkClipboard * /*clipboard*/)
{
}

void  XAP_UnixClipboard::clipboard_clear_func (GtkClipboard * /*clipboard*/)
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
  return (gtk_clipboard_set_with_data (gtkClipboardForTarget(TAG_PrimaryOnly),
										 m_Targets,
										 m_nTargets,
										 s_primary_get_func,
										 s_primary_clear_func,
										 this) == TRUE); 
}

bool XAP_UnixClipboard::addData(T_AllowGet tFrom, const char* format, const void* pData, UT_sint32 iNumBytes)
{
	if(tFrom == TAG_PrimaryOnly)
		return m_fakePrimaryClipboard.addData(format,pData,iNumBytes);
	else 
    {
		if(!m_fakeClipboard.addData(format,pData,iNumBytes))
			return false;
		
		return true;
    }
}

void XAP_UnixClipboard::finishedAddingData(void)
{
  gtk_clipboard_set_with_data (gtkClipboardForTarget(TAG_ClipboardOnly),
			       m_Targets,
			       m_nTargets,
			       s_clipboard_get_func,
			       s_clipboard_clear_func,
			       this);

  gtk_clipboard_set_can_store (gtkClipboardForTarget(TAG_ClipboardOnly), m_Targets, m_nTargets);
}

void XAP_UnixClipboard::clearData(bool bClipboard, bool bPrimary)
{
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
	// the prioritized list of formats.  Return pointer to clipboard's buffer. 
	*pszFormatFound = NULL;
	*ppData = NULL;
	*pLen = 0;
	if (TAG_ClipboardOnly == tFrom)
		return _getDataFromServer(tFrom,formatList,ppData,pLen,pszFormatFound);
	else if (TAG_PrimaryOnly == tFrom)
        return _getDataFromServer(tFrom,formatList,ppData,pLen,pszFormatFound);
	else
		return false;
}

bool XAP_UnixClipboard::getTextData(T_AllowGet tFrom, void ** ppData, 
									UT_uint32 * pLen)
{
	// start out pessimistic
	*ppData = NULL;
	*pLen = 0;
	
	GtkClipboard * clippy = gtkClipboardForTarget (tFrom);
	
	char * txt = gtk_clipboard_wait_for_text (clippy);
	
	if (!txt)
		return false;
	
	size_t len = strlen (txt);
	if (!len)
		return false;
	
	XAP_FakeClipboard & which_clip = ( tFrom == TAG_ClipboardOnly ? m_fakeClipboard : m_fakePrimaryClipboard );
	
	which_clip.addData("text/plain",txt,len);
	
	g_free (txt);
	
	// ignored
	const char * pszFormatFound = NULL;
	
	static const char * txtFormatList [] = {
		"text/plain",
		0
	};
	
	return _getDataFromFakeClipboard(tFrom, txtFormatList, ppData, pLen, &pszFormatFound);
}
	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

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
	
	// should never happen since this is our internal buffer
	return false;
}


#if DEBUG
static void allTargets(GtkClipboard * /*clipboard*/,
                                                         GdkAtom *atoms,
                                                         gint n_atoms,
                                                         gpointer /*data*/)
{
  gint i= 0;
  UT_DEBUGMSG(("Found %d target atoms \n", n_atoms));
  for(i=0;i<n_atoms;i++)
  {
      GdkAtom Atom = atoms[i];
      UT_DEBUGMSG((" Found Atom %s on clipboard \n", gdk_atom_name(Atom)));
  }
}
#endif

bool XAP_UnixClipboard::_getDataFromServer(T_AllowGet tFrom, const char** formatList,
										   void ** ppData, UT_uint32 * pLen,
										   const char **pszFormatFound)
{
	bool rval = false;
	if(formatList == NULL)
	  return false;

	GtkClipboard * clipboard = gtkClipboardForTarget (tFrom);
#if DEBUG
	gtk_clipboard_request_targets(clipboard,( GtkClipboardTargetsReceivedFunc) allTargets, this);
#endif
	UT_GenericVector<GdkAtom> atoms ;
	for(int atomCounter = 0; formatList[atomCounter]; atomCounter++)
		atoms.addItem(gdk_atom_intern(formatList[atomCounter],FALSE));
	
	int len = atoms.size () ;	
	
	//	for(int i = 0; i < len && !rval; i++)
	for(int i = 0; i < len; i++)
    {
		GdkAtom atom = atoms.getNthItem(i);
		GtkSelectionData * selection = gtk_clipboard_wait_for_contents (clipboard, atom);
		UT_DEBUGMSG(("Looking for %s on clipbaord \n",formatList[i]));
		if(selection)
		{
			if (gtk_selection_data_get_data(selection) && (gtk_selection_data_get_length(selection) > 0))
			{
			  if(!rval)
			    {
				m_databuf.truncate(0);
				*pLen = gtk_selection_data_get_length(selection);
				m_databuf.append(static_cast<const UT_Byte *>(gtk_selection_data_get_data(selection)),
				                 *pLen);
				*ppData = (void *)(m_databuf.getPointer(0));
				*pszFormatFound = formatList[i];
				rval = true;
			    }
			  UT_DEBUGMSG(("Found format %s on clipbaord \n",formatList[i]));
			}
			gtk_selection_data_free(selection);
		}
    }

	return rval;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_UnixClipboard::canPaste(T_AllowGet tFrom)
{
#if 0
	bool found = false;
	GtkClipboard * clipboard = gtkClipboardForTarget(tFrom);
	GtkSelectionData * selection = gtk_clipboard_wait_for_contents (clipboard, gdk_atom_intern("TARGETS", FALSE));
	
	if (selection) 
    {
		gint abi_targets = m_vecFormat_GdkAtom.getItemCount();
		
		GdkAtom *targets;
		gint clipboard_targets;
		
		if (gtk_selection_data_get_targets (selection, &targets, &clipboard_targets))
		{
			for (gint i = 0; (i < abi_targets) && !found; i++)
			{
				GdkAtom needle = m_vecFormat_GdkAtom.getNthItem(i);
				for (gint j = 0; (j < clipboard_targets) && !found; j++)
					if (targets[j] == needle)
						found = true; 
			}
			
			g_free (targets);
		}
		gtk_selection_data_free(selection);
    }
	
	return found;
#else
	UT_UNUSED(tFrom);
	return true;
#endif
}
