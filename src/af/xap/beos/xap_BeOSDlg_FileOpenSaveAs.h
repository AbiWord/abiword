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

#ifndef AP_BEOSDIALOG_FILEOPENSAVEAS_H
#define AP_BEOSDIALOG_FILEOPENSAVEAS_H

#include "xap_Dlg_FileOpenSaveAs.h"
#include "ut_string.h"

class XAP_BeOSFrame;
class DLGHandler;

#include <OS.h>
extern sem_id					sync_sem;


/*****************************************************************/

class XAP_BeOSDialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_BeOSDialog_FileOpenSaveAs(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_BeOSDialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);
	void				SetAnswer(XAP_Dialog_FileOpenSaveAs::tAnswer ans) { 
								m_answer = ans; 
							}
	void				SetPathname(const char *pathname) { 
								UT_cloneString(m_szFinalPathname, pathname);
							}
	
protected:	
	DLGHandler				*m_pHandler;
	XAP_BeOSFrame 			*m_pBeOSFrame;
	BFilePanel				*m_pOpenPanel, *m_pSavePanel;
};

#endif /* AP_BEOSDIALOG_FILEOPENSAVEAS_H */
