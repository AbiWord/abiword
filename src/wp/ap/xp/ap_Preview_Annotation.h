/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301 USA.
*/

#ifndef AP_PREVIEW_ANNOTATION_H
#define AP_PREVIEW_ANNOTATION_H

#include <string>
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "gr_Graphics.h"
#include "xap_Preview.h"
#include "xap_Dialog.h"

// default annotation preview values
#define PREVIEW_DEFAULT_WIDTH 320
#define PREVIEW_DEFAULT_HEIGHT 80

class ABI_EXPORT AP_Preview_Annotation : public XAP_Preview, public XAP_Dialog_Modeless
{
public:
	AP_Preview_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Preview_Annotation(void);

	void					setTitle(const gchar * pTitle);
	void					setTitle(const std::string & sTitle)
		{ m_sTitle = sTitle; }
	const std::string&		getTitle() const
		{ return m_sTitle; }

	void					setAuthor(const gchar * pAuthor);
	void					setAuthor(const std::string & sAuthor)
		{ m_sAuthor = sAuthor; }
	const std::string&		getAuthor() const
		{ return m_sAuthor; }

	void					setDescription(const gchar * pDescription);
	void					setDescription(const std::string & sDescription)
		{ m_sDescription = sDescription; }
	const std::string&		getDescription() const
		{ return m_sDescription; }

	void		setAnnotationID(UT_uint32 aID);
	UT_uint32	getAnnotationID() const
	{  return m_iAID; }


	virtual void drawImmediate(const UT_Rect* clip = NULL) override;
	void			clearScreen(void);
	void			setXY(UT_sint32 x, UT_sint32 y);
	void            setOffset(UT_sint32 ioff);
	virtual void	setActiveFrame(XAP_Frame *pFrame) override;
	void			setSizeFromAnnotation(void);

protected:
	void			_createAnnotationPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);
	UT_sint32		m_width;
	UT_sint32		m_height;
	UT_sint32		m_left;
	UT_sint32		m_top;
	UT_sint32       m_Offset;
	void            ConstructWindowName(void);
	UT_RGBColor		m_clrBackground;

private:
	UT_uint32			m_iAID;
	// assume that these strings are always UTF8
	std::string			m_sTitle;
	std::string			m_sAuthor;
	std::string			m_sDescription;
	UT_UCS4String		m_drawString;
	GR_Font *			m_pFont;
	UT_sint32			m_iAscent;
	UT_sint32			m_iDescent;
	UT_sint32			m_iHeight;
};

#endif /* AP_PREVIEW_ANNOTATION_H */
