/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indents-tab-mode:t; -*- */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych, Yaacov Akiba Slama
 * © 2016-2017 Hubert Figuière
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

#ifndef FL_AUTONUM_H
#define FL_AUTONUM_H

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"
#include "fl_AutoLists.h"

#if defined(__MINGW32__)
#undef snprintf
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

// fwd. decl.
class fl_BlockLayout;
class fl_Layout;
class pf_Frag_Strux;
class PD_Document;
class FV_View;

class fl_AutoNum;

typedef std::shared_ptr<fl_AutoNum> fl_AutoNumPtr;
typedef std::shared_ptr<const fl_AutoNum> fl_AutoNumConstPtr;

class ABI_EXPORT fl_AutoNum
    : public std::enable_shared_from_this<fl_AutoNum>
{
private:
    class ItemStorage {
    public:
        typedef std::vector<pf_Frag_Strux*>::size_type size_type;
        typedef std::vector<pf_Frag_Strux*>::value_type value_type;

        void addItem(pf_Frag_Strux* pItem);
        void deleteNthItem(size_type n);
        UT_sint32 findItem(const pf_Frag_Strux* pItem) const {
            auto iter = std::find(m_vec.cbegin(), m_vec.cend(), pItem);
            if (iter == m_vec.cend()) {
                return -1;
            }
            return iter - m_vec.cbegin();
        }
        pf_Frag_Strux* front() const {
            return m_vec.front();
        }
        pf_Frag_Strux* back() const {
            return m_vec.back();
        }
        size_type size() const {
            return m_vec.size();
        }
        bool empty() const {
            return m_vec.empty();
        }
        pf_Frag_Strux* at(size_type n) const {
            return m_vec.at(n);
        }

        bool hasItem(const pf_Frag_Strux* pItem) const;
        void insertItemAt(pf_Frag_Strux* pItem, size_type idx);
        void sort(const std::function<bool(const value_type &, const value_type &)> & compar) {
            std::sort(m_vec.begin(), m_vec.end(), compar);
        }
    private:
        std::vector<pf_Frag_Strux*> m_vec;
        std::unordered_set<const pf_Frag_Strux*> m_set;
    };
public:
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

	const UT_UCSChar *			getLabel(const pf_Frag_Strux*) const;
	void						addItem(pf_Frag_Strux* pItem);
	FL_ListType					getType() const;
	UT_uint32					getValue(const pf_Frag_Strux*) const;
	UT_uint32					getLevel() const { return m_iLevel; }
	UT_uint32					getNumLabels() const;
	bool                        checkReference(const fl_AutoNum & pAuto) const;

	void						setLevel(UT_uint32 level) { m_iLevel = level; }
	UT_sint32					getPositionInList(const pf_Frag_Strux* pItem) const;
	void						setListType(FL_ListType lType);
	void						setDelim(const gchar * pszDelim);
	void						setDelim(const std::string & delim)
	{
		setDelim(delim.c_str());
	}
	const gchar *			getDelim() const;
	void						setDecimal(const gchar * pszDecimal);
	void						setDecimal(const std::string & decimal)
	{
		setDecimal(decimal.c_str());
	}
	const gchar *			getDecimal() const;
	bool						isDirty() const;
	UT_uint16					getStartValue() const { return m_iStartValue; }

	UT_uint32					getStartValue32() const;
	void						setStartValue(UT_uint32 start);

	void						insertFirstItem(pf_Frag_Strux* pItem,
												pf_Frag_Strux* pLast,
												bool bDoFix=true);
	void						insertItem(pf_Frag_Strux* pItem, const pf_Frag_Strux* pBefore, bool bDoFix = true);
	void						prependItem(pf_Frag_Strux* pItem, const pf_Frag_Strux* pAfter, bool bDoFix = true);
	void						removeItem(const pf_Frag_Strux* pItem);
	pf_Frag_Strux*			getParentItem() const;
	void						setParentItem(pf_Frag_Strux* pItem);
	bool					isContainedByList(pf_Frag_Strux* pItem) const;
	pf_Frag_Strux*			getNthBlock(UT_uint32 i) const;
	pf_Frag_Strux*			getPrevInList(const pf_Frag_Strux* pItem) const;

	bool					isItem(const pf_Frag_Strux* pItem) const;
	bool						doesItemHaveLabel(const fl_BlockLayout * pItem) const;
	bool					isEmpty(void) const;
	pf_Frag_Strux*			getFirstItem(void) const;
	pf_Frag_Strux*			getLastItem(void) const;
	bool						isLastOnLevel(const pf_Frag_Strux* pItem) const;

	fl_AutoNumPtr				getParent(void) const { return m_pParent; }
	fl_AutoNumPtr				getActiveParent(void) const;
	fl_AutoNumConstPtr			getAutoNumFromSdh(const pf_Frag_Strux* sdh) const;
	void						fixListOrder(void);
	void						markAsDirty(void);
	void						findAndSetParentItem(void);
	void						setAsciiOffset(UT_uint32 new_asciioffset);

	void						update(UT_uint32 start);
	bool						isUpdating(void) const { return m_bUpdatingItems; }
	UT_uint32					getID() const { return m_iID; }
	UT_uint32					getParentID() const { return m_iParentID; }
	bool						isIDSomeWhere(UT_uint32 ID) const;
	static char *				dec2roman(UT_sint32 value, bool lower);
	static char *				dec2ascii(UT_sint32 value, UT_uint32 offset);
	static void					dec2hebrew(UT_UCSChar labelStr[], UT_uint32 * insPoint, UT_sint32 value);
	void                        getAttributes(std::vector<std::string>&v,
											  bool bEscapeXML) const;
	PD_Document *				getDoc(void) const
	{return m_pDoc;}
	pf_Frag_Strux*			 getLastItemInHeiracy(void) const;
protected:
	void                        _setParent(const fl_AutoNumPtr & pParent);
	void                        _setParentID(UT_uint32 iParentID);
	void						_calculateLabelStr(UT_uint32 depth);
	void						_getLabelstr(	UT_UCSChar labelStr[],
												UT_uint32 * insPoint,
												UT_uint32 depth,
												const pf_Frag_Strux* pLayout) const;
	bool						_updateItems(UT_sint32 start, const pf_Frag_Strux* notMe);
	UT_uint32					_getLevelValue(const fl_AutoNumConstPtr & pAutoNum) const;

	fl_AutoNumPtr				m_pParent;

private:
	ItemStorage                 m_items;
protected:
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
	pf_Frag_Strux*			m_pParentItem;
};

#endif
