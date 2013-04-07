/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#ifndef XAP_DIALOG_DOCCOMPARISON_H
#define XAP_DIALOG_DOCCOMPARISON_H

#include "xap_Dialog.h"
#include "ut_vector.h"
#include <time.h>

/*
    This dialogue displays information about results of document
    comparison; it takes no user input what so ever (except for
    closing it, of course).

    The dialogue consists of three segements (see PNG attached to
    commit message of Feb 7, 2004)

    Buttons: there is one button which should be localised by
             getButtonLabel().

    Header section: the upper part of the dialogue inside of a frame
             with a title getFrame1Label(); the frame contains two
             static controls that are to be filled with getPath1() and
             getPath1().

    Result section: a frame with a title getFrame2Label(), which
             contains getResultCount() pairs of static controls, each
             pair on one line, filled by getResultLabel(n) and
             getResultValue(n).

    Title for the dialogue window is obtained by getWindowLabel().
 */

class XAP_Frame;
class AD_Document;
class XAP_StringSet;

const UT_uint32 iResultCount = 4;

class ABI_EXPORT XAP_Dialog_DocComparison : public XAP_Dialog_NonPersistent
{
  public:
	XAP_Dialog_DocComparison(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	virtual ~XAP_Dialog_DocComparison(void) {};

	virtual void runModal(XAP_Frame * pFrame) = 0;
	bool         calculate(AD_Document * pDoc1, AD_Document * pDoc2);

	const char * getWindowLabel() const;
	const char * getButtonLabel() const;
	const char * getFrame1Label() const;
	const char * getFrame2Label() const;

	char *       getPath1() const;
	char *       getPath2() const;

	UT_uint32    getResultCount() const {return iResultCount;}
	const char * getResultLabel(UT_uint32 n) const;
	char *       getResultValue(UT_uint32 n) const;

  private:
	const AD_Document *   m_pDoc1;
	const AD_Document *   m_pDoc2;
	const XAP_StringSet * m_pSS;

	UT_uint32             m_iVersionOfDiff;
	time_t                m_tTimeOfDiff;
	UT_uint32             m_iPosOfDiff;
	UT_uint32             m_iPosOfFmtDiff;
	bool                  m_bStylesEqual;
};

#endif /* XAP_DIALOG_DOCCOMPARISON_H */
