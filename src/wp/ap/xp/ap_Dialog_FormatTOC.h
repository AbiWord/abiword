/* AbiWord
 * Copyright (C) 2004 Martin Sevior
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

#ifndef AP_DIALOG_FORMATTOC_H
#define AP_DIALOG_FORMATTOC_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_vector.h"
#include "ut_string_class.h"

class UT_Timer;
class XAP_Frame;
class PD_Document;
class PD_Style;

class AP_Dialog_FormatTOC : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatTOC(void);

	virtual void runModeless(XAP_Frame * pFrame) = 0;

	void              startUpdater(void);
	void              stopUpdater(void);
    void              setActiveFrame(XAP_Frame *pFrame);
	void              event_update(void);
	void              finalize(void);
	void              Apply(void);
	virtual void      setTOCPropsInGUI(void) = 0;
    virtual void      setSensitivity(bool bSensitive) = 0;       
	static void       autoUpdate(UT_Worker * pTimer);
	void              updateDialog(void);
	void              setTOCProperty(UT_UTF8String & sProp, UT_UTF8String & sVal);
	void              fillTOCPropsFromDoc(void);
	void              applyTOCPropsToDoc(void);
    UT_UTF8String     getTOCPropVal(UT_UTF8String & sProp);
	UT_Vector *       getVecTABLeadersLabel(void)
		{ return & m_vecTABLeadersLabel;}
	UT_Vector *       getVecTABLeadersProp(void)
		{ return & m_vecTABLeadersProp;}

private:
	PD_Document *         m_pDoc;
	UT_Timer *            m_pAutoUpdater;
	UT_uint32             m_iTick;
	UT_UTF8String         m_sTOCProps;
	UT_Vector             m_vecTABLeadersLabel;
	UT_Vector             m_vecTABLeadersProp;
};

#endif /* AP_DIALOG_FORMATTOC_H */
