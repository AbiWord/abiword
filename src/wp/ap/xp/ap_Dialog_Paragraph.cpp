/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_debugmsg.h"
#include "ut_units.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Prefs.h"

#include "ap_Strings.h"

#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"

#include "fp_PageSize.h"
#include "fp_Run.h"

#include "ap_Preview_Paragraph.h"
#include "ap_Dialog_Paragraph.h"
#include "ap_Prefs_SchemeIds.h"

#include "pp_Property.h"

AP_Dialog_Paragraph::AP_Dialog_Paragraph(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogparagraph")
{
	m_answer = a_OK;
	m_paragraphPreview = NULL;
	m_pFrame = NULL;

	// determine unit system to use in this dialog
	const gchar * szRulerUnits;
	UT_return_if_fail (m_pApp);

	XAP_Prefs* pPrefs = m_pApp->getPrefs();
	UT_return_if_fail (pPrefs);

	const bool bHasRulerUnits =
		pPrefs->getPrefsValue((gchar*)AP_PREF_KEY_RulerUnits, &szRulerUnits);

	m_dim = bHasRulerUnits ? UT_determineDimension(szRulerUnits) : DIM_IN;


	_addPropertyItem (id_MENU_ALIGNMENT,		sControlData(align_UNDEF));
	_addPropertyItem (id_SPIN_LEFT_INDENT,		sControlData());
	_addPropertyItem (id_SPIN_RIGHT_INDENT,		sControlData());
	_addPropertyItem (id_MENU_SPECIAL_INDENT,	sControlData(indent_UNDEF));
	_addPropertyItem (id_SPIN_SPECIAL_INDENT,	sControlData());
	_addPropertyItem (id_SPIN_BEFORE_SPACING,	sControlData());
	_addPropertyItem (id_SPIN_AFTER_SPACING,	sControlData());
	_addPropertyItem (id_MENU_SPECIAL_SPACING,	sControlData(spacing_UNDEF));
	_addPropertyItem (id_SPIN_SPECIAL_SPACING,	sControlData());
	_addPropertyItem (id_CHECK_WIDOW_ORPHAN,	sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_KEEP_LINES,		sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_PAGE_BREAK,		sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_SUPPRESS,		sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_NO_HYPHENATE,	sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_KEEP_NEXT,		sControlData(check_INDETERMINATE));
	_addPropertyItem (id_CHECK_DOMDIRECTION,	sControlData(check_INDETERMINATE));
}

AP_Dialog_Paragraph::~AP_Dialog_Paragraph(void)
{
	DELETEP(m_paragraphPreview);

	UT_VECTOR_PURGEALL(sControlData *, m_vecProperties);
}

bool AP_Dialog_Paragraph::setDialogData(const PP_PropertyVector & pProps)
{
	UT_return_val_if_fail (!pProps.empty(), false);

	// NOTICE : When setting values, this function always calls
	// NOTICE : _set[thing]ItemValue() with the bToggleDirty flag
	// NOTICE : set to false, because these are the "un-dirty"
	// NOTICE : values.

	{
		std::string sz;

		sz = PP_getAttribute("text-align", pProps);
		if (!sz.empty())
		{
			tAlignState t = align_LEFT;

			if (sz == "center")
				t = align_CENTERED;
			else if (sz == "right")
				t = align_RIGHT;
			else if (sz == "justify")
				t = align_JUSTIFIED;
			else if (sz == "left")
				t = align_LEFT;
			else {
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}

			_setMenuItemValue(id_MENU_ALIGNMENT, t, op_INIT);
		}

		sz = PP_getAttribute("dom-dir", pProps);
		if (!sz.empty())
		{
			tCheckState t = check_FALSE;

			if (sz == "ltr")
				t = check_FALSE;
			else if (sz == "rtl")
				t = check_TRUE;
			else {
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}

			_setCheckItemValue(id_CHECK_DOMDIRECTION, t, op_INIT);
		}

		sz = PP_getAttribute("margin-left", pProps);
		if (!sz.empty())
			_setSpinItemValue(id_SPIN_LEFT_INDENT, sz.c_str(), op_INIT);

		sz = PP_getAttribute("margin-right", pProps);
		if (!sz.empty())
			_setSpinItemValue(id_SPIN_RIGHT_INDENT, sz.c_str(), op_INIT);

		sz = PP_getAttribute("text-indent", pProps);
		if (!sz.empty())
		{
			// NOTE : Calling UT_convertDimensionless() _discards_ all
			// NOTE : unit system information.  IFF all units are
			// NOTE : consistent among all paragraph properties will
			// NOTE : the comparisons be valid.  For now this should be
			// NOTE : valid.

			double f = UT_convertDimensionless(sz.c_str());
			if (f > (double) 0)
			{
				// if text-indent is greater than margin-left, we have a "first line" case
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_FIRSTLINE, op_INIT);
			}
			else if (f < (double) 0)
			{
				// if text-indent is less than margin-left, we have a "hanging" case
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_HANGING, op_INIT);
			}
			else
			{
				// they're equal then there's nothing special about them
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_NONE, op_INIT);
			}

			// set the value regardless; dialog will enable/disable field
			// if spacing is "NONE".  Must flip the sign (strip minus)
			// to give illusion of Word's definitions of indent/margin.

			std::string newSz;

			if (sz[0] == '-') {
				newSz = sz.substr(1);
			} else {
				newSz = sz;
			}

			_setSpinItemValue(id_SPIN_SPECIAL_INDENT, newSz.c_str(), op_INIT);
		}

		sz = PP_getAttribute("line-height", pProps);
		if (!sz.empty())
		{
			std::size_t idx = sz.find('+');
			if ((idx != std::string::npos) && (idx + 1 == sz.size()))
			{
				_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ATLEAST, op_INIT);

				// need to strip off that plus
				std::string val = sz.substr(0, idx);
				_setSpinItemValue(id_SPIN_SPECIAL_SPACING, val.c_str(), op_INIT);
			}
			else
			{
				if(UT_hasDimensionComponent(sz.c_str()))
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_EXACTLY, op_INIT);
					//see Bug 10086 for fabs() usage
				else if((sz == "1.0") || (fabs(UT_convertDimensionless(sz.c_str()) - (double) 1.0) < 1.0e-7))
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_SINGLE, op_INIT);
				else if((sz == "1.5") || (fabs(UT_convertDimensionless(sz.c_str()) - (double) 1.5) < 1.0e-7))
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_ONEANDHALF, op_INIT);
				else if((sz == "2.0") || (fabs(UT_convertDimensionless(sz.c_str()) - (double) 2.0) < 1.0e-7))
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_DOUBLE, op_INIT);
				else
					_setMenuItemValue(id_MENU_SPECIAL_SPACING, spacing_MULTIPLE, op_INIT);

				// set the spin contents regardless of menu content; platforms will
				// enable or disable the spin item for varying states of menu
				_setSpinItemValue(id_SPIN_SPECIAL_SPACING, sz.c_str(), op_INIT);
			}
		}

		sz = PP_getAttribute("margin-top", pProps);
		if (!sz.empty())
			_setSpinItemValue(id_SPIN_BEFORE_SPACING, sz.c_str(), op_INIT);

		sz = PP_getAttribute("margin-bottom", pProps);
		if (!sz.empty())
			_setSpinItemValue(id_SPIN_AFTER_SPACING, sz.c_str(), op_INIT);

		{
			// NOTE : "orphans" and "widows" hold a number specifying the number
			// NOTE : of lines to consider an orphaned or widowed piece of text.
			// NOTE : If they're both 0 they're off.  If either is greater than
			// NOTE : 0, then some form of control is in effect.  If the property
			// NOTE : is not set, they're indeterminate (e.g. because we're setting
		    // NOTE : properties for text with multiple "orphans" values), or because
  		    // NOTE : the block has no value for orphans/widows here.

			bool bNoOrphans = false;
			bool bNoWidows = false;

			double orphans = 0, widows = 0;

			sz = PP_getAttribute("orphans", pProps);
			if (!sz.empty())
				orphans = UT_convertDimensionless(sz.c_str());
			else
				bNoOrphans = true;

			sz = PP_getAttribute("widows", pProps);
			if (!sz.empty())
				widows = UT_convertDimensionless(sz.c_str());
			else
				bNoWidows = true;

			if (bNoOrphans && bNoWidows)
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_INDETERMINATE, op_INIT);
			else if (orphans > 0 || widows > 0)
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_WIDOW_ORPHAN, check_FALSE, op_INIT);
		}

		sz = PP_getAttribute("keep-together", pProps);
		if (!sz.empty())
		{
			if (sz == "yes")
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_KEEP_LINES, check_FALSE, op_INIT);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_LINES, check_INDETERMINATE, op_INIT);

		sz = PP_getAttribute("keep-with-next", pProps);
		if (!sz.empty())
		{
			if (sz == "yes")
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_TRUE, op_INIT);
			else
				_setCheckItemValue(id_CHECK_KEEP_NEXT, check_FALSE, op_INIT);
		}
		else
			_setCheckItemValue(id_CHECK_KEEP_NEXT, check_INDETERMINATE, op_INIT);

		// these are not like the others, they set fields on this, not dialogData.
		sz = PP_getAttribute("page-margin-left", pProps);
		if (!sz.empty())
		{
			m_pageLeftMargin = sz;
		}
		else
		{
			m_pageLeftMargin = PP_lookupProperty("page-margin-left")->getInitial();
		}

		sz = PP_getAttribute("page-margin-right", pProps);
		if (!sz.empty())
		{
			m_pageRightMargin = sz;
		}
		else
		{
			m_pageRightMargin = PP_lookupProperty("page-margin-right")->getInitial();
		}

		// TODO : add these to PP_Property (pp_Property.cpp) !!!
		// TODO : and to FV_View::getBlockFormat (or else they won't come in)
		/*
		  m_pageBreakBefore;
		  m_suppressLineNumbers;
		  m_noHyphenate;
		*/
	}

	return true;
}

