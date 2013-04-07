/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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


#ifndef _IE_IMP_OPENXML_H_
#define _IE_IMP_OPENXML_H_

// AbiWord includes
#include <ie_imp.h>

// External includes
#include <gsf/gsf-infile.h>

/**
 * Class used to import OpenXML files
 */
class IE_Imp_OpenXML : public IE_Imp
{
public:
	IE_Imp_OpenXML (PD_Document * pDocument);
	virtual ~IE_Imp_OpenXML ();

protected:
	virtual UT_Error _loadFile(GsfInput * input);

private:
	void _setDocumentProperties();
	void _cleanup();
};

#endif //_IE_IMP_OPENXML_H_

