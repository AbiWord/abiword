/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019 Hubert Figuière
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

#pragma once

#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ap_App.h"
#include "ap_Args.h"
#include "ap_UnixPrefs.h"
#include "ap_UnixClipboard.h"
#include "pt_Types.h"

//#define LOGFILE
/* Define if your user name is msevior */
/* #undef LOGFILE */

#ifdef LOGFILE
FILE * getlogfile(void);
#endif

class XAP_StringSet;
class AV_View;
class GR_Image;
class AP_Args;
class AP_BuiltinStringSet;
class AP_DiskStringSet;

class ABI_EXPORT AP_UnixApp : public AP_App
{
public:
	AP_UnixApp(const char * szAppName);
	virtual ~AP_UnixApp();

	virtual bool					initialize(bool has_display);
	virtual XAP_Frame *				newFrame(void) override;
	virtual bool					forgetFrame(XAP_Frame * pFrame) override;
	virtual GR_Graphics *           newDefaultScreenGraphics() const override;

	virtual bool					shutdown(void);
	virtual bool getPrefsValueDirectory(bool bAppSpecific, const std::string& key,
										std::string& value) const;
	virtual const XAP_StringSet *	                getStringSet(void) const override;
	virtual const char *			        getAbiSuiteAppDir(void) const override;
	virtual const std::string&			getAbiSuiteAppUIDir(void) const override;

	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) override;
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) override;
	virtual bool canPasteFromClipboard(void) const override;
	virtual void					addClipboardFmt (const char * szFormat) override {m_pClipboard->addFormat(szFormat);}
	virtual void					deleteClipboardFmt (const char * szFormat) override {m_pClipboard->deleteFormat(szFormat);}

	virtual void					setSelectionStatus(AV_View * pView) override;

	/*!
	  Sets the view selection
	  \param pView The veiw the selection view is to be set to.
	*/
	inline virtual void                             setViewSelection( AV_View * pView) override
	{ m_pViewSelection = pView; }

	/*!
	  Gets the View Selection
	  \return The View currently selected.
	*/
	inline virtual AV_View* getViewSelection(void) const override
	{ return m_pViewSelection; }
	virtual void					clearSelection(void) override;
	virtual bool					getCurrentSelection(const char** formatList,
														void ** ppData, UT_uint32 * pLen,
														const char **pszFormatFound) override;
	virtual void					cacheCurrentSelection(AV_View *) override;

	static int main (const char * szAppName, int argc, char ** argv);

	virtual void	catchSignals(int sig_num) override ABI_NORETURN;
	void loadAllPlugins ();

	virtual void errorMsgBadFile(XAP_Frame * pFrame, const char * file,
								 UT_Error error) override;
	virtual bool doWindowlessArgs (const AP_Args *, bool & bSuccess) override;
	bool makePngPreview(const char * pszInFile,const char * pszPNGFile,  UT_sint32 iWidth, UT_sint32 iHeight);
	AP_DiskStringSet * loadStringsFromDisk(const char 		   * szStringSet,
										   AP_BuiltinStringSet * pFallbackStringSet);

	virtual XAP_UnixClipboard * getClipboard () override { return m_pClipboard; }

	void _appActivate();
	void _appOpen(GFile* files[], gint n_files);
	void setArgs(std::unique_ptr<AP_Args>&& args)
	{
		m_args = std::move(args);
	}
protected:
	// JCA: Why in the hell we have so many (any) protected
		// variables?
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
private:
	std::unique_ptr<AP_Args> m_args;
};

// HACK What follows is an ugly hack. It is neccessitated by the
// C/C++ conflict over pointers to member functions. It is,
// however, what the C++ FAQ reccommends.

void signalWrapper(int);