bool AP_Dialog_Paragraph::getDialogData(PP_PropertyVector & pProps)
{
	// ensure it is empty first.
	pProps.clear();

	// only do this if the control has a proper value
	if (_wasChanged(id_MENU_ALIGNMENT)  && _getMenuItemValue(id_MENU_ALIGNMENT))
	{
		const char* val = nullptr;
		switch (_getMenuItemValue(id_MENU_ALIGNMENT))
		{
			case align_LEFT:
				val = "left";
				break;
			case align_CENTERED:
				val = "center";
				break;
			case align_RIGHT:
				val = "right";
				break;
			case align_JUSTIFIED:
				val = "justify";
				break;
			default:
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
		if (val) {
			pProps.push_back("text-align");
			pProps.push_back(val);
		}
	}


	if (_wasChanged(id_CHECK_DOMDIRECTION))
	{
		pProps.push_back("dom-dir");
		pProps.push_back(_getCheckItemValue(id_CHECK_DOMDIRECTION) == check_TRUE
						 ? "rtl" : "ltr");
	}


	if (_wasChanged(id_SPIN_LEFT_INDENT))
	{
		pProps.push_back("margin-left");
		pProps.push_back(_getSpinItemValue(id_SPIN_LEFT_INDENT));
	}

	if (_wasChanged(id_SPIN_RIGHT_INDENT))
	{
		pProps.push_back("margin-right");
		pProps.push_back(_getSpinItemValue(id_SPIN_RIGHT_INDENT));
	}

	// TODO : The logic here might not be bulletproof.  If the user triggers
	// TODO : a change in the TYPE of special indent (hanging, first line,
	// TODO : none), we will always save what's in the box as a property.
	// TODO : One could make it smarter with a stronger contract with
	// TODO : the platform interfaces.

	if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) &&
		(_wasChanged(id_MENU_SPECIAL_INDENT) || _wasChanged(id_SPIN_SPECIAL_INDENT)))
	{
		pProps.push_back("text-indent");

		tIndentState i = (tIndentState) _getMenuItemValue(id_MENU_SPECIAL_INDENT);

		if (i == indent_NONE)
			pProps.push_back(UT_convertInchesToDimensionString(m_dim, 0));
		else if (i == indent_FIRSTLINE)
			pProps.push_back(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		else if (i == indent_HANGING)
		{
			// we have to flip the sign for "hanging" indents to a negative quantity for
			// storage in the document as a text-indent

			// get with no dimension
			UT_Dimension dim = UT_determineDimension(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			double val = UT_convertDimensionless(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
			// Convert to inches
			val = UT_convertDimToInches(val, dim);

			// flip sign
			val = val * (double) -1;

			// store the reconstructed
			pProps.push_back(UT_convertInchesToDimensionString(dim, val));
		}
	}

	if (_wasChanged(id_SPIN_BEFORE_SPACING))
	{
		pProps.push_back("margin-top");
		pProps.push_back(_getSpinItemValue(id_SPIN_BEFORE_SPACING));
	}

	if (_wasChanged(id_SPIN_AFTER_SPACING))
	{
		pProps.push_back("margin-bottom");
		pProps.push_back(_getSpinItemValue(id_SPIN_AFTER_SPACING));
	}

	// TODO : The logic here might not be bulletproof.  If the user triggers
	// TODO : a change in the TYPE of special indent (single, double, etc.)
	// TODO : we will always save what's in the box as a property.
	// TODO : One could make it smarter with a stronger contract with
	// TODO : the platform interfaces.

	if(_getMenuItemValue(id_MENU_SPECIAL_SPACING) &&
		 (_wasChanged(id_MENU_SPECIAL_SPACING) || _wasChanged(id_SPIN_SPECIAL_SPACING)))
	{
		pProps.push_back("line-height");

		// normal spacings (single, 1.5, double) are just simple numbers.
		// "at least" needs a "+" at the end of the number (no units).
		// "exactly" simply has units.
		// "multiple" has no units.

		const gchar * pString = _getSpinItemValue(id_SPIN_SPECIAL_SPACING);

		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
			pProps.push_back("1.0");
			break;
		case spacing_ONEANDHALF:
			pProps.push_back("1.5");
			break;
		case spacing_DOUBLE:
			pProps.push_back("2.0");
			break;
		case spacing_ATLEAST: {
			std::string value = pString;
			// stick a '+' at the end
			value += '+';
			pProps.push_back(value);
			break;
		}
		case spacing_EXACTLY:
			// fallthrough
		case spacing_MULTIPLE:
			// both these cases either do or don't have units associated with them.
			// the platform dialog code takes care of that.
			pProps.push_back(pString);
			break;
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
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
			pProps.push_back("orphans");

			if (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE) {
				pProps.push_back("2");
			} else {
				pProps.push_back("0");
			}
		}

		{
			pProps.push_back("widows");

			if (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE) {
				pProps.push_back("2");
			} else {
				pProps.push_back("0");
			}
		}
	}

	if (_wasChanged(id_CHECK_KEEP_LINES))
	{
		pProps.push_back("keep-together");

		if (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE) {
			pProps.push_back("yes");
		} else {
			pProps.push_back("no");
		}
	}

	if (_wasChanged(id_CHECK_KEEP_NEXT))
	{
		pProps.push_back("keep-with-next");

		if (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE) {
			pProps.push_back("yes");
		} else {
			pProps.push_back("no");
		}
	}

	// TODO : add these to PP_Property (pp_Property.cpp) !!!
	/*
	  m_pageBreakBefore;
	  m_suppressLineNumbers;
	  m_noHyphenate;
	*/

	return true;
}

AP_Dialog_Paragraph::tAnswer AP_Dialog_Paragraph::getAnswer(void) const
{
	return m_answer;
}

/************************************************************************/

// how many characters do we want to pull from the current paragraph
// to fill our preview

#define NUM_CHARS_FOR_SAMPLE 100

void AP_Dialog_Paragraph::_createPreviewFromGC(GR_Graphics * gc,
											   UT_uint32 width,
											   UT_uint32 height)
{
	UT_return_if_fail (gc);

	// g_free any attached preview
	DELETEP(m_paragraphPreview);

	// platform's runModal should have set this
	UT_return_if_fail (m_pFrame);

	AV_View * baseview = m_pFrame->getCurrentView();
	UT_return_if_fail (baseview);

	FV_View * view = static_cast<FV_View *> (baseview);

	FL_DocLayout * dl = view->getLayout();
	UT_return_if_fail (dl);

	fl_BlockLayout * bl = dl->findBlockAtPosition((PT_DocPosition) view->getPoint());
	UT_return_if_fail (bl);

	const char *pfont = NULL;

	fp_Run * run = bl->findRunAtOffset(view->getPoint()-bl->getPosition());
	if (run) {
		const PP_AttrProp *prop = run->getSpanAP();
		if (prop) {
			if (prop->getProperty("font-family",pfont)) {
				UT_DEBUGMSG(("PREVIEW font=%s\n",pfont));
			}
		}
	}

	UT_GrowBuf gb;
	bool hadMem = bl->getBlockBuf(&gb);

	UT_UCSChar * tmp = NULL;
	if (hadMem && gb.getLength() > 0)
	{
		gb.truncate(NUM_CHARS_FOR_SAMPLE);
		UT_UCS4_cloneString(&tmp, (UT_UCSChar *) gb.getPointer(0));
	}
	else
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();

		// if the paragraph was empty, use our sample
		std::string s;
		pSS->getValueUTF8(AP_STRING_ID_DLG_Para_PreviewSampleFallback, s);
		UT_UCS4String ucs4str(s.c_str());
		UT_UCS4_cloneString(&tmp, ucs4str.ucs4_str());
	}

	m_paragraphPreview = new AP_Preview_Paragraph(gc, tmp, this, pfont);

	FREEP(tmp);

	UT_return_if_fail (m_paragraphPreview);

	m_paragraphPreview->setWindowSize(width, height);

	// TODO : any setup of the GC for drawing

}

