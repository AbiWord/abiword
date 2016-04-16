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

#include "pp_Revision.h"
#include "pp_AttrProp.h"
#include "pd_Style.h"
#include "pd_Document.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_std_map.h"

//#include <limits.h>

#include <sstream>


PP_Revision::PP_Revision(UT_uint32 Id, PP_RevisionType eType, const gchar * props, const gchar * attrs):
	m_iID(Id), m_eType(eType), m_bDirty(true)
{
	if(!props && !attrs)
		return;

	const char * empty = "";
	
	if(props)
	{
		char * pProps = g_strdup(props);
		UT_return_if_fail (pProps);

		char * p = strtok(pProps, ":");

		while(p)
		{
			char * n = p;

			// skip over spaces ...
			while(n && *n == ' ')
				++n;
			
			p = strtok(NULL, ";");

			// if we have no p, that means the property is being removed ...
			const char * v = p ? p : empty;
			if(! strcmp(v, "-/-"))
				v = empty;
		
			if(n)
			{
				setProperty(n,v);
				p = strtok(NULL,":");
			}
			else
			{
				// malformed property
				UT_DEBUGMSG(("PP_Revision::PP_Revision: malformed props string [%s]\n", props));
				// if we have not reached the end, we will keep trying ...
				if(p)
					p = strtok(NULL,":");
			}
		}

		FREEP(pProps);
	}

	if(attrs)
	{
		char * pAttrs = g_strdup(attrs);

		UT_ASSERT_HARMLESS(pAttrs);
		if(!pAttrs)
		{
			UT_DEBUGMSG(("PP_Revision::PP_Revision: out of memory\n"));
			return;
		}

		char * p = strtok(pAttrs, ":");

		while(p)
		{
			char * n = p;
			p = strtok(NULL, ";");

			const char * v = p ? p : empty;
			if(! strcmp(v, "-/-"))
				v = empty;
			
			if(n)
			{
				setAttribute(n,v);
				p = strtok(NULL,":");
			}
			else
			{
				// malformed property
				UT_DEBUGMSG(("PP_Revision::PP_Revision: malformed props string [%s]\n", props));
				// if we have not reached the end, we will keep trying ...
				if(p)
					p = strtok(NULL,":");
			}
		}

		FREEP(pAttrs);
	}
}

PP_Revision::PP_Revision(UT_uint32 Id, PP_RevisionType eType,
                         const PP_PropertyVector & props,
                         const PP_PropertyVector & attrs)
	: m_iID(Id)
	, m_eType(eType)
	, m_bDirty(true)
{
	setProperties(props);
	setAttributes(attrs);
}

/*!
    Sets attributes taking care of any nested revision attribute (which needs to be parsed
    and combined with the current AP set.
*/
bool PP_Revision::setAttributes(const PP_PropertyVector & attributes)
{
	if(!PP_AttrProp::setAttributes(attributes)) {
		return false;
	}

	return _handleNestedRevAttr();
}



bool PP_Revision::_handleNestedRevAttr()
{
	const gchar * pNestedRev = NULL;
	getAttribute("revision", pNestedRev);
	
	if(pNestedRev)
	{
		PP_RevisionAttr NestedAttr(pNestedRev);

		// now remove "revision"
		setAttribute("revision", NULL);
		prune();

		// overlay the attrs and props from the revision attribute
		for(UT_uint32 i = 0; i < NestedAttr.getRevisionsCount(); ++i)
		{
			const PP_Revision * pRev = NestedAttr.getNthRevision(i);
			UT_return_val_if_fail( pRev, false );
					
			// ignore inserts and deletes
			if(pRev->getType() == PP_REVISION_ADDITION || pRev->getType() == PP_REVISION_DELETION)
				continue;

			setProperties(pRev->getProperties());
			setAttributes(pRev->getAttributes());
		}

		prune();
	}

	return true;
}


/*! converts the internal vector of properties into XML string */
const gchar * PP_Revision::getPropsString() const
{
	if(m_bDirty)
		_refreshString();

	return (const gchar*) m_sXMLProps.c_str();
}

/*! converts the internal vector of attributes into XML string */
const gchar * PP_Revision::getAttrsString() const
{
	if(m_bDirty)
		_refreshString();

	return (const gchar*) m_sXMLAttrs.c_str();
}

