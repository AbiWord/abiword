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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_Dialog_Columns_H
#define AP_Dialog_Columns_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"

class XAP_Frame;

#include "xap_Preview.h"

class AP_Columns_preview_drawer
{
public:

	void			draw(GR_Graphics *gc, UT_Rect &rect, UT_sint32 iColumns, UT_Bool bLines);
};

class AP_Columns_preview : public XAP_Preview
{
public:

	AP_Columns_preview(GR_Graphics * gc);
	virtual ~AP_Columns_preview(void);
				
	// data twiddlers
	void			draw(void);

	void			set(UT_uint32 iColumns, UT_Bool bLines)
					{
						m_iColumns = iColumns;
						m_bLineBetween = bLines;
						draw();
					}

private:
	AP_Columns_preview_drawer	m_previewDrawer;

protected:

	UT_uint32		m_iColumns;
	UT_Bool			m_bLineBetween;


};

class AP_Dialog_Columns : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Columns(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	virtual void					enableLineBetweenControl(UT_Bool bState = UT_TRUE) = 0;


	void							setColumns(UT_uint32 iColumns);

	typedef enum { a_OK, a_CANCEL } tAnswer;
	AP_Dialog_Columns::tAnswer		getAnswer(void) const;
	UT_uint32						getColumns(void) const {return m_iColumns;}

	void							setLineBetween(UT_Bool bState);
	UT_Bool							getLineBetween(void) const { return m_bLineBetween;}

	

protected:

	void							_createPreviewFromGC(GR_Graphics * gc,
										   UT_uint32 width,
										   UT_uint32 height);
	void							_drawColumnButton(GR_Graphics *gc, UT_Rect rect, UT_uint32 iColumns);

	AP_Dialog_Columns::tAnswer		m_answer;
	AP_Columns_preview *			m_pColumnsPreview;
	AP_Columns_preview_drawer		m_previewDrawer;

private:

	UT_uint32						m_iColumns;
	UT_Bool							m_bLineBetween;
};

#endif /* AP_Dialog_Columns_H */
