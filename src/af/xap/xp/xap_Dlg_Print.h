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

#ifndef AP_DIALOG_PRINT_H
#define AP_DIALOG_PRINT_H

#include "xap_Dialog.h"
class DG_Graphics;

/*****************************************************************/

class AP_Dialog_Print : public AP_Dialog_AppPersistent
{
public:
	AP_Dialog_Print(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_Print(void);

	virtual void					useStart(void);
	virtual void					runModal(AP_Frame * pFrame) = 0;
	virtual void					useEnd(void);

	typedef enum { a_VOID, a_OK, a_CANCEL }	tAnswer;
	
	void							setDocumentTitle(const char *);
	void							setDocumentPathname(const char * );
	void							setEnablePageRangeButton(UT_Bool bEnable,
															 UT_uint32 nFirst,
															 UT_uint32 nLast);
	void							setEnablePrintSelection(UT_Bool bEnable);
	void							setEnablePrintToFile(UT_Bool bEnable);
	void							setTryToBypassActualDialog(UT_Bool bTry);

	AP_Dialog_Print::tAnswer		getAnswer(void) const;
	UT_Bool							getDoPrintRange(UT_uint32 * pnFirst, UT_uint32 * pnLast) const;
	UT_Bool							getDoPrintSelection(void) const;
	UT_Bool							getDoPrintToFile(const char *) const;
	UT_uint32						getNrCopies(void) const;
	UT_Bool							getCollate(void) const;

	virtual DG_Graphics *			getPrinterGraphicsContext(void) = 0;
	virtual void					releasePrinterGraphicsContext(DG_Graphics * pGraphics) = 0;
	
protected:
	UT_Bool							_getPrintToFilePathname(AP_Frame * pFrame,
															const char * szSuggestedName);
	
	UT_uint32						m_bPersistValid;		/* persists (internal) */
	UT_uint32						m_persistNrCopies;		/* persists (internal) */
	UT_Bool							m_persistCollate;		/* persists (internal) */
	UT_Bool							m_persistPrintToFile;	/* persists (internal) */

	char *							m_szDocumentTitle;		/* input */
	char *							m_szDocumentPathname;	/* input */
	UT_Bool							m_bBypassActualDialog;	/* input */
	UT_Bool							m_bEnablePageRange;		/* input */
	UT_Bool							m_bEnablePrintSelection;/* input */
	UT_Bool							m_bEnablePrintToFile;	/* input */
	UT_Bool							m_bDoPrintRange;		/* output */
	UT_Bool							m_bDoPrintSelection;	/* output */
	UT_Bool							m_bDoPrintToFile;		/* output */
	UT_Bool							m_bCollate;				/* output */
	UT_uint32						m_nFirstPage;			/* input/output */
	UT_uint32						m_nLastPage;			/* input/output */
	UT_uint32						m_nCopies;				/* output */
	AP_Dialog_Print::tAnswer		m_answer;				/* output */
	char *							m_szPrintToFilePathname;/* output */
	char *							m_szPrintCommand;		/* output */
};

#endif /* AP_DIALOG_PRINT_H */
