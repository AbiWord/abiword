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
#include "pp_AttrProp.h"

typedef enum {
	PP_REVISION_ADDITION,
	PP_REVISION_DELETION,
	PP_REVISION_FMT_CHANGE,
	PP_REVISION_ADDITION_AND_FMT
} PP_RevisionType;

/*! PP_Revision is a class that encapsulates a single revision,
    holding its id, type and associated properties and attributes. It
    provides functions for retrieving information and from merging
    properties
*/
class PP_Revision: public PP_AttrProp
{
  public:
	PP_Revision(UT_uint32 Id,
				PP_RevisionType eType,
				const XML_Char *  props,
				const XML_Char * attrs);

	PP_Revision(UT_uint32 Id,
				PP_RevisionType eType,
				const XML_Char ** props,
				const XML_Char ** attrs);

	UT_uint32        getId()    const {return m_iID;}
	PP_RevisionType  getType()  const {return m_eType;}
	const XML_Char * getPropsString();
	const XML_Char * getAttrsString();

	bool operator == (const PP_Revision &op2) const;

  private:
	void             _refreshString();

	UT_uint32        m_iID;
	PP_RevisionType  m_eType;
	UT_String        m_sXMLProps;
	UT_String        m_sXMLAttrs;
	bool             m_bDirty;
};



/*! PP_RevisionAttr is class that represent a revision attribute; it
    is initialized by an attribute string:

      <c revision="R1[,R2,R3,...]">some text</>
                   ^^^^^^^^^^^^^^

      R1, etc., conform to the following syntax (items in square
      brackets are optional):

      [+]n[{props}[{atrrs}]]    -- addition with optional properties
                                   and attributes; props and attrs
                                   are formed as `name:value'
      -n                        -- deletion
      !n{props}                 -- formating change only

      where n is a numerical id of the revision and props is regular
      property string, for instance
          font-family:Times New Roman


  The class provides methods for adding and removing individual
  revisions and evaluating how a particular revised fragment should be
  displayed in the document
*/

class PP_RevisionAttr
{
  public:
	PP_RevisionAttr()
		:m_vRev(),m_sXMLstring(),m_bDirty(true),m_iSuperfluous(0),m_pLastRevision(NULL)
		{};
	PP_RevisionAttr(const XML_Char * r);
	~PP_RevisionAttr();

	void                  setRevision(const XML_Char * r);

	void                  addRevision(UT_uint32 iId,
									  PP_RevisionType eType,
									  const XML_Char ** pAttrs,
									  const XML_Char ** pProps);

	void                  removeRevisionIdWithType(UT_uint32 iId, PP_RevisionType eType);
	void                  removeRevisionIdTypeless(UT_uint32 iId);
	void                  removeAllLesserOrEqualIds(UT_uint32 id);
	void                  removeRevision(const PP_Revision * pRev);

	const PP_Revision *   getGreatestLesserOrEqualRevision(UT_uint32 id);
	const PP_Revision *   getLastRevision();

	/*! please not that the following are convenience functions; if
	    you need to make repeated enqueries, it is better to call
	    getGreatestLesserOrEqualRevision() or getLastRevision() and
	    querie the returned PP_Revision object.
    */
	bool                  isVisible(UT_uint32 id);
	bool                  isVisible();
	bool                  hasProperty(UT_uint32 iId, const XML_Char * pName, const XML_Char * &pValue);
	bool                  hasProperty(const XML_Char * pName, const XML_Char * &pValue);
	PP_RevisionType       getType(UT_uint32 iId);
	PP_RevisionType       getType();
#if 0
	const UT_Vector *     getProps(UT_uint32 iId);
	const UT_Vector *     getProps();
#endif
	const XML_Char *      getXMLstring();
	bool                  isFragmentSuperfluous() const;

	bool operator == (const PP_RevisionAttr &op2) const;

  private:
	void _init(const XML_Char *r);
	void _clear();
	void _refreshString();

	UT_Vector           m_vRev;
	UT_String           m_sXMLstring;
	bool                m_bDirty; // indicates whether m_sXMLstring corresponds
						          // to current state of the instance
	UT_uint32           m_iSuperfluous;
	const PP_Revision * m_pLastRevision;
};

#endif // #ifndef PT_REVISION_H
