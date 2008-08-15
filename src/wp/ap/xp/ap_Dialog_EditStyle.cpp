/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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

#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_EditStyle.h"

AP_Dialog_EditStyle::AP_Dialog_EditStyle(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogeditstyle"),
	m_answer(a_OK),
	m_sAllProperties("")
{
}

AP_Dialog_EditStyle::~AP_Dialog_EditStyle(void)
{
}

/*!
 * Sets initial data about the target of our operations in this dialog.
 * Called before runModal by the calling procedure.
 */
void AP_Dialog_EditStyle::setStyleToEdit(UT_UTF8String sName, PD_Style * pStyle)
{
	m_sName=sName;
	m_pStyle=pStyle;
}

/*!
 * Takes the style pointed to by m_pStyle (m_sName) and creates the member
 * vectors and other data. Used before populating the dialog at a platform level.
 */
bool AP_Dialog_EditStyle::_deconstructStyle()
{

	// DO NOT CHANGE without also changing below and the enum in the header and
	// ap_stringid.h!
	const gchar * fields [] = {"text-align", "text-indent",
		"margin-left", "margin-right", "margin-top", "margin-bottom",
		"line-height", "tabstops", "start-value", "list-delim", "list-style",
		"list-decimal", "field-font", "field-color", "keep-together",
		"keep-with-next", "orphans", "widows", "dom-dir", "nulldummy",
		"bgcolor", "color", "font-family",
		"font-size", "font-stretch", "font-style", "font-variant",
		"font-weight", "text-decoration", "lang", "\0"};
	
	UT_return_val_if_fail(m_pStyle, false);
	UT_GenericVector<const gchar *> vProps;
		
	m_pStyle->getAllProperties(&vProps, pp_BASEDON_DEPTH_LIMIT); // using limit to
											// only get this style's props
	
	m_sUnrecognizedProps="";
	
	const gchar * pszName;
	const gchar * pszVal;
	unsigned int i=0;
	unsigned int k=0;
	while (i<vProps.size()-1)
	{
		pszName=vProps.getNthItem(i);
		pszVal=vProps.getNthItem(i+1);
		k=0;
		while (strcmp(pszName, fields[k])!=0 && *fields[k]!='\0')
		{
			k++;
		}
		if (*fields[k]=='\0')
		{
			m_sUnrecognizedProps+= std::string(pszName) + ':' + std::string(pszVal) + ';';
			UT_DEBUGMSG(("Unrecognized prop in a style, sticking on the 'do not edit' pile.\n"));
			UT_DEBUGMSG(("%s\n", m_sUnrecognizedProps.c_str() ));
		}
		else
		{
			// k now holds the ENUM value of the property name.
			m_vPropertyID.push_back(k);
			m_vPropertyValues.push_back(pszVal);
		}
		i+=2;	
	}
		
}

/*!
 * Reverses the _deconstructStyle() - called in the process of closing the
 * dialog at a platform level.  Updates the target style with the current
 * status of the dialog's member variables.  Please do update those variables
 * on a platform level before calling, or the user won't get what they ask for!
 */
bool AP_Dialog_EditStyle::_reconstructStyle()
{
	// we can rebuild it, we can make it better.
	
	
	// DO NOT CHANGE without also changing above and the enum in the header and
	// ap_stringid.h!
	const gchar * fields [] = {"text-align", "text-indent",
		"margin-left", "margin-right", "margin-top", "margin-bottom",
		"line-height", "tabstops", "start-value", "list-delim", "list-style",
		"list-decimal", "field-font", "field-color", "keep-together",
		"keep-with-next", "orphans", "widows", "dom-dir", "nulldummy",
		"bgcolor", "color", "font-family",
		"font-size", "font-stretch", "font-style", "font-variant",
		"font-weight", "text-decoration", "lang", "\0"};
	
	std::string sProps=m_sUnrecognizedProps;
	unsigned int i;
	
	for (i=0; i<m_vPropertyID.size(); i++)
	{
		sProps += std::string(fields[m_vPropertyID[i]]) + ":" + m_vPropertyValues[i] + ";";
	}
	
	// Remove trailing semicolon
	// property string has semicolon delimiters but a trailing semicolon will cause a failure.
	if (sProps.size() > 0 && sProps[sProps.size()-1] == ';')
	{
		sProps.resize(sProps.size()-1);
	}
	
	m_sAllProperties = sProps;
	
	return true;
}

AP_Dialog_EditStyle::tAnswer AP_Dialog_EditStyle::getAnswer(void) const
{
	return m_answer;
}
