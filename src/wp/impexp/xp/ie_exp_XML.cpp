/* AbiWord
 * Copyright (C) 2015 Jean Brefort <jean.brefort@normalesup.org>
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
#include "ie_exp_XML.h"
#include <sstream>
#include "ut_debugmsg.h"

IE_Exp_XML::IE_Exp_XML(PD_Document * pDocument):
	IE_Exp (pDocument), m_zip(0), m_xml(0)
{
}

IE_Exp_XML::~IE_Exp_XML()
{
}

void IE_Exp_XML::setupFile(bool compressed)
{
	UT_ASSERT_HARMLESS (!m_xml);
	if (compressed)
	{
		m_zip = gsf_output_gzip_new(getFp (), NULL);
		m_xml = gsf_xml_out_new (m_zip);
	}
	else
	{
		m_zip = GSF_OUTPUT(g_object_ref(getFp ()));
		m_xml = gsf_xml_out_new (getFp());
	}
}

void IE_Exp_XML::closeHandle()
{
	if (m_xml) {
		g_object_unref(m_xml);
		m_xml = 0;
	}
	if (m_zip) {
		g_object_unref(m_zip);
		m_zip = 0;
	}
}

void IE_Exp_XML::setDocType(char const *doctype)
{
	gsf_xml_out_set_doc_type(m_xml, doctype);
}

void IE_Exp_XML::startElement(char const *name)
{
	gsf_xml_out_start_element(m_xml, name);
}

void IE_Exp_XML::endElement(void)
{
	gsf_xml_out_end_element(m_xml);
}

void IE_Exp_XML::addBool(char const *id, bool value)
{
	gsf_xml_out_add_bool(m_xml, id, value);
}

void IE_Exp_XML::addComment(char const *comment)
{
	gsf_output_write(m_zip, 5, reinterpret_cast<const guint8*>("<!-- "));
	gsf_output_write(m_zip, strlen(comment), reinterpret_cast<const guint8*>(comment));
	gsf_output_write(m_zip, 5, reinterpret_cast<const guint8*>(" -->\n"));
}

void IE_Exp_XML::addFloat(char const *id, double value, int precision)
{
	gsf_xml_out_add_float(m_xml, id, value, precision);
}

void IE_Exp_XML::addInt(char const *id, int value)
{
	gsf_xml_out_add_int(m_xml, id, value);
}

void IE_Exp_XML::addLint(char const *id, long value)
{
	std::ostringstream buf;
	buf << value;
	gsf_xml_out_add_cstr_unchecked(m_xml, id, buf.str().c_str());
}

void IE_Exp_XML::addLuint(char const *id, unsigned long value)
{
	std::ostringstream buf;
	buf << value;
	gsf_xml_out_add_cstr_unchecked(m_xml, id, buf.str().c_str());
}

void IE_Exp_XML::addString(char const *id, char const *value)
{
	gsf_xml_out_add_cstr(m_xml, id, value);
}

void IE_Exp_XML::addString(char const *id, std::string const &value)
{
	gsf_xml_out_add_cstr(m_xml, id, value.c_str());
}

void IE_Exp_XML::addString(char const *id, UT_UCSChar const *data, int length)
{
	UT_UTF8String sBuf;
	const UT_UCSChar * pData;

	UT_return_if_fail(sizeof(UT_Byte) == sizeof(char));
	sBuf.reserve(length);

	for (pData=data; (pData<data+length); /**/)
	{
		switch (*pData)
		{
		case '<':
			sBuf += "&lt;";
			pData++;
			break;

		case '>':
			sBuf += "&gt;";
			pData++;
			break;

		case '&':
			sBuf += "&amp;";
			pData++;
			break;

		case UCS_LF:					// LF -- representing a Forced-Line-Break
			sBuf += "<br/>";
			pData++;
			break;

		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			sBuf += "<cbr/>";
			pData++;
			break;

		case UCS_TAB:
			sBuf += "\t";
			pData++;
			break;

		case UCS_FF:					// FF -- representing a Forced-Page-Break
			sBuf += "<pbr/>";
			pData++;
			break;

		default:
			if (*pData < 0x20)         // Silently eat these characters.
				pData++;
			else
				{
					sBuf.appendUCS4 (pData, 1);
					pData++;
				}
		}
	}
	gsf_xml_out_add_cstr_unchecked(m_xml, id, sBuf.utf8_str());
}

void IE_Exp_XML::addStringUnchecked(char const *id, char const *value)
{
	gsf_xml_out_add_cstr_unchecked(m_xml, id, value);
}

void IE_Exp_XML::addStringUnchecked(char const *id, std::string const &value)
{
	gsf_xml_out_add_cstr(m_xml, id, value.c_str());
}

void IE_Exp_XML::addUint(char const *id, unsigned value)
{
	gsf_xml_out_add_uint(m_xml, id, value);
}

void IE_Exp_XML::setPrettyPrint(bool pretty)
{
	g_object_set (m_xml, "pretty-print", pretty, NULL);
}
