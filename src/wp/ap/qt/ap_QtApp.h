/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2013 Hubert Figuiere
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

#ifndef __AP_QT_APP_H__
#define __AP_QT_APP_H__

#include "ut_types.h"
#include "ap_App.h"

class AP_QtClipboard;
class AP_DiskStringSet;
class AP_BuiltinStringSet;

class ABI_EXPORT AP_QtApp
	: public AP_App
{
public:
	AP_QtApp(const char * szAppName);
	virtual ~AP_QtApp();

	virtual bool                                    initialize(bool has_display);
	virtual bool					shutdown(void);

	virtual XAP_Frame *				newFrame(void);
	virtual GR_Graphics *           newDefaultScreenGraphics() const;

	virtual const XAP_StringSet *	                getStringSet(void) const;
	virtual const char *			        getAbiSuiteAppDir(void) const;

	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true);
	virtual bool					canPasteFromClipboard(void);

	virtual void					cacheCurrentSelection(AV_View *);

	virtual bool 					doWindowlessArgs (const AP_Args *, bool & bSuccess);

	static int main (const char * szAppName, int argc, char ** argv);

	void							catchSignals(int sig_num) ABI_NORETURN;

private:

	AP_DiskStringSet * loadStringsFromDisk(const char 		   * szStringSet,
										   AP_BuiltinStringSet * pFallbackStringSet);

	XAP_StringSet*			m_pStringSet;
	AP_QtClipboard*         m_pClipboard;
};

#endif
