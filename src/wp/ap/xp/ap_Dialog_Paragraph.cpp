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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_debugmsg.h"
#include "ut_units.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Prefs.h"

#include "ap_Strings.h"

#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"

#include "ap_Preview_Paragraph.h"
#include "ap_Dialog_Paragraph.h"
#include "ap_Prefs_SchemeIds.h"

#define ADD_PROPERTY_ITEM(index, value)							\
	do	{	sControlData * pItem = new sControlData;			\
            UT_ASSERT(pItem);									\
            pItem->bChanged = UT_FALSE;							\
            pItem->pData = (void *) value;						\
            void * pTmp;									   	\
            m_vecProperties.setNthItem(index, (void *) pItem,	\
									   &pTmp);					\
	} while (0)


AP_Dialog_Paragraph::AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_paragraphPreview = NULL;
	m_pFrame = NULL;
	
	// determine unit system to use in this dialog
	const XML_Char * szRulerUnits;
	UT_ASSERT(m_pApp);
	if (m_pApp->getPrefs()->getPrefsValue(AP_PREF_KEY_RulerUnits, &szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	// terminate the static buffers for cleanliness
	m_bufLeftIndent[0] = 0;
	m_bufRightIndent[0] = 0;
	m_bufSpecialIndent[0] = 0;
	m_bufBeforeSpacing[0] = 0;
	m_bufAfterSpacing[0] = 0;
	m_bufSpecialSpacing[0] = 0;
	
	// initialize vector of control/value/changed items

	ADD_PROPERTY_ITEM(id_MENU_ALIGNMENT, align_LEFT);

	ADD_PROPERTY_ITEM(id_SPIN_LEFT_INDENT, &m_bufLeftIndent);
	ADD_PROPERTY_ITEM(id_SPIN_RIGHT_INDENT, &m_bufRightIndent);
	ADD_PROPERTY_ITEM(id_MENU_SPECIAL_INDENT, indent_NONE);
	ADD_PROPERTY_ITEM(id_SPIN_SPECIAL_INDENT, &m_bufSpecialIndent);	

	ADD_PROPERTY_ITEM(id_SPIN_BEFORE_SPACING, &m_bufBeforeSpacing);
	ADD_PROPERTY_ITEM(id_SPIN_AFTER_SPACING, &m_bufAfterSpacing);
	ADD_PROPERTY_ITEM(id_MENU_SPECIAL_SPACING, spacing_SINGLE);
	ADD_PROPERTY_ITEM(id_SPIN_SPECIAL_SPACING, &m_bufSpecialSpacing);	

	ADD_PROPERTY_ITEM(id_CHECK_WIDOW_ORPHAN, check_INDETERMINATE);
	ADD_PROPERTY_ITEM(id_CHECK_KEEP_LINES, check_INDETERMINATE);
	ADD_PROPERTY_ITEM(id_CHECK_PAGE_BREAK, check_INDETERMINATE);
	ADD_PROPERTY_ITEM(id_CHECK_SUPPRESS, check_INDETERMINATE);
	ADD_PROPERTY_ITEM(id_CHECK_NO_HYPHENATE, check_INDETERMINATE);
	ADD_PROPERTY_ITEM(id_CHECK_KEEP_NEXT, check_INDETERMINATE);
}

AP_Dialog_Paragraph::~AP_Dialog_Paragraph(void)
{
	DELETEP(m_paragraphPreview);

	// This macro will remove all the sControlData items in
	// the vector, but will not free the sControlData.pValue items
	// (which is desirable, they will be either ints or static
	// buffers).
	UT_VECTOR_PURGEALL(sControlData *, m_vecProperties);
}

UT_Bool AP_Dialog_Paragraph::setDialogData(const XML_Char ** pProps)
{
	UT_ASSERT(pProps);

	// NOTICE : When setting values, this function always calls
	// NOTICE : _set[thing]ItemValue() with the bToggleDirty flag
	// NOTICE : set to UT_FALSE, because these are the "un-dirty"
	// NOTICE : values.
	
	if (pProps[0])
	{
		const XML_Char * sz;
		
		sz = UT_getAttribute("text-align", pProps);
		if (sz)
		{
			tAlignState t;

			if (UT_XML_strcmp(sz, "center") == 0)
				t = align_CENTERED;
			else if (UT_XML_strcmp(sz, "right") == 0)
				t = align_RIGHT;
			else if (UT_XML_strcmp(sz, "justify") == 0)
				t = align_JUSTIFIED;
			else if (UT_XML_strcmp(sz, "left") == 0)
				t = align_LEFT;
			else
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

			_setMenuItemValue(id_MENU_ALIGNMENT, t, op_INIT);			
		}
								 
		sz = UT_getAttribute("margin-left", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_LEFT_INDENT, sz, op_INIT);

		sz = UT_getAttribute("margin-right", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_RIGHT_INDENT, sz, op_INIT);

		sz = UT_getAttribute("text-indent", pProps);
		if (sz)
		{
			// NOTE : Calling UT_convertDimensionless() _discards_ all
			// NOTE : unit system information.  IFF all units are
			// NOTE : consistent among all paragraph properties will
			// NOTE : the comparisons be valid.  For now this should be
			// NOTE : valid.

			if (UT_convertDimensionless(sz) > (double) 0)
			{
				// if text-indent is greater than margin-left, we have a "first line" case
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_FIRSTLINE, op_INIT);
			}
			else if (UT_convertDimensionless(sz) < (double) 0)
			{
				// if text-indent is less than margin-left, we have a "hanging" case
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_HANGING, op_INIT);
			}
			else
			{
				// they're equal then there's nothing special about them
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_NONE, op_INIT);
			}

			// TODO : FLIP THE SIGN ON THIS FIELD IF THE CASE IS "HANGING"!
			
			// set the value regardless; dialog will enable/disable field
			// if spacing is "NONE"
			_setSpinItemValue(id_SPIN_SPECIAL_INDENT, sz, op_INIT);
		}

		sz = UT_getAttribute("line-height", pProps);
		if (sz)
		{
			UT_uint32 nLen = strlen(sz);
			if (nLen > 0)
			{
				char * pPlusFound = strrchr(sz, '+');
				if (pPlusFound && *(pPlusFound + 1) == 0)
				{
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ATLEAST, op_INIT);

					// need to strip off that plus
					int posPlus = pPlusFound - sz;
					UT_ASSERT(posPlus>=0);
					UT_ASSERT(posPlus<100);

					char pTmp[100];
					strcpy(pTmp, sz);
					pTmp[posPlus] = 0;

					_setSpinItemValue(id_SPIN_SPECIAL_SPACING, pTmp, op_INIT);
				}
				else
				{
					if(UT_hasDimensionComponent(sz))
						_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_EXACTLY, op_INIT);
					else if(UT_strcmp("1.0", sz) == 0)
						_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_SINGLE, op_INIT);
					else if(UT_strcmp("1.5", sz) == 0)
						_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ONEANDHALF, op_INIT);
					else if(UT_strcmp("2.0", sz) == 0)
						_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_DOUBLE, op_INIT);
					else
						_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_MULTIPLE, op_INIT);

					// set the spin contents regardless of menu content; platforms will
					// enable or disable the spin item for varying states of menu
					_setSpinItemValue(id_SPIN_SPECIAL_SPACING, sz, op_INIT);
				}
			}
		}

		sz = UT_getAttribute("margin-top", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_BEFORE_SPACING, sz, op_INIT);
			
		sz = UT_getAttribute("margin-bottom", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_AFTER_SPACING, sz, op_INIT);
		
		{
			// NOTE : "orphans" and "widows" hold a number specifying the number
			// NOTE : of lines to consider an orphaned or widowed piece of text.
			// NOTE : If they're both 0 they're off.  If either is greater than
			// NOTE : 0, then some form of control is in effect.  If the property
			// NOTE : is not set, they're indeterminate.

			UT_Bool bNoOrphans = UT_FALSE;
			UT_Bool bNoWidows = UT_FALSE;

			double orphans, widows;
			
			sz = UT_getAttribute("orphans", pProps);
			if (sz)
				orphans = UT_convertDimensionless(sz);
			else
				bNoOrphans = UT_TRUE;
			
			sz = UT_getAttribute("widows", pProps);
			if (sz)
				widows = UT_convertDimensionless(sz);
			else
				bNoWidows = UT_TRUE;

			if (bNoOrphans && bNoWidows)
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_INDETERMINATE, op_INIT);
			else if (orphans > 0 || widows > 0)
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_FALSE, op_INIT);
		}

		sz = UT_getAttribute("keep-together", pProps);
		if (sz)
		{
			if (UT_stricmp(sz, "yes") == 0)
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_FALSE, op_INIT);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_LINES, check_INDETERMINATE, op_INIT);

		sz = UT_getAttribute("keep-with-next", pProps);
		if (sz)
		{
			if (UT_stricmp(sz, "yes") == 0)
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_FALSE, op_INIT);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_NEXT, check_INDETERMINATE, op_INIT);
			
		// TODO : add these to PP_Property (pp_Property.cpp) !!!
		// TODO : and to FV_View::getBlockFormat (or else they won't come in)
		/*
		  m_pageBreakBefore;
		  m_suppressLineNumbers;
		  m_noHyphenate;
		*/
	}

	return UT_TRUE;
}