void PP_Revision::_refreshString() const
{
	m_sXMLProps.clear();
	m_sXMLAttrs.clear();

	UT_uint32 i;
	UT_uint32 iCount = getPropertyCount();
	const gchar * n, *v;

	for(i = 0; i < iCount; i++)
	{
		if(!getNthProperty(i,n,v))
		{
			// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}
		
		if(!v || !*v) v = "-/-";
		
		m_sXMLProps += n;
		m_sXMLProps += ":";
		m_sXMLProps += v;
		if(i < iCount - 1)
			m_sXMLProps += ";";
	}

	iCount = getAttributeCount();
	for(i = 0; i < iCount; i++)
	{
		if(!getNthAttribute(i,n,v))
		{
			// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}
		
		if(!v || !*v) v = "-/-";

		m_sXMLAttrs += n;
		m_sXMLAttrs += ":";
		m_sXMLAttrs += v;
		if(i < iCount - 1)
			m_sXMLAttrs += ";";
	}

	m_bDirty = false;
}

std::string PP_Revision::toString() const
{
    std::stringstream ret;
    PP_RevisionType r_type = getType();

    if(r_type == PP_REVISION_FMT_CHANGE)
        ret << "!";

    // print the id with appropriate sign
    ret << (int)(getId()* ((r_type == PP_REVISION_DELETION)?-1:1));
    
    if(r_type != PP_REVISION_DELETION)
    {
        // if we have no props but have attribs, we have to issue empty braces so as not to
        // confuse attribs with props
        if(hasProperties() || hasAttributes())
            ret << "{";
        
        if(hasProperties())
            ret << getPropsString();
        
        if(hasProperties() || hasAttributes())
            ret << "}";
			
        if(hasAttributes())
        {
            ret << "{" << getAttrsString() << "}";
        }
    }
    
    return ret.str();
}

bool PP_Revision::onlyContainsAbiwordChangeTrackingMarkup() const
{
    UT_DEBUGMSG(("onlyContainsAbiwordChangeTrackingMarkup(top) ac:%ld pc:%ld\n",
		 (long)getAttributeCount(), (long)getPropertyCount() ));

    if( !getAttributeCount() )
        return false;
    if( getPropertyCount() )
        return false;
    
    bool ret = true;
	UT_uint32 i;
	UT_uint32 iCount = getAttributeCount();
	const gchar * n, *v;

	for(i = 0; i < iCount; i++)
	{
		if(!getNthAttribute(i,n,v))
		{
			// UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}
        UT_DEBUGMSG(("onlyContainsAbiwordChangeTrackingMarkup() n:%s\n", n ));
        
        if( n != strstr( n, "abi-para" ) )
        {
            return false;
        }
    }
    
    return ret;
}


bool PP_Revision::operator == (const PP_Revision &op2) const
{
	// this is quite involved, but we will start with the simple
	// non-equality cases

	if(getId() != op2.getId())
		return false;

	if(getType() != op2.getType())
		return false;


	// OK, so we have the same type and id, do we have the same props ???
	UT_uint32 iPCount1 = getPropertyCount();
	UT_uint32 iPCount2 = op2.getPropertyCount();
	UT_uint32 iACount1 = getAttributeCount();
	UT_uint32 iACount2 = op2.getAttributeCount();

	if((iPCount1 != iPCount2) || (iACount1 != iACount2))
		return false;

	// now the lengthy comparison
	UT_uint32 i;
	const gchar * n;
	const gchar * v1, * v2;

	for(i = 0; i < iPCount1; i++)
	{

		getNthProperty(i,n,v1);
		op2.getProperty(n,v2);

		if(strcmp(v1,v2))
			return false;
	}

	for(i = 0; i < iACount1; i++)
	{

		getNthAttribute(i,n,v1);
		op2.getAttribute(n,v2);

		if(strcmp(v1,v2))
			return false;
	}
	return true;
}


// PP_Revision*
// PP_Revision::clone() const
// {
//     PP_Revision* ret = new PP_Revision( *this );
//     return ret;
// }


/************************************************************
 ************************************************************/

/*! create class instance from an XML attribute string
 */
PP_RevisionAttr::PP_RevisionAttr(const gchar * r):
	m_pLastRevision(NULL)
{
	_init(r);
}

/*! create class instance from a single revision data */
PP_RevisionAttr::PP_RevisionAttr(UT_uint32 iId, PP_RevisionType eType,
                                 const PP_PropertyVector & attrs,
                                 const PP_PropertyVector & props)
{
	PP_Revision * pRevision = new PP_Revision((UT_uint32)iId, eType, props, attrs);
	m_vRev.addItem((void*)pRevision);
}


PP_RevisionAttr::~PP_RevisionAttr()
{
	_clear();
}

/*! initialize instance with XML attribute string
 */
void PP_RevisionAttr::setRevision(const gchar * r)
{
	_clear();
	_init(r);
}

void
PP_RevisionAttr::setRevision(std::string&  r)
{
    setRevision( r.c_str() );
}


/*! destroys all internal data */
void PP_RevisionAttr::_clear()
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		delete (PP_Revision *) m_vRev.getNthItem(i);
	}

	m_vRev.clear();
	m_bDirty = true;
	m_pLastRevision = NULL;
}


/*! parse given XML attribute string and fill the
    instance with the data
*/
void PP_RevisionAttr::_init(const gchar *r)
{
	if(!r)
		return;

	// the string we are parsing looks like
	// "+1,-2,!3{font-family: Times New Roman}"

	// first duplicate the string so we can play with it ...
	char * s = (char*) g_strdup(r);
	char * end_s = s + strlen(s); // we need to remember where this
								  // string ends because we cannot use strtok(NULL,...)

	UT_sint32 iId;
	PP_RevisionType eType;
	gchar * pProps, * pAttrs,
		     * cl_brace = 0, * op_brace = 0,
		     * cl_brace2 = 0;

	char * t = strtok(s,",");

	// we have to remember the end of this token for future calls to
	// strtok since strtok is also used in the PP_Revision class and
	// it screws us up, so we have to start always with explicit
	// string
	char * next_s = s;

	while(t)
	{
		next_s = next_s + strlen(t) + 1; // 1 for the token separator

		if(*t == '!')
		{
			eType = PP_REVISION_FMT_CHANGE;
			t++;
		}
		else if(*t == '-')
		{
			eType = PP_REVISION_DELETION;
			t++; // so we do not need to deal with sign later
		}

		else
			eType = PP_REVISION_ADDITION; // this value is only
										  // temporary because this
										  // could equally be addition
										  // + format

		cl_brace = strchr(t, '}');
		op_brace = strchr(t, '{');

		if(!cl_brace || !op_brace)
		{
			// no props
			if(eType == PP_REVISION_FMT_CHANGE)
			{
				// malformed token, move onto the next one
				UT_DEBUGMSG(("PP_RevisionAttr::_init: invalid ! token [%s]\n",t));
				goto skip_this_token;
			}
			pProps = NULL;
			pAttrs = NULL;
		}
		else
		{
			// OK this is a case where we have some props, i.e., it
			// must be either fmt change or addition
			if(eType == PP_REVISION_DELETION)
			{
				// malformed token, move onto the next one
				UT_DEBUGMSG(("PP_RevisionAttr::_init: invalid - token [%s]\n",t));
				goto skip_this_token;
			}

			// insert null as needed to be able to parse the id and props
			*op_brace = 0;
			*cl_brace = 0;
			pProps = op_brace+1;

			// now see if the props are followed by attributes
			if(*(cl_brace + 1) == '{')
			{
				cl_brace2 = strchr(cl_brace + 2,'}');
				if(cl_brace2)
				{
					pAttrs = cl_brace + 2;
					*cl_brace2 = 0;
				}
				else
				{
					UT_DEBUGMSG(( "PP_RevisionAttr::_init: invalid token - [%s]\n", t ));
					pAttrs = NULL;
				}
			}
			else
				pAttrs = NULL;

			if(eType == PP_REVISION_ADDITION)
				eType = PP_REVISION_ADDITION_AND_FMT;
		}

		// now we can retrieve the id
		iId = atol(t);

		{
			PP_Revision * pRevision = new PP_Revision((UT_uint32)iId, eType, pProps, pAttrs);

			m_vRev.addItem((void*)pRevision);
		}
		
	skip_this_token:
		if(next_s < end_s)
			t = strtok(next_s,",");
		else
			t = NULL;
	}

	FREEP(s);
	m_bDirty = true;
	m_iSuperfluous = 0;
	m_pLastRevision = NULL;
}

/*!
    changes the type of revision with id iId to eType; if revision
    with that id is not present, returns false
 */
bool PP_RevisionAttr::changeRevisionType(UT_uint32 iId, PP_RevisionType eType)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId == r->getId())
		{
			r->setType(eType);
			m_bDirty = true;
			return true;
		}
	}

	return false;
}

