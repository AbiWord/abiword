/* AbiSuite
 * Copyright (C) Jordi Mas i Hernàndez
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

#include "ut_vector.h"
#include "ut_xml.h"
#include "ut_hash.h"

#include "ut_string_class.h"

#ifndef BARBARISMS_H
#define BARBARISMS_H

class ABI_EXPORT BarbarismChecker : public UT_XML::Listener
{
public:
	BarbarismChecker();
	~BarbarismChecker();

	bool load(const char *szLang);

	bool checkWord(const UT_UCSChar * word32, size_t length);

	bool suggestWord(const UT_UCSChar *word32, size_t length, UT_GenericVector<UT_UCSChar*>* pVecsugg);

	/*
		Implementation of UT_XML::Listener
	*/
	void startElement(const gchar *name, const gchar **atts);
	void endElement(const gchar *){};
	void charData(const gchar *, int){};

private:

	bool suggestExactWord(const UT_UCSChar *word32, size_t length,	UT_GenericVector<UT_UCSChar*>* pVecsugg);

	UT_GenericStringMap<UT_GenericVector<UT_UCS4Char *>*>	m_map;
	UT_GenericVector<UT_UCS4Char *>*		m_pCurVector;

	std::string m_sLang;
};

#endif // BARBARISMS_H
