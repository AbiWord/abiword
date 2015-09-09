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


#ifndef IE_EXP_XML_H
#define IE_EXP_XML_H

#include "ie_exp.h"
#include <string>

class ABI_EXPORT IE_Exp_XML: public IE_Exp
{
public:
	IE_Exp_XML(PD_Document * pDocument);
	virtual ~IE_Exp_XML();

public:
    void setupFile(bool compressed);
    void closeHandle();
    void setDocType(char const *doctype);
    void startElement(char const *name);
    void endElement();
	void addBool(char const *id, bool value);
	void addComment(char const *comment);
	void addFloat(char const *id, double value, int precision = -1);
	void addInt(char const *id, int value);
	void addLint(char const *id, long value);
	void addLuint(char const *id, unsigned long value);
	void addString(char const *id, char const *value);
	void addString(char const *id, std::string const &value);
	void addString(char const *id, UT_UCSChar const *data, int length);
	void addStringUnchecked(char const *id, char const *value);
	void addStringUnchecked(char const *id, std::string const &value);
	void addUint(char const *id, unsigned value);

	void setPrettyPrint(bool pretty);

private:

    GsfOutput *m_zip;
    GsfXMLOut *m_xml;
};

#endif /* IE_EXP_XML_H */
