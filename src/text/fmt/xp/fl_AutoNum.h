/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych, Yaacov Akiba Slama
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

#include <string>

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_AutoLists.h"

#if defined(__MINGW32__)
#undef snprintf
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#include <vector>

// fwd. decl.
class fl_BlockLayout;
class fl_Layout;
class PD_Document;
class FV_View;
class UT_UTF8String;

class ABI_EXPORT fl_AutoNum
{
public:
	fl_AutoNum(	UT_uint32 id,
				UT_uint32 start,
				PL_StruxDocHandle pItem,
				fl_AutoNum * pParent,
				const gchar * lDelim,
				const gchar * lDecimal,
				FL_ListType lType,
				PD_Document * pDoc,
				FV_View * pView);

	fl_AutoNum(	UT_uint32 id,
				UT_uint32 parent_id,
				FL_ListType lType,
				UT_uint32 start,
				const gchar * lDelim,
				const gchar * lDecimal,
				PD_Document * pDoc,
				FV_View * pView);

	~fl_AutoNum();

	void						fixHierarchy(void);

	const UT_UCSChar *			getLabel(PL_StruxDocHandle) const;
	void						addItem(PL_StruxDocHandle pItem);
	FL_ListType					getType() const;
	UT_uint32					getValue(PL_StruxDocHandle) const;
	UT_uint32					getLevel() const { return m_iLevel; }
	UT_uint32					getNumLabels() const;
	bool                        checkReference(fl_AutoNum * pAuto);

	void						setLevel(UT_uint32 level) { m_iLevel = level; }
	UT_sint32					getPositionInList( PL_StruxDocHandle pItem, UT_uint32 depth) const;
	void						setListType(FL_ListType lType);
	void						setDelim(const gchar * pszDelim);
	void						setDelim(const std::string & delim)
    {
        setDelim(delim.c_str());
    }
	const gchar *			getDelim() const;
	void						setDecimal(const gchar * pszDecimal);
    void                        setDecimal(const std::string & decimal)
    { 
        setDecimal(decimal.c_str());
    }
	const gchar *			getDecimal() const;
	bool						isDirty() const;
	UT_uint16					getStartValue() const { return m_iStartValue; }

	UT_uint32					getStartValue32() const;
	void						setStartValue(UT_uint32 start);

	void						insertFirstItem(PL_StruxDocHandle pItem,
												PL_StruxDocHandle pLast,
												UT_uint32 depth,
												bool bDoFix=true);
	void						insertItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pBefore, bool bDoFix = true);
	void						prependItem(PL_StruxDocHandle pItem, PL_StruxDocHandle pAfter, bool bDoFix = true);
	void						removeItem(PL_StruxDocHandle pItem);
	PL_StruxDocHandle			getParentItem() const;
	void						setParentItem(PL_StruxDocHandle pItem);
	bool                                 isContainedByList(PL_StruxDocHandle pItem);
	PL_StruxDocHandle			getNthBlock(UT_sint32 i);
	PL_StruxDocHandle			getPrevInList(PL_StruxDocHandle pItem);

	bool					isItem(PL_StruxDocHandle pItem) const;
	bool						doesItemHaveLabel(fl_BlockLayout * pItem);
	bool					isEmpty(void);
	PL_StruxDocHandle			getFirstItem(void) const;
	PL_StruxDocHandle			getLastItem(void);
	bool						isLastOnLevel(PL_StruxDocHandle pItem);

	fl_AutoNum *				getParent(void) const { return m_pParent; }
	fl_AutoNum *				getActiveParent(void);
	fl_AutoNum *				getAutoNumFromSdh(PL_StruxDocHandle sdh);
	const fl_AutoNum *			getAutoNumFromSdh(PL_StruxDocHandle sdh) const;
	void						fixListOrder(void);
	void						markAsDirty(void);
	void						findAndSetParentItem(void);
	void						setAsciiOffset(UT_uint32 new_asciioffset);

	void						update(UT_uint32 start);
	bool						isUpdating(void) { return m_bUpdatingItems; }
	UT_uint32					getID() const { return m_iID; }
	UT_uint32					getParentID() const { return m_iParentID; }
	bool                        isIDSomeWhere(UT_uint32 ID);
	static char *				dec2roman(UT_sint32 value, bool lower);
	static char *				dec2ascii(UT_sint32 value, UT_uint32 offset);
	static void					dec2hebrew(UT_UCSChar labelStr[], UT_uint32 * insPoint, UT_sint32 value);
	void                        getAttributes(std::vector<UT_UTF8String>&v,
											  bool bEscapeXML);
	
	PL_StruxDocHandle           getLastItemInHeiracy(void);
protected:
	void                        _setParent(fl_AutoNum * pParent);
	void                        _setParentID(UT_uint32 iParentID);
	void						_calculateLabelStr(UT_uint32 depth);
	void						_getLabelstr(	UT_UCSChar labelStr[],
												UT_uint32 * insPoint,
												UT_uint32 depth,
												PL_StruxDocHandle pLayout) const;
	void						_updateItems(UT_sint32 start, PL_StruxDocHandle notMe );
	UT_uint32					_getLevelValue(fl_AutoNum * pAutoNum);

	fl_AutoNum *				m_pParent;

	UT_Vector					m_pItems;
	PD_Document *               m_pDoc;
	FV_View *				    m_pView;
	FL_ListType					m_List_Type;
	UT_uint32					m_iID;
	UT_uint32					m_iParentID;
	UT_uint32					m_iLevel;
	UT_uint32					m_iStartValue;
	UT_uint16					m_iAsciiOffset;
	bool						m_bUpdatingItems;
	bool						m_bDirty;
	UT_sint32					m_ioffset;
	gchar					m_pszDecimal[80]; // BAD BAD HARDCODED ARRAY LENGTHS
	gchar					m_pszDelim[80];
	gchar					m_pszIndent[80];
	bool						m_bWordMultiStyle;
	PL_StruxDocHandle			m_pParentItem;
};

#endif
