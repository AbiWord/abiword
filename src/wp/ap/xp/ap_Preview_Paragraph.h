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


#ifndef AP_PREVIEW_PARAGRAPH_H
#define AP_PREVIEW_PARAGRAPH_H

#include "ut_misc.h"
#include "ut_types.h"

#include "xap_Preview.h"

class AP_Preview_Paragraph : public XAP_Preview
{
public:

	AP_Preview_Paragraph(GR_Graphics * gc);
	virtual ~AP_Preview_Paragraph(void);

	// TODO : add special types

	// TODO : add public methods

    // where all the paragraph-specific drawing happens
	void				draw(void);
	
protected:

};

#endif /* AP_PREVIEW_PARAGRAPH_H */