// This function returns a pointer to newly allocated memory, which contains
// pointers to newly allocated memory.  The caller must free both dimensions
// of this structure (use the FREEP() macro, we use calloc() to allocate it).
// This function does not free any memory pointed to by its argument before
// writing into it.

#define ALLOC_PROP_PAIR(p)									\
        do {                								\
            p = (propPair *) calloc(1, sizeof(propPair));	\
            if (!p) return UT_FALSE;						\
        } while (0)											\

UT_Bool AP_Dialog_Paragraph::getDialogData(const XML_Char **& pProps)
{
	UT_Vector v;

	struct propPair
	{
		XML_Char * prop;
		XML_Char * val;
	};

	propPair * p;
		
	if (_wasChanged(id_MENU_ALIGNMENT))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "text-align");

		switch (_getMenuItemValue(id_MENU_ALIGNMENT))
		{
		case align_LEFT:
			UT_XML_cloneString(p->val, "left");
			break;
		case align_CENTERED:
			UT_XML_cloneString(p->val, "center");
			break;
		case align_RIGHT:
			UT_XML_cloneString(p->val, "right");
			break;
		case align_JUSTIFIED:
			UT_XML_cloneString(p->val, "justify");
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		v.addItem(p);
	}
	
	if (_wasChanged(id_SPIN_LEFT_INDENT))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "margin-left");
		UT_XML_cloneString(p->val, _getSpinItemValue(id_SPIN_LEFT_INDENT));

		v.addItem(p);
	}
	
	if (_wasChanged(id_SPIN_RIGHT_INDENT))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "margin-right");
		UT_XML_cloneString(p->val, _getSpinItemValue(id_SPIN_RIGHT_INDENT));

		v.addItem(p);
	}

	// TODO : The logic here might not be bulletproof.  If the user triggers
	// TODO : a change in the TYPE of special indent (hanging, first line,
	// TODO : none), we will always save what's in the box as a property.
	// TODO : One could make it smarter with a stronger contract with
	// TODO : the platform interfaces.

	if (_wasChanged(id_MENU_SPECIAL_INDENT) || _wasChanged(id_SPIN_SPECIAL_INDENT))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "text-indent");

		tIndentState i = (tIndentState) _getMenuItemValue(id_MENU_SPECIAL_INDENT);

		if (i == indent_NONE)
			UT_XML_cloneString(p->val, UT_convertToDimensionString(m_dim, 0));
		else if (i == indent_FIRSTLINE)
			UT_XML_cloneString(p->val, _getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		else if (i == indent_HANGING)
		{
			// we have to flip the sign for "hanging" indents to a negative quantity for
			// storage in the document as a text-indent

			// get with no dimension
			UT_Dimension dim = UT_determineDimension(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			double val = UT_convertDimensionless(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));

			// flip sign
			val = val * (double) -1;

			// store the reconstructed
			UT_XML_cloneString(p->val, UT_convertToDimensionString(dim, val));
		}

		v.addItem(p);
	}

	if (_wasChanged(id_SPIN_BEFORE_SPACING))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "margin-top");
		UT_XML_cloneString(p->val, _getSpinItemValue(id_SPIN_BEFORE_SPACING));

		v.addItem(p);
	}
	
	if (_wasChanged(id_SPIN_AFTER_SPACING))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "margin-bottom");
		UT_XML_cloneString(p->val, _getSpinItemValue(id_SPIN_AFTER_SPACING));

		v.addItem(p);
	}
	
	// TODO : The logic here might not be bulletproof.  If the user triggers
	// TODO : a change in the TYPE of special indent (single, double, etc.)
	// TODO : we will always save what's in the box as a property.
	// TODO : One could make it smarter with a stronger contract with
	// TODO : the platform interfaces.

	if(_wasChanged(id_MENU_SPECIAL_SPACING))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "line-height");
		
		// normal spacings (single, 1.5, double) are just simple numbers.
		// "at least" needs a "+" at the end of the number (no units).
		// "exactly" simply has units.
		// "multiple" has no units.

		XML_Char * pTmp = NULL;
		const XML_Char * pString = _getSpinItemValue(id_SPIN_SPECIAL_SPACING);
		UT_uint32 nSize = 0;
		
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
			UT_XML_cloneString(p->val, "1.0");
			break;
		case spacing_ONEANDHALF:
			UT_XML_cloneString(p->val, "1.5");
			break;
		case spacing_DOUBLE:
			UT_XML_cloneString(p->val, "2.0");
			break;
		case spacing_ATLEAST:
			nSize = UT_XML_strlen(pString);
			pTmp = (XML_Char *) calloc(nSize + 2, sizeof(XML_Char));
			UT_ASSERT(pTmp);

			UT_XML_strncpy(pTmp, nSize + 1, pString);
			// stick a '+' at the end
			pTmp[nSize] = '+';
			UT_XML_cloneString(p->val, pTmp);
			FREEP(pTmp);
			break;

		case spacing_EXACTLY:
			// fallthrough
		case spacing_MULTIPLE:
			// both these cases either do or don't have units associated with them.
			// the platform dialog code takes care of that.
			UT_XML_cloneString(p->val, pString);
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		v.addItem(p);
	}

	// NOTE : "orphans" and "widows" hold a number specifying the number
	// NOTE : of lines to consider an orphaned or widowed piece of text.
	// NOTE : If they're both 0 they're off.  If either is greater than
	// NOTE : 0, then some form of control is in effect.  If the property
	// NOTE : is not set, they're indeterminate.

	if (_wasChanged(id_CHECK_WIDOW_ORPHAN))
	{
		// TODO : fix this!  we stomp on both properties (setting them
		// TODO : to "2"s or "0"s) if the one check box was ever
		// TODO : changed
		
		{
			ALLOC_PROP_PAIR(p);
			UT_XML_cloneString(p->prop, "orphans");
		
			if (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE)
				UT_XML_cloneString(p->val, "2");
			else 
				UT_XML_cloneString(p->val, "0");

			v.addItem(p);
		}
		
		{
			ALLOC_PROP_PAIR(p);
			UT_XML_cloneString(p->prop, "widows");
		
			if (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE)
				UT_XML_cloneString(p->val, "2");
			else 
				UT_XML_cloneString(p->val, "0");

			v.addItem(p);
		}
	}
	
	if (_wasChanged(id_CHECK_KEEP_LINES))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "keep-together");

		if (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE)
			UT_XML_cloneString(p->val, "yes");
		else
			UT_XML_cloneString(p->val, "no");

		v.addItem(p);
	}
	
	if (_wasChanged(id_CHECK_KEEP_NEXT))
	{
		ALLOC_PROP_PAIR(p);
		UT_XML_cloneString(p->prop, "keep-with-next");

		if (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE)
			UT_XML_cloneString(p->val, "yes");
		else
			UT_XML_cloneString(p->val, "no");

		v.addItem(p);
	}

	// TODO : add these to PP_Property (pp_Property.cpp) !!!
	/*
	  m_pageBreakBefore;
	  m_suppressLineNumbers;
	  m_noHyphenate;
	*/

	// export everything in the array
	UT_uint32 count = v.getItemCount()*2 + 1;

	const XML_Char ** newprops = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!newprops)
		return UT_FALSE;

	const XML_Char ** newitem = newprops;

	UT_uint32 i = v.getItemCount();

	while (i > 0)
	{
		p = (propPair *) v.getNthItem(i-1);
		i--;

		newitem[0] = p->prop;
		newitem[1] = p->val;
		newitem += 2;
	}

	// DO purge the vector's CONTENTS, which are just propPair structs 
	UT_VECTOR_FREEALL(propPair *, v);

	// DO NOT purge the propPair's CONTENTS, because they will be pointed to
	// by the pointers we just copied into memory typed (XML_Char **)

	pProps = newprops;
	
	return UT_TRUE;
}

