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
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Prefs.h"

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

			_setMenuItemValue(id_MENU_ALIGNMENT, t, UT_FALSE);			
		}
								 
		sz = UT_getAttribute("margin-left", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_LEFT_INDENT, sz, UT_FALSE);

		sz = UT_getAttribute("margin-right", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_RIGHT_INDENT, sz, UT_FALSE);

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
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_FIRSTLINE, UT_FALSE);
			}
			else if (UT_convertDimensionless(sz) < (double) 0)
			{
				// if text-indent is less than margin-left, we have a "hanging" case
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_HANGING, UT_FALSE);
			}
			else
			{
				// they're equal then there's nothing special about them
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_NONE, UT_FALSE);
			}

			// TODO : FLIP THE SIGN ON THIS FIELD IF THE CASE IS "HANGING"!
			
			// set the value regardless; dialog will enable/disable field
			// if spacing is "NONE"
			_setSpinItemValue(id_SPIN_SPECIAL_INDENT, sz, UT_FALSE);
		}

		sz = UT_getAttribute("line-height", pProps);
		if (sz)
		{
			UT_uint32 nLen = strlen(sz);
			if (nLen > 1)
			{
				char * pPlusFound = strrchr(sz, '+');
				if (pPlusFound && *(pPlusFound + 1) == 0)
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ATLEAST, UT_FALSE);
				else if(UT_hasDimensionComponent(sz))
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_EXACTLY, UT_FALSE);
				else if(UT_strcmp("1.0", sz) == 0)
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_SINGLE, UT_FALSE);
				else if(UT_strcmp("1.5", sz) == 0)
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ONEANDHALF, UT_FALSE);
				else if(UT_strcmp("2.0", sz) == 0)
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_DOUBLE, UT_FALSE);
				else
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_MULTIPLE, UT_FALSE);
			}

			// set the spin contents regardless of menu content; platforms will
			// enable or disable the spin item for varying states of menu
			_setSpinItemValue(id_SPIN_SPECIAL_SPACING, sz, UT_FALSE);
		}

		sz = UT_getAttribute("margin-top", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_BEFORE_SPACING, sz, UT_FALSE);
			
		sz = UT_getAttribute("margin-bottom", pProps);
		if (sz)
			_setSpinItemValue(id_SPIN_AFTER_SPACING, sz, UT_FALSE);
		
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
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_INDETERMINATE, UT_FALSE);
			else if (orphans > 0 || widows > 0)
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_TRUE, UT_FALSE);
			else
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_FALSE, UT_FALSE);
		}

		sz = UT_getAttribute("keep-together", pProps);
		if (sz)
		{
			if (UT_stricmp(sz, "yes") == 0)
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_TRUE, UT_FALSE);
			else
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_FALSE, UT_FALSE);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_LINES, check_INDETERMINATE, UT_FALSE);

		sz = UT_getAttribute("keep-with-next", pProps);
		if (sz)
		{
			if (UT_stricmp(sz, "yes") == 0)
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_TRUE, UT_FALSE);
			else
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_FALSE, UT_FALSE);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_NEXT, check_INDETERMINATE, UT_FALSE);
			
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

			UT_XML_strncpy(pTmp, nSize, pString);
			// stick a '+' at the end
			pTmp[nSize] = '+';
			UT_XML_cloneString(p->val, pTmp);
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
	
	m_paragraphPreview = new AP_Preview_Paragraph(gc);
	UT_ASSERT(m_paragraphPreview);
	
	m_paragraphPreview->setWindowSize(width, height);

	// TODO : any setup of the GC for drawing

}

void AP_Dialog_Paragraph::_setMenuItemValue(tControl item, UT_sint32 value,
											UT_Bool bToggleChanged /* = UT_FALSE */)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	// menu items have integers as data, so store it in a pointer
	pItem->pData = (void *) value;

	if (bToggleChanged)
		pItem->bChanged = UT_TRUE;
}


UT_uint32 AP_Dialog_Paragraph::_getMenuItemValue(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	return (UT_uint32) pItem->pData;
}
		
