/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Francis James Franklin
 * Copyright (C) 2001-2004, 2009 Hubert Figuiere
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

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_COCOAAPP_H
#define AP_COCOAAPP_H

#include "ut_types.h"
#include "ap_App.h"
#include "ut_bytebuf.h"
#include "xap_CocoaApp.h"
#include "pt_Types.h"


class AP_Args;
class XAP_StringSet;
class AV_View;
class GR_Image;
class AP_CocoaClipboard;

class AP_CocoaApp : public AP_App
{
public:
	AP_CocoaApp(const char * szAppName);

	virtual ~AP_CocoaApp();

	virtual bool					initialize(void);
	virtual void					rebuildMenus(void);
	virtual XAP_Frame *				newFrame(void);
	virtual bool					forgetFrame(XAP_Frame * pFrame);
	virtual bool					shutdown(void);
	virtual bool					getPrefsValueDirectory(bool bAppSpecific, const gchar * szKey, const gchar ** pszValue) const;
	virtual const XAP_StringSet *	getStringSet(void) const;
	virtual const char *			getAbiSuiteAppDir(void) const;
	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true);
	virtual bool					canPasteFromClipboard(void);

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

	void							catchSignals(int sig_num) ABI_NORETURN;

	void loadAllPlugins ();

	virtual void errorMsgBadArg(const char*);
	virtual void errorMsgBadFile(XAP_Frame * pFrame, const char * file,
								 UT_Error error);
	virtual bool doWindowlessArgs (const AP_Args *, bool & bSuccess);
	virtual GR_Graphics * newDefaultScreenGraphics() const
		{ UT_ASSERT(UT_NOT_IMPLEMENTED); return NULL; };

private:	// JCA: Why in the hell we have so many (any) protected variables?
	XAP_StringSet *			m_pStringSet;
	AP_CocoaClipboard *		m_pClipboard;

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

void signalWrapper(int) ABI_NORETURN;

#endif /* AP_COCOAAPP_H */
