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

#include "pp_Revision.h"
#include "pp_AttrProp.h"
#include "ut_debugmsg.h"
//#include <limits.h>

PP_Revision::PP_Revision(UT_uint32 Id, PP_RevisionType eType, const XML_Char * props, const XML_Char * attrs):
	m_iID(Id), m_eType(eType), m_bDirty(true)
{
	if(!props && !attrs)
		return;

	if(props)
	{
		char * pProps = UT_strdup(props);

		UT_ASSERT(pProps);
		if(!pProps)
		{
			UT_DEBUGMSG(("PP_Revision::PP_Revision: out of memory\n"));
			return;
		}

		char * p = strtok(pProps, ":");

		while(p)
		{
			char * n = UT_strdup(p);
			p = strtok(NULL, ";");
			UT_ASSERT(p && n);

			if(p && n)
			{
				m_vProps.addItem((void*)n);
				m_vProps.addItem((void*)UT_strdup(p));
				p = strtok(NULL,":");
			}
			else
			{
				// malformed property
				UT_DEBUGMSG(("PP_Revision::PP_Revision: malformed props string [%s]\n", props));
				FREEP(n);

				// if we have not reached the end, we will keep trying ...
				if(p)
					p = strtok(NULL,":");
			}
		}

		FREEP(pProps);
		UT_ASSERT(m_vProps.getItemCount() % 2 == 0);
	}

	if(attrs)
	{
		char * pAttrs = UT_strdup(attrs);

		UT_ASSERT(pAttrs);
		if(!pAttrs)
		{
			UT_DEBUGMSG(("PP_Revision::PP_Revision: out of memory\n"));
			return;
		}

		char * p = strtok(pAttrs, ":");

		while(p)
		{
			char * n = UT_strdup(p);
			p = strtok(NULL, ";");
			UT_ASSERT(p && n);

			if(p && n)
			{
				m_vAttrs.addItem((void*)n);
				m_vAttrs.addItem((void*)UT_strdup(p));
				p = strtok(NULL,":");
			}
			else
			{
				// malformed property
				UT_DEBUGMSG(("PP_Revision::PP_Revision: malformed props string [%s]\n", props));
				FREEP(n);

				// if we have not reached the end, we will keep trying ...
				if(p)
					p = strtok(NULL,":");
			}
		}

		FREEP(pAttrs);
		UT_ASSERT(m_vAttrs.getItemCount() % 2 == 0);
	}

}

PP_Revision::PP_Revision(UT_uint32 Id, PP_RevisionType eType, const XML_Char ** props, const XML_Char ** attrs):
	m_iID(Id), m_eType(eType), m_bDirty(true)
{
	if(!props && !attrs)
		return;

	if(props)
	{
		const XML_Char ** pProps = props;

		while(*pProps)
		{
			m_vProps.addItem((void*) UT_strdup(*pProps++));
		}
		UT_ASSERT(m_vProps.getItemCount() % 2 == 0);
	}

	if(attrs)
	{
		const XML_Char ** pAttrs = attrs;

		while(*pAttrs)
		{
			m_vAttrs.addItem((void*) UT_strdup(*pAttrs++));
		}
		UT_ASSERT(m_vAttrs.getItemCount() % 2 == 0);
	}
}

PP_Revision::~PP_Revision()
{
	_clear();
}

void PP_Revision::_clear()
{
	UT_uint32 i;

	for (i = 0; i < m_vProps.getItemCount(); i++)
	{
		XML_Char * p = (XML_Char *)m_vProps.getNthItem(i);
		FREEP(p);
	}
	m_vProps.clear();

	for (i = 0; i < m_vAttrs.getItemCount(); i++)
	{
		XML_Char * p = (XML_Char *)m_vAttrs.getNthItem(i);
		FREEP(p);
	}
	m_vAttrs.clear();

	m_bDirty = true;
}


/*! merges pProps with the properties aready stored in this
    revision
*/
void PP_Revision::mergeProps(const XML_Char * pProps)
{
	// first the simple cases
	if(pProps == NULL)
		return;

	// we will parse the string into individual properties
	// use these to init an PP_AttrProp class and then use it to
	// create the new string
	PP_AttrProp attrProp;

	UT_ASSERT(m_vProps.getItemCount() % 2 == 0);

	for(UT_uint32 i = 0; i < m_vProps.getItemCount(); i += 2)
	{
		attrProp.setProperty((const XML_Char *) m_vProps.getNthItem(i),
							 (const XML_Char *) m_vProps.getNthItem(i+1));
	}


	char * pTemp = UT_strdup(pProps);

	UT_ASSERT(pTemp);
	if(!pTemp)
	{
		UT_DEBUGMSG(("PP_Revision::mergeProps: out of memory\n"));
		return;
	}

	char *p = strtok(pTemp, ":");

	while(p)
	{
		char * v = strtok(NULL, ";");
		attrProp.setProperty(p,v);
		p = strtok(NULL,":");
	}

	FREEP(pTemp);

	// now refil our vector
	_clear();
	const XML_Char * n, *v;

	for(UT_uint32 j = 0; i < attrProp.getPropertyCount(); i++)
	{
		attrProp.getNthProperty(i, n, v);
		m_vProps.addItem((void*)UT_strdup(n));
		m_vProps.addItem((void*)UT_strdup(v));
	}
}

