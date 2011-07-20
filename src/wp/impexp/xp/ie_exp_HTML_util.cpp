/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
* Copyright (C) 2007, 2009 Hubert Figuiere
* Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
* Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
* Copyright (C) 2001-2002 AbiSource, Inc.
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

#include "ie_exp_HTML_util.h"

const char * s_DTD_XHTML_AWML = "!DOCTYPE html PUBLIC \"-//ABISOURCE//DTD XHTML plus AWML 2.2//EN\" \"http://www.abisource.com/2004/xhtml-awml/xhtml-awml.mod\"";

const char * s_DTD_XHTML = "!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"";

const char * s_DTD_HTML4 = "!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"";

const char * s_Delimiter =
    "=======================================================";

const char * s_Header[2] = {
"Created by AbiWord, a free, Open Source wordprocessor. ",
"For more information visit http://www.abisource.com.   "
};

const char * s_HeaderCompact = "Created by Abiword, www.abisource.com";

bool m_bSecondPass = false;
bool m_bInAFENote = false;
bool m_bInAnnotation = false;
#include "MathSVGScript.h"

UT_UTF8String s_string_to_url (const UT_String & str)
{
	UT_UTF8String url;

	static const char hex[16] = {
		'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
	};
	char buf[4];
	buf[0] = '%';
	buf[3] = 0;

	const char * ptr = str.c_str ();
	while (*ptr)
	{
		bool isValidPunctuation = false;
		switch (*ptr)
		{
			case '-': // TODO: any others?
			case '_':
			case '.':
				isValidPunctuation = true;
				break;
			default:
				break;
		}
		unsigned char u = (unsigned char) *ptr;
		if (!isalnum (static_cast<int>(u)) && !isValidPunctuation)
		{
			buf[1] = hex[(u >> 4) & 0x0f];
			buf[2] = hex[ u       & 0x0f];
			url += buf;
		}
		else
		{
			buf[2] = (char) *ptr;
			url += (buf + 2);
		}
		ptr++;
	}
	return url;
}

UT_UTF8String s_string_to_url (const UT_UTF8String & str)
{
	UT_String s(str.utf8_str());
	return s_string_to_url(s);
}

extern const char * s_prop_list[] = {
	"background-color",	"transparent",
	"color",			"",
	"font-family",		"",
	"font-size",		"medium",
	"font-style",		"normal",
	"font-variant",		"normal",
	"font-weight",		"normal",
	"height",			"auto",
	"margin-bottom",	"0pt",
	"margin-left",		"0pt",
	"margin-right",		"0pt",
	"margin-top",		"0pt",
	"orphans",			"2",
	"text-align",		"",
	"text-decoration",	"none",
	"text-transform",	"none",
	"text-indent",		"0in",
	"vertical-align",	"baseline",
	"widows",			"2",
	"width",			"auto",
	0, 0
};
extern const UT_uint32 s_PropListLen = G_N_ELEMENTS(s_prop_list) - 2; /* don't include the zeros */

/*!	This function returns true if the given property is a valid CSS
  property.  It is based on the list in pp_Property.cpp, and, as such,
  is quite brittle.

  prop_default may be zero on return, indicating that the default is not fixed
*/
bool is_CSS (const char * prop_name, const char ** prop_default)
{
	if (prop_name == 0)
		return false;
	if (*prop_name == 0)
		return false;

	bool bCSS = false;

	for (UT_uint32 i = 0; i < s_PropListLen; i += 2)
	{
		if (!strcmp (prop_name, s_prop_list[i]))
		{
			if (prop_default) *prop_default = s_prop_list[i+1];
			bCSS = true;
			break;
		}
	}
	return bCSS;
}

/*!	This function copies a string to a new string, removing all the white
  space in the process.
*/
char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase)
{
	utf8str = "";

	if (text)
	{
		char buf[2]; // ick! [TODO ??]
		buf[1] = 0;
		const char * ptr = text;
		while (*ptr)
		{
			if (isspace ((int) ((unsigned char) *ptr)))
			{
				buf[0] = '_';
			}
			else
			{
				buf[0] = *ptr;
			}
			utf8str += buf;
			ptr++;
		}

		if(bLowerCase)
			utf8str.lowerCase();
	}
	return 0;
}