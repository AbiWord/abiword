/* AbiWord
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef PT_REVISION_H
#define PT_REVISION_H

#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_vector.h"

/*! a simple class for parsing and manipulating a revision attribute
  string of the type "+1;-3;+5;" where the numbers indicate revision
  id and the sign whether the text is to be added or deleted
*/

class PP_Revision
{
  public:
	PP_Revision():m_bDirty(true){};
	PP_Revision(const XML_Char * r);
	/*~PP_Revision();*/

	void             setRevision(const XML_Char * r);

	void             addRevisionId(UT_sint32 id);
	void             removeRevisionIdWithSign(UT_sint32 id);
	void             removeRevisionIdSignless(UT_uint32 id);
	void             removeAllLesserOrEqualIds(UT_uint32 id);

	UT_sint32        getGreatestLesserOrEqualRevision(UT_uint32 id) const;
	bool             isVisible(UT_uint32 id) const;

	const XML_Char * getXMLstring();


  private:
	void _setVector(const XML_Char *r);
	void _refreshString();

	UT_Vector m_vRev;
	UT_String m_sXMLstring;
	bool      m_bDirty; // indicates whether m_sXMLstring corresponds
						// to current state of the instance

};

#endif // #ifndef PT_REVISION_H
