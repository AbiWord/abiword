/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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
#ifndef IE_EXP_EPUB_H_
#define IE_EXP_EPUB_H_

// External includes
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-output.h>
#include <gsf/gsf-libxml.h>
#include <vector>

// Abiword includes
#include <ie_exp.h>



#define EPUB_MIMETYPE "application/epub+zip"


class IE_Exp_EPUB : public IE_Exp {

public:
	IE_Exp_EPUB(PD_Document * pDocument);
	virtual ~IE_Exp_EPUB();

protected:
	virtual UT_Error _writeDocument();

private:

};



#endif /* IE_EXP_EPUB_H_ */
