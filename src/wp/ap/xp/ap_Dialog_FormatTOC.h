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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
class PP_AttrProp;

class ABI_EXPORT AP_Dialog_FormatTOC : public XAP_Dialog_Modeless
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
	std::string     getNewStyle(const std::string & sProp) const;
	bool              setPropFromDoc(const char * szProp);
	void              setTOCProperty(const std::string & sProp, 
					 const std::string & sVal);
	void              setTOCProperty(const char * szProp, const char * szVal);
	void              fillTOCPropsFromDoc(void);
	void              applyTOCPropsToDoc(void);
	std::string     getTOCPropVal(const std::string & sProp) const;
	std::string     getTOCPropVal(const char * szProp) const;
	std::string     getTOCPropVal(const char * szProp,UT_sint32 i) const;
	const UT_GenericVector<const gchar*> *       getVecTABLeadersLabel(void)
		{ return & m_vecTABLeadersLabel;}
	const UT_GenericVector<const gchar*> *       getVecTABLeadersProp(void)
		{ return & m_vecTABLeadersProp;}
	void              incrementStartAt(UT_sint32 iLevel, bool bInc);
    double            getIncrement(const char * sz);
    void              incrementIndent(UT_sint32 iLevel, bool bInc);

	void					setDetailsLevel(UT_sint32 v)
		{ m_iDetailsLevel = v; }
	UT_sint32               getDetailsLevel(void)
		{ return m_iDetailsLevel;}
	void					setMainLevel(UT_sint32 v)
		{ m_iMainLevel = v; }
	UT_sint32               getMainLevel(void)
		{ return m_iMainLevel;}
private:
	PD_Document *         m_pDoc;
	UT_Timer *            m_pAutoUpdater;
	UT_uint32             m_iTick;
	const PP_AttrProp *   m_pAP;
	bool                  m_bTOCFilled;
	std::string           m_sTOCProps;
	UT_GenericVector<const gchar*> m_vecTABLeadersLabel;
	UT_GenericVector<const gchar*> m_vecTABLeadersProp;
	UT_sint32   m_iMainLevel;
	UT_sint32   m_iDetailsLevel;
};

#endif /* AP_DIALOG_FORMATTOC_H */