void AP_Dialog_Paragraph::_setMenuItemValue(tControl item, UT_sint32 value,
											tOperation op /* = op_UICHANGE */)
{
	UT_return_if_fail (item <= m_vecProperties.getItemCount());

	sControlData * pItem = _getPropertyItem (item);
	UT_return_if_fail (pItem);

	pItem->setData (value);

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->changed (true);

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}


UT_sint32 AP_Dialog_Paragraph::_getMenuItemValue(tControl item)
{
	UT_return_val_if_fail (item <= m_vecProperties.getItemCount(), 0);

	sControlData * pItem = _getPropertyItem (item);
	UT_return_val_if_fail (pItem, 0);

	UT_sint32 value = 0;
	pItem->getData (value);
	return value;
}

void AP_Dialog_Paragraph::_setCheckItemValue(tControl item, tCheckState value,
											tOperation op /* = op_UICHANGE */)
{
	UT_return_if_fail (item <= m_vecProperties.getItemCount());

	sControlData * pItem = _getPropertyItem (item);
	UT_return_if_fail (pItem);

	pItem->setData (value);

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->changed (true);

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}

AP_Dialog_Paragraph::tCheckState AP_Dialog_Paragraph::_getCheckItemValue(tControl item)
{
	UT_return_val_if_fail (item <= m_vecProperties.getItemCount(), check_INDETERMINATE);

	sControlData * pItem = _getPropertyItem (item);
	UT_return_val_if_fail (pItem, check_INDETERMINATE);

	tCheckState value = check_INDETERMINATE;
	if (pItem)
		pItem->getData (value);
	return value;
}

