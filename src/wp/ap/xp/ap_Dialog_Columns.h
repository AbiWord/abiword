/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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

#pragma once

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "pd_Document.h"


class XAP_Frame;
class AP_Dialog_Columns;

#include "xap_Preview.h"

// this is needed to get the ruler units.
#include "ap_StatusBar.h"
#include "ut_units.h"


class ABI_EXPORT AP_Columns_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect, UT_sint32 iColumns, bool bLines, double maxHeightPercent, double SpaceAfterPercent);
};

class ABI_EXPORT AP_Columns_preview : public XAP_Preview
{
public:

	AP_Columns_preview(GR_Graphics * gc, AP_Dialog_Columns * pColumns);
	virtual ~AP_Columns_preview(void);

	// data twiddlers
	virtual void drawImmediate(const UT_Rect* clip = nullptr) override;

	void			set(UT_uint32 iColumns, bool bLines)
					{
						m_iColumns = iColumns;
						m_bLineBetween = bLines;
						queueDraw();
					}

private:
	AP_Columns_preview_drawer	m_previewDrawer;
	AP_Dialog_Columns *  m_pColumns;
protected:

	UT_uint32		m_iColumns;
	bool			m_bLineBetween;

};

class ABI_EXPORT AP_Dialog_Columns : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Columns(void);

	virtual void runModal(XAP_Frame * pFrame) override = 0;
	virtual void					enableLineBetweenControl(bool bState = true) = 0;


	void							setColumns(UT_uint32 iColumns);

	AP_Columns_preview* getColumnsPreview() const
	{
		return m_pColumnsPreview;
	}

	typedef enum { a_OK, a_CANCEL } tAnswer;
	AP_Dialog_Columns::tAnswer getAnswer(void) const;
	UT_uint32		   getColumns(void) const {return m_iColumns;}

	void			   setLineBetween(bool bState);
	bool			   getLineBetween(void) const { return m_bLineBetween;}
	double             getMaxHeightPercent(void);
	double             getSpaceAfterPercent(void);
	void			   setColumnOrder(UT_uint32 iOrder);
	UT_uint32		   getColumnOrder(void) const {return m_iColumnOrder;}
	const char *       getHeightString(void);
	const char *       getSpaceAfterString(void);
	bool               isSpaceAfterChanged(void) const {return m_bSpaceAfterChanged;}
	bool               isMaxHeightChanged(void) const {return m_bMaxHeightChanged;}

protected:
	void			   _createPreviewFromGC(GR_Graphics * gc,
							UT_uint32 width,
							UT_uint32 height);
	void			   _drawColumnButton(GR_Graphics *gc, UT_Rect rect, UT_uint32 iColumns);
	void			_convertToPreferredUnits(XAP_Frame * pFrame,const
	char *sz, gchar * pRet);
	AP_Dialog_Columns::tAnswer m_answer;
	AP_Columns_preview *	   m_pColumnsPreview;
	AP_Columns_preview_drawer  m_previewDrawer;
	void                       incrementMaxHeight(bool bIncrement);
	void                       incrementSpaceAfter(bool bIncrement);
	double                     getIncrement(const char * sz);
	void                       setViewAndDoc(XAP_Frame * pFrame);
	double                     getPageWidth(void);
	double                     getPageHeight(void);
	void                       setMaxHeight( const char * szHeight);
	void                       setSpaceAfter( const char * szAfter);
private:

	UT_uint32		           m_iColumns;
	bool			           m_bLineBetween;
	UT_uint32		           m_iColumnOrder;
	UT_String                  m_HeightString;
	UT_String                  m_SpaceAfterString;
	PD_Document *              m_pDoc;
	FV_View *                  m_pView;
	bool                       m_bSpaceAfterChanged;
	bool                       m_bMaxHeightChanged;
	double                     m_dMarginTop;
	double                     m_dMarginBottom;
	double                     m_dMarginLeft;
	double                     m_dMarginRight;
};
