/* Copyright etc. */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
		 
#ifndef FL_AUTONUM_H
#define FL_AUTONUM_H

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "fl_BlockLayout.h"

class fl_BlockLayout;
class fl_Layout;
class PD_Document;
class fl_AutoNum
{
public:
	fl_AutoNum(UT_uint32 id, UT_uint32 start, PL_StruxDocHandle pItem, fl_AutoNum * pParent, const XML_Char * lDelim, const XML_Char * lDecimal, List_Type lType, PD_Document * pDoc);
	fl_AutoNum(UT_uint32 id, UT_uint32 parent_id, List_Type lType, UT_uint32 start, const XML_Char * lDelim, const XML_Char * lDecimal, PD_Document * pDoc);
	void					fixHierarchy(PD_Document *);
	~fl_AutoNum();
		
	const XML_Char *			getLabel(PL_StruxDocHandle);
        void                                    addItem(PL_StruxDocHandle pItem);
	List_Type				getType(void);
	UT_uint32				getValue(PL_StruxDocHandle);
	UT_uint32				getLevel(void) { return m_iLevel; }
	void				        setLevel(UT_uint32 level) { m_iLevel = level; }
	UT_sint32				getPositionInList( PL_StruxDocHandle pItem, UT_uint32 depth);
	void					setListType(List_Type lType);
	void					setDelim(const XML_Char * pszDelim);
	const XML_Char *                        getDelim(void);
	void					setDecimal(const XML_Char * pszDecimal);
	const XML_Char *			getDecimal(void);
	UT_Bool                                 isDirty(void);
	UT_uint16				getStartValue(void) { return m_iStartValue; }

	UT_uint32				getStartValue32(void);
	void					setStartValue(UT_uint32 start);

	void					insertFirstItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pLast, UT_uint32 depth);
	void					insertItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pBefore);
	void					prependItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pAfter);
	void					removeItem(PL_StruxDocHandle pItem);
	PL_StruxDocHandle                       getParentItem(void);
	void                                    setParentItem(PL_StruxDocHandle pItem);
	PL_StruxDocHandle				getNthBlock(UT_uint32 i);
	PL_StruxDocHandle				getPrevInList(PL_StruxDocHandle pItem);

	const UT_Bool					isItem(PL_StruxDocHandle pItem);
	UT_Bool					doesItemHaveLabel( fl_BlockLayout * pItem);
	const UT_Bool				isEmpty(void) ;
	inline PL_StruxDocHandle		getFirstItem(void) { return (PL_StruxDocHandle) m_pItems.getFirstItem(); }	
	UT_Bool					isLastOnLevel(PL_StruxDocHandle pItem);

	fl_AutoNum *			getParent(void) { return m_pParent; }
	fl_AutoNum * 			getActiveParent(void) ;
	fl_AutoNum *                    getAutoNumFromSdh(PL_StruxDocHandle sdh);
	void				setParent(fl_AutoNum *);
	void                            setUpdatePolicy(UT_Bool bUpdate);
	void			      	setAsciiOffset(UT_uint32 new_asciioffset);

	void					update(UT_uint32 start);
	inline UT_Bool			isUpdating(void) { return m_bUpdatingItems; }
	inline UT_uint32		getID(void)  { return m_iID; }
	char *                          dec2roman(UT_sint32 value, UT_Bool lower) ;
	char *				dec2ascii(UT_sint32 value, UT_uint32 offset);
	
	const char **			getAttributes(void) ;
	
protected:

	void				_calculateLabelStr(UT_uint32 depth);
	void                            _getLabelstr( XML_Char labelStr[], UT_uint32 * insPoint, UT_uint32 depth, PL_StruxDocHandle pLayout);
	void		       	        _updateItems(UT_uint32 start, PL_StruxDocHandle notMe );
	inline UT_uint32		_getLevelValue(fl_AutoNum * pAutoNum); 

	fl_AutoNum *			m_pParent;
	
	UT_Vector				m_pItems;
        PD_Document *                           m_pDoc;
	List_Type                               m_List_Type;
	UT_uint32				m_iID;
	UT_uint32				m_iParentID;
	UT_uint32				m_iLevel;
	UT_uint32				m_iStartValue;
	UT_uint16				m_iAsciiOffset;
	UT_Bool					m_bUpdatingItems;
	UT_Bool                                 m_bUpdate;
	UT_Bool                                 m_bDirty;
	UT_sint32				m_ioffset;
	XML_Char                                m_pszDecimal[80];
	XML_Char                                m_pszDelim[80];
	XML_Char                                m_pszIndent[80];
	UT_Bool					m_bWordMultiStyle;
	PL_StruxDocHandle                       m_pParentItem;
};

#endif