bool PP_RevisionAttr::changeRevisionId(UT_uint32 iOldId, UT_uint32 iNewId)
{
	UT_return_val_if_fail(iNewId >= iOldId, false);
	
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iOldId == r->getId())
		{
			r->setId(iNewId);
			m_bDirty = true;
			return true;
		}
	}

	return false;
}

/*!
    this function removes any revisions that no-longer contribute to the cumulative effect
    it is used in full-history mode when transfering attrs and props from the revision attribute
    into the main attrs and props
*/
void PP_RevisionAttr::pruneForCumulativeResult(PD_Document * pDoc)
{
	// first we are looking for any deletions which cancel anything below them
	bool bDelete = false;
	UT_sint32 i;
	if(m_vRev.getItemCount() == 0)
	{
		return;
	}

    m_bDirty = true;
	
	for(i = m_vRev.getItemCount()-1; i >=0; --i)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);
		
		if(!bDelete && r->getType() == PP_REVISION_DELETION)
		{
			bDelete = true;
			continue; // we do not want the top revision deleted
		}
		

		if(bDelete)
		{
			delete r;
			m_vRev.deleteNthItem(i);
		}
	}

	// now we merge props and attrs in what is left
	if(m_vRev.getItemCount() == 0)
	{
		return;
	}

	PP_Revision * r0 = (PP_Revision *)m_vRev.getNthItem(0);
	UT_return_if_fail(r0);
	
	for(i = 1; i < m_vRev.getItemCount(); ++i)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);
		UT_return_if_fail( r );
		
		r0->setProperties(r->getProperties());
		r0->setAttributes(r->getAttributes());

		delete r;
		m_vRev.deleteNthItem(i);
		--i;
	}

    // explode the style if present
	if(pDoc)
		r0->explodeStyle(pDoc);

#if 0
	// I do not think we should do this -- the emptiness indicates that the props and
	// attributes should be removed from the format; if we remove them, we irreversibly
	// lose this information
	
	// get rid of any empty props and attrs
	r0->prune();
#endif
	
	// finally, remove the revision attribute if present
	const gchar * v;
	if(r0->getAttribute("revision", v))
		r0->setAttribute("revision", NULL);

	UT_ASSERT_HARMLESS( m_vRev.getItemCount() == 1 );
}


/*! return highest revision associated with this revision attribute
    that has ID at most equal to id; the returned PP_Revision can be
    used to determine whether this text should be displayed or hidden
    in revision with the original id

    \param UT_uint32 id : the id of this revision
    \param PP_Revision ** ppR: location where to store pointer to one
                               of the special revisions in case return
                               value is NULL
                              
    \return : pointer to PP_Revision, or NULL if the revision
    attribute should be ignored for the present level
*/

