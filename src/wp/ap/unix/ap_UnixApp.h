/* AbiWord
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

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_UNIXAPP_H
#define AP_UNIXAPP_H

#include "ut_bytebuf.h"
#include "xap_Args.h"
#include "ap_UnixPrefs.h"
#include "ap_UnixClipboard.h"
#include "pt_Types.h"

#ifdef HAVE_GNOME
#include "xap_UnixGnomeApp.h"
#define XAP_UNIXBASEAPP XAP_UnixGnomeApp
#else
#include "xap_UnixApp.h"
#define XAP_UNIXBASEAPP XAP_UnixApp
#endif

class XAP_StringSet;
class AV_View;

class AP_UnixApp : public XAP_UNIXBASEAPP
{
public:
	AP_UnixApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~AP_UnixApp();

	virtual bool					initialize(void);
	virtual XAP_Frame *				newFrame(void);
	virtual bool					forgetFrame(XAP_Frame * pFrame);
	virtual bool					shutdown(void);
	virtual bool					getPrefsValueDirectory(bool bAppSpecific,
									       const XML_Char * szKey, const XML_Char ** pszValue) const;
	virtual const XAP_StringSet *	                getStringSet(void) const;
	virtual const char *			        getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true);
	virtual bool					canPasteFromClipboard(void);
	
	virtual bool					parseCommandLine(void);

	virtual void					setSelectionStatus(AV_View * pView);

	/*!
	  Sets the view selection
	  \param pView The veiw the selection view is to be set to.
	*/
	inline virtual void                             setViewSelection( AV_View * pView)
	{ m_pViewSelection = pView; }

	/*!
	  Gets the View Selection
	  \return The View currently selected.
	*/
	inline virtual AV_View *                        getViewSelection(void)
	{ return m_pViewSelection; }
	virtual void					clearSelection(void);
	virtual bool					getCurrentSelection(const char** formatList,
														void ** ppData, UT_uint32 * pLen,
														const char **pszFormatFound);
	virtual void					cacheCurrentSelection(AV_View *);

	static int main (const char * szAppName, int argc, char ** argv);

	void							catchSignals(int sig_num);
#ifdef ABI_OPT_PERL
	virtual void					perlEvalFile(const UT_String &filename);
#endif

protected:	// JCA: Why in the hell we have so many (any) protected variable?

	static GR_Image *                                       _showSplash(UT_uint32);

	XAP_StringSet *			m_pStringSet;
	AP_UnixClipboard *		m_pClipboard;

	bool					m_bHasSelection;
	bool					m_bSelectionInFlux;
	bool					m_cacheDeferClear;
	AV_View *				m_pViewSelection;
	AV_View *				m_cacheSelectionView;
	XAP_Frame *				m_pFrameSelection;
	UT_ByteBuf				m_selectionByteBuf;
	PD_DocumentRange		m_cacheDocumentRangeOfSelection;
};

// HACK What follows is an ugly hack. It is neccessitated by the 
// C/C++ conflict over pointers to member functions. It is,
// however, what the C++ FAQ reccommends.

void signalWrapper(int);

#endif /* AP_UNIXAPP_H */