/*! merges pProps with the properties aready stored in this
    revision
*/
void PP_Revision::mergeProps(const XML_Char ** pProps)
{
	// first the simple cases
	if((pProps == NULL) || *pProps == NULL)
		return;

	PP_AttrProp attrProp;

	UT_ASSERT(m_vProps.getItemCount() % 2 == 0);

	attrProp.setProperties((const UT_Vector *)& m_vProps);
	attrProp.setProperties(pProps);

	// now refil our vector
	_clear();
	const XML_Char * n, *v;

	for(UT_uint32 i = 0; i < attrProp.getPropertyCount(); i++)
	{
		attrProp.getNthProperty(i, n, v);
		m_vProps.addItem((void*)UT_strdup(n));
		m_vProps.addItem((void*)UT_strdup(v));
	}
}

/*! merges pProps with the properties aready stored in this
    revision
*/
void PP_Revision::mergeAttrs(const XML_Char ** pAttrs)
{
	// first the simple cases
	if((pAttrs == NULL) || *pAttrs == NULL)
		return;

	PP_AttrProp attrProp;

	UT_ASSERT(m_vAttrs.getItemCount() % 2 == 0);

	attrProp.setAttributes((const UT_Vector *)&m_vAttrs);
	attrProp.setAttributes(pAttrs);

	// now refil our vector
	_clear();
	const XML_Char * n, *v;

	for(UT_uint32 i = 0; i < attrProp.getAttributeCount(); i++)
	{
		attrProp.getNthAttribute(i, n, v);
		m_vAttrs.addItem((void*)UT_strdup(n));
		m_vAttrs.addItem((void*)UT_strdup(v));
	}
}

/*! converts the internal vector of properties into XML string */
const XML_Char * PP_Revision::getPropsString()
{
	if(m_bDirty)
		_refreshString();

	return (const XML_Char*) m_sXMLProps.c_str();
}

/*! converts the internal vector of attributes into XML string */
const XML_Char * PP_Revision::getAttrsString()
{
	if(m_bDirty)
		_refreshString();

	return (const XML_Char*) m_sXMLAttrs.c_str();
}

void PP_Revision::_refreshString()
{
	m_sXMLProps.clear();
	m_sXMLAttrs.clear();

	UT_uint32 i;
	UT_uint32 iCount = m_vProps.getItemCount();

	for(i = 0; i < iCount; i += 2)
	{
		m_sXMLProps += (char *)m_vProps.getNthItem(i);
		m_sXMLProps += ":";
		m_sXMLProps += (char *)m_vProps.getNthItem(i+1);
		if(i < iCount - 2)
			m_sXMLProps += ";";
	}

	iCount = m_vAttrs.getItemCount();
	for(i = 0; i < iCount; i += 2)
	{
		m_sXMLAttrs += (char *)m_vAttrs.getNthItem(i);
		m_sXMLAttrs += ":";
		m_sXMLAttrs += (char *)m_vAttrs.getNthItem(i+1);
		if(i < iCount - 2)
			m_sXMLAttrs += ";";
	}

	m_bDirty = false;
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
	UT_uint32 iPCount1 = m_vProps.getItemCount();
	UT_uint32 iPCount2 = op2.m_vProps.getItemCount();
	UT_uint32 iACount1 = m_vAttrs.getItemCount();
	UT_uint32 iACount2 = op2.m_vAttrs.getItemCount();

	if((iPCount1 != iPCount2) || (iACount1 != iACount2))
		return false;

	// now the lengthy comparison
	UT_uint32 i,j;
	for(i = 0; i < iPCount1; i += 2)
	{
		const XML_Char * n1 = (const XML_Char *) m_vProps.getNthItem(i);
		const XML_Char * v1 = (const XML_Char *) m_vProps.getNthItem(i+1);

		for(j = 0; j < iPCount2; j += 2)
		{
			const XML_Char * n2 = (const XML_Char *) op2.m_vProps.getNthItem(j);
			const XML_Char * v2 = (const XML_Char *) op2.m_vProps.getNthItem(j+1);

			if(!UT_strcmp(n1,n2) && UT_strcmp(v1,v2))
				return false;
		}
	}

	for(i = 0; i < iACount1; i += 2)
	{
		const XML_Char * n1 = (const XML_Char *) m_vAttrs.getNthItem(i);
		const XML_Char * v1 = (const XML_Char *) m_vAttrs.getNthItem(i+1);

		for(j = 0; j < iACount2; j += 2)
		{
			const XML_Char * n2 = (const XML_Char *) op2.m_vAttrs.getNthItem(j);
			const XML_Char * v2 = (const XML_Char *) op2.m_vAttrs.getNthItem(j+1);

			if(!UT_strcmp(n1,n2) && UT_strcmp(v1,v2))
				return false;
		}
	}
	return true;
}