// these are special instances of PP_Revision that are used to in the
// following function to handle special cases
static const PP_Revision s_del(0, PP_REVISION_DELETION, (gchar*)0, (gchar*)0);
static const PP_Revision s_add(0, PP_REVISION_ADDITION, (gchar*)0, (gchar*)0);

const PP_Revision *  PP_RevisionAttr::getGreatestLesserOrEqualRevision(UT_uint32 id,
																	   const PP_Revision ** ppR) const
{
	if(ppR)
		*ppR = NULL;
	
	if(id == 0)
		return getLastRevision();

	const PP_Revision *r = NULL; // this will be the revision we are looking for
	UT_uint32 r_id = 0;

	const PP_Revision *m = NULL; // this will be the lowest revision present
	UT_uint32 m_id = 0xFFFF;

	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *) m_vRev.getNthItem(i);
		UT_uint32 t_id = t->getId();

		// the special case speedup - if we hit our id, then we can return immediately
		if(t_id == id)
			return t;

		if(t_id < m_id)
		{
			m = t;
			m_id = t_id;
		}

		if((t_id < id) && (t_id > r_id))
		{
			r = t;
			r_id = t_id;
		}
	}

	// now that we have the biggest revision with ID lesser or equal
	// id, we have to deal with the special case when this is NULL
	// i.e., this fragment only figures in revisions > id; the problem
	// with NULL is that it is visible if the smallest revision ID is
	// negative, and hidden in the opposite case -- we use the special
	// static variables s_del and s_add to indicate what should
	// be done

	if(r == NULL && ppR)
	{
		if(!m)
		{
			//UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// this happens when there was no revision attribute
			return NULL;
		}

		if(m->getType() == PP_REVISION_DELETION)
			*ppR = &s_del;
		else if((m->getType() == PP_REVISION_ADDITION)
				||(m->getType() == PP_REVISION_ADDITION_AND_FMT))
			*ppR = &s_add;
		else // the initial revision was fmt change, so ignore it
			*ppR = NULL;
	}

	return r;
}

const PP_Revision * PP_RevisionAttr::getLowestGreaterOrEqualRevision(UT_uint32 id) const
{
	if(id == 0)
		return NULL;

	const PP_Revision *r = NULL; // this will be the revision we are looking for
	UT_uint32 r_id = PD_MAX_REVISION;

	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *) m_vRev.getNthItem(i);
		UT_uint32 t_id = t->getId();

		// the special case speedup - if we hit our id, then we can return immediately
		if(t_id == id)
			return t;

		if((t_id > id) && (t_id < r_id))
		{
			r = t;
			r_id = t_id;
		}
	}

	return r;
}


/*! finds the highest revision number in this attribute
 */
const PP_Revision * PP_RevisionAttr::getLastRevision() const
{
	// since this is rather involved, we will cache the result and
	// use the cache if it is uptodate
	if(m_pLastRevision)
		return m_pLastRevision;

	//const PP_Revision * r = NULL;
	UT_uint32 r_id = 0;

	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *)m_vRev.getNthItem(i);
		UT_uint32 t_id = t->getId();

		if(t_id > r_id)
		{
			r_id = t_id;
			m_pLastRevision = t;
		}
	}

	// UT_ASSERT_HARMLESS( m_pLastRevision );
	// it is legal for this to be NULL -- it happens when the revision was pruned for
	// cumulative effect and the last revision was a deletion.
	return m_pLastRevision;
}


UT_uint32 PP_RevisionAttr::getHighestId() const
{
    UT_uint32 ret = 0;
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *)m_vRev.getNthItem(i);
        ret = std::max( ret, t->getId() );
    }
    return ret;
}


/*!
   find revision with id == iId; if revision is not found minId
   contains the smallest id in this set greater than iId; if return value is and minId
   is PD_MAX_REVISION then there are revisions preset
*/
const PP_Revision * PP_RevisionAttr::getRevisionWithId(UT_uint32 iId, UT_uint32 &minId) const
{
	minId = PD_MAX_REVISION;

	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *)m_vRev.getNthItem(i);
		UT_uint32 t_id = t->getId();

		if(t_id == iId)
		{
			return t;
		}

		if(minId > t_id && t_id > iId)
			minId = t_id;
	}

	return NULL;
}


/*! given revision level id, this function returns true if given
    segment of text is to be visible, false if it is to be hidden
*/
bool PP_RevisionAttr::isVisible(UT_uint32 id) const
{
	if(id == 0)
	{
		// id 0 means show all revisions
		return true;
	}

	const PP_Revision * pSpecial;
	const PP_Revision * pR = getGreatestLesserOrEqualRevision(id, &pSpecial);

	if(pR)
	{
		// found compliant revision ...
		return true;
	}
	

	if(pSpecial)
	{
		// pSpecial is of the same type as the revision with the
		// lowest id
		PP_RevisionType eType = pSpecial->getType();

		// deletions and fmt changes can be ignored; insertions need
		// to be hidden
		return ((eType != PP_REVISION_ADDITION) && (eType == PP_REVISION_ADDITION_AND_FMT));
	}

	// the revision with the lowest id is a change of format, this
	// text has to remain visible
	return true;
}


