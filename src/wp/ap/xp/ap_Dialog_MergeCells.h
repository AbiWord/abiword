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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_MERGECELLS_H
#define AP_DIALOG_MERGECELLS_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "pt_Types.h"

class UT_Timer;
class XAP_Frame;
class fp_TableContainer;

class ABI_EXPORT AP_Dialog_MergeCells : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_MergeCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_MergeCells(void);

	virtual void					runModeless(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { radio_left, radio_right, radio_above, radio_below } mergeWithCell;

	AP_Dialog_MergeCells::tAnswer		getAnswer(void) const;
	PT_DocPosition						getCellSource(void);
	PT_DocPosition						getCellDestination(void);
	virtual void                        startUpdater(void);
	virtual void                        stopUpdater(void);
	static void                         autoUpdateMC(UT_Worker * pTimer);
	virtual void                        setSensitivity( mergeWithCell mergeThis, bool bSens) = 0;
    virtual void                        setActiveFrame(XAP_Frame *pFrame);
	void                                ConstructWindowName(void);
	void                                setAllSensitivities(void);
	void                                event_update(void);
	void                                finalize(void);
	void                                setMergeType( mergeWithCell iMergeType);
	void                                onMerge(void);

protected:
	AP_Dialog_MergeCells::tAnswer		m_answer;
	char                                m_WindowName[100];
private:
	void                                _generateSrcDest(void);

	PT_DocPosition                      m_iCellSource;
	PT_DocPosition                      m_iCellDestination;
	AP_Dialog_MergeCells::mergeWithCell m_mergeType;
	UT_sint32                           m_iLeft;
	UT_sint32                           m_iRight;
	UT_sint32                           m_iTop;
	UT_sint32                           m_iBot;
	UT_sint32                           m_iNumRows;
	UT_sint32                           m_iNumCols;
	fp_TableContainer *                 m_pTab;
	UT_Timer *                         m_pAutoUpdaterMC;
	// Handshake variables
	bool m_bDestroy_says_stopupdating;
	bool m_bAutoUpdate_happening_now;

};

#endif /* AP_DIALOG_MERGECELLS_H */