/*! returns true if the property of pName is found in this revision
    and sets pValue to its value; note that NULL is a valid value for
    pName, it means that given property was removed by this revision
*/
bool PP_Revision::hasProperty(const XML_Char * pName, const XML_Char *& pValue) const
{
	for(UT_uint32 i = 0; i < m_vProps.getItemCount(); i += 2)
	{
		if(!UT_strcmp(pName, (char *)m_vProps.getNthItem(i)))
		{
			pValue = (const XML_Char *) m_vProps.getNthItem(i+1);
			return true;
		}
	}

	pValue = NULL;
	return false;
}


/************************************************************
 ************************************************************/

/*! create class instance from an XML attribute string
 */
PP_RevisionAttr::PP_RevisionAttr(const XML_Char * r)
{
	_init(r);
}

PP_RevisionAttr::~PP_RevisionAttr()
{
	_clear();
}

/*! initialize instance with XML attribute string
 */
void PP_RevisionAttr::setRevision(const XML_Char * r)
{
	_clear();
	_init(r);
}

/*! destroys all internal data */
void PP_RevisionAttr::_clear()
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		delete (PP_Revision *) m_vRev.getNthItem(i);
	}

	m_vRev.clear();
}


/*! parse given XML attribute string and fill the
    instance with the data
*/
void PP_RevisionAttr::_init(const XML_Char *r)
{
	if(!r)
		return;

	// the string we are parsing looks like
	// "+1,-2,!3{font-family: Times New Roman}"

	// first duplicate the string so we can play with it ...
	char * s = (char*) UT_strdup(r);
	char * end_s = s + strlen(s); // we need to remember where this
								  // string ends because we cannot use strtok(NULL,...)

	UT_sint32 iId;
	PP_RevisionType eType;
	XML_Char * pProps, * pAttrs,
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

		if(!cl_brace && !op_brace)
		{
			// no props
			if(eType == PP_REVISION_FMT_CHANGE)
			{
				// malformed token, move onto the next one
				UT_DEBUGMSG(("PP_RevisionAttr::_init: invalid ! token [%s]\n",t));
				continue;
			}
			pProps = NULL;
		}
		else
		{
			// OK this is a case where we have some props, i.e., it
			// must be either fmt change or addition
			if(eType == PP_REVISION_DELETION)
			{
				// malformed token, move onto the next one
				UT_DEBUGMSG(("PP_RevisionAttr::_init: invalid - token [%s]\n",t));
				continue;
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

		PP_Revision * pRevision = new PP_Revision((UT_uint32)iId, eType, pProps, pAttrs);

		m_vRev.addItem((void*)pRevision);

		if(next_s < end_s)
			t = strtok(next_s,",");
		else
			t = NULL;
	}

	FREEP(s);
	m_bDirty = true;
	m_iSuperfluous = 0;
}


/*! return highest revision associated with this revision attribute
    that has ID at most equal to id; the returned PP_Revision can be
    used to determine whether this text should be displayed or hidden
    in revision with the original id

    \param UT_uint32 id : the id of this revision
    \return : pointer to PP_Revision, or NULL if the revision
    attribute should be ignored for the present level
*/

// these are special instances of PP_Revision that are used to in the
// following function to handle special cases
static const PP_Revision s_hidden(0, PP_REVISION_DELETION, (XML_Char*)0, (XML_Char*)0);
static const PP_Revision s_visible(0, PP_REVISION_ADDITION, (XML_Char*)0, (XML_Char*)0);

const PP_Revision *  PP_RevisionAttr::getGreatestLesserOrEqualRevision(UT_uint32 id) const
{
	const PP_Revision *r = NULL; // this will be the revision we are looking for
	UT_uint32 r_id = 0;

	const PP_Revision *m = NULL; // this will be the lowest revision present
	UT_uint32 m_id = 0xFFFF;

	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
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
	// i.e., this fragment only figures in revisions > id the problem
	// with NULL is that it is visible if the smallest revision ID is
	// negative, and hidden in the opposite case -- if this is to be
	// visible we return the special static variables s_visible and
	// s_hidden

	if(r == NULL)
	{
		if(!m)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return NULL;
		}

		if(m->getType() == PP_REVISION_DELETION)
			return &s_visible;
		else if((m->getType() == PP_REVISION_ADDITION)
				||(m->getType() == PP_REVISION_ADDITION_AND_FMT))
			return &s_hidden;
		else // the initial revision was fmt change, so ignore it
			return NULL;
	}
	else
		return r;
}

