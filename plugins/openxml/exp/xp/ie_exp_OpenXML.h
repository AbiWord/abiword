/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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


#ifndef _IE_EXP_OPENXML_H_
#define _IE_EXP_OPENXML_H_

// AbiWord includes
#include <ie_exp.h>
#include <ut_debugmsg.h>
#include <ut_types.h>
#include <ut_assert.h>
#include <ut_string_class.h>

#include <OXML_Document.h>

// External includes
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-output-stdio.h>

class OXML_Document;

/**
 * Class used to export OpenXML files
 */
class IE_Exp_OpenXML : public IE_Exp
{
public:
	IE_Exp_OpenXML (PD_Document * pDocument);
	virtual ~IE_Exp_OpenXML ();
	UT_Error startDocument();
	UT_Error finishDocument();

protected:
    virtual UT_Error _writeDocument(void);
    
private:
	UT_Error writeContentTypes(GsfOutfile* root);
	UT_Error writeRelations(GsfOutfile* root);
	UT_Error writeMainPart(GsfOutfile* root);
	UT_Error writeXmlHeader(GsfOutput* file);

	void _cleanup();
};

#endif //_IE_EXP_OPENXML_H_