void AP_Dialog_Paragraph::_setCheckItemValue(tControl item, tCheckState value,
											 UT_Bool bToggleChanged /* = UT_FALSE */)											 
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);

	// check buttons have integers as data, so store it in a pointer
	pItem->pData = (void *) value;

	if (bToggleChanged)
		pItem->bChanged = UT_TRUE;
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
											UT_Bool bToggleChanged /* = UT_FALSE */)											
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);
	
	// spin items have pointers to static XML_Char buffers as data, so we
	// don't free them, but extract a pointer and copy the string contents
	UT_ASSERT(pItem->pData && value);
	UT_XML_strncpy((XML_Char *) pItem->pData, SPIN_BUF_TEXT_SIZE, value);

	if (bToggleChanged)
		pItem->bChanged = UT_TRUE;
}

const XML_Char * AP_Dialog_Paragraph::_getSpinItemValue(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem && pItem->pData);
	
	return (const XML_Char *) pItem->pData;
}

UT_Bool AP_Dialog_Paragraph::_wasChanged(tControl item)
{
	UT_ASSERT((UT_uint32) item <= m_vecProperties.getItemCount());

	sControlData * pItem = (sControlData *) m_vecProperties.getNthItem((UT_uint32) item);
	UT_ASSERT(pItem);
	
	return pItem->bChanged;
}

/************************************************************************/

/*
  _formatAsUnitQuantity() takes a free-form string from the user (as one
  might type into the spinbutton entry area) and converts it to a proper
  string for update to the display.  Input might look like "2.3cm".
  The output might look like "4.34in", with m_dim used as the output
  dimension.  Any necessary conversions to m_dim will be performed.

  This function returns a pointer to a static buffer; use it quickly.

  WARNING : this function might not handle spaces between numerical
  WARNING : and unit portions of the string (i.e. "1.34 in").
  
*/

#if 0
const XML_Char * AP_Dialog_Paragraph::_formatAsUnitQuantity(const XML_Char * input)
{
	double num = UT_convertToInches((const char *) input);
	const char * string = UT_convertToDimensionString(m_dim, num, ".1");
	UT_ASSERT(string);
	
	// WARNING : I don't know much about XML_Char.  I think we're using UTF-8
	// WARNING : throughout, which means we aren't using wide characters
	// WARNING : but multi-byte encoding, so ASCII <=> XML_Char should be
	// WARNING : "safe".  I wrote this code in a book store where I couldn't
	// WARNING : ask anyone who knows more than me.

	return (XML_Char *) string;
}

const XML_Char * AP_Dialog_Paragraph::_formatAsUnitlessQuantity(const XML_Char * input)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return (XML_Char *) NULL;
}
#endif

/*
  _extractDimension() takes a number/unit string and tries to
  determine the unit system used within.  Input might look like
  "1.25in", and this function would return DIM_IN.  If this function
  can't find a unit tag in the string, it returns the default for
  this instance of this class (m_dim).
*/

#if 0

UT_Dimension AP_Dialog_Paragraph::_extractDimension(XML_Char * input)
{
	UT_ASSERT(input);
	
	// algorithm is cheap:  search foreward until we hit letters,
	// ignoring spaces, punctuation, and numbers.  compare the rest
	// to known dimensions.

	XML_Char * tmp = (XML_Char *) calloc(UT_XML_strlen(input), sizeof(XML_Char));
	UT_ASSERT(tmp);
	
	XML_Char * t = tmp;
	for (XML_Char * i = input; *i != NULL; i++)
		if (isAsciiLetter(*i))
		{
			*t = *i;
			t++;
		}
	++t = NULL;

	// find a dimension
	UT_Dimension foundDim = UT_determineDimension((const char *) tmp, DIM_IN);

	// find the numeric part
	double foundNum = UT_convertDimensionless((const char *) input);

	// this function was not finished because I don't think I need it
	
	FREEP(tmp);

	return 
}

#endif