/*! adds id to the revision vector handling the special cases where id
    is already present in this attribute.
*/
void PP_RevisionAttr::addRevision(UT_uint32 iId, PP_RevisionType eType,
                                  const PP_PropertyVector & pAttrs,
                                  const PP_PropertyVector & pProps)
{
	UT_sint32 i;

	for(i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision*) m_vRev.getNthItem(i);
		UT_uint32 r_id = r->getId();
		PP_RevisionType r_type = r->getType();

		if(iId != r_id)
			continue;
		
		if(eType != r_type)
		{
			// we are trying to add a revision id already in the vector
			// but of a different -- this is legal, i.e., the
			// editor just changed his mind
			// we need to make distinction between different cases

			if((eType == PP_REVISION_DELETION) && (   r_type == PP_REVISION_ADDITION
												   || r_type == PP_REVISION_ADDITION_AND_FMT))
			{
				// the editor originally inserted a new segment of
				// text but now wants it out; we cannot just remove
				// the id, because the original operation resulted in
				// a new fragment in the piece table; rather we will
				// mark this with '-' and will remember the superfluous
				// id, so if queried later we can work out if this
				// whole fragment should in fact go

				delete r;
				m_vRev.deleteNthItem(i);

				m_iSuperfluous = iId;

				const PP_Revision * pRevision = new PP_Revision(iId, eType, (gchar*)0, (gchar*)0);
				m_vRev.addItem((void*)pRevision);
			}
			else if((eType == PP_REVISION_ADDITION) && (r_type == PP_REVISION_DELETION))
			{
				// in the opposite case, when the editor originally
				// marked the text for removal and now wants it back,
				// we just remove the attribute (which we have done)
				// this also happens when we have been left with a
				// superfluous deletion id in the vector; if that is
				// the case we need to reset m_iSuperfluous, since
				// this fragment can no more be superfluous

				delete r;
				m_vRev.deleteNthItem(i);

				if(m_iSuperfluous == iId)
				{
					// the editor has had another change of heart
					m_iSuperfluous  = 0;
				}
			}
			else if((eType == PP_REVISION_DELETION) && (r_type == PP_REVISION_FMT_CHANGE))
			{
				// this is the case when the editor changed
				// formatting, but now wants the whole fragment out
				// instead -- we simly replace the old revision with
				// the new, since the original action did not result
				// in inserting new text

				delete r;
				m_vRev.deleteNthItem(i);

				const PP_Revision * pRevision = new PP_Revision(iId, eType, (gchar*)0, (gchar*)0);
				m_vRev.addItem((void*)pRevision);
			}
			else if((eType == PP_REVISION_FMT_CHANGE) && (r_type == PP_REVISION_DELETION))
			{
				// originally the editor marked the text for deletion,
				// but now he just wants a format change, in this case
				// we just replace the old revision with the new

				delete r;
				m_vRev.deleteNthItem(i);

				const PP_Revision * pRevision = new PP_Revision(iId, eType, pProps, pAttrs);
				
				m_vRev.addItem((void*)pRevision);
			}
			else if((eType == PP_REVISION_FMT_CHANGE) && (r_type == PP_REVISION_ADDITION))
			{
				// the editor first added this fragment, and now wants
				// to apply a format change on the top of that
				// so we will keep the old revision record, but need
				// to merge any existing props in the revision with
				// the new ones
				r->setProperties(pProps);
				r->setAttributes(pAttrs);
			}
			else if((eType == PP_REVISION_FMT_CHANGE) && (r_type == PP_REVISION_ADDITION_AND_FMT))
			{
				// addition with a fmt change, just add our changes to it
				r->setProperties(pProps);
				r->setAttributes(pAttrs);
			}

			m_bDirty = true;
			m_pLastRevision = NULL;
			return;
		}
		else //(eType == r_type)
		{
			// we are trying to add a type already in the vector; this is legal but makes sense only
			// if both are fmt changes
			if(!((eType == PP_REVISION_FMT_CHANGE) && (r_type == PP_REVISION_FMT_CHANGE)))
				return;
			
			r->setProperties(pProps);
			r->setAttributes(pAttrs);
			
			m_bDirty = true;
			m_pLastRevision = NULL;
			return;
		}
	}

	// if we got here then the item is not in our vector so add it
	const PP_Revision * pRevision = new PP_Revision(iId, eType, pProps, pAttrs);
	
	m_vRev.addItem((void*)pRevision);
	m_bDirty = true;
	m_pLastRevision = NULL;
}


/**
 * Logically Performs addRevision( iId, eType, 0, 0 ). This method is
 * mainly useful for loading an ODT+GCT file where you want to add and
 * delete revisions but don't actually care about the attrs/props for
 * that action.
 */
void PP_RevisionAttr::addRevision(UT_uint32 iId, PP_RevisionType eType )
{
    addRevision( iId, eType, PP_NOPROPS, PP_NOPROPS );
}


