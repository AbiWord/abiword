/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef XAP_DIALOG_IMAGE_H
#define XAP_DIALOG_IMAGE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "xap_Frame.h"
#include "xap_Dialog.h"

typedef enum _WRAPPING_TYPE
{
    WRAP_INLINE,
    WRAP_TEXTRIGHT,
    WRAP_TEXTLEFT,
    WRAP_TEXTBOTH,
    WRAP_NONE
} WRAPPING_TYPE;


typedef enum _POSITION_TO
{
    POSITION_TO_PARAGRAPH,
    POSITION_TO_COLUMN,
    POSITION_TO_PAGE
} POSITION_TO;

/* #include "xav_View.h" */


class ABI_EXPORT XAP_Dialog_Image : public XAP_Dialog_NonPersistent
{
public:

  typedef enum { a_OK, a_Cancel } tAnswer;

	XAP_Dialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Image(void);

	virtual void  runModal(XAP_Frame * pFrame) = 0;

	void setWrapping(WRAPPING_TYPE iWrap);
	void setPositionTo(POSITION_TO iPos);
	bool isInHdrFtr(void);
	void setInHdrFtr(bool b)
	  { m_bInHdrFtr = b;}

	WRAPPING_TYPE getWrapping(void) const
		{ return m_iWrappingType;}
	POSITION_TO getPositionTo(void) const
		{ return m_iPositionTo;}


	double getWidth () const { return m_width; }
	double getHeight () const { return m_height; }
	bool getPreserveAspect() const { return m_bAspect; }

	// needed to set the initial width & height
	void setWidth (double w) { setWidth(w,false); }
	void setHeight (double h) { setHeight(h,false); }
	void setPreserveAspect( bool b ) { m_bAspect = b; }


	// needed to set the initial width & height
	void setWidth (UT_sint32 w);
	void setHeight (UT_sint32 h);

	void setMaxHeight ( double h ) { m_maxHeight = h; }
	void setMaxWidth ( double w ) { m_maxWidth = w; }

	double getMaxWidth () const { return m_maxWidth; }
	double getMaxHeight () const { return m_maxHeight; }

	tAnswer getAnswer () const { return m_answer; }
	const char * getHeightString(void);
	const char * getWidthString(void);
	double getIncrement(const char * sz);
	void incrementHeight(bool bIncrement);
	void incrementWidth(bool bIncrement);
	void setHeight(const char * szHeight);
	void setWidth(const char * szWidth);
	UT_Dimension getPreferedUnits(void);
	void  setPreferedUnits(UT_Dimension dim);
	void setTightWrap(bool bTight)
	  { m_bTightWrap = bTight;}
	bool isTightWrap(void)
	  { return m_bTightWrap;}
	void setTitle(const UT_UTF8String & title) {
		m_title = title;
	}

	const UT_UTF8String & getTitle() const {
		return m_title;
	}

	void setDescription(const UT_UTF8String & description) {
		m_description = description;
	}

	const UT_UTF8String & getDescription() const {
		return m_description;
	}


 protected:
	void setAnswer ( tAnswer ans ) { m_answer = ans; }
	void _convertToPreferredUnits(const char *sz, UT_String & pRet);

 private:
	bool   m_bAspect;
	double m_width;
	double m_height;
	double m_maxWidth;
	double m_maxHeight;
	tAnswer m_answer;
	UT_String m_HeightString;
	UT_String m_WidthString;
	bool m_bHeightChanged;
	bool m_bWidthChanged;
	UT_Dimension m_PreferedUnits;

	UT_UTF8String m_title;
	UT_UTF8String m_description;

	void setWidth (double w, bool checkaspect);
	void setHeight (double h, bool checkaspect);
	void setWidthAndHeight (double wh, bool iswidth);

	WRAPPING_TYPE m_iWrappingType;
	POSITION_TO m_iPositionTo;
	bool m_bInHdrFtr;
	bool m_bTightWrap;
};

#endif /* XAP_DIALOG_IMAGE_H */
