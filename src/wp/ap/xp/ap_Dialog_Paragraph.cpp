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

AP_Dialog_Paragraph::AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer = a_OK;
	m_blockProps = NULL;
	m_paragraphPreview = NULL;
	
	// determine unit system to use in this dialog
	const XML_Char * szRulerUnits;
	UT_ASSERT(m_pApp);
	if (m_pApp->getPrefs()->getPrefsValue(AP_PREF_KEY_RulerUnits, &szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;
}

AP_Dialog_Paragraph::~AP_Dialog_Paragraph(void)
{
	DELETEP(m_paragraphPreview);
	FREEP(m_blockProps);
}

UT_Bool AP_Dialog_Paragraph::setDialogData(const XML_Char * props[])
{
	UT_ASSERT(props);

	return UT_XML_replaceList(m_blockProps, props);
}

// This function returns a pointer to newly allocated memory.  The caller must free
// it (use the FREEP() macro).  This function does not free any memory pointed to
// by its argument.

UT_Bool AP_Dialog_Paragraph::getDialogData(const XML_Char *** props)
{
	if (m_blockProps)
	{
		// TODO: do we really want these stored internally as a list of props?
		// TODO: wouldn't an editable vector or alphahash be more convenient?

		// TODO: at minimum, do a better job of fixing the API mismatch
		XML_Char** pProps = (XML_Char **) *props;

		return UT_XML_cloneList(pProps, (const XML_Char **) m_blockProps);
	}

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
