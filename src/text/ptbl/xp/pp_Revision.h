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

typedef enum {
	PP_REVISION_ADDITION,
	PP_REVISION_DELETION,
	PP_REVISION_FMT_CHANGE,
	PP_REVISION_ADDITION_AND_FMT
} PP_RevisionType;

/*! PP_Revision is a class that encapsulates a single revision,
    holding its id, type and associated properties. It provides
    functions for retrieving information and from merging properties
*/
class PP_Revision
{
  public:
	PP_Revision(UT_uint32 Id, PP_RevisionType eType, const XML_Char * props);
	~PP_Revision();

	UT_uint32        getId()    const {return m_iID;}
	PP_RevisionType  getType()  const {return m_eType;}
	const XML_Char * getProps() const {return m_pProps;}
	void             mergeProps(const XML_Char * pProps);

  private:
	UT_uint32       m_iID;
	PP_RevisionType m_eType;
	XML_Char *      m_pProps;
};



/*! PP_RevisionAttr is class that represent a revision attribute; it
  is initialized by an attribute string:

      <c revision="R1[,R2,R3,...]">some text</>
                   ^^^^^^^^^^^^^^

      R1, etc., conform to the following syntax (items in square
      brackets are optional):

      [+]n[{props}]     -- addition with optional properties
      -n                -- deletion
      !n{props}         -- formating change only

      where n is a numerical id of the revision and props is regular
      property string, for instance
          font-family:Times New Roman


  The class provides methods for adding and removing individual
  revisions and evaluating how a particular revised string should be
  displayed in the document
*/

class PP_RevisionAttr
{
  public:
	PP_RevisionAttr():m_bDirty(true),m_iSuperfluous(0){};
	PP_RevisionAttr(const XML_Char * r);
	~PP_RevisionAttr();

	void                  setRevision(const XML_Char * r);

	void                  addRevision(UT_uint32 iId, PP_RevisionType eType, const XML_Char * pProp);
	void                  removeRevisionIdWithType(UT_uint32 iId, PP_RevisionType eType);
	void                  removeRevisionIdTypeless(UT_uint32 iId);
	void                  removeAllLesserOrEqualIds(UT_uint32 id);

	const PP_Revision *   getGreatestLesserOrEqualRevision(UT_uint32 id) const;
	const PP_Revision *   getLastRevision() const;
	bool                  isVisible(UT_uint32 id) const;
	bool                  isVisible() const;
	bool                  isFragmentSuperfluous() const;

	const XML_Char * getXMLstring();


  private:
	void _init(const XML_Char *r);
	void _clear();
	void _refreshString();

	UT_Vector m_vRev;
	UT_String m_sXMLstring;
	bool      m_bDirty; // indicates whether m_sXMLstring corresponds
						// to current state of the instance
	UT_uint32 m_iSuperfluous;
};

#endif // #ifndef PT_REVISION_H