void
PP_RevisionAttr::addRevision( const PP_Revision* r )
{
    std::stringstream ss;
    if(r->getType() & PP_REVISION_FMT_CHANGE)
        ss << "!";
    
    ss << (r->getId() * ((r->getType() == PP_REVISION_DELETION)?-1:1));

    if(r->hasProperties())
    {
        ss << "{" << r->getPropsString() << "}";
    }
    if(r->hasAttributes())
    {
        ss << "{" << r->getAttrsString() << "}";
    }

    PP_RevisionAttr us( getXMLstring() );
    _clear();
    std::string tmp = (std::string)us.getXMLstring() + "," + ss.str();
    setRevision( tmp.c_str() );
}

void PP_RevisionAttr::mergeAttr( UT_uint32 iId, PP_RevisionType t,
                                 const gchar* pzName, const gchar* pzValue )
{
    PP_RevisionAttr ra;
    const PP_PropertyVector ppAtts = {
        pzName, pzValue
    };
    ra.addRevision(iId, t, ppAtts, PP_NOPROPS);

    mergeAll( ra );
}

/**
 * Do not replace the attribute/value if it exists in the given revision already.
 */
void PP_RevisionAttr::mergeAttrIfNotAlreadyThere( UT_uint32 iId,
                                                  PP_RevisionType t,
                                                  const gchar* pzName,
                                                  const gchar* pzValue )
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * tr = (const PP_Revision *)m_vRev.getNthItem(i);
		UT_uint32 tid = tr->getId();

        if( tid == iId )
        {
            if( t == PP_REVISION_NONE || t == tr->getType() )
            {
                const gchar * tattrs = tr->getAttrsString();
                if( strstr( tattrs, pzName ))
                {
                    return;
                }
            }
        }
    }
    
    return mergeAttr( iId, t, pzName, pzValue );
}



//
//                           getId()    getType()                rev
typedef std::map< std::pair< UT_uint32, PP_RevisionType >, const PP_Revision* > revidx_t;

static revidx_t toIndex( const PP_RevisionAttr& ra )
{
    revidx_t ret;
    for( UT_uint32 i=0; i < ra.getRevisionsCount(); ++i )
    {
        const PP_Revision* r = ra.getNthRevision( i );
        ret[ std::make_pair( r->getId(), r->getType() ) ] = r;
    }
    return ret;
}

static std::string mergeAPStrings( const std::string& a, const std::string& b )
{
    if( b.empty() )
        return a;
    if( a.empty() )
        return b;
    std::stringstream ss;
    ss << a << ";" << b;
    return ss.str();
}


#define DEBUG_MERGEALL false

void PP_RevisionAttr::mergeAll( const PP_RevisionAttr& ra )
{
    PP_RevisionAttr us( getXMLstring() );
    _clear();
    std::string tmp = (std::string)us.getXMLstring() + "," + ra.getXMLstring();

    revidx_t oldidx = toIndex( us );
    revidx_t newidx = toIndex( ra );

    /*
     * Iterate over the entries in the oldidx merging the data from
     * newidx if found whenever a entry from newidx is used it is
     * removed from newidx too. This way, we can then just iterate
     * over newidx to add the entries which are in newidx but not in
     * oldidx.
     */
    revidx_t output;
    for( revidx_t::iterator iter = oldidx.begin(); iter != oldidx.end(); ++iter )
    {
        const PP_Revision* r = iter->second;
        revidx_t::iterator niter = newidx.find( iter->first );
        // UT_DEBUGMSG(("ODTCT ra::merge() id:%d attrs:%s props:%s\n",
        //              r->getId(), r->getAttrsString(), r->getPropsString() ));

        /*
         * If there is an entry in oldidx and newidx then merge them
         */
        if( niter != newidx.end() )
        {
            const PP_Revision* nr = niter->second;
            
            std::string attrs = mergeAPStrings( r->getAttrsString(), nr->getAttrsString() );
            std::string props = mergeAPStrings( r->getPropsString(), nr->getPropsString() );
            output[ iter->first ] = new PP_Revision( iter->first.first,
                                                     iter->first.second,
                                                     props.c_str(), attrs.c_str() );
            newidx.erase( niter );
        }
        else
        {
            /*
             * disregard entries without anything to tell
             */
            if( r->getType() != PP_REVISION_DELETION
                && !strlen(r->getAttrsString())
                && !strlen(r->getPropsString()) )
            {
                // UT_DEBUGMSG(("ODTCT ra::merge() rev as no attr/props, skipping old id:%d type:%d\n",
                //              r->getId(), r->getType() ));
                
                continue;
            }

            /*
             * no matching entry in the newidx, just copy the data
             */
            output[ iter->first ] = new PP_Revision( iter->first.first,
                                                     iter->first.second,
                                                     r->getPropsString(),
                                                     r->getAttrsString() );
        }
    }
    
    /*
     * copy over new revisions which didn't have a matching entry in the oldidx
     */
    for( revidx_t::iterator iter = newidx.begin(); iter != newidx.end(); ++iter )
    {
            output[ iter->first ] = new PP_Revision( iter->first.first,
                                                     iter->first.second,
                                                     iter->second->getPropsString(),
                                                     iter->second->getAttrsString() );
    }

    /*
     * Build the XML string for the merged revision attribute from the output index
     */
    bool outputssVirgin = true;
    std::stringstream outputss;
    for( revidx_t::iterator iter = output.begin(); iter != output.end(); ++iter )
    {
        const PP_Revision* r = iter->second;

        if( DEBUG_MERGEALL )
        {
            UT_DEBUGMSG(("ODTCT ra::merge() output id:%d t:%d attr:%s\n",
                         r->getId(), r->getType(), r->getAttrsString() ));
            UT_DEBUGMSG(("ODTCT ra::merge() output id:%d t:%d prop:%s\n",
                         r->getId(), r->getType(), r->getPropsString() ));
        }
        
        if( outputssVirgin ) outputssVirgin = false;
        else                 outputss << ",";
        
        outputss << r->toString();
    }
    UT_map_delete_all_second( output );
    

    
    setRevision( outputss.str().c_str() );

    if( DEBUG_MERGEALL )
    {
        UT_DEBUGMSG(("ODTCT ra::merge() outputss::%s\n", outputss.str().c_str() ));
        UT_DEBUGMSG(("ODTCT ra::merge() ret:%s\n", getXMLstring() ));
    }
    return;
}