/*! finds the highest revision number in this attribute
 */
const PP_Revision * PP_RevisionAttr::getLastRevision() const
{
	const PP_Revision * r = NULL;
	UT_uint32 r_id = 0;

	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * t = (const PP_Revision *)m_vRev.getNthItem(i);
		UT_uint32 t_id = t->getId();

		if(t_id > r_id)
		{
			r_id = t_id;
			r = t;
		}
	}

	return r;
}


/*! given revision level id, this function returns true if given
    segment of text is to be visible, false if it is to be hidden
*/
bool PP_RevisionAttr::isVisible(UT_uint32 id) const
{
	PP_RevisionType eType = getGreatestLesserOrEqualRevision(id)->getType();

	return ((eType == PP_REVISION_ADDITION) || (eType == PP_REVISION_ADDITION_AND_FMT));
}

/*! returns true if the text should be visible according to the last
    revision
*/

bool PP_RevisionAttr::isVisible() const
{
	PP_RevisionType eType = getLastRevision()->getType();

	return ((eType == PP_REVISION_ADDITION) || (eType == PP_REVISION_ADDITION_AND_FMT));
}

/*! adds id to the revision vector handling the special cases where id
    is already present in this attribute.
*/
void PP_RevisionAttr::addRevision(UT_uint32 iId, PP_RevisionType eType, const XML_Char ** pProps, const XML_Char ** pAttrs)
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision*) m_vRev.getNthItem(i);
		UT_uint32 r_id = r->getId();
		PP_RevisionType r_type = r->getType();

		if(iId == r_id && eType != r_type)
		{
			// we are trying to add a revision id already in the vector
			// but of a different -- this is legal, i.e., the
			// editor just changed his mind
			// we need to make distinction between different cases

			if((eType == PP_REVISION_DELETION) && (r_type == PP_REVISION_ADDITION))
			{
				// the editor originally inserted a new segment of
				// text but now wants it out; we cannot just remove
				// the id, because the original operation resulted in
				// a new fragment in the piece table; rather we will
				// mark this with '-' and will remember the superfluous
				// id, so if queried later we can work out if this
				// whole fragment should in fact go

				m_vRev.deleteNthItem(i);

				m_iSuperfluous = iId;

				const PP_Revision * pRevision = new PP_Revision(iId, eType, (XML_Char*)0, (XML_Char*)0);
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

				m_vRev.deleteNthItem(i);

				const PP_Revision * pRevision = new PP_Revision(iId, eType, (XML_Char*)0, (XML_Char*)0);
				m_vRev.addItem((void*)pRevision);
			}
			else if((eType == PP_REVISION_FMT_CHANGE) && (r_type == PP_REVISION_DELETION))
			{
				// originally the editor marked the text for deletion,
				// but now he just wants a format change, in this case
				// we just replace the old revision with the new

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

				// I will implement this here, but this case should
				// really be handled higher up and the property
				// changes should be applied directly to the fragments props
				r->mergeProps(pProps);
				r->mergeAttrs(pAttrs);
			}

			m_bDirty = true;
			return;
		}

		if(iId == r_id && eType == r_type)
		{
			// we are trying to add an id already in the vector
			// do nothing
			return;
		}
	}

	// if we got here then the item is not in our vector so add it
	const PP_Revision * pRevision = new PP_Revision(iId, eType, pProps, pAttrs);
	m_vRev.addItem((void*)pRevision);
	m_bDirty = true;
}

/*! removes id from this revision, respecting the sign, i.e., it will
  not remove -5 if given 5
 */
void PP_RevisionAttr::removeRevisionIdWithType(UT_uint32 iId, PP_RevisionType eType)
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if((iId == r->getId()) && (eType == r->getType()))
		{
			m_vRev.deleteNthItem(i);
			m_bDirty = true;
			return;
		}
	}
}

