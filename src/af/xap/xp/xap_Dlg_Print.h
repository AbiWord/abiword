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

#ifndef XAP_DIALOG_PRINT_H
#define XAP_DIALOG_PRINT_H

#include "xap_Dialog.h"
#include "gr_Graphics.h"

/*****************************************************************/

class XAP_Dialog_Print : public XAP_Dialog_AppPersistent
{
public:
	XAP_Dialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Print(void);

	virtual void					useStart(void);
	virtual void					runModal(XAP_Frame * pFrame) = 0;
	virtual void					useEnd(void);

	typedef enum { a_VOID, a_OK, a_CANCEL }	tAnswer;
	
	void							setDocumentTitle(const char *);
	void							setDocumentPathname(const char * );
	void							setEnablePageRangeButton(bool bEnable,
															 UT_uint32 nFirst,
															 UT_uint32 nLast);
	void							setEnablePrintSelection(bool bEnable);
	void							setEnablePrintToFile(bool bEnable);
	void							setTryToBypassActualDialog(bool bTry);
	void setPaperSize (const char * pageSize);


	XAP_Dialog_Print::tAnswer		getAnswer(void) const;
	bool							getDoPrintRange(UT_uint32 * pnFirst, UT_uint32 * pnLast) const;
	bool							getDoPrintSelection(void) const;
	bool							getDoPrintToFile(const char *) const;
	UT_uint32						getNrCopies(void) const;
	bool							getCollate(void) const;
	GR_Graphics::ColorSpace			getColorSpace(void) const;
	
	virtual GR_Graphics *			getPrinterGraphicsContext(void) = 0;
	virtual void					releasePrinterGraphicsContext(GR_Graphics * pGraphics) = 0;
	
protected:
	bool							_getPrintToFilePathname(XAP_Frame * pFrame,
															const char * szSuggestedName);
	
	UT_uint32						m_bPersistValid;		/* persists (internal) */
	UT_uint32						m_persistNrCopies;		/* persists (internal) */
	bool							m_persistCollate;		/* persists (internal) */
	GR_Graphics::ColorSpace			m_persistColorSpace;	/* persists (internal) */
	bool							m_persistPrintToFile;	/* persists (internal) */

	char *							m_szDocumentTitle;		/* input */
	char *							m_szDocumentPathname;	/* input */
	bool							m_bBypassActualDialog;	/* input */
	bool							m_bEnablePageRange;		/* input */
	bool							m_bEnablePrintSelection;/* input */
	bool							m_bEnablePrintToFile;	/* input */
	bool							m_bDoPrintRange;		/* output */
	bool							m_bDoPrintSelection;	/* output */
	bool							m_bDoPrintToFile;		/* output */
	bool							m_bCollate;				/* output */
	GR_Graphics::ColorSpace			m_cColorSpace;			/* output */
	UT_uint32						m_nFirstPage;			/* input/output */
	UT_uint32						m_nLastPage;			/* input/output */
	UT_uint32						m_nCopies;				/* output */
	XAP_Dialog_Print::tAnswer		m_answer;				/* output */
	char *							m_szPrintToFilePathname;/* output */
	char *							m_szPrintCommand;		/* output */

	char * m_pageSize;
};

#endif /* XAP_DIALOG_PRINT_H */