/*! removes id from this revision, respecting the sign, i.e., it will
  not remove -5 if given 5
 */
void PP_RevisionAttr::removeRevisionIdWithType(UT_uint32 iId, PP_RevisionType eType)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if((iId == r->getId()) && (eType == r->getType()))
		{
			delete r;
			m_vRev.deleteNthItem(i);
			m_bDirty = true;
			m_pLastRevision = NULL;
			return;
		}
	}
}

/*! removes id from the attribute disregarding sign, i.e.,
    if given 5 it will remove both -5 and +5
*/
void PP_RevisionAttr::removeRevisionIdTypeless(UT_uint32 iId)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId == r->getId())
		{
			delete r;
			m_vRev.deleteNthItem(i);
			m_bDirty = true;
			m_pLastRevision = NULL;
			return;
		}
	}
}

/*! removes pRev unconditionally from the attribute
*/
void PP_RevisionAttr::removeRevision(const PP_Revision * pRev)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(r == pRev)
		{
			delete r;
			m_vRev.deleteNthItem(i);
			m_bDirty = true;
			m_pLastRevision = NULL;
			return;
		}
	}
}


/*! removes all IDs from the attribute whose value is lesser or
    equal the given id
*/
void PP_RevisionAttr::removeAllLesserOrEqualIds(UT_uint32 iId)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId >= r->getId())
		{
			delete r;
			m_vRev.deleteNthItem(i);
			--i; // the vector just shrunk
		}
	}

	m_bDirty = true;
	m_pLastRevision = NULL;
}

/*! removes all IDs from the attribute whose value is higher or
    equal the given id
*/
void PP_RevisionAttr::removeAllHigherOrEqualIds(UT_uint32 iId)
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId <= r->getId())
		{
			delete r;
			m_vRev.deleteNthItem(i);
			--i; // the vector just shrunk
		}
	}

	m_bDirty = true;
	m_pLastRevision = NULL;
}


/*! create XML string from our vector
 */
void PP_RevisionAttr::_refreshString() const
{
  //	char buf[30];
	m_sXMLstring.clear();
	UT_uint32 iCount = m_vRev.getItemCount();

	for(UT_uint32 i = 0; i < iCount; i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

        if( !m_sXMLstring.empty() )
            m_sXMLstring += ",";
        
        m_sXMLstring += r->toString();
            
		// PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);
		// PP_RevisionType r_type = r->getType();

		// if(r_type == PP_REVISION_FMT_CHANGE)
		// 	m_sXMLstring += "!";

		// // print the id with appropriate sign
		// sprintf(buf,"%d",r->getId()* ((r_type == PP_REVISION_DELETION)?-1:1));
		// m_sXMLstring += buf;

		// if(r_type != PP_REVISION_DELETION)
		// {
		// 	// if we have no props but have attribs, we have to issue empty braces so as not to
		// 	// confuse attribs with props
		// 	if(r->hasProperties() || r->hasAttributes())
		// 		m_sXMLstring += "{";
			
        // if(r->hasProperties())
        // 	m_sXMLstring += r->getPropsString();
			
        // if(r->hasProperties() || r->hasAttributes())
        // 	m_sXMLstring += "}";
			
		// 	if(r->hasAttributes())
		// 	{
		// 		m_sXMLstring += "{";
		// 		m_sXMLstring += r->getAttrsString();
		// 		m_sXMLstring += "}";
		// 	}
		// };

		// if(i != iCount - 1)
		// {
		// 	//not the last itteration, append ','
		// 	m_sXMLstring += ",";
		// }

	}
	m_bDirty = false;
}



/*! get an gchar string representation of this revision
 */
const gchar * PP_RevisionAttr::getXMLstring() const
{
	if(m_bDirty)
		_refreshString();

	return (const gchar*) m_sXMLstring.c_str();
}

std::string
PP_RevisionAttr::getXMLstringUpTo( UT_uint32 iId ) const
{
    PP_RevisionAttr rat;
    rat.setRevision( getXMLstring() );
    UT_DEBUGMSG(("PP_RevisionAttr::getXMLstringUpTo() id:%d before:%s\n", iId, rat.getXMLstring() ));
    rat.removeAllHigherOrEqualIds( iId );
    UT_DEBUGMSG(("PP_RevisionAttr::getXMLstringUpTo() id:%d  after:%s\n", iId, rat.getXMLstring() ));
    // PP_RevisionAttr rat;
    // const PP_Revision* r = 0;
    // for( int raIdx = 0;
    //      raIdx < iId && (r = ra.getNthRevision( raIdx ));
    //      raIdx++ )
    // {
    //     rat.addRevision( r );
    // }
    return rat.getXMLstring();
}



