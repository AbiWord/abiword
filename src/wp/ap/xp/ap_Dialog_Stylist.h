/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef AP_DIALOG_STYLIST_H
#define AP_DIALOG_STYLIST_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;

class AP_Dialog_Stylist : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Stylist(void);

	virtual void runModeless(XAP_Frame * pFrame) = 0;

	void startUpdater(void);
	void stopUpdater(void);
    void setActiveFrame(XAP_Frame *pFrame);
	void event_update(void);
	void finalize(void);

	static void autoUpdate(UT_Worker * pTimer);

private:

	UT_Timer * m_pAutoUpdater;
};

#endif /* AP_DIALOG_STYLIST_H */