const gchar * AP_Dialog_Paragraph::_makeAbsolute(const gchar * value)
{
	UT_uint32 i = 0;
	const gchar * tempstring = value;

	// from the start of the string, if a character is a space, walk on.
	// when we hit a '-', we leave the pointer at value + i + 1
	while (value[i] && value[i] == ' ')
	{
		tempstring++;
		i++;
	}

	// we're at a non-space
	if (value[i] == '-')
		tempstring++;

	return tempstring;
}
void AP_Dialog_Paragraph::_setSpinItemValue(tControl item, const gchar * value,
											tOperation op /* = op_UICHANGE */)
{
	UT_return_if_fail (item <= m_vecProperties.getItemCount() && value);

	sControlData * pItem = _getPropertyItem (item);
	UT_return_if_fail (pItem);

	// some spinbuttons have special data requirements
	switch(item)
	{
	case id_SPIN_LEFT_INDENT:
	case id_SPIN_RIGHT_INDENT:
	case id_SPIN_SPECIAL_INDENT:
		pItem->setData (reinterpret_cast<const gchar *>(UT_reformatDimensionString (m_dim, value)));
		break;

	case id_SPIN_BEFORE_SPACING:
	case id_SPIN_AFTER_SPACING:
		{
			/* NOTE : line spacing can't be negative, so take absolute value:
			 */
			const char * abs_value = UT_reformatDimensionString (DIM_PT, _makeAbsolute (value));
			pItem->setData (reinterpret_cast<const gchar *>(abs_value));
		}
		break;

	case id_SPIN_SPECIAL_SPACING:
		if (_getMenuItemValue (id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
			{
				const char * abs_value = UT_reformatDimensionString (DIM_none, _makeAbsolute (value), ".2");
				pItem->setData (reinterpret_cast<const gchar *>(abs_value));
			}
		else
			{
				const char * abs_value = UT_reformatDimensionString (DIM_PT, _makeAbsolute (value));
				pItem->setData (reinterpret_cast<const gchar *>(abs_value));
			}
		break;

	default:
		/* all others get a simple string copy to the static member
		 */
		pItem->setData (value);
	}

	if ((op == op_UICHANGE) || (op == op_SYNC))
		pItem->changed (true);

	// for UI-driven changes, may need to sync other controls
	if (op == op_UICHANGE)
		_syncControls(item);
}

const gchar * AP_Dialog_Paragraph::_getSpinItemValue(tControl item)
{
	UT_return_val_if_fail (item <= m_vecProperties.getItemCount(), NULL);

	sControlData * pItem = _getPropertyItem (item);
	UT_return_val_if_fail (pItem, NULL);

	const gchar * value = NULL;
	pItem->getData (value);
	UT_ASSERT_HARMLESS(value);
	return value;
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
	UT_ASSERT_HARMLESS(amt); // zero makes no sense

	// get current value from member
	const gchar* szOld = _getSpinItemValue(edit);
	double d = UT_convertDimensionless(szOld);

	// figure out which dimension and units to spin in
	UT_Dimension dimSpin = m_dim;
	double dSpinUnit = SPIN_INCR_PT;
	double dMin = 0.0;
	bool bMin = false;

	switch (edit)
	{
	case id_SPIN_SPECIAL_INDENT:
		dMin = 0.0;
		bMin = true;
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
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		break;

	case id_SPIN_BEFORE_SPACING:
	case id_SPIN_AFTER_SPACING:
		dimSpin = DIM_PT;
		dSpinUnit = 6.0;
		dMin = 0.0;
		bMin = true;
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
			dSpinUnit = 0.1;
			dMin = 0.5;
			bMin = true;
			break;

		case spacing_EXACTLY:
			dMin = 1;
			// fall through
		case spacing_ATLEAST:
			dimSpin = DIM_PT;
			dSpinUnit = SPIN_INCR_PT;
			bMin = true;
			break;

		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	const gchar* szNew = UT_formatDimensionString(dimSpin, d, szPrecision);

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
// bAll set to true.  this should happen *after* all of the
// member variables have been copied to the screen for the first
// time, but before the dialog is displayed.

void AP_Dialog_Paragraph::_syncControls(tControl changed, bool /*bAll  = false */)
{
	if(changed == id_SPIN_LEFT_INDENT)
	{
		// need to check the limits
		// cannot go past left page margin.
		// TODO : is there a minimum text width?

		double leftPageMargin = UT_convertToDimension(m_pageLeftMargin.c_str(), m_dim);
		double rightIndent = UT_convertToDimension(_getSpinItemValue(id_SPIN_RIGHT_INDENT), m_dim);

		if(-UT_convertToDimension(_getSpinItemValue(id_SPIN_LEFT_INDENT), m_dim) >
					leftPageMargin)
		{
			_setSpinItemValue(id_SPIN_LEFT_INDENT,
									(const gchar *)UT_formatDimensionString(m_dim, -leftPageMargin),
									op_SYNC);
		}

		// nor past pagesize - rightIndent on right.
  		if(UT_convertDimensionless(_getSpinItemValue(id_SPIN_LEFT_INDENT)) >
						UT_convertInchesToDimension(m_iMaxWidth, m_dim) - rightIndent)
  		{
  			_setSpinItemValue(id_SPIN_LEFT_INDENT,
  									(const gchar *)UT_convertInchesToDimensionString(m_dim, m_iMaxWidth - rightIndent),
  									op_SYNC);
  		}
	}

	if(changed == id_SPIN_RIGHT_INDENT)
	{
		// need to check the limits
		// cannot go past right page margin.

		double rightPageMargin = UT_convertToDimension(m_pageRightMargin.c_str(), m_dim);
		double leftIndent = UT_convertToDimension(_getSpinItemValue(id_SPIN_LEFT_INDENT), m_dim);

		if(-UT_convertToDimension(_getSpinItemValue(id_SPIN_RIGHT_INDENT), m_dim) >
					rightPageMargin)
		{
			_setSpinItemValue(id_SPIN_RIGHT_INDENT,
									(const gchar *)UT_formatDimensionString(m_dim, -rightPageMargin),
									op_SYNC);
		}

		// nor can we force text left past pagesize, minus left margin
  		if(UT_convertDimensionless(_getSpinItemValue(id_SPIN_RIGHT_INDENT)) >
						UT_convertInchesToDimension(m_iMaxWidth, m_dim) - leftIndent)
  		{
  			_setSpinItemValue(id_SPIN_RIGHT_INDENT,
  									(const gchar *)UT_convertInchesToDimensionString(m_dim, m_iMaxWidth - leftIndent),
  									op_SYNC);
  		}
	}

	if (changed == id_MENU_SPECIAL_INDENT || changed == id_SPIN_SPECIAL_INDENT)
	{
		double dDefault = 0.0;
		bool bDefault = true;

		double sign = -1.0;
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		  sign = +1.0;

		if (changed == id_MENU_SPECIAL_INDENT)
		{
			switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
			{
			case indent_NONE:
				dDefault = 0.0;
				break;

			case indent_FIRSTLINE:
			case indent_HANGING:
				// only change to default if existing value is zero
				dDefault = UT_convertDimensionless(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
				if (dDefault == 0)
				{
					bDefault = false;
				}
				else
				{
					dDefault = 0.5;
				}
				break;

			default:
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				break;
			}

			if (bDefault)
			{
				if (m_dim != DIM_IN)
					dDefault = UT_convertInchesToDimension(dDefault, m_dim);

				const gchar* szNew = UT_convertInchesToDimensionString(m_dim, dDefault, ".1");

				_setSpinItemValue(id_SPIN_SPECIAL_INDENT, szNew, op_SYNC);
			}
		}
		else /* (changed == id_SPIN_SPECIAL_INDENT) */
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

		// if spin contains a negative number, we flip the direction of the indent.
		double val = UT_convertDimensionless(_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		if (val < 0)
		{
			sign = -sign;

			// sometimes this appears to have no effect. why?
			if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_HANGING, op_SYNC);
			else if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_HANGING)
				_setMenuItemValue(id_MENU_SPECIAL_INDENT, indent_FIRSTLINE, op_SYNC);

			const gchar* szNew = UT_convertInchesToDimensionString(m_dim, -val, ".1");
			_setSpinItemValue(id_SPIN_SPECIAL_INDENT, szNew, op_SYNC);
		}

		// sanity check.

		double leftIndent =
		UT_convertToDimension(_getSpinItemValue(id_SPIN_LEFT_INDENT), m_dim);

		double effectiveLeftMargin = leftIndent + (UT_convertToDimension
		  (_getSpinItemValue(id_SPIN_SPECIAL_INDENT), m_dim) * sign);

		double leftPageMargin = UT_convertToDimension(m_pageLeftMargin.c_str(), m_dim);
		double rightIndent = UT_convertToDimension(_getSpinItemValue(id_SPIN_RIGHT_INDENT), m_dim);

		if(-effectiveLeftMargin > leftPageMargin)
		{
			_setSpinItemValue(id_SPIN_SPECIAL_INDENT,
									(const gchar *)UT_formatDimensionString(m_dim, -leftPageMargin),
									op_SYNC);
		} 

  		if(effectiveLeftMargin >
			UT_convertInchesToDimension(m_iMaxWidth, m_dim) - rightIndent)
  		{
  			_setSpinItemValue(id_SPIN_SPECIAL_INDENT,
  									(const gchar *)UT_convertInchesToDimensionString(m_dim, m_iMaxWidth - rightIndent),
  									op_SYNC);
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
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}

	// the preview needs to suck in the changed data (to cache it
	// for subsequent draws)
	UT_BidiCharType iDir;
	if(_getCheckItemValue(id_CHECK_DOMDIRECTION) == check_TRUE)
		iDir = UT_BIDI_RTL;
	else if(_getCheckItemValue(id_CHECK_DOMDIRECTION) == check_FALSE)
		iDir = UT_BIDI_LTR;
	else
	{
		// nothing given -- default to LTR
		UT_DEBUGMSG(("AP_Dialog_Paragraph::_syncControls: no value of dom-dir,"
					 " defaulting to LTR\n"));
		iDir = UT_BIDI_LTR;
	}

	m_paragraphPreview->setFormat(m_pageLeftMargin.c_str(),
									m_pageRightMargin.c_str(),
									(AP_Dialog_Paragraph::tAlignState) _getMenuItemValue(id_MENU_ALIGNMENT),
									_getSpinItemValue(id_SPIN_SPECIAL_INDENT),
									(AP_Dialog_Paragraph::tIndentState) _getMenuItemValue(id_MENU_SPECIAL_INDENT),
									_getSpinItemValue(id_SPIN_LEFT_INDENT),
									_getSpinItemValue(id_SPIN_RIGHT_INDENT),
									_getSpinItemValue(id_SPIN_BEFORE_SPACING),
									_getSpinItemValue(id_SPIN_AFTER_SPACING),
									_getSpinItemValue(id_SPIN_SPECIAL_SPACING),
									(AP_Dialog_Paragraph::tSpacingState) _getMenuItemValue(id_MENU_SPECIAL_SPACING),
								  iDir);

	m_paragraphPreview->queueDraw();
}

bool AP_Dialog_Paragraph::_wasChanged(tControl item)
{
	UT_return_val_if_fail (item <= m_vecProperties.getItemCount(), false);

	sControlData * pItem = _getPropertyItem (item);
	UT_return_val_if_fail (pItem, false);

	return pItem->changed ();
}

void AP_Dialog_Paragraph::_addPropertyItem (tControl index, const sControlData & control_data)
{
	sControlData * pDataCopy = 0;

	try
		{
			pDataCopy = new sControlData(control_data);
		}
	catch(...)
		{
			pDataCopy = 0;
		}
	UT_return_if_fail (pDataCopy);

	m_vecProperties.setNthItem (static_cast<UT_uint32>(index), pDataCopy, 0);
}

AP_Dialog_Paragraph::sControlData::sControlData (UT_sint32 data) :
	m_siData(data),
	m_csData(check_INDETERMINATE),
	m_szData(0),
	m_bChanged(false)
{
	// 
}

AP_Dialog_Paragraph::sControlData::sControlData (tCheckState data) :
	m_siData(0),
	m_csData(data),
	m_szData(0),
	m_bChanged(false)
{
	// 
}

/* default is empty string
 */
AP_Dialog_Paragraph::sControlData::sControlData (gchar * data) :
	m_siData(0),
	m_csData(check_INDETERMINATE),
	m_szData(new gchar[SPIN_BUF_TEXT_SIZE]),
	m_bChanged(false)
{
	m_szData[SPIN_BUF_TEXT_SIZE-1] = 0;
	setData (data);
}

AP_Dialog_Paragraph::sControlData::sControlData (const sControlData & rhs) :
	m_siData(rhs.m_siData),
	m_csData(rhs.m_csData),
	m_szData(rhs.m_szData ? new gchar[SPIN_BUF_TEXT_SIZE] : 0),
	m_bChanged(false)
{
	if (m_szData)
		memcpy (m_szData, rhs.m_szData, SPIN_BUF_TEXT_SIZE * sizeof (gchar));
}

AP_Dialog_Paragraph::sControlData::~sControlData ()
{
	DELETEPV(m_szData);
}

AP_Dialog_Paragraph::sControlData & AP_Dialog_Paragraph::sControlData::operator= (const sControlData & rhs)
{
	m_siData = rhs.m_siData;
	m_csData = rhs.m_csData;

	if (rhs.m_szData)
		{
			if (!m_szData)
				{
					try
						{
							m_szData = new gchar[SPIN_BUF_TEXT_SIZE];
						}
					catch(...)
						{
							m_szData = 0;
						}
				}
			UT_return_val_if_fail (m_szData, *this);
			memcpy (m_szData, rhs.m_szData, SPIN_BUF_TEXT_SIZE * sizeof (gchar));
		}
	else if (m_szData)
		{
			m_szData[0] = 0;
		}
	m_bChanged = false;

	return *this;
}

bool AP_Dialog_Paragraph::sControlData::setData (const gchar * data)
{
	if (!m_szData)
		{
			try
				{
					m_szData = new gchar[SPIN_BUF_TEXT_SIZE];
				}
			catch(...)
				{
					m_szData = 0;
				}
			UT_return_val_if_fail (m_szData, false);

			m_szData[SPIN_BUF_TEXT_SIZE-1] = 0;
		}
	if (data)
		strncpy (m_szData, data, SPIN_BUF_TEXT_SIZE - 1);
	else
		m_szData[0] = 0;

	return true;
}