AP_Dialog_Paragraph::tAnswer AP_Dialog_Paragraph::getAnswer(void) const
{
	return m_answer;
}

/************************************************************************/

void AP_Dialog_Paragraph::_createPreviewFromGC(GR_Graphics * gc,
											   UT_uint32 width,
											   UT_uint32 height)
{
	UT_ASSERT(gc);

	// free any attached preview
	DELETEP(m_paragraphPreview);

	// platform's runModal should have set this
	UT_ASSERT(m_pFrame);

	AV_View * baseview = m_pFrame->getCurrentView();
	UT_ASSERT(baseview);

	FV_View * view = static_cast<FV_View *> (baseview);

	FL_DocLayout * dl = view->getLayout();
	UT_ASSERT(dl);

	fl_BlockLayout * bl = dl->findBlockAtPosition((PT_DocPosition) view->getPoint());
	UT_ASSERT(bl);

	UT_GrowBuf gb;
	UT_Bool hadMem = bl->getBlockBuf(&gb);

	// we use the first 200 characters from the document's current block.
	UT_UCSChar * tmp = NULL;

	if (hadMem && gb.getLength() > 0)
	{
		// cap it at 200
		gb.truncate(200);
		UT_UCS_cloneString(&tmp, (UT_UCSChar *) gb.getPointer(0));
	}
	else
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();
	
		// if the paragraph was empty, use our sample
		UT_UCS_cloneString_char(&tmp, pSS->getValue(AP_STRING_ID_DLG_Para_PreviewSampleFallback));
	}

	m_paragraphPreview = new AP_Preview_Paragraph(gc, tmp, this);

	FREEP(tmp);
	
	UT_ASSERT(m_paragraphPreview);
	
	m_paragraphPreview->setWindowSize(width, height);

	// TODO : any setup of the GC for drawing

}