/*! removes id from the attribute disregarding sign, i.e.,
    if given 5 it will remove both -5 and +5
*/
void PP_RevisionAttr::removeRevisionIdTypeless(UT_uint32 iId)
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId == r->getId())
		{
			m_vRev.deleteNthItem(i);
			m_bDirty = true;
			return;
		}
	}
}

/*! removes all IDs from the attribute whose value is lesser or
    equal the given id
*/
void PP_RevisionAttr::removeAllLesserOrEqualIds(UT_uint32 iId)
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);

		if(iId >= r->getId())
		{
			m_vRev.deleteNthItem(i);
		}
	}

	m_bDirty = true;
}


/*! create XML string from our vector
 */
void PP_RevisionAttr::_refreshString()
{
	char buf[30];
	m_sXMLstring.clear();
	UT_uint32 iCount = m_vRev.getItemCount();

	for(UT_uint32 i = 0; i < iCount; i++)
	{
		PP_Revision * r = (PP_Revision *)m_vRev.getNthItem(i);
		PP_RevisionType r_type = r->getType();

		if(r_type == PP_REVISION_FMT_CHANGE)
			m_sXMLstring += "!";

		// print the id with appropriate sign
		sprintf(buf,"%d",r->getId()* ((r_type == PP_REVISION_DELETION)?-1:1));
		m_sXMLstring += buf;

		if((r_type == PP_REVISION_FMT_CHANGE)||(r_type == PP_REVISION_ADDITION_AND_FMT))
		{
			m_sXMLstring += "{";
			m_sXMLstring += r->getPropsString();
			m_sXMLstring += "}";

			if(r->getAttrsVector()->getItemCount())
			{
				m_sXMLstring += "{";
				m_sXMLstring += r->getAttrsString();
				m_sXMLstring += "}";
			}
		};

		if(i != iCount - 1)
		{
			//not the last itteration, append ','
			m_sXMLstring += ",";
		}

	}
	m_bDirty = false;
}



/*! get an XML_Char string representation of this revision
 */
const XML_Char * PP_RevisionAttr::getXMLstring()
{
	if(m_bDirty)
		_refreshString();

	return (const XML_Char*) m_sXMLstring.c_str();
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
		UT_ASSERT(((PP_Revision *)m_vRev.getNthItem(0))->getId() == m_iSuperfluous);
		return true;
	}
	else
		return false;
}

bool PP_RevisionAttr::operator == (const PP_RevisionAttr &op2) const
{
	for(UT_uint32 i = 0; i < m_vRev.getItemCount(); i++)
	{
		const PP_Revision * r1 = (const PP_Revision *) m_vRev.getNthItem(i);

		for(UT_uint32 j = 0; j < op2.m_vRev.getItemCount(); j++)
		{
			const PP_Revision * r2 = (const PP_Revision *) op2.m_vRev.getNthItem(j);

			if(!(*r1 == *r2))
				return false;
		}
	}
	return true;
}

/*! returns true if after revision iId this fragment carries revised
    property pName, the value of which will be stored in pValue; see
    notes on PP_Revision::hasProperty(...)
*/
bool PP_RevisionAttr::hasProperty(UT_uint32 iId, const XML_Char * pName, const XML_Char * &pValue) const
{
	const PP_Revision * r = getGreatestLesserOrEqualRevision(iId);
	return r->hasProperty(pName, pValue);
}

/*! returns true if after the last revision this fragment carries revised
    property pName, the value of which will be stored in pValue; see
    notes on PP_Revision::hasProperty(...)
*/
bool PP_RevisionAttr::hasProperty(const XML_Char * pName, const XML_Char * &pValue) const
{
	const PP_Revision * r = getLastRevision();
	return r->hasProperty(pName, pValue);
}

/*! returns the type of cumulative revision up to iId represented by this attribute
 */
PP_RevisionType PP_RevisionAttr::getType(UT_uint32 iId) const
{
	const PP_Revision * r = getGreatestLesserOrEqualRevision(iId);
	return r->getType();
}

/*! returns the type of overall cumulative revision represented by this attribute
 */
PP_RevisionType PP_RevisionAttr::getType() const
{
	const PP_Revision * r = getLastRevision();
	return r->getType();
}

/*! get properties associated with revision level iId */
const UT_Vector * PP_RevisionAttr::getProps(UT_uint32 iId) const
{
	const PP_Revision * r = getGreatestLesserOrEqualRevision(iId);
	return r->getPropsVector();
}

/*! get properties associated with last revision */
const UT_Vector * PP_RevisionAttr::getProps() const
{
	const PP_Revision * r = getLastRevision();
	return r->getPropsVector();
}

