/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef PT_REVISION_H
#define PT_REVISION_H

#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_vector.h"
#include "pp_AttrProp.h"

class PD_Document;

ABI_EXPORT const char* UT_getAttribute( const PP_AttrProp* pAP, const char* name, const char* def = 0 );
/**
 * Like UT_getAttribute(name,atts,def) but check for a revision attribute and
 * if found first look for the most recent value of atts in the revision.
 */
ABI_EXPORT std::string UT_getLatestAttribute( const PP_AttrProp* pAP,
                                              const char* name,
                                              const char* def );


typedef enum {
	PP_REVISION_NONE             = 0,
	PP_REVISION_ADDITION         = 0x01,
	PP_REVISION_DELETION         = 0x02,
	PP_REVISION_FMT_CHANGE       = 0x04,
	PP_REVISION_ADDITION_AND_FMT = 0x05
} PP_RevisionType;

/*! PP_Revision is a class that encapsulates a single revision,
    holding its id, type and associated properties and attributes. It
    provides functions for retrieving information and from merging
    properties
*/
class ABI_EXPORT PP_Revision: public PP_AttrProp
{
  public:
	PP_Revision(UT_uint32 Id,
				PP_RevisionType eType,
				const gchar *  props,
				const gchar * attrs);

	PP_Revision(UT_uint32 Id,
                    PP_RevisionType eType,
                    const PP_PropertyVector & props,
                    const PP_PropertyVector & attrs);

	virtual ~PP_Revision(){};

	UT_uint32        getId()    const {return m_iID;}
	void             setId(UT_uint32 iId) {UT_ASSERT_HARMLESS( iId >= m_iID); m_iID = iId;}

	PP_RevisionType  getType()  const {return m_eType;}
	void             setType(PP_RevisionType t) {m_eType = t; m_bDirty = true;}

	const gchar * getPropsString() const;
	const gchar * getAttrsString() const;

	// this is intentionally not virtual (no need for that)
	bool	setAttributes(const std::vector<std::string> & attributes);

	bool operator == (const PP_Revision &op2) const;

//    PP_Revision* clone() const;

    std::string toString() const;
    bool onlyContainsAbiwordChangeTrackingMarkup() const;

  private:
	void             _refreshString() const;
	bool             _handleNestedRevAttr();

	UT_uint32        m_iID;
	PP_RevisionType  m_eType;
	// these next three are a cache, therefor mutable
	mutable UT_String        m_sXMLProps;
	mutable UT_String        m_sXMLAttrs;
	mutable bool             m_bDirty;
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

      revoval of property/attribute is indicated by setting to -/-, e.g.,

          font-family:-/-

      (the revision attribute parser in the class translates that into "")


  The class provides methods for adding and removing individual
  revisions and evaluating how a particular revised fragment should be
  displayed in the document
*/

class ABI_EXPORT PP_RevisionAttr
{
  public:
	PP_RevisionAttr()
		:m_vRev(),m_sXMLstring(),m_bDirty(true),m_iSuperfluous(0),m_pLastRevision(NULL)
		{};
	PP_RevisionAttr(const gchar * r);


	PP_RevisionAttr(UT_uint32 iId, PP_RevisionType eType,
                        const PP_PropertyVector & attrs,
                        const PP_PropertyVector & props);

	~PP_RevisionAttr();

	void                  setRevision(const gchar * r);
	void                  setRevision(std::string&  r);

	void                  addRevision(UT_uint32 iId,
                                          PP_RevisionType eType,
                                          const PP_PropertyVector & pAttrs,
                                          const PP_PropertyVector & pProps);
	void                  addRevision(UT_uint32 iId, PP_RevisionType eType );
    // No ownership of the given revision is taken.
    void                  addRevision( const PP_Revision* r );

	bool                  changeRevisionType(UT_uint32 iId, PP_RevisionType eType);
	bool                  changeRevisionId(UT_uint32 iOldId, UT_uint32 iNewId);

	void                  removeRevisionIdWithType(UT_uint32 iId, PP_RevisionType eType);
	void                  removeRevisionIdTypeless(UT_uint32 iId);
	void                  removeAllLesserOrEqualIds(UT_uint32 id);
	void                  removeAllHigherOrEqualIds(UT_uint32 id);
	void                  removeRevision(const PP_Revision * pRev);

	const PP_Revision *   getGreatestLesserOrEqualRevision(UT_uint32 id,
														   const PP_Revision ** ppR) const;
	const PP_Revision *   getLowestGreaterOrEqualRevision(UT_uint32 id) const;

	const PP_Revision *   getLastRevision() const;
	const PP_Revision *   getRevisionWithId(UT_uint32 iId, UT_uint32 & iMinId) const;
    UT_uint32             getHighestId() const;

	UT_uint32             getRevisionsCount() const {return m_vRev.getItemCount();}
    bool                  empty() const { return !getRevisionsCount(); }
	const PP_Revision *   getNthRevision(UT_uint32 n) const {return (const PP_Revision*)m_vRev.getNthItem(n);}

	void                  pruneForCumulativeResult(PD_Document * pDoc);

	/*! please note that the following are convenience functions; if
	    you need to make repeated enqueries, it is better to call
	    getGreatestLesserOrEqualRevision() or getLastRevision() and
	    query the returned PP_Revision object.
    */
	bool                  isVisible(UT_uint32 id) const;
	bool                  hasProperty(UT_uint32 iId, const gchar * pName, const gchar * &pValue) const;
	bool                  hasProperty(const gchar * pName, const gchar * &pValue) const;
	PP_RevisionType       getType(UT_uint32 iId) const;
	PP_RevisionType       getType() const;
    UT_uint32             getHighestRevisionNumberWithAttribute( const gchar * pName ) const;
#if 0
	const UT_Vector *     getProps(UT_uint32 iId);
	const UT_Vector *     getProps();
#endif
	const gchar *      getXMLstring() const;
    std::string        getXMLstringUpTo( UT_uint32 iId ) const;
	void                  forceDirty() {m_bDirty = true;}
	bool                  isFragmentSuperfluous() const;

	bool operator== (const PP_RevisionAttr &op2) const;

    // MIQ: This would be nice, but there are ownership issues I don't know about with M
//    PP_RevisionAttr& operator=(const PP_RevisionAttr &rhs);

    void mergeAll( const PP_RevisionAttr& ra );
    void mergeAttr( UT_uint32 iId, PP_RevisionType t,
                    const gchar* pzName, const gchar* pzValue );
    void mergeAttrIfNotAlreadyThere( UT_uint32 iId, PP_RevisionType t,
                                     const gchar* pzName, const gchar* pzValue );

	const PP_Revision *   getLowestDeletionRevision() const;


  private:
	void _init(const gchar *r);
	void _clear();
	void _refreshString() const;

	UT_Vector           m_vRev;
	// these next 2 are a cache, hence mutable
	mutable UT_String           m_sXMLstring;
	mutable bool                m_bDirty; // indicates whether m_sXMLstring corresponds
						          // to current state of the instance
	UT_uint32           m_iSuperfluous;
	// also a cache
	mutable const PP_Revision * m_pLastRevision;
};

#endif // #ifndef PT_REVISION_H