void AP_Dialog_Paragraph::_setMenuItemValue(tControl item, UT_sint32 value,
											tOperation op /* = op_UICHANGE */)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	// menu items have integers as data, so store it in a pointer
	pItem->pData = (void *) value;

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->bChanged = UT_TRUE;

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}


UT_uint32 AP_Dialog_Paragraph::_getMenuItemValue(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	return (UT_uint32) pItem->pData;
}
		
void AP_Dialog_Paragraph::_setCheckItemValue(tControl item, tCheckState value,
											tOperation op /* = op_UICHANGE */)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	// check buttons have integers as data, so store it in a pointer
	pItem->pData = (void *) value;

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->bChanged = UT_TRUE;

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}

AP_Dialog_Paragraph::tCheckState AP_Dialog_Paragraph::_getCheckItemValue(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	// stupid casting tricks to keep the compiler happy
	UT_uint32 tmp = (UT_uint32) pItem->pData;
	return (tCheckState) tmp;
}

void AP_Dialog_Paragraph::_setSpinItemValue(tControl item, const XML_Char * value,
											tOperation op /* = op_UICHANGE */)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);
	
	// spin items have pointers to static XML_Char buffers as data, so we
	// don't free them, but extract a pointer and copy the string contents
	UT_ASSERT(pItem->pData && value);
	UT_XML_strncpy((XML_Char *) pItem->pData, SPIN_BUF_TEXT_SIZE, value);

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->bChanged = UT_TRUE;

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}