/*! returns true if the fragment marked by this attribute is
    superfluous, i.e, it was created in the process of the present
    revision but the editor has later changed his/her mind and decided
    it should go away
*/
bool PP_RevisionAttr::isFragmentSuperfluous() const
{
	// the fragment is superfluous if the superfluous flag is set
	// and the fragment belongs only to a single revision level
	if(m_iSuperfluous != 0 && m_vRev.getItemCount() == 1)
	{
		UT_return_val_if_fail (((PP_Revision *)m_vRev.getNthItem(0))->getId() == m_iSuperfluous,false);
		return true;
	}
	else
		return false;
}

bool PP_RevisionAttr::operator== (const PP_RevisionAttr &op2) const
{
	for(UT_sint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * r1 = (const PP_Revision *) m_vRev.getNthItem(i);

		for(UT_sint32 j = 0; j < op2.m_vRev.getItemCount(); j++)
		{
			const PP_Revision * r2 = (const PP_Revision *) op2.m_vRev.getNthItem(j);

			if(!(*r1 == *r2))
				return false;
		}
	}
	return true;
}

// PP_RevisionAttr&
// PP_RevisionAttr::operator=(const PP_RevisionAttr &rhs)
// {
//     setRevision( rhs.getXMLstring() );
//     return *this;
// }


/*! returns true if after revision iId this fragment carries revised
    property pName, the value of which will be stored in pValue; see
    notes on PP_Revision::hasProperty(...)
*/
bool PP_RevisionAttr::hasProperty(UT_uint32 iId, const gchar * pName, const gchar * &pValue) const
{
	const PP_Revision * s;
	const PP_Revision * r = getGreatestLesserOrEqualRevision(iId, &s);

	if(r)
		return r->getProperty(pName, pValue);

	return false;
}

/*! returns true if after the last revision this fragment carries revised
    property pName, the value of which will be stored in pValue; see
    notes on PP_Revision::hasProperty(...)
*/
bool PP_RevisionAttr::hasProperty(const gchar * pName, const gchar * &pValue) const
{
	const PP_Revision * r = getLastRevision();
	return r->getProperty(pName, pValue);
}

/*! returns the type of cumulative revision up to iId represented by this attribute
 */
PP_RevisionType PP_RevisionAttr::getType(UT_uint32 iId) const
{
	const PP_Revision * s;
	const PP_Revision * r = getGreatestLesserOrEqualRevision(iId,&s);

	if(!r)
	{
		// HACK need to return something
		return PP_REVISION_FMT_CHANGE;
	}
	
	return r->getType();
}

/*! returns the type of overall cumulative revision represented by this attribute
 */
PP_RevisionType PP_RevisionAttr::getType() const
{
	const PP_Revision * r = getLastRevision();
	return r->getType();
}


UT_uint32 PP_RevisionAttr::getHighestRevisionNumberWithAttribute( const gchar * attrName ) const
{
    const PP_Revision* r = 0;

    for( UT_uint32 raIdx = 0;
         raIdx < getRevisionsCount() && (r = getNthRevision( raIdx ));
         raIdx++ )
    {
        if( UT_getAttribute( r, attrName, 0 ))
            return r->getId();
    }
    return 0;
}


const char* UT_getAttribute( const PP_AttrProp* pAP, const char* name, const char* def  )
{
    const gchar* pValue;
    bool ok;
    
    ok = pAP->getAttribute( name, pValue );
    if (!ok)
    {
        pValue = def;
    }
    return pValue;
}

const PP_Revision *
PP_RevisionAttr::getLowestDeletionRevision() const
{
    if( !getRevisionsCount() )
        return 0;

    UT_uint32 rmax = getRevisionsCount();
    const PP_Revision* last  = getNthRevision( rmax-1 );
    if( last->getType() != PP_REVISION_DELETION )
        return 0;
    
    for( long idx = rmax - 1; idx >= 0; --idx )
    {
        const PP_Revision* p = getNthRevision( idx );
        if( p->getType() != PP_REVISION_DELETION )
        {
            return last;
        }
        last = p;
    }
    return 0;
}


std::string UT_getLatestAttribute( const PP_AttrProp* pAP,
                                   const char* name,
                                   const char* def )
{
    const char* t = 0;
    std::string ret = def;
    bool ok = false;
    
    if( const char* revisionString = UT_getAttribute( pAP, "revision", 0 ))
    {
        PP_RevisionAttr ra( revisionString );
        const PP_Revision* r = 0;
            
        for( int raIdx = ra.getRevisionsCount()-1;
             raIdx >= 0 && (r = ra.getNthRevision( raIdx ));
             --raIdx )
        {
            ok = r->getAttribute( name, t );
            if (ok)
            {
                ret = t;
                return ret;
            }
        }
    }

    ok = pAP->getAttribute( name, t );
    if (ok)
    {
        ret = t;
        return ret;
    }
    ret = def;
    
    return ret;
}