const XML_Char * AP_Dialog_Paragraph::_getSpinItemValue(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem && pItem->pData);
	
	return (const XML_Char *) pItem->pData;
}


// _doSpin() spins the current value of the edit control (already stored 
// in member variables) by amt units.  this method handles all of the 
// unit conversions required, updating the member variables as needed.
// the results get copied back to the dialog controls in the platform 
// implementation of _syncControls().  

#define SPIN_INCR_IN	0.1
#define SPIN_INCR_CM	0.5
#define SPIN_INCR_PI	6.0
#define SPIN_INCR_PT	1.0
#define SPIN_INCR_none	0.1

void AP_Dialog_Paragraph::_doSpin(tControl edit, UT_sint32 amt)
{
	UT_ASSERT(amt); // zero makes no sense

	// get current value from member
	const XML_Char* szOld = _getSpinItemValue(edit);
	double d = UT_convertDimensionless(szOld);

	// figure out which dimension and units to spin in
	UT_Dimension dimSpin = m_dim;
	double dSpinUnit = SPIN_INCR_PT;
	double dMin = 0.0;
	UT_Bool bMin = UT_FALSE;

	switch (edit)
	{
	case id_SPIN_SPECIAL_INDENT:
		dMin = 0.0;
		bMin = UT_TRUE;
		// fall through
	case id_SPIN_LEFT_INDENT:
	case id_SPIN_RIGHT_INDENT:
		dimSpin = m_dim;
		switch (dimSpin)
		{
		case DIM_IN:	dSpinUnit = SPIN_INCR_IN;	break;
		case DIM_CM:	dSpinUnit = SPIN_INCR_CM;	break;
		case DIM_PI:	dSpinUnit = SPIN_INCR_PI;	break;
		case DIM_PT:	dSpinUnit = SPIN_INCR_PT;	break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		break;

	case id_SPIN_BEFORE_SPACING:
	case id_SPIN_AFTER_SPACING:
		dimSpin = DIM_PT;
		dSpinUnit = 6.0;
		dMin = 0.0;
		bMin = UT_TRUE;
		break;

	case id_SPIN_SPECIAL_SPACING:
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_MULTIPLE);
			// fall through
		case spacing_MULTIPLE:
			dimSpin = DIM_none;
			dSpinUnit = 0.5;
			dMin = 0.5;
			bMin = UT_TRUE;
			break;

		case spacing_EXACTLY:
			dMin = 1;
			// fall through
		case spacing_ATLEAST:
			dimSpin = DIM_PT;
			dSpinUnit = SPIN_INCR_PT;
			bMin = UT_TRUE;
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	// figure out spin precision, too
	const char * szPrecision = ".1";
	if ((dimSpin == DIM_PT) || 
		(dimSpin == DIM_PI))
		szPrecision = ".0";

	// if needed, switch unit systems and round off
	UT_Dimension dimOld = UT_determineDimension(szOld, dimSpin);

	if (dimOld != dimSpin)
	{
		double dInches = UT_convertToInches(szOld);
		d = UT_convertInchesToDimension(dInches, dimSpin); 
	}

	// value is now in desired units, so change it
	d += (dSpinUnit * amt);
	if (bMin)
	{
		// some spinners clamp at bottom of range
		if (d < dMin)
			d = dMin;
	}
	const XML_Char* szNew = UT_convertToDimensionString(dimSpin, d, szPrecision); 

	_setSpinItemValue(edit, szNew);
}

// after the member variable for a control has been changed by the 
// user, we may need to synchronize the values of other controls. 
// this happens in two steps via this virtual function.
//
// step 1 -- the XP code fixes the necessary member variables
// step 2 -- the platform code copies them into dialog controls
//
// to make this work, the *first* step of a platform implementation 
// must be to call the XP (parent) implementation
//
// also, the platform code has to call _syncControls() once with 
// bAll set to UT_TRUE.  this should happen *after* all of the 
// member variables have been copied to the screen for the first 
// time, but before the dialog is displayed.

void AP_Dialog_Paragraph::_syncControls(tControl changed, UT_Bool bAll /* = UT_FALSE */)
{
	if (changed == id_SPIN_SPECIAL_INDENT)
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_FIRSTLINE, op_SYNC);
			break;

		default:
			break;
		}
	}

	if (changed == id_MENU_SPECIAL_INDENT)
	{
		double dDefault = 0.0;
		UT_Bool bDefault = UT_TRUE;

		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			dDefault = 0.0;
			break;

		case indent_FIRSTLINE:
		case indent_HANGING:
			// only change to default if existing value is zero
			dDefault = UT_convertDimensionless(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			if (dDefault > 0)
			{
				bDefault = UT_FALSE;
			}
			else
			{
				dDefault = 0.5;
			}
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}

		if (bDefault)
		{
			if (m_dim != DIM_IN)
				dDefault = UT_convertInchesToDimension(dDefault, m_dim); 

			const XML_Char* szNew = UT_convertToDimensionString(m_dim, dDefault, ".1"); 

			_setSpinItemValue(id_SPIN_SPECIAL_INDENT, szNew, op_SYNC);
		}
	}

	if (changed == id_SPIN_SPECIAL_SPACING)
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_MULTIPLE, op_SYNC);
			break;

		default:
			break;
		}
	}

	if (changed == id_MENU_SPECIAL_SPACING)
	{
		UT_Dimension dimOld = UT_determineDimension(_getSpinItemValue(id_SPIN_SPECIAL_SPACING), DIM_none);

		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
			_setSpinItemValue(id_SPIN_SPECIAL_SPACING, "1.0", op_SYNC);
			break;

		case spacing_ONEANDHALF:
			_setSpinItemValue(id_SPIN_SPECIAL_SPACING, "1.5", op_SYNC);
			break;

		case spacing_DOUBLE:
			_setSpinItemValue(id_SPIN_SPECIAL_SPACING, "2.0", op_SYNC);
			break;

		case spacing_ATLEAST:
		case spacing_EXACTLY:
			// only change to default if not dimensioned
			if (dimOld == DIM_none)
				_setSpinItemValue(id_SPIN_SPECIAL_SPACING, "12pt", op_SYNC);
			break;

		case spacing_MULTIPLE:
			// only change to default if dimensioned
			if (dimOld != DIM_none)
				_setSpinItemValue(id_SPIN_SPECIAL_SPACING, "1.0", op_SYNC);
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}

	// the preview needs to suck in the changed data (to cache it
	// for subsequent draws)
	m_paragraphPreview->setFormat((AP_Dialog_Paragraph::tAlignState) _getMenuItemValue(id_MENU_ALIGNMENT),
								  _getSpinItemValue(id_SPIN_SPECIAL_INDENT),
								  (AP_Dialog_Paragraph::tIndentState) _getMenuItemValue(id_MENU_SPECIAL_INDENT),
								  _getSpinItemValue(id_SPIN_LEFT_INDENT),
								  _getSpinItemValue(id_SPIN_RIGHT_INDENT),
								  _getSpinItemValue(id_SPIN_BEFORE_SPACING),
								  _getSpinItemValue(id_SPIN_AFTER_SPACING),
								  _getSpinItemValue(id_SPIN_SPECIAL_SPACING),
								  (AP_Dialog_Paragraph::tSpacingState) _getMenuItemValue(id_MENU_SPECIAL_SPACING));
	
	m_paragraphPreview->draw();
}

UT_Bool AP_Dialog_Paragraph::_wasChanged(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);
	
	return pItem->bChanged;
}

