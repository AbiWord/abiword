/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (c) 2010 GPL. V2+ copyright to AbiSource B.V.
 * This file was originally written by Ben Martin in 2010.
 * Updates by Ben Martin in 2011.
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


#include "pd_DocumentRDF.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"
#include "ut_debugmsg.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "ut_std_string.h"
#include "ut_conversion.h"
#include "xap_App.h"

#include <sstream>
#include <set>
#include <fstream>

#include <iterator>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pd_RDFQuery.h"


#define DEBUG_LOWLEVEL_IO    0

typedef std::map< std::string, std::string > stringmap_t;

/******************************/
/******************************/
/******************************/

class PD_SemanticItemFactoryNull
    :
    public PD_SemanticItemFactory
{
public:
    virtual PD_RDFContact*  createContact(PD_DocumentRDFHandle /*rdf*/, 
										  PD_ResultBindings_t::iterator /*it*/)
    {
        return 0;
    }
    virtual PD_RDFEvent*    createEvent(PD_DocumentRDFHandle /*rdf*/, 
										PD_ResultBindings_t::iterator /*it*/)
    {
        return 0;
    }
    virtual PD_RDFLocation* createLocation(PD_DocumentRDFHandle /*rdf*/, 
										   PD_ResultBindings_t::iterator /*it*/,
										   bool isGeo84 = false )
    {
		UT_UNUSED(isGeo84);
        return 0;
    }
};
PD_SemanticItemFactory *PD_DocumentRDF::s_SemanticItemFactory; 
PD_SemanticItemFactory *PD_DocumentRDF::getSemanticItemFactory()
{
    if (!s_SemanticItemFactory) s_SemanticItemFactory = new PD_SemanticItemFactoryNull;
    return s_SemanticItemFactory;
}
void
PD_DocumentRDF::setSemanticItemFactory( PD_SemanticItemFactory* f )
{
    s_SemanticItemFactory = f;
}


class PD_RDFDialogsNull : public PD_RDFDialogs
{
  public:
    virtual void runSemanticStylesheetsDialog(FV_View* /*pView*/)
    {
    }
    virtual std::pair< PT_DocPosition, PT_DocPosition > runInsertReferenceDialog(FV_View* /*pView*/)
    {
		return std::make_pair(0,0);
    }
};
PD_RDFDialogs *PD_DocumentRDF::s_RDFDialogs;
PD_RDFDialogs *PD_DocumentRDF::getRDFDialogs()
{
    if (!s_RDFDialogs) s_RDFDialogs = new PD_RDFDialogsNull; 
    return s_RDFDialogs;
}
void
PD_DocumentRDF::setRDFDialogs( PD_RDFDialogs* d )
{
    s_RDFDialogs = d;
}

std::pair< PT_DocPosition, PT_DocPosition > runInsertReferenceDialog( FV_View* pView )
{
    return PD_DocumentRDF::getRDFDialogs()->runInsertReferenceDialog( pView );
}

void runSemanticStylesheetsDialog( FV_View* pView )
{
    PD_DocumentRDF::getRDFDialogs()->runSemanticStylesheetsDialog( pView );
}



/******************************/
/******************************/
/******************************/



PD_RDFModel::PD_RDFModel()
    : m_version(0)
{
}


void
PD_RDFModel::incremenetVersion()
{
    m_version++;
}



/**
 * Grab the first URI in the list or a default constructed one for empty lists.
 */
PD_URI
PD_RDFModel::front( const PD_URIList& l ) const
{
    if(l.empty())
    {
        return PD_URI();
    }
    return l.front();
}
PD_Object
PD_RDFModel::front( const PD_ObjectList& l ) const
{
    if(l.empty())
    {
        return PD_Object();
    }
    return l.front();
}


/**
 * Get an object which has the given subject and predicate. If there
 * are more than one such object, any one of those will be returned.
 * So you should only use this method when the RDF only allows a
 * single matching triple.
 */
PD_Object
PD_RDFModel::getObject( const PD_URI& s, const PD_URI& p )
{
    PD_ObjectList l = getObjects(s,p);
    return front(l);
}

/**
 * Similar to getSubjects but returns any one of the matches if there are
 * more than one. Use this method when the RDF schema only allows a single
 * matching triple.
 */
PD_URI
PD_RDFModel::getSubject( const PD_URI& p, const PD_Object& o )
{
    PD_URIList l = getSubjects( p,o );
    return front(l);
}

PD_ObjectList
PD_RDFModel::getObjects( const PD_URI& s, const PD_URI& p )
{
    PD_ObjectList ret;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        if( st.getSubject() == s && st.getPredicate() == p )
        {
            ret.push_back( st.getObject() );
        }
    }
    return ret;    
}

PD_URIList
PD_RDFModel::getSubjects( const PD_URI& p, const PD_Object& o )
{
    PD_URIList ret;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        if( st.getPredicate() == p && st.getObject() == o )
        {
            ret.push_back( st.getSubject() );
        }
    }
    return ret;
}


POCol
PD_RDFModel::getArcsOut( const PD_URI& s )
{
    POCol ret;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        if( st.getSubject() == s )
        {
            ret.insert( std::make_pair( st.getPredicate(), st.getObject() ));
        }
    }
    return ret;
}


bool
PD_RDFModel::contains( const PD_RDFStatement& st )
{
    return contains( st.getSubject(), st.getPredicate(), st.getObject() );
}

/**
 * Test if the given triple is in the RDF or not.
 */
bool
PD_RDFModel::contains( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    PD_RDFStatement sought( s, p, o );
    
    bool ret = false;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        if( st == sought )
            return true;
    }
    return ret;
}

long
PD_RDFModel::getTripleCount()
{
    long ret = 0;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        ++ret;
    }
    return ret;
}



/**
 * Get a list of every subject in the model.
 * If you want to get all triples, then first get all subjects
 * and then use getArcsOut() to get the predicate-object combinations
 * for each of the subjects.
 */
PD_URIList
PD_RDFModel::getAllSubjects()
{
    PD_URIList ret;
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        ret.push_back( st.getSubject() );
    }
    return ret;
}



/**
 * Test if the given subject+predicate is in the RDF or not.
 */
bool
PD_RDFModel::contains( const PD_URI& s, const PD_URI& p )
{
    PD_URI u = getObject( s, p );
    return u.isValid();
}

void
PD_RDFModel::dumpModel( const std::string& headerMsg )
{
	UT_DEBUG_ONLY_ARG(headerMsg);

#ifdef DEBUG    
    PD_RDFModelIterator iter = begin();
    PD_RDFModelIterator    e = end();

    UT_DEBUGMSG(("PD_RDFModel::dumpModel() ----------------------------------\n"));
    UT_DEBUGMSG(("PD_RDFModel::dumpModel() %s\n", headerMsg.c_str()));
    UT_DEBUGMSG(("PD_RDFModel::dumpModel() triple count:%ld\n", getTripleCount()));
    UT_DEBUGMSG(("PD_RDFModel::dumpModel() ----------------------------------\n"));
    
    for( ; iter != e; ++iter )
    {
        PD_RDFStatement st = *iter;
        UT_DEBUGMSG(("PD_RDFModel::dumpModel() st:%s\n", st.toString().c_str() ));
        
    }
    UT_DEBUGMSG(("PD_RDFModel::dumpModel() --- done -------------------------\n"));
#endif
    
}

std::string
PD_RDFModel::uriToPrefixed( const std::string& uri )
{
    uriToPrefix_t& m = getUriToPrefix();

    for( uriToPrefix_t::iterator iter = m.begin();
         iter != m.end(); ++iter )
    {
        const std::string& p  = iter->second;
        const std::string& ns = iter->first;
        
        if( starts_with( uri, p ))
        {
            return ns + ":" + uri.substr(p.length());
        }
    }
    return uri;
}

std::string
PD_RDFModel::prefixedToURI( const std::string& prefixedstr )
{
	std::string::size_type colonLocation = prefixedstr.find(":");
    if( colonLocation != std::string::npos )
    {
        std::string prefix = prefixedstr.substr( 0, colonLocation );
        std::string rest   = prefixedstr.substr( colonLocation+1 );
        uriToPrefix_t& m = getUriToPrefix();
        uriToPrefix_t::iterator mi = m.find( prefix );
        if( mi != m.end() )
        {
            std::stringstream ss;
            ss << mi->second << rest;
            return ss.str();
        }
    }
    return prefixedstr;
}

PD_RDFModel::uriToPrefix_t&
PD_RDFModel::getUriToPrefix()
{
    static uriToPrefix_t m;
    if( m.empty() )
    {
        m.insert( std::make_pair( "pkg",   "http://docs.oasis-open.org/opendocument/meta/package/common#" ));
        m.insert( std::make_pair( "odf",   "http://docs.oasis-open.org/opendocument/meta/package/odf#" ));
        m.insert( std::make_pair( "rdf",   "http://www.w3.org/1999/02/22-rdf-syntax-ns#" ));
        m.insert( std::make_pair( "dcterms", "http://dublincore.org/documents/dcmi-terms/#" ));
        m.insert( std::make_pair( "cite",  "http://docs.oasis-open.org/prototype/opendocument/citation#" ));
        m.insert( std::make_pair( "foaf",  "http://xmlns.com/foaf/0.1/" ));
        m.insert( std::make_pair( "example", "http://www.example.org/xmlns/ex#" ));
        m.insert( std::make_pair( "geo84", "http://www.w3.org/2003/01/geo/wgs84_pos#"  ));
        m.insert( std::make_pair( "rdfs",  "http://www.w3.org/2000/01/rdf-schema#" ));
        m.insert( std::make_pair( "dc",    "http://purl.org/dc/elements/1.1/"  ));
        m.insert( std::make_pair( "cal",   "http://www.w3.org/2002/12/cal/icaltzd#"  ));

        m.insert( std::make_pair( "abifoaf",  "http://abicollab.net/rdf/foaf#" ));
    }
    
    return m;   
}



/**
 * Read a string which is prefixed with it's length in ascii from the
 * given stream.
 * @param iss stream to read length and string from
 * @return string
 */
static std::string readLengthPrefixedString( std::istream& iss )
{
    char ch;
    int len = 0;
    iss >> len >> std::noskipws >> ch;

#if DEBUG
    if( DEBUG_LOWLEVEL_IO )
    {
        off_t loc = iss.tellg();
        UT_DEBUGMSG(("PD_DocumentRDF::readLengthPrefixedString() len:%d loc:%ld\n", 
					 len, (long)loc));
    }
#endif
    
    char* p = new char[len+2];
    memset( p, 0, len+2 );
    iss.read( p, len );
    std::string ret = p;
    delete [] p;
    return ret;
}

/**
 * Given a string z, create a new string which contains the length of z
 * as an ascii string, a space and then z itself.
 * @param s the string z to create a length z string from
 * @return a string containing the length of z, a space, then z
 */
static std::string createLengthPrefixedString( const std::string& s )
{
    std::stringstream ss;
    ss << s.length() << " " << s;
    return ss.str();
}

/**
 * Given an predicate and object, create a serialized "packed" double of these two.
 * @see splitPO()
 */
static std::string combinePO(const PD_URI& p, const PD_Object& o )
{
    std::stringstream ss;
    p.write(ss);
    o.write(ss);
    return ss.str();
}


/**
 * Split up a packed double that contains a predicate and object.
 * @see combinePO()
 */
static std::pair< PD_URI, PD_Object > splitPO( const std::string& po )
{
    std::stringstream ss;
    ss << po;
    
    PD_URI p;
    PD_Object o;
    p.read( ss );
    o.read( ss );
    return std::make_pair( p, o );
}

/**
 * Decode a list of predicate,object pairings that was created with encodePOCol().
 *
 * @see encodePOCol()
 */
static POCol decodePOCol( const std::string& data )
{
//    UT_DEBUGMSG(("PD_DocumentRDF::decodePOCol() data:%s\n", data.c_str() ));
    POCol ret;
    if( data.empty() )
        return ret;

    char ch;
    int sz = 0;
    std::stringstream ss;
    ss << data;
    ss >> sz >> std::noskipws >> ch;
//    UT_DEBUGMSG(("PD_DocumentRDF::decodePOCol() sz:%d\n", sz ));
    for( int i=0; i<sz; ++i )
    {
        std::string po = readLengthPrefixedString( ss );
        ss >> std::noskipws >> ch;
//        UT_DEBUGMSG(("PD_DocumentRDF::decodePOCol() po:%s\n", po.c_str() ));
        
        std::pair< PD_URI, PD_Object > p = splitPO( po );
        ret.insert(p);
    }
    return ret;
}


/**
 * Encode a list of predicate,object pairings into a single string.
 * 
 * @see decodePOCol()
 */
static std::string encodePOCol( const POCol& l )
{
    std::stringstream ss;
    ss << l.size() << " ";
    POCol::const_iterator e = l.end();
    for( POCol::const_iterator iter = l.begin(); iter != e; ++iter )
    {
        std::string po = combinePO( iter->first, iter->second );
        ss << createLengthPrefixedString(po) << ' ';
    }
    return ss.str();
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

PD_URI::PD_URI( const std::string& v )
    :
    m_value( v )
{
}

/**
 * get a std::string representation
 */
std::string PD_URI::toString() const
{
    return m_value;
}

/**
 * Only very basic tests for validity are performed. The
 * main use of this method is to detect default constructed
 * PD_URI values
 */
bool PD_URI::isValid() const
{
    return !m_value.empty();
}

bool PD_URI::operator==(const PD_URI& b) const
{
    return m_value == b.m_value;
}

bool PD_URI::operator==(const std::string& b) const
{
    return m_value == b;
}

bool PD_URI::operator<(const PD_URI& b) const
{
    return m_value < b.m_value;
}

PD_URI
PD_URI::prefixedToURI( const PD_RDFModelHandle & model ) const
{
    PD_URI ret( model->prefixedToURI( toString() ));
    return ret;
}

/**
 * Deserialize the PD_URI from the given stream.
 *
 * I thought about using boost::serialization for this, as it
 * is doing a similar thing. Using b::s though would have required
 * a link dependancy.
 *
 * @see write()
 */
bool PD_URI::read( std::istream& ss )
{
    char ch;
    int version = 0;
    int numParts = 0;
    ss >> version  >> std::noskipws >> ch;
    ss >> numParts >> std::noskipws >> ch;
    m_value = readLengthPrefixedString(ss);
    ss >> std::noskipws >> ch;
    return true;
}

/**
 * Write the URI to the given stream.
 * @see read()
 */
bool PD_URI::write( std::ostream& ss ) const
{
    int version = 1;
    int numParts = 1;
    ss << version << " " << numParts << " ";
    ss << createLengthPrefixedString(m_value) << " ";
    return true;
}

bool operator<(const std::pair< PD_URI, PD_URI > & a, const PD_URI & b)
{
    return a.first.toString() < b.toString();
}

bool operator<(const PD_URI &a, const std::pair< PD_URI, PD_URI > & b )
{
    return a.toString() < b.first.toString();
}

/****************************************/
/****************************************/
/****************************************/

PD_Object::PD_Object( const std::string& v )
    : PD_URI(v)
    , m_objectType( OBJECT_TYPE_URI )
{
}

PD_Object::PD_Object( const PD_URI& u )
    : PD_URI(u.toString())
    , m_objectType( OBJECT_TYPE_URI )
{
}



PD_Object::PD_Object( const std::string& v, int objectType, const std::string& type )
    : PD_URI(v)
    , m_xsdType(type)
    , m_objectType(objectType)
{
}

int PD_Object::getObjectType() const
{
    return m_objectType;
}

bool PD_Object::isLiteral() const
{
    return m_objectType == OBJECT_TYPE_LITERAL;
}

bool PD_Object::isURI() const
{
    return m_objectType == OBJECT_TYPE_URI;
}

bool PD_Object::isBNode() const
{
    return m_objectType == OBJECT_TYPE_BNODE;
}




std::string PD_Object::getXSDType() const
{
    return m_xsdType;
}

bool PD_Object::hasXSDType() const
{
    return !m_xsdType.empty();
}


bool PD_Object::read( std::istream& ss )
{
    char ch;
    int version = 0;
    int numParts = 0;
    ss >> version  >> std::noskipws >> ch;
    ss >> numParts >> std::noskipws >> ch;
    ss >> m_objectType >> std::noskipws >> ch;
    m_value = readLengthPrefixedString(ss);
    ss >> std::noskipws >> ch;
    m_xsdType = readLengthPrefixedString(ss);
    ss >> std::noskipws >> ch;
    m_context = readLengthPrefixedString(ss);
    ss >> std::noskipws >> ch;
    return true;
}

bool PD_Object::write( std::ostream& ss ) const
{
    int version = 1;
    int numParts = 4;
    ss << version << " " << numParts << " ";
    ss << m_objectType << " ";
    ss << createLengthPrefixedString(m_value)   << " ";
    ss << createLengthPrefixedString(m_xsdType) << " ";
    ss << createLengthPrefixedString(m_context) << " ";
    return true;
}

PD_Literal::PD_Literal( const std::string& v, const std::string& xsdtype )
    :
    PD_Object( v, OBJECT_TYPE_LITERAL, xsdtype )
{
}

/****************************************/
/****************************************/
/****************************************/

PD_RDFStatement::PD_RDFStatement()
    : m_isValid( false )
{
}

PD_RDFStatement
PD_RDFStatement::prefixedToURI( PD_RDFModelHandle model ) const
{
    PD_RDFStatement ret( model->prefixedToURI( getSubject().toString()),
                         model->prefixedToURI( getPredicate().toString()),
                         PD_Object( model->prefixedToURI( getObject().toString())));
    return ret;
}


PD_RDFStatement
PD_RDFStatement::uriToPrefixed( PD_RDFModelHandle model ) const
{
    PD_RDFStatement ret( model->uriToPrefixed( getSubject().toString()),
                         model->uriToPrefixed( getPredicate().toString()),
                         PD_Object( model->uriToPrefixed( getObject().toString())));
    return ret;
}

bool
PD_RDFStatement::operator==(const PD_RDFStatement& b) const
{
    return getSubject()   == b.getSubject()
        && getPredicate() == b.getPredicate()
        && getObject()    == b.getObject();
}



PD_RDFStatement::PD_RDFStatement( PD_RDFModelHandle model, const PD_URI& s, const PD_URI& p, const PD_Object& o )
    : m_subject( s.prefixedToURI(model) )
    , m_predicate( p.prefixedToURI(model) )
    , m_object( o.prefixedToURI(model).toString() )
    , m_isValid( true )
{
}


PD_RDFStatement::PD_RDFStatement( const PD_URI& s, const PD_URI& p, const PD_Object& o )
    : m_subject( s )
    , m_predicate( p )
    , m_object( o )
    , m_isValid( true )
{
}

PD_RDFStatement::PD_RDFStatement( const std::string& s, const std::string& p, const PD_Object& o )
    : m_subject( PD_URI(s) )
    , m_predicate( PD_URI(p) )
    , m_object( o )
    , m_isValid( true )
{
}

PD_RDFStatement::PD_RDFStatement( const std::string& s, const std::string& p, const PD_Literal& o )
    : m_subject( PD_URI(s) )
    , m_predicate( PD_URI(p) )
    , m_object( o )
    , m_isValid( true )
{
}




const PD_URI&
PD_RDFStatement::getSubject() const
{
    return m_subject;
}

const PD_URI&
PD_RDFStatement::getPredicate() const
{
    return m_predicate;
}

const PD_Object&
PD_RDFStatement::getObject() const
{
    return m_object;
}

bool
PD_RDFStatement::isValid() const
{
    return m_isValid;
}

std::string
PD_RDFStatement::toString() const
{
    std::stringstream ss;
    ss << " s:" << m_subject.toString()
       << " p:" << m_predicate.toString()
       << " ot:" << m_object.getObjectType() << " o:" << m_object.toString() << " ";
    return ss.str();
}




/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/

class PD_RDFModelFromAP : public PD_DocumentRDF
{
    // Can't do this...
    PD_RDFModelFromAP& operator=( const PD_RDFModelFromAP& other );
    
protected:
    
    const PP_AttrProp* m_AP;

public:

    explicit PD_RDFModelFromAP( PD_Document* doc, const PP_AttrProp* AP )
        :
        PD_DocumentRDF( doc ),
        m_AP(AP)
    {
        UT_DEBUGMSG((" PD_RDFModelFromAP() this:%p\n",this));
    }
    virtual ~PD_RDFModelFromAP()
    {
        UT_DEBUGMSG(("~PD_RDFModelFromAP() this:%p\n",this));
        delete m_AP;
    }
    virtual const PP_AttrProp* getAP(void)
    {
        return m_AP;
    }
    virtual UT_Error setAP( PP_AttrProp* newAP )
    {
        delete m_AP;
        m_AP = newAP;
        return UT_OK;
    }
    virtual bool isStandAlone() const
    {
        return true;
    }
    virtual void maybeSetDocumentDirty()
    {
    }
    
};


/**
 * The idea of this class is to be able to slice up a document's RDF.
 * So you can explicitly say, give me the RDF that is contained in all
 * the AP between two positions in the document.
 */
class PD_RDFModelFromStartEndPos : public PD_DocumentRDF
{
    PT_DocPosition m_beginPos;
    PT_DocPosition m_endPos;
    
public:
    explicit PD_RDFModelFromStartEndPos( PD_Document* doc, PT_DocPosition b, PT_DocPosition e )
        : PD_DocumentRDF(doc)
        , m_beginPos(b)
        , m_endPos(e)
    {
        updateAPList();
    }
    virtual ~PD_RDFModelFromStartEndPos()
    {
    }
    virtual const PP_AttrProp* getAP(void)
    {
        UT_DEBUGMSG(("ERROR: getAP() is not valid for a start-end position rdf model\n"));
        return 0;
    }
    virtual UT_Error setAP( PP_AttrProp* newAP )
    {
        UT_UNUSED( newAP );
        return UT_OK;
    }
    virtual bool isStandAlone() const
    {
        return true;
    }

    ////////////////
    // For iterating over the PP_AttrProp that are in range. 
    typedef std::list< const PP_AttrProp* > m_APList_t;
    m_APList_t m_APList;
    void updateAPList()
    {
        m_APList.clear();

        pf_Frag* frag    = m_doc->getFragFromPosition( m_beginPos );
        pf_Frag* endFrag = m_doc->getFragFromPosition( m_endPos );
        if( !frag || !endFrag )
        {
            UT_DEBUGMSG(("updateAPList() bpos:%d epos:%d frag:%p endFrag:%p\n",
                         m_beginPos, m_endPos, frag, endFrag ));
            return;
        }
        
        endFrag = endFrag->getNext();

        for( ; frag != endFrag; frag = frag->getNext() )
        {
            PT_AttrPropIndex api = frag->getIndexAP();
            const PP_AttrProp * pAP = 0;
            m_doc->getAttrProp( api, &pAP );
            m_APList.push_back( pAP );
        }
    }
    m_APList_t::iterator apBegin()
    {
        return m_APList.begin();
    }
    
    m_APList_t::iterator apEnd()
    {
        return m_APList.end();
    }

    ////////////////
    // Statement iterator methods...
    // PD_RDFStatement
    class StatementIterator
        :
        public std::iterator< std::forward_iterator_tag, PD_RDFStatement >
    {
    private:
        typedef std::list< const PP_AttrProp* > m_APList_t;
        bool m_end;
        m_APList_t::iterator m_apiter;
        m_APList_t::iterator m_apenditer;
        size_t          m_apPropertyNumber;
        std::string     m_subject;
        POCol           m_pocol;
        POCol::iterator m_pocoliter;
        PD_RDFStatement m_current;
        
    public:
        typedef StatementIterator& self_reference;
        typedef StatementIterator  self_type;
        
        StatementIterator()
            : m_end( true )
            , m_apPropertyNumber( 0 )
        {
        }
        StatementIterator( m_APList_t::iterator iter, m_APList_t::iterator enditer )
            : m_end( false )
            , m_apiter( iter )
            , m_apenditer( enditer )
            , m_apPropertyNumber( 0 )
        {
        }

        void advance_apiter()
        {
            ++m_apiter;
            if( m_apiter == m_apenditer )
            {
                m_end = true;
                return;
            }
            m_apPropertyNumber = 0;
            m_subject = "";
            m_pocol.clear();
            m_pocoliter = m_pocol.end();
        }
        void setup_pocol()
        {
            UT_DEBUGMSG(("SI... statement iter++/setup_pocol(top)\n" ));            
            const gchar * szName  = 0;
            const gchar * szValue = 0;
            const PP_AttrProp* AP = *m_apiter;
            if( AP->getNthProperty( m_apPropertyNumber, szName, szValue) )
            {
//                UT_DEBUGMSG(("statement iter++/setup_pocol szName :%s\n", szName ));
//                UT_DEBUGMSG(("statement iter++/setup_pocol szValue:%s\n", szValue ));
                m_subject   = szName;
                m_pocol     = decodePOCol( szValue );
                if( m_pocol.empty() )
                    return;
                
                m_pocoliter = m_pocol.begin();

                std::string pred = m_pocoliter->first.toString();
                PD_Object   obj  = m_pocoliter->second;
                m_current = PD_RDFStatement( m_subject, pred, obj );
            }
        }
        
        self_reference operator++()
        {
            if( m_end )
                return *this;
            if( m_apiter == m_apenditer )
            {
                m_end = true;
                return *this;
            }

            /**
             * We have to walk over each AP using m_apiter until we hit m_apenditer
             * 
             * For each of these AP;
             *   we have to walk over all the properties
             *     ( each prop is a subject -> list[ pred+obj ] )
             *
             *   For each of these properties;
             *     we have to walk over all the pairs in the pocol.
             */

            /// FIXME:
            
            
            const PP_AttrProp* AP = *m_apiter;
            size_t count = AP->getPropertyCount();
            // if( m_apPropertyNumber == count )
            // {
            //     advance_apiter();
            //     return operator++();
            // }
            while( m_pocol.empty() )
            {
                if( m_apPropertyNumber == count )
                {
                    advance_apiter();
                    return operator++();
                }
                setup_pocol();
                ++m_apPropertyNumber;
            }
            
            std::string pred = m_pocoliter->first.toString();
            PD_Object   obj  = m_pocoliter->second;
            m_current = PD_RDFStatement( m_subject, pred, obj );
            ++m_pocoliter;
            if( m_pocoliter == m_pocol.end() )
            {
                m_pocol.clear();
            }
            
            return *this;
        }
        self_type operator++(int)
        {
            self_type result( *this );
            ++( *this );
            return result;
        }

        bool operator==( const self_reference other )
        {
            if( m_end && other.m_end )
                return true;
            if( (!m_end && other.m_end)
                || (m_end && !other.m_end) )
            {
                return false;
            }
            return m_apPropertyNumber == other.m_apPropertyNumber
                && m_pocoliter == other.m_pocoliter
                && m_apiter == other.m_apiter;
        }
        bool operator!=( const self_reference other )
        {
            return !operator==(other);
        }
        reference operator*()
        {
            return m_current;
        }
    };

    // StatementIterator begin()
    // {
    //     return StatementIterator( apBegin(), apEnd() );
    // }
    // StatementIterator end()
    // {
    //     return StatementIterator();
    // }
    
    
    

    ////////////////
    // PD_RDFModel methods...

    virtual PD_ObjectList getObjects( const PD_URI& s, const PD_URI& p )
    {
        PD_ObjectList ret;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apGetObjects( *iter, ret, s, p );
        return ret;
    }
    
    virtual PD_Object getObject( const PD_URI& s, const PD_URI& p )
    {
        PD_ObjectList l = getObjects(s,p);
        return front(l);
    }
    
    virtual PD_URIList getSubjects( const PD_URI& p, const PD_Object& o )
    {
        PD_URIList ret;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apGetSubjects( *iter, ret, p, o );
        return ret;
    }
    
    virtual PD_URI getSubject( const PD_URI& p, const PD_Object& o )
    {
        PD_URIList l = getSubjects( p,o );
        return front(l);
    }
    
    virtual PD_URIList getAllSubjects()
    {
        PD_URIList ret;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apGetAllSubjects( *iter, ret );
        return ret;
    }
    
    virtual POCol getArcsOut( const PD_URI& s )
    {
        POCol ret;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apGetArcsOut( *iter, ret, s );
        return ret;
    }
    
    virtual bool contains( const PD_URI& s, const PD_URI& p, const PD_Object& o )
    {
        bool ret = false;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
        {
            ret |= apContains( *iter, s, p, o );
            if( ret )
                break;
        }
        return ret;
    }
    
    virtual void dumpModel( const std::string& headerMsg = "dumpModel()" )
    {
        UT_DEBUG_ONLY_ARG(headerMsg);

#ifdef DEBUG
        UT_DEBUGMSG(("PD_RDFModelFromStartEndPos::dumpModel() doc:%p\n", m_doc));
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apDumpModel( *iter, headerMsg );
#endif
    }
    

    
    
};



/****************************************/
/****************************************/
/****************************************/

PD_RDFModelIterator::PD_RDFModelIterator()
    : m_AP( 0 )
    , m_end( true )
    , m_apPropertyNumber( 0 )
{
}

PD_RDFModelIterator::~PD_RDFModelIterator()
{
    xxx_UT_DEBUGMSG(("~PD_RDFModelIterator() model: %p\n", m_model.get()));
}

PD_RDFModelIterator::PD_RDFModelIterator( PD_RDFModelHandle model, const PP_AttrProp* AP )
    : m_model( model )
    , m_AP( AP )
    , m_end( false )
    , m_apPropertyNumber( 0 )
{
    xxx_UT_DEBUGMSG(("PD_RDFModelIterator() model: %p\n", model.get()));
    operator++();
}

void
PD_RDFModelIterator::setup_pocol()
{
    xxx_UT_DEBUGMSG(("MI statement iter++/setup_pocol(top) apn:%d\n", m_apPropertyNumber ));
    const gchar * szName  = 0;
    const gchar * szValue = 0;
    if( m_AP->getNthProperty( m_apPropertyNumber, szName, szValue) )
    {
        xxx_UT_DEBUGMSG(("MI statement iter++/setup_pocol szName :%s\n", szName ));
        xxx_UT_DEBUGMSG(("MI statement iter++/setup_pocol szValue:%s\n", szValue ));
        m_subject   = szName;
        m_pocol     = decodePOCol( szValue );
        if( m_pocol.empty() )
            return;
                
        m_pocoliter = m_pocol.begin();

        std::string pred = m_pocoliter->first.toString();
        PD_Object   obj  = m_pocoliter->second;
        m_current = PD_RDFStatement( m_subject, pred, obj );
    }
}

bool
PD_RDFModelIterator::moveToNextSubjectHavePOCol()
{
    return !m_pocol.empty();
}

void
PD_RDFModelIterator::moveToNextSubjectReadPO()
{
    setup_pocol();
    std::string pred = m_pocoliter->first.toString();
    PD_Object   obj  = m_pocoliter->second;
    m_current = PD_RDFStatement( m_subject, pred, obj );
    ++m_pocoliter;
    if( m_pocoliter == m_pocol.end() )
    {
        m_pocol.clear();
    }
}


PD_RDFModelIterator::self_reference
PD_RDFModelIterator::moveToNextSubject()
{
    if( m_end )
        return *this;
    ++m_apPropertyNumber;
    if( m_apPropertyNumber == m_AP->getPropertyCount() )
    {
        m_end = true;
        return *this;
    }
    const gchar * szName  = 0;
    const gchar * szValue = 0;
    m_AP->getNthProperty( m_apPropertyNumber, szName, szValue );
    m_subject = szName;
    m_current = PD_RDFStatement( m_subject, PD_URI(), PD_Object() );
    m_pocol.clear();
    return *this;
}


PD_RDFModelIterator::self_reference
PD_RDFModelIterator::operator++()
{
    if( m_end )
        return *this;

    /**
     * We have to walk over each AP using m_apiter until we hit m_apenditer
     * 
     * For each of these AP;
     *   we have to walk over all the properties
     *     ( each prop is a subject -> list[ pred+obj ] )
     *
     *   For each of these properties;
     *     we have to walk over all the pairs in the pocol.
     */

    /// FIXME:
            
            
    size_t count = m_AP->getPropertyCount();
    while( m_pocol.empty() )
    {
        if( m_apPropertyNumber == count )
        {
            m_end = true;
            return *this;
        }
        setup_pocol();
        ++m_apPropertyNumber;
    }
            
    std::string pred = m_pocoliter->first.toString();
    PD_Object   obj  = m_pocoliter->second;
    m_current = PD_RDFStatement( m_subject, pred, obj );
    ++m_pocoliter;
    if( m_pocoliter == m_pocol.end() )
    {
        m_pocol.clear();
    }
            
    return *this;
}

bool
PD_RDFModelIterator::operator==( self_constref other )
{
    if( m_end && other.m_end )
        return true;
    if( (!m_end && other.m_end)
        || (m_end && !other.m_end) )
    {
        return false;
    }
    return m_apPropertyNumber == other.m_apPropertyNumber
        && m_pocoliter == other.m_pocoliter;
}

PD_RDFModelIterator&
PD_RDFModelIterator::operator=( const PD_RDFModelIterator& r )
{
    if( this != &r )
    {
        xxx_UT_DEBUGMSG(("PD_RDFModelIterator op=() model: %p r.model: %p\n", m_model.get(),
						 r.m_model.get()));
        m_model = r.m_model;
        m_AP = r.m_AP;
        m_end = r.m_end;
        m_apPropertyNumber = r.m_apPropertyNumber;
        m_subject = r.m_subject;
        m_pocol = r.m_pocol;
        m_current = r.m_current;

        //
        // m_pocoliter is an iterator into *our* m_pocol
        // the default op=() will just copy the iterator into
        // r.pocol.
        //
        {
            POCol::const_iterator b = r.m_pocol.begin();
            POCol::const_iterator i = r.m_pocoliter;
            int d = std::distance( b, i );
            m_pocoliter = m_pocol.begin();
            advance( m_pocoliter, d );
        }
        
    }
    
    return *this;
}



/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


PD_RDFSemanticItem::PD_RDFSemanticItem( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
    : m_rdf(rdf)
    , m_context( PD_DocumentRDF::getManifestURI() )
{
    m_name = bindingAsString( it, "name" );
}

PD_RDFSemanticItem::~PD_RDFSemanticItem()
{
}

std::string
PD_RDFSemanticItem::name() const
{
    return m_name;
}

void
PD_RDFSemanticItem::setName( const std::string& n )
{
    m_name = n;
}

std::string
PD_RDFSemanticItem::getDisplayLabel() const
{
    return "Semantic Item";
}




std::list< std::pair< std::string, std::string> >
PD_RDFSemanticItem::getImportTypes() const
{
    std::list< std::pair< std::string, std::string> > ret;
    return ret;
}



std::list< std::pair< std::string, std::string> >
PD_RDFSemanticItem::getExportTypes() const
{
    std::list< std::pair< std::string, std::string> > ret;
    return ret;
}

std::string
PD_RDFSemanticItem::getDefaultExtension() const
{
    return ".unknown";
}



PD_DocumentRDFHandle
PD_RDFSemanticItem::getRDF() const
{
    return m_rdf;
}

PD_DocumentRDFMutationHandle
PD_RDFSemanticItem::createMutation()
{
    return m_rdf->createMutation();
}

std::string
PD_RDFSemanticItem::requestExportFileNameByDialog()
{
    std::string ret = getExportToFileName( "", getDefaultExtension(), getExportTypes() );
    return ret;
}



PD_URI
PD_RDFSemanticItem::linkingSubject() const
{
    return m_linkingSubject;
}


PD_URI
PD_RDFSemanticItem::context() const
{
    return m_context;
}

std::set< std::string >
PD_RDFSemanticItem::getXMLIDs() const
{
    std::set< std::string > ret;

    PD_URI linksubj = linkingSubject();
    PD_ObjectList ol = m_rdf->getObjects( linksubj,
                                          PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"));
    for( PD_ObjectList::iterator it = ol.begin(); it != ol.end(); ++it )
    {
        std::string xmlid = it->toString();
        ret.insert(xmlid);
    }

    return ret;
}



void
PD_RDFSemanticItem::updateFromEditorData()
{
    PD_DocumentRDFMutationHandle m = m_rdf->createMutation();
    updateFromEditorData( m );
    m->commit();
}




#include "xap_Frame.h"
#include "fv_View.h"

std::pair< PT_DocPosition, PT_DocPosition >
PD_RDFSemanticItem::insertTextWithXMLID( const std::string& textconst,
                                         const std::string& xmlid )
{
    PT_DocPosition startpos = 0, endpos = 0;
    XAP_Frame* lff = XAP_App::getApp()->getLastFocussedFrame();
    if(lff) 
    {
        FV_View * pView = static_cast<FV_View*>( lff->getCurrentView() );
    
        std::string text = " " + textconst + " ";
        startpos = pView->getPoint();
        m_rdf->getDocument()->insertSpan( startpos, text );
        endpos = pView->getPoint();
        startpos++;
        endpos--;
    
        pView->selectRange( startpos, endpos );
        pView->cmdInsertXMLID( xmlid );
    }

    return std::make_pair( startpos, endpos );    
}




std::string
PD_RDFSemanticItem::bindingAsString( PD_ResultBindings_t::iterator& it, const std::string k )
{
    return (*it)[k];
}

std::string
PD_RDFSemanticItem::optionalBindingAsString( PD_ResultBindings_t::iterator& it, const std::string k )
{
    std::map< std::string, std::string >& m = *it;
    if( m.end() == m.find(k) || m[k] == "NULL" )
        return "";
    return m[k];
}


PD_URI&
PD_RDFSemanticItem::handleSubjectDefaultArgument( PD_URI& subj )
{
    if( subj.toString().empty() )
    {
        subj = linkingSubject();
    }
    return subj;
}


void
PD_RDFSemanticItem::setRDFType(PD_DocumentRDFMutationHandle m, const std::string& type, PD_URI subj )
{
    handleSubjectDefaultArgument( subj );

    std::string t = type;
    updateTriple( m, t, type, PD_URI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type") );  
    
//    PD_URI pred("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
//    m->add( subj, pred, PD_Object(type), context() );
}

void
PD_RDFSemanticItem::setRDFType(const std::string& type, PD_URI subj )
{
    PD_DocumentRDFMutationHandle m = createMutation();
    setRDFType( m, type, subj );
    m->commit();
}

void
PD_RDFSemanticItem::updateTriple( PD_DocumentRDFMutationHandle m, std::string& toModify, const std::string& newValue, const PD_URI& predString )
{
    m->remove( linkingSubject(), PD_URI( predString ) );
    updateTriple_remove( m, toModify, predString, linkingSubject() );
    toModify = newValue;
    updateTriple_add( m, toModify, predString, linkingSubject() );
}

void
PD_RDFSemanticItem::updateTriple( PD_DocumentRDFMutationHandle m, time_t& toModify, time_t newValue, const PD_URI& predString )
{
    m->remove( linkingSubject(), PD_URI( predString ) );
    updateTriple_remove( m, PD_URI(tostr(toModify)), predString, linkingSubject() );
    toModify = newValue;
    updateTriple_add( m, PD_URI(toTimeString(toModify)), predString, linkingSubject() );
}

void
PD_RDFSemanticItem::updateTriple( PD_DocumentRDFMutationHandle m, double& toModify, double newValue, const PD_URI& predString )
{
    m->remove( linkingSubject(), PD_URI( predString ) );
    //
    // I abstracted the below code to an ::remove() method in mutation
    // {
    //     PD_URI pred( predString );
    //     PD_ObjectList objects = m_rdf->getObjects( linkingSubject(), pred );
    //     std::list< PD_RDFStatement > removeList;
    //     for( PD_ObjectList::iterator it = objects.begin(); it != objects.end(); ++it )
    //     {
    //         PD_Object obj = *it;
    //         PD_RDFStatement s( linkingSubject(), pred, obj );
        
    //         removeList.push_back( s );
    //     }
    //     m->remove( removeList );    
    // }
    updateTriple_remove( m, PD_URI(tostr(toModify)), predString, linkingSubject() );
    toModify = newValue;
    updateTriple_add( m, PD_URI(tostr(toModify)), predString, linkingSubject() );
}

void
PD_RDFSemanticItem::updateTriple( PD_DocumentRDFMutationHandle m,
                                  double& toModify, double newValue,
                                  const PD_URI& predString,
                                  PD_URI linkingSubj )
{
    updateTriple_remove( m, PD_URI(tostr(toModify)), predString, linkingSubj );
    toModify = newValue;
    updateTriple_add( m, PD_URI(tostr(toModify)), predString, linkingSubj );
}

void
PD_RDFSemanticItem::updateTriple( std::string& toModify, const std::string& newValue, const PD_URI& predString )
{
    PD_DocumentRDFMutationHandle m = createMutation();
    updateTriple( m, toModify, newValue, predString );
    m->commit();
}

void
PD_RDFSemanticItem::updateTriple( time_t&      toModify, time_t newValue, const PD_URI& predString )
{
    PD_DocumentRDFMutationHandle m = createMutation();
    updateTriple( m, toModify, newValue, predString );
    m->commit();
}

void
PD_RDFSemanticItem::updateTriple( double&      toModify, double newValue, const PD_URI& predString )
{
    PD_DocumentRDFMutationHandle m = createMutation();
    updateTriple( m, toModify, newValue, predString );
    m->commit();
}

void PD_RDFSemanticItem::updateTriple_remove( PD_DocumentRDFMutationHandle m,
                                              const PD_URI& toModify,
                                              const PD_URI& predString,
                                              const PD_URI& explicitLinkingSubject )
{
    PD_URI pred( predString );
    m->remove( explicitLinkingSubject, pred, PD_Literal(toModify.toString()) );

    //
    // Typeless remove, I found that if a object literal did not
    // stipulate its type in the input RDF, just using
    // remove() above might not pick it up. So the below code
    // looks through all statements with subj+pred and checks typeless
    // string identity of the object() and removes it if strings match.
    //
    PD_ObjectList objects = m_rdf->getObjects( explicitLinkingSubject, pred );
    std::list< PD_RDFStatement > removeList;
    for( PD_ObjectList::iterator it = objects.begin(); it != objects.end(); ++it )
    {
        PD_Object obj = *it;
        PD_RDFStatement s( explicitLinkingSubject, pred, obj );
        
        if( obj.toString() == toModify.toString())
        {
            removeList.push_back( s );
        }

        // //
        // // Sometimes the object value is serialized as 51.47026 or
        // // 5.1470260000e+01. There are also slight rounding errors
        // // which are introduced that complicate comparisons.
        // //
        // if (toModify.isDouble())
        // {
        //     removeList.push_back( s );
        // }
    }
    
    m->remove( removeList );    
}


void PD_RDFSemanticItem::updateTriple_add( PD_DocumentRDFMutationHandle m,
                                           const PD_URI& toModify,
                                           const PD_URI& predString,
                                           const PD_URI& explicitLinkingSubject )
{
    if( toModify.empty() )
        return;
    PD_URI pred( predString );
    m->add( explicitLinkingSubject, pred, PD_Literal(toModify.toString()), context() ); 
}



PD_URI
PD_RDFSemanticItem::createUUIDNode()
{
    std::string uuid = XAP_App::getApp()->createUUIDString();
    return PD_URI( uuid );
    
}

void
PD_RDFSemanticItem::importFromFile( const std::string& filename_const )
{
    std::string filename = getImportFromFileName( filename_const, getImportTypes() );
    std::ifstream iss( filename.c_str() );
    importFromData( iss, m_rdf );
}

PD_RDFSemanticItemHandle
PD_RDFSemanticItem::createSemanticItem( PD_DocumentRDFHandle rdf, const std::string& semanticClass )
{
    PD_ResultBindings_t b;
    b.push_back( std::map< std::string, std::string >() );
    PD_ResultBindings_t::iterator it = b.begin();
    return createSemanticItem( rdf, it, semanticClass );
}

PD_RDFSemanticItemHandle
PD_RDFSemanticItem::createSemanticItem( PD_DocumentRDFHandle rdf,
                                        PD_ResultBindings_t::iterator it,
                                        const std::string& semanticClass )
{
    if (semanticClass == "Contact")
    {
        return PD_RDFSemanticItemHandle( PD_DocumentRDF::getSemanticItemFactory()->createContact( rdf, it ) );
    }
    if (semanticClass == "Event")
    {
        return PD_RDFSemanticItemHandle( PD_DocumentRDF::getSemanticItemFactory()->createEvent( rdf, it ));
    }
#ifdef WITH_CHAMPLAIN
    if (semanticClass == "Location")
    {
        return PD_RDFSemanticItemHandle( PD_DocumentRDF::getSemanticItemFactory()->createLocation( rdf, it ));
    }
#endif
    return PD_RDFSemanticItemHandle();
}

void
PD_RDFSemanticItem::showEditorWindow( const PD_RDFSemanticItems& cl )
{
	UT_DEBUG_ONLY_ARG(cl);

    UT_DEBUGMSG(("showEditorWindow(base) list... sz:%lu\n", (long unsigned)cl.size() ));
}


void
PD_RDFSemanticItem::showEditorWindow( const PD_RDFSemanticItemHandle& c )
{
	UT_DEBUG_ONLY_ARG(c);

    UT_DEBUGMSG(("showEditorWindow(base) name:%s linksubj:%s\n",
                 c->name().c_str(), c->linkingSubject().toString().c_str() ));
}

void
PD_RDFSemanticItem::importFromDataComplete( std::istream& /*iss*/,
                                            PD_DocumentRDFHandle /*rdf*/,
                                            PD_DocumentRDFMutationHandle /*m*/,
                                            PD_DocumentRange* /*pDocRange*/ )
{
    UT_DEBUGMSG(("importFromDataComplete(base)\n"));
}

std::string
PD_RDFSemanticItem::getImportFromFileName( const std::string& /*filename_const*/,
                                           std::list< std::pair< std::string, std::string> > /*types*/ ) const
{
    return "";
}
std::string
PD_RDFSemanticItem::getExportToFileName( const std::string& /*filename_const*/,
                                         std::string /*defaultExtension*/,
                                         std::list< std::pair< std::string, std::string> > /*types*/ ) const
{
    return "";
}

/***********/
/***********/
/***********/

PD_RDFContact::PD_RDFContact( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
    : PD_RDFSemanticItem( rdf, it )
{
    m_linkingSubject = PD_URI( bindingAsString( it, "person") );
    m_nick     = optionalBindingAsString( it, "nick");
    m_email    = optionalBindingAsString( it, "email");
    m_homePage = optionalBindingAsString( it, "homepage");
    m_imageUrl = optionalBindingAsString( it, "img");
    m_phone    = optionalBindingAsString( it, "phone");
    m_jabberID = optionalBindingAsString( it, "jabberid");
}


PD_RDFContact::~PD_RDFContact()
{
}


std::string
PD_RDFContact::getDisplayLabel() const
{
    return "Contact";
}

void
PD_RDFContact::setupStylesheetReplacementMapping( std::map< std::string, std::string >& m )
{
    m["%NICK%"]     = m_nick;
    m["%HOMEPAGE%"] = m_homePage;
    m["%PHONE%"]    = m_phone;
    m["%EMAIL%"]    = m_email;
}


PD_RDFSemanticStylesheets
PD_RDFContact::stylesheets() const
{
    PD_RDFSemanticStylesheets ret;
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("143c1ba3-d7bb-440b-8528-7f07d2eff5f2", RDF_SEMANTIC_STYLESHEET_CONTACT_NAME, "%NAME%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("2fad34d1-42a0-4b10-b17e-a87db5208f6d", RDF_SEMANTIC_STYLESHEET_CONTACT_NICK, "%NICK%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("0dd5878d-95c5-47e5-a777-63ec36da3b9a", RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_PHONE, "%NAME%, %PHONE%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("9cbeb4a6-34c5-49b2-b3ef-b94277db0c59", RDF_SEMANTIC_STYLESHEET_CONTACT_NICK_PHONE, "%NICK%, %PHONE%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("47025a4a-5da5-4a32-8d89-14c03658631d", RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_HOMEPAGE_PHONE, "%NAME%, (%HOMEPAGE%), %PHONE%")));
    return ret;
}

std::string
PD_RDFContact::className() const
{
    return "Contact";
}




#ifdef WITH_EVOLUTION_DATA_SERVER
extern "C" {
  #include <libebook/libebook.h>
};

static std::string get( EVCard* c, const char* v )
{
    EVCardAttribute* a = e_vcard_get_attribute( c, v );
    UT_DEBUGMSG((" cvard.group:%s v:%s\n", e_vcard_attribute_get_group( a ), v ));

    if( a && e_vcard_attribute_is_single_valued(a) )
    {
        return e_vcard_attribute_get_value(a);
    }
    return "";
}

static void set( EVCard* c, const char* k, const std::string& v )
{
    EVCardAttribute* a = e_vcard_get_attribute( c, k );
    if( !a )
    {
        a = e_vcard_attribute_new( 0, k );
        e_vcard_append_attribute( c, a );
    }
    
    if( a )
    {
        e_vcard_attribute_add_value( a, v.c_str() );
    }
}
#endif

std::list< std::pair< std::string, std::string> >
PD_RDFContact::getImportTypes() const
{
    std::list< std::pair< std::string, std::string> > types;
    types.push_back( std::make_pair( "VCard File", "vcf" ));
    return types;
}

void
PD_RDFContact::importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange )
{
#ifdef WITH_EVOLUTION_DATA_SERVER

    std::string vcard = StreamToString( iss );
    UT_DEBUGMSG(("trying to get card for data:%s\n", vcard.c_str() ));
    if( EVCard* c = e_vcard_new_from_string( vcard.c_str() ) )
    {
        std::string textrep = "";
        typedef std::list< const char* > charplist_t;
        charplist_t textreplist;
        textreplist.push_back( EVC_EMAIL );
        textreplist.push_back( EVC_FN );
        textreplist.push_back( EVC_NICKNAME );
        textreplist.push_back( EVC_UID );
        for( charplist_t::iterator iter = textreplist.begin();
             iter != textreplist.end(); ++iter )
        {
            textrep = get( c, *iter );
            if( !textrep.empty() )
                break;
        }
        UT_DEBUGMSG(("have card!\n"));

        std::string fn    = get( c, EVC_FN );
        std::string uid   = get( c, EVC_UID );
        std::string xmlid = rdf->makeLegalXMLID( fn + "_" + uid );
        std::string email = get( c, EVC_EMAIL );
        UT_DEBUGMSG(("uid:%s xmlid:%s\n", uid.c_str(), xmlid.c_str() ));

        m_name  = fn;
        m_nick  = get( c, EVC_NICKNAME );
        m_email = email;
        m_phone = get( c, EVC_TEL );
        m_jabberID = get( c, EVC_X_JABBER );
        
        // std::pair< PT_DocPosition, PT_DocPosition > se = insertTextWithXMLID( textrep, xmlid );
        // PT_DocPosition startpos = se.first;
        // PT_DocPosition   endpos = se.second;

        std::string uuid = "http://abicollab.net/rdf/foaf#" + uid;
        m_linkingSubject = PD_URI( uuid );
        XAP_Frame* lff = XAP_App::getApp()->getLastFocussedFrame();
        if(lff) 
        {
//            FV_View * pView = static_cast<FV_View*>( lff->getCurrentView() );
//            std::pair< PT_DocPosition, PT_DocPosition > se = insert( pView );
//            PT_DocPosition startpos = se.first;
//            PT_DocPosition   endpos = se.second;
        }
        
        // PD_DocumentRDFMutationHandle m = rdf->createMutation();
        // m->add( PD_URI(uuid),
        //         PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"),
        //         PD_Literal( xmlid ) );
        // // m->add( PD_URI(uuid),
        // //         PD_URI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"),
        // //         PD_Object("http://xmlns.com/foaf/0.1/Person") );
        // // addFoafProp( m, c, EVC_TEL,      uuidnode, "phone" );
        // // addFoafProp( m, c, EVC_NICKNAME, uuidnode, "nick" );
        // // addFoafProp( m, c, EVC_FN,       uuidnode, "name" );
        // // addFoafProp( m, c, EVC_N,        uuidnode, "givenName" );
        // // addFoafProp( m, c, EVC_X_JABBER, uuidnode, "jabberID" );
            
        PD_DocumentRDFMutationHandle m = rdf->createMutation();
        importFromDataComplete( iss, rdf, m, pDocRange );
        m->commit();
    }
#else
	UT_UNUSED(iss);
	UT_UNUSED(rdf);
	UT_UNUSED(pDocRange);
#endif
}



std::list< std::pair< std::string, std::string> >
PD_RDFContact::getExportTypes() const
{
    std::list< std::pair< std::string, std::string> > ret;
    ret.push_back( std::make_pair( "VCard File", "vcf" ));
    return ret;
}

std::string
PD_RDFContact::getDefaultExtension() const
{
    return ".vcf";
}


void
PD_RDFContact::exportToFile( const std::string& filename_const ) const
{
    std::string filename = getExportToFileName( filename_const,
                                                ".vcf",
                                                getExportTypes() );

    UT_DEBUGMSG(( "saving vcard to file:%s\n", filename.c_str() ));

#ifdef WITH_EVOLUTION_DATA_SERVER

    if( EVCard* c = e_vcard_new() )
    {
        set( c, EVC_FN,    m_name );
        set( c, EVC_UID,   m_linkingSubject.toString() );
        set( c, EVC_EMAIL, m_email );
        set( c, EVC_NICKNAME, m_nick );
        set( c, EVC_TEL,      m_phone );
        set( c, EVC_X_JABBER, m_jabberID );

        gchar* data =  e_vcard_to_string( c, EVC_FORMAT_VCARD_30 );
        UT_DEBUGMSG(( "saving vcard to file:%s vcard.len:%ld\n", filename.c_str(), strlen(data) ));
        std::ofstream oss( filename.c_str() );
        oss.write( data, strlen(data) );
        oss.flush();
        oss.close();
        g_free(data);
    }
    
#endif    
}

/******************************/
/******************************/
/******************************/


#ifdef WITH_LIBICAL
extern "C" {
  #include <libical/ical.h>
};


#if 0
// static std::string tostr( time_t v )
// {
//     std::stringstream ss;
//     ss << v;
//     return ss.str();
// }


static void addCalProp( PD_DocumentRDFMutationHandle m,
                        const PD_URI& uuidnode,
                        const std::string& predend,
                        const std::string& value )
{
    std::string predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    m->add( uuidnode,
            PD_URI(predBase + predend),
            PD_Literal( value ) );
}
static void addCalPropSZ( PD_DocumentRDFMutationHandle m,
                          const PD_URI& uuidnode,
                          const std::string& predend,
                          const char* value )
{
    std::string predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    if( value )
    {
        addCalProp( m, uuidnode, predend, (std::string)value );
    }
}

#endif
#endif


PD_RDFEvent::PD_RDFEvent( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
    : PD_RDFSemanticItem( rdf, it )
{
    m_linkingSubject = PD_URI( bindingAsString( it, "ev") );
    m_summary  = optionalBindingAsString( it, "summary");
    m_location = optionalBindingAsString( it, "location");
    m_uid      = optionalBindingAsString( it, "uid");
    m_desc     = optionalBindingAsString( it, "description");
    m_dtstart  = parseTimeString(optionalBindingAsString( it, "dtstart"));
    m_dtend    = parseTimeString(optionalBindingAsString( it, "dtend"));
    if( m_name.empty() )
        m_name = m_uid;
}
 
PD_RDFEvent::~PD_RDFEvent()
{
}

void
PD_RDFEvent::setupStylesheetReplacementMapping( std::map< std::string, std::string >& m )
{
    m["%UID%"]      = m_uid;
    m["%DESCRIPTION%"] = m_desc;
    m["%DESC%"]     = m_desc;
    m["%SUMMARY%"]  = m_summary;
    m["%LOCATION%"] = m_location;
    m["%START%"]    = toTimeString(m_dtstart);
    m["%END%"]      = toTimeString(m_dtend);
}

PD_RDFSemanticStylesheets
PD_RDFEvent::stylesheets() const
{
    PD_RDFSemanticStylesheets ret;
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("92f5d6c5-2c3a-4988-9646-2f29f3731f89",
                                         RDF_SEMANTIC_STYLESHEET_EVENT_NAME, "%NAME%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("b4817ce4-d2c3-4ed3-bc5a-601010b33363",
                                         RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY, "%SUMMARY%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("853242eb-031c-4a36-abb2-7ef1881c777e",
                                         RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION, "%SUMMARY%, %LOCATION%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("2d6b87a8-23be-4b61-a881-876177812ad4",
                                         RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION_TIMES, "%SUMMARY%, %LOCATION%, %START%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("115e3ceb-6bc8-445c-a932-baee09686895",
                                         RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_TIMES, "%SUMMARY%, %START%")));
    return ret;
}

std::string
PD_RDFEvent::className() const
{
    return "Event";
}

std::list< std::pair< std::string, std::string> >
PD_RDFEvent::getImportTypes() const
{
    std::list< std::pair< std::string, std::string> > types;
    types.push_back( std::make_pair( "ICalendar files", "ics" ));
    return types;
}

std::list< std::pair< std::string, std::string> >
PD_RDFEvent::getExportTypes() const
{
    std::list< std::pair< std::string, std::string> > types;
    types.push_back( std::make_pair( "ICalendar files", "ics" ));
    return types;
}

std::string
PD_RDFEvent::getDisplayLabel() const
{
    return "Event";
}


#ifdef WITH_LIBICAL
static void setFromString( std::string& s, const char* input )
{
    if( input )
        s = input;
    else
        s = "";
}
#endif


void
PD_RDFEvent::importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange )
{
#ifdef WITH_LIBICAL

    std::string data = StreamToString(iss);
    UT_DEBUGMSG(("trying to get calendar for data:%s\n", data.c_str() ));

    if( icalcomponent* c = icalcomponent_new_from_string( data.c_str() ) )
    {
        const char* zdesc = icalcomponent_get_description( c );
        const char* zloc  = icalcomponent_get_location( c );
        const char* zsummary = icalcomponent_get_summary( c );
        const char* zuid  = icalcomponent_get_uid( c );
        struct icaltimetype zdtstart = icalcomponent_get_dtstart( c );
        struct icaltimetype zdtend   = icalcomponent_get_dtend( c );

        std::string xmlid;
        if( zsummary )
            xmlid += (std::string)"" + zsummary + "_";
        if( zuid )
            xmlid += zuid;

        xmlid = rdf->makeLegalXMLID( xmlid );

        setFromString( m_desc, zdesc );
        setFromString( m_location, zloc );
        setFromString( m_summary, zsummary );
        setFromString( m_uid, zuid );
        m_name    = m_uid;
        m_dtstart = icaltime_as_timet( zdtstart );
        m_dtend   = icaltime_as_timet( zdtend );
        
        
        std::string uuid = "http://abicollab.net/rdf/cal#" + xmlid;
        m_linkingSubject = PD_URI( uuid );
        XAP_Frame* lff = XAP_App::getApp()->getLastFocussedFrame();
        if(lff) 
        {
//            FV_View * pView = static_cast<FV_View*>( lff->getCurrentView() );
//            std::pair< PT_DocPosition, PT_DocPosition > se = insert( pView );
//            PT_DocPosition startpos = se.first;
//            PT_DocPosition   endpos = se.second;
        }

        PD_DocumentRDFMutationHandle m = rdf->createMutation();
        importFromDataComplete( iss, rdf, m, pDocRange );
        m->commit();
    }
#else
	UT_UNUSED(iss);
	UT_UNUSED(rdf);
	UT_UNUSED(pDocRange);
#endif 
}

std::string
PD_RDFEvent::getDefaultExtension() const
{
    return ".ical";
}

void
PD_RDFEvent::exportToFile( const std::string& filename_const ) const
{
    std::string filename = getExportToFileName( filename_const,
                                                ".ical",
                                                getExportTypes() );

    UT_DEBUGMSG(( "saving ical to file:%s\n", filename.c_str() ));

#ifdef WITH_LIBICAL

    if( icalcomponent* c = icalcomponent_new( ICAL_VEVENT_COMPONENT ) )
    {
        icalcomponent_set_uid( c,         m_uid.c_str() );
        icalcomponent_set_location( c,    m_location.c_str() );
        icalcomponent_set_description( c, m_desc.c_str() );
        icalcomponent_set_dtstart( c,     icaltime_from_timet( m_dtstart, 0 ) );
        icalcomponent_set_dtend( c,       icaltime_from_timet( m_dtend, 0 ) );

        char* data = icalcomponent_as_ical_string( c );
        std::ofstream oss( filename.c_str() );
        oss.write( data, strlen(data) );
        oss.flush();
        oss.close();
    }
#endif 
    
}


/******************************/
/******************************/
/******************************/





std::list< std::pair< std::string, std::string> >
PD_RDFLocation::getImportTypes() const
{
    std::list< std::pair< std::string, std::string> > types;
    types.push_back( std::make_pair( "KML files", "kml" ));
    return types;
}

std::list< std::pair< std::string, std::string> >
PD_RDFLocation::getExportTypes() const
{
    std::list< std::pair< std::string, std::string> > types;
    types.push_back( std::make_pair( "KML files", "kml" ));
    return types;
}

PD_RDFLocation::PD_RDFLocation( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it, bool isGeo84 )
    : PD_RDFSemanticItem( rdf, it )
    , m_isGeo84( isGeo84 )
{
    m_linkingSubject = PD_URI( bindingAsString( it, "geo") );

    m_name     = optionalBindingAsString( it, "name");
    m_uid      = optionalBindingAsString( it, "uid");
    m_desc     = optionalBindingAsString( it, "desc");
    m_dlat     = toType<double>(optionalBindingAsString( it, "lat"));
    m_dlong    = toType<double>(optionalBindingAsString( it, "long"));
    m_joiner = PD_Object(optionalBindingAsString( it, "joiner"));
    if( m_name.empty() )
        m_name = m_uid;
    if( m_name.empty() )
    {
        m_name = tostr(m_dlat) + "_" + tostr(m_dlong);
        if( m_uid.empty() )
            m_uid = m_name;
    }
    
    UT_DEBUGMSG(("PD_RDFLocation() name:%s long:%f lat:%f geo84:%d\n", m_name.c_str(), m_dlat, m_dlong, isGeo84 ));
}

PD_RDFLocation::~PD_RDFLocation()
{
}

void
PD_RDFLocation::setupStylesheetReplacementMapping( std::map< std::string, std::string >& m )
{
    m["%UID%"]       = m_uid;
    m["%DESCRIPTION%"] = m_desc;
    m["%DESC%"]      = m_desc;
    m["%LAT%"]       = tostr(m_dlat);
    m["%LONG%"]      = tostr(m_dlong);
    m["%DLAT%"]      = tostr(m_dlat);
    m["%DLONG%"]     = tostr(m_dlong);
}

PD_RDFSemanticStylesheets
PD_RDFLocation::stylesheets() const
{
    PD_RDFSemanticStylesheets ret;
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("33314909-7439-4aa1-9a55-116bb67365f0", RDF_SEMANTIC_STYLESHEET_LOCATION_NAME, "%NAME%")));
    ret.push_back(
        PD_RDFSemanticStylesheetHandle(
            new PD_RDFSemanticStylesheet("34584133-52b0-449f-8b7b-7f1ef5097b9a",
                                         RDF_SEMANTIC_STYLESHEET_LOCATION_NAME_LATLONG,
                                         "%NAME%, %DLAT%, %DLONG%")));
    return ret;
}

std::string
PD_RDFLocation::className() const
{
    return "Location";
}


std::string
PD_RDFLocation::getDisplayLabel() const
{
    return "Location";
}

void
PD_RDFLocation::importFromData( std::istream& /*iss*/, 
								PD_DocumentRDFHandle /*rdf*/, 
								PD_DocumentRange * /*pDocRange*/ )
{
    UT_DEBUGMSG(( "FIXME: importFromData()\n" ));
}

std::string
PD_RDFLocation::getDefaultExtension() const
{
    return ".kml";
}

void
PD_RDFLocation::exportToFile( const std::string& filename_const ) const
{
    std::string filename = getExportToFileName( filename_const, ".kml", getExportTypes() );
    UT_DEBUGMSG(( "Saving KML to file:%s\n", filename.c_str() ));


    std::ofstream xmlss( filename.c_str() );
    xmlss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
        << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" > \n"
        << " \n"
        << "<Placemark> \n"
        << "  <name>" << name() << "</name> \n"
        << "  <LookAt> \n"
        << "    <longitude>" << m_dlong << "</longitude> \n"
        << "    <latitude>" << m_dlat << "</latitude> \n"
        << "  </LookAt> \n"
        << "</Placemark> \n"
        << "</kml>\n";
    xmlss.flush();
    xmlss.close();
}

std::set< std::string >
PD_RDFLocation::getXMLIDs() const
{
    std::set< std::string > ret;
    
    std::stringstream ss;
    ss << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  " << std::endl
       << "prefix foaf: <http://xmlns.com/foaf/0.1/> " << std::endl
       << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> " << std::endl
       << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#>" << std::endl
       << "" << std::endl
       << "select distinct ?s ?p ?o ?xmlid" << std::endl
       << "where { " << std::endl
       << " ?s pkg:idref ?xmlid ." << std::endl
       << " ?s ?p ?o " << std::endl
       << " . filter( str(?o) = \"" << m_linkingSubject.toString() << "\" )" << std::endl
       << "}" << std::endl;
    std::set<std::string> uniqfilter;
    PD_RDFQuery q( getRDF(), getRDF() );
    PD_ResultBindings_t bindings = q.executeQuery( ss.str() );
    for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
    {
        std::map< std::string, std::string > d = *iter;
        std::string xmlid = d["xmlid"];
        if (uniqfilter.count(xmlid))
            continue;
        uniqfilter.insert(xmlid);
        
        if( !xmlid.empty() )
            ret.insert( xmlid );
        UT_DEBUGMSG(("PD_RDFLocation::getXMLIDs() xmlid:%s\n", xmlid.c_str() ));
    }
    
    return ret;
}



/************************************************************/
/************************************************************/
/************************************************************/




/************************************************************/
/************************************************************/
/************************************************************/


std::pair< PT_DocPosition, PT_DocPosition >
PD_RDFSemanticItem::insert( PD_DocumentRDFMutationHandle m, FV_View* /*pView*/ )
{
    std::string xmlid = m_rdf->makeLegalXMLID( name() );
    std::pair< PT_DocPosition, PT_DocPosition > se = insertTextWithXMLID( name(), xmlid );
    if (m_linkingSubject.toString().empty())
    {
        std::string uuid = XAP_App::getApp()->createUUIDString();
        m_linkingSubject = uuid;
    }
    m->add( m_linkingSubject,
            PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"),
            PD_Literal( xmlid ) );
    return se;
}



std::pair< PT_DocPosition, PT_DocPosition >
PD_RDFSemanticItem::insert( FV_View* pView )
{
    PD_DocumentRDFMutationHandle m = m_rdf->createMutation();
    std::pair< PT_DocPosition, PT_DocPosition > ret = insert( m, pView );
    m->commit();
    return ret;
}




std::list< std::string >
PD_RDFSemanticItem::getClassNames()
{
    std::list< std::string > ret;
    ret.push_back( "Contact" );
    ret.push_back( "Event" );
    ret.push_back( "Location" );
    return ret;
}

void
PD_RDFSemanticItem::setupStylesheetReplacementMapping( std::map< std::string, std::string >& /*m*/ )
{
}


PD_RDFSemanticStylesheetHandle
PD_RDFSemanticItem::findStylesheetByUuid(const std::string &uuid) const
{
    PD_RDFSemanticStylesheetHandle ret;
    if( uuid.empty() )
        return ret;

    PD_RDFSemanticStylesheets ssl = stylesheets();
    for( PD_RDFSemanticStylesheets::iterator iter = ssl.begin(); iter != ssl.end(); ++iter )
    {
        PD_RDFSemanticStylesheetHandle ss = *iter;
        if ( ss->uuid() == uuid )
        {
            return ss;
        }
    }
    return ret;
}


PD_RDFSemanticStylesheetHandle
PD_RDFSemanticItem::findStylesheetByName(const std::string & /*sheetType*/, const std::string &n) const
{
    return findStylesheetByName( stylesheets(), n );
}


PD_RDFSemanticStylesheetHandle
PD_RDFSemanticItem::findStylesheetByName(const PD_RDFSemanticStylesheets& ssl, const std::string &n) const
{
    PD_RDFSemanticStylesheetHandle ret;
    if( n.empty() )
        return ret;
    for( PD_RDFSemanticStylesheets::const_iterator iter = ssl.begin(); iter != ssl.end(); ++iter )
    {
        PD_RDFSemanticStylesheetHandle ss = *iter;
        if ( ss->name() == n )
        {
            return ss;
        }
    }
    return ret;
    
}

std::string
PD_RDFSemanticItem::getProperty( std::string subj, std::string pred, std::string defVal ) const
{
    PD_Object o = m_rdf->getObject( subj, pred );
    if( o.empty() )
        return defVal;
    return o.toString();
    
}


PD_RDFSemanticStylesheetHandle
PD_RDFSemanticItem::defaultStylesheet() const
{
    std::string semanticClass = className();
    std::string name_ = getProperty( "http://calligra-suite.org/rdf/document/" + semanticClass,
                                    "http://calligra-suite.org/rdf/stylesheet",
                                    RDF_SEMANTIC_STYLESHEET_NAME );
    std::string type = getProperty( "http://calligra-suite.org/rdf/document/" + semanticClass,
                                    "http://calligra-suite.org/rdf/stylesheet-type",
                                    PD_RDFSemanticStylesheet::stylesheetTypeSystem() );
    std::string uuid = getProperty( "http://calligra-suite.org/rdf/document/" + semanticClass,
                                    "http://calligra-suite.org/rdf/stylesheet-uuid",
                                    "" );
    
    PD_RDFSemanticStylesheetHandle ret = findStylesheetByUuid( uuid );
    if (!ret)
    {
        ret = findStylesheetByName( type, name_ );
    }
    if (!ret)
    {
        // The "name" stylesheet should always exist
        ret = findStylesheetByName( PD_RDFSemanticStylesheet::stylesheetTypeSystem(), RDF_SEMANTIC_STYLESHEET_NAME );
    }
    return ret;
    
}

void
PD_RDFSemanticItem::relationAdd( PD_RDFSemanticItemHandle dst, RelationType rt )
{
    std::string predBase("http://xmlns.com/foaf/0.1/" );
    PD_URI pred( predBase + "knows");

    PD_DocumentRDFMutationHandle m = m_rdf->createMutation();
    switch( rt )
    {
        case RELATION_FOAF_KNOWS:
            // symetric relation
            m->add( linkingSubject(),      pred, PD_Object(dst->linkingSubject()) );
            m->add( dst->linkingSubject(), pred, PD_Object(linkingSubject()) );
            break;
    }
    m->commit();
}

PD_RDFSemanticItems
PD_RDFSemanticItem::relationFind( RelationType rt )
{
    std::string predBase("http://xmlns.com/foaf/0.1/" );
    PD_URI pred( predBase + "knows");

    switch( rt )
    {
        case RELATION_FOAF_KNOWS:
            pred = PD_URI( predBase + "knows");
            break;
    }
    UT_DEBUGMSG(("relationFind() this->linkingSubj:%s\n", linkingSubject().c_str() ));

    std::set< std::string > xmlids;
    PD_ObjectList ol = m_rdf->getObjects( linkingSubject(), pred );
    for( PD_ObjectList::iterator iter = ol.begin(); iter != ol.end(); ++iter )
    {
        PD_URI linkingSubj = *iter;
        UT_DEBUGMSG(("relationFind() linkingSubj:%s\n", linkingSubj.c_str() ));

        std::set< std::string > t = getXMLIDsForLinkingSubject( m_rdf, linkingSubj.toString() );
        UT_DEBUGMSG(("relationFind() t.sz:%ld\n", (long)t.size() ));
        xmlids.insert( t.begin(), t.end() );
    }

    UT_DEBUGMSG(("relationFind() xmlids.sz:%ld\n", (long)xmlids.size() ));
    PD_RDFSemanticItems ret = m_rdf->getSemanticObjects( xmlids );
    return ret;
}

std::set< std::string >
PD_RDFSemanticItem::getXMLIDsForLinkingSubject( PD_DocumentRDFHandle rdf, const std::string& linkingSubj )
{
    std::set< std::string > ret;
    
    std::stringstream ss;
    ss << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  " << std::endl
       << "prefix foaf: <http://xmlns.com/foaf/0.1/> " << std::endl
       << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> " << std::endl
       << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#>" << std::endl
       << "" << std::endl
       << "select distinct ?s ?xmlid" << std::endl
       << "where { " << std::endl
       << " ?s pkg:idref ?xmlid " << std::endl
       << " . filter( str(?s) = \"" << linkingSubj << "\" )" << std::endl
       << "}" << std::endl;
    std::set<std::string> uniqfilter;
    PD_RDFQuery q( rdf, rdf );
    UT_DEBUGMSG(("getXMLIDsForLinkingSubject() sparql:%s\n", ss.str().c_str() ));
    PD_ResultBindings_t bindings = q.executeQuery( ss.str() );
    for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
    {
        std::map< std::string, std::string > d = *iter;
        std::string xmlid = d["xmlid"];
        if (uniqfilter.count(xmlid))
            continue;
        uniqfilter.insert(xmlid);
        if( !xmlid.empty() )
            ret.insert( xmlid );
    }
    return ret;
}


/************************************************************/
/************************************************************/
/************************************************************/

PD_RDFSemanticStylesheet::PD_RDFSemanticStylesheet(
    const std::string &_uuid,
    const std::string &_name,
    const std::string &_templateString,
    const std::string &_type,
    bool _isMutable
    )
    : m_uuid(_uuid)
    , m_name(_name)
    , m_templateString(_templateString)
    , m_type(_type)
    , m_isMutable(_isMutable)
{
}


void
PD_RDFSemanticStylesheet::format( PD_RDFSemanticItemHandle obj, FV_View* pView, const std::string& xmlid_const )
{
    PD_Document* pDoc = pView->getDocument();
    PD_DocumentRDFHandle rdf = obj->getRDF();
    std::string xmlid = xmlid_const;
    if( xmlid.empty() )
    {
        std::set< std::string > tmp;
        rdf->addRelevantIDsForPosition( tmp, pView->getPoint() );
        if( tmp.empty() )
        {
            UT_DEBUGMSG(("ss:format(no xmlid!) obj->name:%s\n", obj->name().c_str() ));
            return;
        }
        xmlid = *(tmp.begin());
    }
    
    UT_DEBUGMSG(("ss:format() obj->name:%s\n", obj->name().c_str() ));
    UT_DEBUGMSG(("xmlid:%s pView:%p sheetname:%s\n", xmlid.c_str(), pView, name().c_str() ));

    std::pair< PT_DocPosition, PT_DocPosition > p = rdf->getIDRange( xmlid );
    PT_DocPosition startpos = p.first + 1;
    PT_DocPosition endpos   = p.second;
    if (!endpos)
    {
        UT_DEBUGMSG(("ss:format(invalid range!) obj->name:%s\n", obj->name().c_str() ));
        return;
    }
    UT_DEBUGMSG(("formating start:%d end:%d\n", startpos, endpos ));

    // Grab the text that was there and remove it.
    pView->selectRange( startpos, endpos );
    pView->cmdCut();
    pView->setPoint( startpos );

    std::string data = templateString();
    std::map< std::string, std::string > m;
    m["%NAME%"] = obj->name();
    obj->setupStylesheetReplacementMapping( m );

    for( std::map< std::string, std::string >::iterator mi = m.begin(); mi != m.end(); ++mi)
    {
        std::string k = mi->first;
        std::string v = mi->second;
        data = replace_all( data, k, v );
    }

    // make sure there is something in the replacement other than commas and spaces
    std::string tmpstring = data;
    tmpstring = replace_all( tmpstring, " ", "" );
    tmpstring = replace_all( tmpstring, ",", "" );
    if (tmpstring.empty()) {
        UT_DEBUGMSG(("stylesheet results in empty data, using name() instead\n"));
        data = name();
    }

    UT_DEBUGMSG(("Updating with new formatting:%s\n", data.c_str() ));
    pDoc->insertSpan( startpos, data );
    pView->setPoint( startpos );
}


PD_RDFSemanticStylesheet::~PD_RDFSemanticStylesheet()
{
}

std::string
PD_RDFSemanticStylesheet::stylesheetTypeSystem()
{
    return "System";
}

std::string
PD_RDFSemanticStylesheet::stylesheetTypeUser()
{
    return "User";
}


std::string
PD_RDFSemanticStylesheet::uuid() const
{
    return m_uuid;
}

std::string
PD_RDFSemanticStylesheet::name() const
{
    return m_name;
}

std::string
PD_RDFSemanticStylesheet::templateString() const
{
    return m_templateString;
}

std::string
PD_RDFSemanticStylesheet::type() const
{
    return m_type;
}

bool
PD_RDFSemanticStylesheet::isMutable() const
{
    return m_isMutable;
}


/************************************************************/
/************************************************************/
/************************************************************/

PD_RDFSemanticItemViewSite::PD_RDFSemanticItemViewSite( PD_RDFSemanticItemHandle _si, const std::string& _xmlid )
    : m_xmlid(_xmlid)
    , m_semItem(_si)
{
}

PD_RDFSemanticItemViewSite::PD_RDFSemanticItemViewSite( PD_RDFSemanticItemHandle _si, PT_DocPosition pos )
    : m_semItem(_si)
{
	std::set< std::string > posxml;
	m_semItem->getRDF()->addRelevantIDsForPosition( posxml, pos );
	std::set< std::string > sixml = m_semItem->getXMLIDs();
    std::set< std::string > tmp;
    std::set_intersection( posxml.begin(), posxml.end(),
                           sixml.begin(),  sixml.end(),
                           inserter( tmp, tmp.end() ));
    if( tmp.empty() )
    {
        // error
    }
    else
    {
        m_xmlid = *(tmp.begin());
    }
}



PD_RDFSemanticItemViewSite::~PD_RDFSemanticItemViewSite()
{
}

PD_RDFSemanticStylesheetHandle
PD_RDFSemanticItemViewSite::stylesheet() const
{
    std::string name = getProperty("stylesheet", RDF_SEMANTIC_STYLESHEET_NAME);
    std::string type = getProperty("stylesheet-type", PD_RDFSemanticStylesheet::stylesheetTypeSystem());
    std::string uuid = getProperty("stylesheet-uuid", "");

    UT_DEBUGMSG(("stylesheet at site, format(), xmlid:%s\n", m_xmlid.c_str() ));
    UT_DEBUGMSG((" sheet:%s type:%s\n", name.c_str(), type.c_str() ));

    PD_RDFSemanticStylesheetHandle ret;
    if (!uuid.empty())
    {
        ret = m_semItem->findStylesheetByUuid( uuid );
    }
    if (!ret)
    {
        ret = m_semItem->findStylesheetByName( type, name );
    }
    if (!ret)
    {
        // safety first, there will always be a default stylesheet
        ret = m_semItem->defaultStylesheet();
    }
    return ret;
}


void
PD_RDFSemanticItemViewSite::disassociateStylesheet()
{
    UT_DEBUGMSG(("stylesheet at site. xmlid:%s\n", m_xmlid.c_str() ));
    setProperty("stylesheet", "");
    setProperty("stylesheet-type", "");
    setProperty("stylesheet-uuid", "");
}

void
PD_RDFSemanticItemViewSite::applyStylesheet( FV_View* pView,
                                             PD_RDFSemanticStylesheetHandle ss )
{
    if( !ss )
    {
        UT_DEBUGMSG(("apply stylesheet at site. format(), xmlid:%s NO SHEET\n", m_xmlid.c_str()));
        return;
    }
    
    
    UT_DEBUGMSG(("apply stylesheet at site. format(), xmlid:%s sheet:%s\n",
                 m_xmlid.c_str(), ss->name().c_str() ));
    setStylesheetWithoutReflow( ss );
    reflowUsingCurrentStylesheet( pView );
}

void
PD_RDFSemanticItemViewSite::setStylesheetWithoutReflow( PD_RDFSemanticStylesheetHandle ss )
{
    // Save the stylesheet property
    UT_DEBUGMSG(("apply stylesheet at site. format(), xmlid:%s sheet:%s\n",
                 m_xmlid.c_str(), ss->name().c_str() ));
    setProperty("stylesheet", ss->name());
    setProperty("stylesheet-type", ss->type());
    setProperty("stylesheet-uuid", ss->uuid());
}

void
PD_RDFSemanticItemViewSite::reflowUsingCurrentStylesheet( FV_View* pView )
{
    PD_RDFSemanticStylesheetHandle ss = stylesheet();
    ss->format( m_semItem, pView, m_xmlid );
}

void
PD_RDFSemanticItemViewSite::select( FV_View* pView )
{
    std::set< std::string > xmlids;
    xmlids.insert( m_xmlid );
    m_semItem->getRDF()->selectXMLIDs( xmlids, pView );
}

PD_URI
PD_RDFSemanticItemViewSite::linkingSubject() const
{
    PD_DocumentRDFHandle rdf = m_semItem->getRDF();
    PD_URI     pred("http://calligra-suite.org/rdf/site/package/common#idref");
    PD_Literal obj(m_xmlid);
    
    // try to find it if it already exists
    PD_URIList ul = rdf->getSubjects( pred, obj );
    for( PD_URIList::iterator iter = ul.begin(); iter != ul.end(); ++iter )
    {
        return *iter;
    }

    PD_DocumentRDFMutationHandle m = rdf->createMutation();
    PD_URI ret = m->createBNode();
    m->add( ret, pred, obj );
    m->commit();
    return ret;
}

std::string
PD_RDFSemanticItemViewSite::getProperty(const std::string &prop, const std::string &defval) const
{
    PD_DocumentRDFHandle rdf = m_semItem->getRDF();
    PD_URI          ls = linkingSubject();
    std::string fqprop = "http://calligra-suite.org/rdf/site#" + prop;

    PD_ObjectList ol = rdf->getObjects( ls, PD_URI(fqprop) );
    for( PD_ObjectList::iterator iter = ol.begin(); iter != ol.end(); ++iter )
    {
        return iter->toString();
    }
    return defval;
}

void
PD_RDFSemanticItemViewSite::setProperty(const std::string &prop, const std::string &v)
{
    PD_DocumentRDFHandle rdf = m_semItem->getRDF();
    std::string fqprop = "http://calligra-suite.org/rdf/site#" + prop;

    PD_URI   ls = linkingSubject();
    PD_URI pred(fqprop);
    
    PD_DocumentRDFMutationHandle m = rdf->createMutation();
    m->remove( ls, pred );
    if( !v.empty() )
        m->add( ls, pred, PD_Literal(v) );
    m->commit();    
}

void
PD_RDFSemanticItemViewSite::selectRange( FV_View* pView, std::pair< PT_DocPosition, PT_DocPosition > range )
{
    pView->selectRange( range );
}


















/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    
    

PD_DocumentRDF::PD_DocumentRDF( PD_Document* doc )
    :
    m_doc( doc ),
    m_indexAP( 0 ),
    m_haveSemItems( false )
{
    UT_DEBUGMSG(("PD_DocumentRDF() this:%p doc:%p\n",this,doc));
}

PD_DocumentRDF::~PD_DocumentRDF()
{
    UT_DEBUGMSG(("~PD_DocumentRDF() this:%p\n", this));
}

std::string
PD_DocumentRDF::makeLegalXMLID( const std::string& s )
{
    std::string ret;
    for( std::string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
    {
        char ch = *iter;
        if( ch >= 'a' && ch <= 'z' )
            ret += ch;
        else if( ch >= 'A' && ch <= 'Z' )
            ret += ch;
        else if( ch >= '0' && ch <= '9' )
            ret += ch;
        else
            ret += '_';
    }

    return ret;
}

void
PD_DocumentRDF::relinkRDFToNewXMLID( const std::string& oldxmlid,
                                     const std::string& newxmlid,
                                     bool deepCopyRDF )
{
    if( deepCopyRDF )
    {
        // FIXME: todo
        UT_DEBUGMSG(("relinkRDFToNewXMLID DEEP COPY FIXME oldid:%s newid:%s\n",
                     oldxmlid.c_str(), newxmlid.c_str() ));
    }
    
    UT_DEBUGMSG(("relinkRDFToNewXMLID oldid:%s newid:%s\n",
                 oldxmlid.c_str(), newxmlid.c_str() ));
    
    PD_DocumentRDFMutationHandle m = createMutation();
    PD_URI idref("http://docs.oasis-open.org/opendocument/meta/package/common#idref");
    
    std::set< std::string > oldlist;
    oldlist.insert( oldxmlid );
    std::string sparql = getSPARQL_LimitedToXMLIDList( oldlist );
    UT_DEBUGMSG(("relinkRDFToNewXMLID sparql:%s\n", sparql.c_str() ));

    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFQuery q( rdf, rdf );
    PD_ResultBindings_t bindings = q.executeQuery( sparql );

    for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
    {
        std::map< std::string, std::string > d = *iter;
        
        PD_URI    s( d["s"] );
        PD_URI    p( d["p"] );
        PD_Object o( d["o"] );

        UT_DEBUGMSG(("relinkRDFToNewXMLID oldid:%s newid:%s subj:%s\n",
                     oldxmlid.c_str(), newxmlid.c_str(), s.toString().c_str() ));
        
        m->add( s, idref, PD_Literal( newxmlid ));
    }

    m->commit();
}



std::string
PD_DocumentRDF::getSPARQL_LimitedToXMLIDList( const std::set< std::string >& xmlids,
                                              const std::string& extraPreds )
{
    if( xmlids.empty() )
        return "";

    std::stringstream ss;
    ss << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
       << "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
       << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
       << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#>\n"
       << "\n"
       << "select ?s ?p ?o ?rdflink \n"
       << "where { \n"
       << " ?s ?p ?o . \n"
       << " ?s pkg:idref ?rdflink . \n"
       << "   filter( ";

    std::string joiner = "";
    for( std::set< std::string >::const_iterator iter = xmlids.begin();
         iter != xmlids.end(); ++iter )
    {
        ss << joiner << " str(?rdflink) = \"" << *iter << "\" ";
        joiner = " || ";
    }
    ss << " ) \n";
    if( !extraPreds.empty() )
    {
        ss << " . " << extraPreds << "\n";
    }
    
    ss << "}\n";

    std::string ret = ss.str();
    return ret;
}

/**
 * If the document changes its m_pPieceTable it needs to call here
 * too.
 * 
 * In various places like PD_Document::_importFile the document
 * creates a new piecetable and thus must notify the DocumentRDF of
 * this change so we can update our AttrProp indexes and the like.
 */
UT_Error PD_DocumentRDF::setupWithPieceTable()
{
    PP_AttrProp* newAP = new PP_AttrProp();
    PT_AttrPropIndex newAPI = 0;
    pt_PieceTable*   pt = getPieceTable();
    pt_VarSet& m_varset = pt->getVarSet();
    bool success = m_varset.addIfUniqueAP( newAP, &newAPI );
    if( !success )
    {
		UT_DEBUGMSG(("PD_DocumentRDF::setupWithPieceTable() -- could not create raw RDF AttrProperties\n"));
        // MIQ 2010 July: addIfUniqueAP() method states false == memory error of some kind.
        return UT_OUTOFMEM;
    }
    m_indexAP = newAPI;
    UT_DEBUGMSG(("PD_DocumentRDF::setupWithPieceTable() m_indexAP:%d\n",m_indexAP));
    return UT_OK;
}


/**
 * Change the AttrProp Index that we are using to store all the RDF.
 * This is used by PD_DocumentRDFMutation to commit it's changes.
 */
void
PD_DocumentRDF::setIndexAP( PT_AttrPropIndex idx )
{
    m_indexAP = idx;
}

/**
 * Get the AttrProp index where we are storing all the RDF
 */
PT_AttrPropIndex
PD_DocumentRDF::getIndexAP(void) const
{
    return m_indexAP;
}

/**
 * Get the AttrProp with all the RDF in it
 */
const PP_AttrProp*
PD_DocumentRDF::getAP(void)
{
    PT_AttrPropIndex indexAP = getIndexAP();
    pt_PieceTable*   pt = getPieceTable();
    pt_VarSet& m_varset = pt->getVarSet();
    const PP_AttrProp* ret = m_varset.getAP(indexAP);
    return ret;
}

UT_Error PD_DocumentRDF::setAP( PP_AttrProp* newAP )
{
    newAP->prune();
    newAP->markReadOnly();
    pt_PieceTable*   pt = getPieceTable();
    pt_VarSet& m_varset = pt->getVarSet();

    PT_AttrPropIndex newAPI = 0;
    bool success = m_varset.addIfUniqueAP( newAP, &newAPI );
    // addIfUniqueAP() eats it
    newAP = 0;
    
    if(!success)
    {
        return UT_OUTOFMEM;
    }
    setIndexAP( newAPI );
    return UT_OK;
}

bool PD_DocumentRDF::isStandAlone() const
{
    return false;
}



PD_Document*
PD_DocumentRDF::getDocument(void) const
{
    return m_doc;
}


/**
 * Get a list of all the objects which have the given subject and predicate.
 *
 * For example:
 * emu has legs
 * emu has eyes
 * would give { legs, eyes }
 */
PD_ObjectList PD_DocumentRDF::getObjects( const PD_URI& s, const PD_URI& p )
{
    PD_ObjectList ret;
    apGetObjects( getAP(), ret, s, p );
    return ret;
}


/**
 * Get a list of all the subjects which have the given predicate and object.
 *
 * For example:
 * emu has legs
 * emu has eyes
 * human has legs
 * when called with ( has, legs ) would give { emu, human }
 */
PD_URIList PD_DocumentRDF::getSubjects( const PD_URI& p, const PD_Object& o )
{
    PD_URIList ret;
    apGetSubjects( getAP(), ret, p, o );
    return ret;
}


PD_URIList
PD_DocumentRDF::getAllSubjects()
{
    PD_URIList ret;
    apGetAllSubjects( getAP(), ret );
    return ret;
}

/**
 * Internal method: Using the supplied AP, insert into ret a list of
 * every subject in the model. If you want to get all triples, then
 * first get all subjects and then use getArcsOut() to get the
 * predicate-object combinations for each of the subjects.
 */

PD_URIList&
PD_DocumentRDF::apGetAllSubjects( const PP_AttrProp* AP, PD_URIList& ret )
{
    size_t count = AP->getPropertyCount();
    for( size_t i = 0; i<count; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        if( AP->getNthProperty( i, szName, szValue) )
        {
            std::string subj = szName;
            ret.push_back(subj);
        }
    }
    return ret;
}



/**
 * Get the predicate and objects which have the given subject.
 * This can be thought of as all the arcs from the node at 's' in the RDF
 * graph.
 */
POCol
PD_DocumentRDF::getArcsOut( const PD_URI& s )
{
    POCol ret;
    apGetArcsOut( getAP(), ret, s );
    return ret;
}

/**
 * Get the predicate and objects which have the given subject.
 * This can be thought of as all the arcs from the node at 's' in the RDF
 * graph.
 */
POCol&
PD_DocumentRDF::apGetArcsOut( const PP_AttrProp* AP, POCol& ret, const PD_URI& s )
{
    const gchar* szValue = 0;
	if( AP->getProperty( s.toString().c_str(), szValue) )
    {
        ret = decodePOCol(szValue);
    }
    return ret;
}


/**
 * Internal method: Using the supplied AP, insert into ret a list of
 * all the objects which have the given subject and predicate.
 *
 * For example:
 * emu has legs
 * emu has eyes
 * would give { legs, eyes }
 */
PD_ObjectList&
PD_DocumentRDF::apGetObjects( const PP_AttrProp* AP, PD_ObjectList& ret, const PD_URI& s, const PD_URI& p )
{
    const gchar* szValue = 0;
	if( AP->getProperty( s.toString().c_str(), szValue) )
    {
        POCol l = decodePOCol(szValue);
        std::pair< POCol::iterator, POCol::iterator > range
            = std::equal_range( l.begin(), l.end(), p );
        for( POCol::iterator iter = range.first; iter != range.second; ++iter )
        {
            ret.push_back( iter->second );
        }        
    }
    return ret;    
}

/**
 * Internal method: Using the supplied AP, insert into ret a list of
 * all the subjects which have the given predicate and object.
 *
 * For example:
 * emu has legs
 * emu has eyes
 * human has legs
 * when called with ( has, legs ) would give { emu, human }
 */
PD_URIList&
PD_DocumentRDF::apGetSubjects( const PP_AttrProp* AP, PD_URIList& ret, const PD_URI& p, const PD_Object& o )
{
    size_t count = AP->getPropertyCount();
    for( size_t i = 0; i<count; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        if( AP->getNthProperty( i, szName, szValue) )
        {
            POCol l = decodePOCol( szValue );
            std::string subj = szName;
            for( POCol::iterator iter = l.begin(); iter!=l.end(); ++iter )
            {
                if( iter->first == p && iter->second == o )
                    ret.push_back(subj);
            }
        }
    }
    return ret;
}



/**
 * Internal method: tests if the AP has the triple or not.
 */
bool PD_DocumentRDF::apContains( const PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    const gchar* szValue = 0;
	if( AP->getProperty( s.toString().c_str(), szValue) )
    {
        POCol l = decodePOCol(szValue);

        std::pair< POCol::iterator, POCol::iterator > range
            = std::equal_range( l.begin(), l.end(), p );
        for( POCol::iterator iter = range.first; iter != range.second; ++iter )
        {
            if( iter->second == o )
                return true;
        }             
    }
    return false;
}

/**
 * Test if the given triple is in the RDF or not.
 */
bool
PD_DocumentRDF::contains( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    return apContains( getAP(), s, p, o );
}
bool
PD_DocumentRDF::contains( const PD_RDFStatement& st )
{
    return contains( st.getSubject(), st.getPredicate(), st.getObject() );
}


/**
 * Get the total number of triples in the model. This can be
 * relatively expensive to calculate. So definately don't keep
 * on doing it in a loop.
 */
long
PD_DocumentRDF::getTripleCount()
{
    long ret = 0;
    
    PD_URIList subjects = getAllSubjects();
    PD_URIList::iterator subjend = subjects.end();
    for( PD_URIList::iterator subjiter = subjects.begin();
         subjiter != subjend; ++subjiter )
    {
        PD_URI subject = *subjiter;
        POCol polist = getArcsOut( subject );
        POCol::iterator poend = polist.end();
        for( POCol::iterator poiter = polist.begin();
             poiter != poend; ++poiter )
        {
            PD_URI    predicate = poiter->first;
            PD_Object object = poiter->second;
            ++ret;
        }
    }
    return ret;
}


PD_RDFModelIterator
PD_DocumentRDF::begin()
{
//    UT_DEBUGMSG(("docrdf::begin() ap:%p\n", getAP() ));

    PD_RDFModelHandle model = getDocument()->getDocumentRDF();
    PD_RDFModelIterator iter( model, getAP() );
    return iter;
}

PD_RDFModelIterator
PD_DocumentRDF::end()
{
    return PD_RDFModelIterator();
}



/**
 * The single way that you can update the document RDF is through
 * an PD_DocumentRDFMutation. This is where you get one of those.
 * 
 * @see PD_DocumentRDFMutation
 */
PD_DocumentRDFMutationHandle
PD_DocumentRDF::createMutation()
{
    PD_DocumentRDFMutationHandle h(new PD_DocumentRDFMutation(this));
    return h;
}

/**
 * Internal method: used by AbiCollab to update the RDF
 */
void
PD_DocumentRDF::handleCollabEvent( gchar** szAtts, gchar** szProps )
{
    PD_DocumentRDFMutationHandle h = createMutation();
    h->handleCollabEvent( szAtts, szProps );
    h->commit();
}


pt_PieceTable *
PD_DocumentRDF::getPieceTable(void) const
{
    return m_doc->getPieceTable();
}



//////////////////////////////////
//////////////////////////////////
//////////////////////////////////

void
RDFAnchor::setup( const PP_AttrProp* pAP )
{
    const gchar * v = 0;
    // if(pAP->getAttribute("this-is-an-rdf-anchor", v) && v)
    //     UT_DEBUGMSG(("RDFAnchor() is-rdf-a:%s\n",v));
    // if(pAP->getAttribute(PT_RDF_END, v) && v)
    //     UT_DEBUGMSG(("RDFAnchor() PT_RDF_END:%s\n",v));

    if( pAP->getAttribute(PT_RDF_END, v) && v)
    {
        m_isEnd = !strcmp(v,"yes");
    }
    if( pAP->getAttribute(PT_XMLID, v) && v)
    {
        m_xmlid = v;
    }
}

RDFAnchor::RDFAnchor( PD_Document* pDoc, PT_AttrPropIndex api )
    :
    m_isEnd( false )
{
    const PP_AttrProp * pAP = NULL;
    pDoc->getAttrProp( api, &pAP );
    setup( pAP );    
}


RDFAnchor::RDFAnchor( PD_Document* doc, pf_Frag* pf )
    :
    m_isEnd( false )
{
    PT_AttrPropIndex api = pf->getIndexAP();
    const PP_AttrProp * pAP = NULL;
    doc->getAttrProp( api, &pAP );
    setup( pAP );    
}


RDFAnchor::RDFAnchor( const PP_AttrProp* pAP )
    :
    m_isEnd( false )
{
    setup( pAP );
}

bool RDFAnchor::isEnd()
{
    return m_isEnd;
}
std::string RDFAnchor::getID()
{
    return m_xmlid;
}
    
//////////////////////////////////
//////////////////////////////////
//////////////////////////////////

std::set< std::string >&
PD_DocumentRDF::getAllIDs( std::set< std::string >& ret )
{
    PD_Document*    doc = getDocument();
    pf_Frag *	   iter = doc->getFragFromPosition(0);
//	pf_Frag*       last = doc->getLastFrag();

    UT_DEBUGMSG(( "getAllIDs() iter starting at:%p\n", iter ));
    
    for( ; iter; iter = iter->getNext() )
    {
        pf_Frag* pf = iter;
        std::string xmlid = pf->getXMLID();
        if( !xmlid.empty() )
            ret.insert( xmlid );
    }
    
    return ret;
}


std::pair< PT_DocPosition, PT_DocPosition >
PD_DocumentRDF::getIDRange( const std::string& xmlid ) const
{
    std::pair< PT_DocPosition, PT_DocPosition > ret( 0, 0 );
    PD_Document*    doc = getDocument();
    pf_Frag *	   iter = doc->getFragFromPosition(0);

//    UT_DEBUGMSG(( "getIDRange() iter starting at:%p\n", iter ));
    
    for( ; iter; iter = iter->getNext() )
    {
        pf_Frag* pf = iter;
        std::string x = pf->getXMLID();
        if( xmlid == x )
        {
            PT_DocPosition epos = pf->getPos() + pf->getLength();
            pf_Frag* e = pf->getNext();
            for( ; e; e = e->getNext() )
            {
                if( e->getType() == pf_Frag::PFT_Strux )
                {
                    const pf_Frag_Strux* pfs = static_cast<const pf_Frag_Strux*>(e);
                    PTStruxType st = pfs->getStruxType();
                    if( st == PTX_Block || st == PTX_SectionCell )
                    {
                        epos = e->getPos() - 1;
                        break;
                    }
                }
                if( e->getType() == pf_Frag::PFT_Object )
                {
                    const pf_Frag_Object* pfo = static_cast<const pf_Frag_Object*>(e);
                    if( pfo->getObjectType() == PTO_RDFAnchor )
                    {
                        RDFAnchor a( doc, e );
                        if( a.getID() == xmlid )
                        {
                            epos = e->getPos();
                            break;
                        }
                    }
                }
                
            }
            return std::make_pair( pf->getPos(), epos );
        }
    }
    
    return ret;
}


std::set< std::string >&
PD_DocumentRDF::addRelevantIDsForRange( std::set< std::string >& ret,
                                        PD_DocumentRange* range )
{
    addRelevantIDsForRange( ret, std::make_pair( range->m_pos1, range->m_pos2 ));
    return ret;
}

std::list< pf_Frag_Object* >
PD_DocumentRDF::getObjectsInScopeOfTypesForRange( std::set< PTObjectType > objectTypes,
                                                  std::pair< PT_DocPosition, PT_DocPosition > range )
{
    std::list< pf_Frag_Object* > ret;
//    PD_Document*    doc  = getDocument();
    pt_PieceTable*   pt  = getPieceTable();
    PT_DocPosition start = range.first;
    PT_DocPosition curr  = range.second;
    PT_DocPosition searchBackThisFar = 0;
    if( !curr )
        curr = start;
    
    //
    // Allow recursion of text:meta where an outside tag might encase the point but
    // another text:meta might completely be before point. Consider:
    // 
    // <text:meta xml:id="outside"> ....
    //   ... <text:meta xml:id="nested">boo</text:meta>
    //   ... POINT ...
    // </text:meta>
    //
    // In this case, we want to ignore the xml:id == nested anchors but find the
    // xml:id == outside one
    //
    std::set< std::string > m_ignoreIDSet;
    
    //
    // FIXME: Some form of index would be nice, rather than walking back the entire
    // document to find the RDF Anchors.
    //
    for( ; curr > searchBackThisFar; )
    {
        pf_Frag* pf = 0;
        PT_BlockOffset boffset;

        if( pt->getFragFromPosition( curr, &pf, &boffset ) )
        {
            xxx_UT_DEBUGMSG(("PD_DocumentRDF::getObjectsInScope() current:%d frag type:%d len:%d \n",
                         curr, pf->getType(), pf->getLength() ));

            // skip backwards fast if possible.
            if(pf->getType() != pf_Frag::PFT_Object)
            {
                xxx_UT_DEBUGMSG(("PD_DocumentRDF::getObjectsInScope() SKIPPING BACK!\n" ));
                curr = pf->getPos() - 1;
                continue;
            }
            // otherwise keep moving backwards slowly anyway
            --curr;

            
            // check what RDF is attached to this object.
            if(pf->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object* pOb = static_cast<pf_Frag_Object*>(pf);
                const PP_AttrProp * pAP = NULL;

                xxx_UT_DEBUGMSG(("PD_DocumentRDF::getObjectsInScope() po type:%d\n",
                             pOb->getObjectType() ));

                if( pOb->getObjectType() == PTO_Bookmark
                    && objectTypes.count(pOb->getObjectType()))
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
                    const char* v = 0;
                    if( pAP->getAttribute(PT_XMLID, v) && v)
                    {
                        std::string xmlid = v;
                        bool isEnd = pAP->getAttribute("type", v) && v && !strcmp(v,"end");
                        
                        xxx_UT_DEBUGMSG(("PD_DocumentRDF::getObjectsInScope() isEnd:%d id:%s\n",
                                     isEnd, xmlid.c_str() ));
                        
                        if( isEnd && curr < start )
                        {
                            m_ignoreIDSet.insert( xmlid );
                        }
                        else
                        {
                            if( !m_ignoreIDSet.count( xmlid ))
                                ret.push_back( pOb );
                        }
                    }
                }
                
                if( pOb->getObjectType() == PTO_RDFAnchor
                    && objectTypes.count(pOb->getObjectType()))
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);

                    RDFAnchor a(pAP);
                    xxx_UT_DEBUGMSG(("PD_DocumentRDF::getObjectsInScope() isEnd:%d id:%s\n",
                                 a.isEnd(), a.getID().c_str() ));
                    
                    if( a.isEnd() && curr < start )
                    {
                        m_ignoreIDSet.insert( a.getID() );
                    }
                    else
                    {
                        if( !m_ignoreIDSet.count( a.getID() ))
                            ret.push_back( pOb );
                    }
                }
            }
        }
    }
    
    return ret;
}


std::set< std::string >&
PD_DocumentRDF::addXMLIDsForObjects( std::set< std::string >& ret, std::list< pf_Frag_Object* > objectList )
{
    const PP_AttrProp * pAP = NULL;

    for( std::list< pf_Frag_Object* >::iterator iter = objectList.begin();
         iter != objectList.end(); ++iter )
    {
        pf_Frag_Object* pOb = *iter;

        if( pOb->getObjectType() == PTO_Bookmark )
        {
            pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
            const char* v = 0;
            if( pAP->getAttribute(PT_XMLID, v) && v)
            {
                std::string xmlid = v;
                ret.insert( xmlid );
            }
        }
        
        if( pOb->getObjectType() == PTO_RDFAnchor )
        {
            pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
            RDFAnchor a(pAP);
            ret.insert( a.getID() );
            xxx_UT_DEBUGMSG(("addXMLIDsForObjects() xmlid:%s\n", a.getID().c_str() ));
        }
    }
    return ret;
}

PT_DocPosition
PD_DocumentRDF::addXMLIDsForBlockAndTableCellForPosition( std::set< std::string >& col, PT_DocPosition pos )
{
    PT_DocPosition  ret = pos;
    PD_Document*    doc = getDocument();
    pt_PieceTable*   pt = getPieceTable();

    pf_Frag* frag = doc->getFragFromPosition( pos );
    ret = frag->getPos() - 1;
    
    //
    // xml:id attached to containing paragraph/header
    // <text:p> / <text:h>
    //
    pf_Frag_Strux* psdh;
    if( pt->getStruxOfTypeFromPosition( pos, PTX_Block, &psdh ) && psdh )
    {
        xxx_UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() block pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                xxx_UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() xmlid:%s \n",v));
                col.insert(v);
            }
        }
    }

    //
    // xml:id attached to containing table:table-cell
    //
    if( pt->getStruxOfTypeFromPosition( pos, PTX_SectionCell, &psdh ) && psdh )
    {
        xxx_UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() cell pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                xxx_UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() xmlid:%s \n",v));
                col.insert(v);
#if 0
                if(AP->getAttribute("props", v)) {
                    xxx_UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() props:%s \n",v));
				}
#endif
            }
        }
    }
    
    return ret;
}


std::set< std::string >&
PD_DocumentRDF::addRelevantIDsForRange( std::set< std::string >& ret,
                                        std::pair< PT_DocPosition, PT_DocPosition > range )
{
    //
    // A sneaky optimization, for the first position we might have to
    // search backward to the start of the document to see opening
    // blocks and text:meta elements which might have RDF attached.
    // However, for startpos+1 to the end position we only have to
    // consider the range itself because we have already added any IDs
    // which might have opened before the range with the first
    // backward search.
    //
    PT_DocPosition pos = range.first;

    xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() xxxyyy begin:%d end:%d\n", range.first, range.second ));
    xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() xxxyyy looking back to find bookmark and rdf anchor openings...\n" ));
    std::set< PTObjectType > objectTypes;
    objectTypes.insert( PTO_Bookmark  );
    objectTypes.insert( PTO_RDFAnchor );
    std::list< pf_Frag_Object* > objectList = getObjectsInScopeOfTypesForRange( objectTypes, range );
    addXMLIDsForObjects( ret, objectList );
    xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() xxxyyy objectList.sz:%ld\n", objectList.size() ));
    addXMLIDsForBlockAndTableCellForPosition( ret, pos );

    
    xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() xxxyyy inspecting range...\n" ));
//    PT_DocPosition searchBackThisFar = pos;
    ++pos;

    PT_DocPosition endPos = range.second;
    if( !endPos ) {
        endPos = range.first+1;
	}

    for( PT_DocPosition curr = endPos; curr >= range.first; )
    {
        xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() xxxyyy looping curr:%d\n", curr ));
        curr = addXMLIDsForBlockAndTableCellForPosition( ret, curr );
    }
    

    
//    priv_addRelevantIDsForPosition( ret, endPos, searchBackThisFar );
    
    // for( ; pos <= range.second; ++pos )
    // {
    //     priv_addRelevantIDsForPosition( ret, pos, searchBackThisFar );
    // }
    xxx_UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForRange() ret begin:%d end:%d\n", range.first, range.second ));
    return ret;
}

std::set< std::string >&
PD_DocumentRDF::addRelevantIDsForPosition( std::set< std::string >& ret,
                                           PT_DocPosition pos )
{
    addRelevantIDsForRange( ret, std::make_pair( pos, (PT_DocPosition)0 ) );
    return ret;
}

std::set< std::string >&
PD_DocumentRDF::priv_addRelevantIDsForPosition( std::set< std::string >& ret,
                                                PT_DocPosition pos,
                                                PT_DocPosition searchBackThisFar )
{
    PD_Document*    doc = getDocument();
    pt_PieceTable*   pt = getPieceTable();
    PT_DocPosition curr = pos;
    
    UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() current:%d searchBackLimit:%d\n",
                 curr, searchBackThisFar ));

    //
    // Allow recursion of text:meta where an outside tag might encase the point but
    // another text:meta might completely be before point. Consider:
    // 
    // <text:meta xml:id="outside"> ....
    //   ... <text:meta xml:id="nested">boo</text:meta>
    //   ... POINT ...
    // </text:meta>
    //
    // In this case, we want to ignore the xml:id == nested anchors but find the
    // xml:id == outside one
    //
    std::set< std::string > m_ignoreIDSet;
    
    //
    // FIXME: Some form of index would be nice, rather than walking back the entire
    // document to find the RDF Anchors.
    //
    for( ; curr > searchBackThisFar; )
    {
        pf_Frag* pf = 0;
        PT_BlockOffset boffset;

        if( pt->getFragFromPosition( curr, &pf, &boffset ) )
        {
            UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() current:%d frag type:%d len:%d \n",
                         curr, pf->getType(), pf->getLength() ));

            // skip backwards fast if possible.
            if(pf->getType() != pf_Frag::PFT_Object)
            {
                curr = pf->getPos() - 1;
                continue;
            }
            // otherwise keep moving backwards slowly anyway
            --curr;

            
            // check what RDF is attached to this object.
            if(pf->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object* pOb = static_cast<pf_Frag_Object*>(pf);
                const PP_AttrProp * pAP = NULL;

                UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() po type:%d\n",
                             pOb->getObjectType() ));

                if(pOb->getObjectType() == PTO_Bookmark)
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
                    const char* v = 0;
                    if( pAP->getAttribute(PT_XMLID, v) && v)
                    {
                        std::string xmlid = v;
                        bool isEnd = pAP->getAttribute("type", v) && v && !strcmp(v,"end");
                        
                        UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() isEnd:%d id:%s\n",
                                     isEnd, xmlid.c_str() ));
                        
                        if( isEnd )
                        {
                            m_ignoreIDSet.insert( xmlid );
                        }
                        else
                        {
                            if( !m_ignoreIDSet.count( xmlid ))
                                ret.insert( xmlid );
                        }
                    }
                }
                
                if(pOb->getObjectType() == PTO_RDFAnchor)
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);

                    RDFAnchor a(pAP);
                    UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() isEnd:%d id:%s\n",
                                 a.isEnd(), a.getID().c_str() ));
                    
                    if( a.isEnd() )
                    {
                        m_ignoreIDSet.insert( a.getID() );
                    }
                    else
                    {
                        if( !m_ignoreIDSet.count( a.getID() ))
                            ret.insert( a.getID() );
                    }
                }
            }
        }
    }

    
    //
    // xml:id attached to containing paragraph/header
    // <text:p> / <text:h>
    //
    pf_Frag_Strux* psdh;
    if( pt->getStruxOfTypeFromPosition( pos, PTX_Block, &psdh ) && psdh )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() block pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() xmlid:%s \n",v));
                ret.insert(v);
            }
        }
    }

    //
    // xml:id attached to containing table:table-cell
    //
    if( pt->getStruxOfTypeFromPosition( pos, PTX_SectionCell, &psdh ) && psdh )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() cell pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() xmlid:%s \n",v));
                ret.insert(v);

                if(AP->getAttribute("props", v)) {
                    UT_DEBUGMSG(("PD_DocumentRDF::priv_addRelevantIDsForPosition() props:%s \n",v));
				}

            }
        }
    }
    
    return ret;
}



PD_RDFModelHandle
PD_DocumentRDF::createScratchModel()
{
    PD_Document* doc = getDocument();
    PP_AttrProp*  AP = new PP_AttrProp();
    PD_RDFModelFromAP* retModel = new PD_RDFModelFromAP( doc, AP );
    PD_RDFModelHandle ret( retModel );
    return ret;
}

PD_URI
PD_DocumentRDF::getManifestURI()
{
    return PD_URI("http://abiword.org/manifest.rdf");
}



PD_RDFModelHandle PD_DocumentRDF::getRDFAtPosition( PT_DocPosition pos )
{
    UT_DEBUG_ONLY_ARG(pos);

    PD_Document*    doc = getDocument();

#ifdef DEBUG
    std::set< std::string > IDList;
    addRelevantIDsForPosition( IDList, pos );

    PP_AttrProp* AP = new PP_AttrProp();
    PD_RDFModelFromAP* retModel = new PD_RDFModelFromAP( doc, AP );
    PD_RDFModelHandle ret( retModel );

    if( !IDList.empty() )
    {
        PD_DocumentRDFMutationHandle m = retModel->createMutation();
        for( std::set< std::string >::iterator iter = IDList.begin();
             iter != IDList.end(); ++iter )
        {
            std::string xmlid = *iter;
            addRDFForID( xmlid, m );
        } 
        m->commit();
    }
    

//    retModel->dumpModel("RDF result for xmlid");
    return ret;
    
    
    // if( pt->getStruxOfTypeFromPosition( pos, PTX_Block, &psdh ) && psdh )
    // {
    //     UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() pos:%d\n", pos ));
    //     PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
    //     const PP_AttrProp * AP = NULL;
    //     doc->getAttrProp(api,&AP);
    //     if( AP )
    //     {
    //         const char * v = NULL;
    //         if(AP->getAttribute("xml:id", v))
    //         {
    //             UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() xmlid:%s \n",v));
    //             PD_RDFModelHandle ret = getRDFForID( v );
    //             return ret;
    //         }
    //     }
    // }

#else
    {
        // return an empty model
        PD_RDFModelHandle x( new PD_RDFModelFromAP( doc, new PP_AttrProp() ));
        return x;
    }
#endif    
}

void PD_DocumentRDF::addRDFForID( const std::string& xmlid, PD_DocumentRDFMutationHandle& m )
{
    // Execute query to find all triples related to xmlid
    // and add them all to the mutation m
    //
    // prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> 
    // prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> 
    // select ?s ?p ?o 
    // where { 
    //  ?s pkg:idref ?xmlid . 
    //  ?s ?p ?o . 
    //  filter( str(?xmlid) = \"" << xmlid << "\" ) 
    // };

    PD_URI pkg_idref("http://docs.oasis-open.org/opendocument/meta/package/common#idref");
    PD_Object xmlidNode( xmlid );
    
    PD_URIList subjects = getAllSubjects();
    PD_URIList::iterator subjend = subjects.end();
    for( PD_URIList::iterator subjiter = subjects.begin();
         subjiter != subjend; ++subjiter )
    {
        bool addSubject = false;

        PD_URI subject = *subjiter;
        {
            POCol polist = getArcsOut( subject );
            POCol::iterator poend = polist.end();
            for( POCol::iterator poiter = polist.begin();
                 poiter != poend; ++poiter )
            {
                PD_URI    predicate = poiter->first;
                PD_Object object = poiter->second;
                if( predicate == pkg_idref && object == xmlidNode )
                {
                    addSubject = true;
                    break;
                }
            }
        }
        

        if( addSubject )
        {
            POCol polist = getArcsOut( subject );
            POCol::iterator poend = polist.end();
            for( POCol::iterator poiter = polist.begin();
                 poiter != poend; ++poiter )
            {
                PD_URI    predicate = poiter->first;
                PD_Object object = poiter->second;

                UT_DEBUGMSG(("PD_DocumentRDF::adding s:%s p:%s o:%s\n",
                             subject.toString().c_str(),
                             predicate.toString().c_str(),
                             object.toString().c_str() ));
                
                m->add( subject, predicate, object );
            }
        }
    }
}


PD_RDFModelHandle PD_DocumentRDF::getRDFForID( const std::string& xmlid )
{
    PP_AttrProp* AP = new PP_AttrProp();
    PD_RDFModelFromAP* retModel = new PD_RDFModelFromAP( m_doc, AP );
    PD_RDFModelHandle ret( retModel );

    PD_DocumentRDFMutationHandle m = retModel->createMutation();
    addRDFForID( xmlid, m );
    m->commit();

//    retModel->dumpModel("RDF result for xmlid");
    return ret;
}








void PD_DocumentRDF::runMilestone2Test()
{
#ifdef DEBUG
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() doc:%p\n", m_doc));

    {
        PD_DocumentRDFMutationHandle m = createMutation();
        m->add( PD_URI("http://www.example.com/foo"),
                PD_URI("http://www.example.com/bar"),
                PD_Literal("AABBCC") );
        m->add( PD_URI("http://www.example.com/koala"),
                PD_URI("http://www.example.com/is-a"),
                PD_Literal("dangeroo",
                           "http://www.w3.org/2001/XMLSchema#boolean" ));
        
    }
    dumpModel();

    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() -------------------------------\n"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() adding some more triples\n"));
    {
        PD_DocumentRDFMutationHandle m = createMutation();
        m->add( PD_URI("http://www.example.com/foo"),
                PD_URI("http://www.example.com/bar2"),
                PD_Literal("extra data") );
        m->add( PD_URI("http://www.example.com/foo"),
                PD_URI("http://www.example.com/magic"),
                PD_Literal("card trick",
                          "http://www.w3.org/2001/XMLSchema#string" ));
        m->add( PD_URI("http://www.example.com/emu"),
                PD_URI("http://www.example.com/lives-in"),
                PD_Object("http://www.example.com/australia"));
        m->add( PD_URI("http://www.example.com/water dragon"),
                PD_URI("http://www.example.com/lives-in"),
                PD_Object("http://www.example.com/australia"));
    }
    dumpModel();


    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() -------------------------------\n"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() removing some triples\n"));
    {
        // This test removes something from foo, which has multiple
        // values in it's PO list. Also the koala is removed, so the
        // whole s->po mapping should disappear from the AP after
        // commit();
        
        PD_DocumentRDFMutationHandle m = createMutation();
        m->remove( PD_URI("http://www.example.com/koala"),
                   PD_URI("http://www.example.com/is-a"),
                   PD_Object("http://www.example.com/dangeroo"));
        m->remove( PD_URI("http://www.example.com/foo"),
                   PD_URI("http://www.example.com/bar"),
                   PD_Literal("AABBCC") );

    }
    dumpModel();


    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() -------------------------------\n"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test() rollback test\n"));
    {
        PD_DocumentRDFMutationHandle m = createMutation();
        m->add( PD_URI("http://www.example.com/et"),
                PD_URI("http://www.example.com/is-a"),
                PD_Object("http://www.example.com/alien"));
        m->rollback();
    }
    dumpModel();    

    runMilestone2Test2();
#endif
}

#ifdef DEBUG
static void dump( const std::string& msg, PD_RDFModelIterator iter, PD_RDFModelIterator e )
{
    int count = 0;
    UT_DEBUGMSG(("dump(top) msg::%s\n", msg.c_str() ));
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st(*iter);
        UT_DEBUGMSG((" st:%s\n", st.toString().c_str() ));
        ++count;
    }
    UT_DEBUGMSG(("dump(end) count:%d msg::%s\n", count, msg.c_str() ));
}
#endif


void PD_DocumentRDF::updateHaveSemItemsCache()
{
    PD_RDFSemanticItems items = getAllSemanticObjects();
    m_haveSemItems = !items.empty();
}

bool PD_DocumentRDF::haveSemItems() const 
{
    return m_haveSemItems;
}



void PD_DocumentRDF::runPlay()
{
#ifdef DEBUG
    UT_DEBUGMSG(("================================================================================\n" ));
    UT_DEBUGMSG(("PD_DocumentRDF::runPlay() o:%s\n", "foo" ));

    PD_RDFContacts cl = getContacts();
    UT_DEBUGMSG(("PD_DocumentRDF::runPlay() contacts.sz:%lu\n", (long unsigned)cl.size() ));

    for( PD_RDFContacts::iterator ci = cl.begin(); ci != cl.end(); ++ci )
    {
        PD_RDFContactHandle c = *ci;

        UT_DEBUGMSG((" subj:%s\n", c->linkingSubject().toString().c_str() ));
        UT_DEBUGMSG((" name:%s\n", c->name().c_str() ));
        std::set< std::string > xmlids = c->getXMLIDs();
        for( std::set< std::string >::iterator iter = xmlids.begin(); iter != xmlids.end(); ++iter )
        {
            std::string xmlid = *iter;
            UT_DEBUGMSG(("   x2mlid:%s ", xmlid.c_str() ));
            std::pair< PT_DocPosition, PT_DocPosition > range = getIDRange( xmlid );
            UT_DEBUGMSG(("   start:%d end:%d ", range.first, range.second ));
            UT_DEBUGMSG((" \n" ));
        }


        // if( c->name() == "James Smith" )
        // {
        //     c->showEditorWindow( c );
        // }

    }

    PD_RDFEvents el = getEvents();
    UT_DEBUGMSG((" events.sz:%lu\n", (long unsigned)el.size() ));
    for( PD_RDFEvents::iterator ei = el.begin(); ei != el.end(); ++ei )
    {
        PD_RDFEventHandle e = *ei;
        UT_DEBUGMSG((" events.name:%s\n", e->name().c_str() ));

        if( e->name() == "uri:campingtime" )
        {
            e->showEditorWindow( e );
        }
    }


    return;


    UT_DebugOnly<int> count = 0;
    dump( "whole model", begin(), end() );

    UT_DEBUGMSG(("runPlay() triple count:%d\n", (int)count ));
    UT_DEBUGMSG(("runPlay()\n\n"));

    PD_RDFModelHandle m = getRDFForID( "wingb" );
    dump( "wingb", m->begin(), m->end() );
#endif
}


void PD_DocumentRDF::runMilestone2Test2()
{
#ifdef DEBUG
    PD_URI s;
    PD_URI o = getObject( PD_URI("http://www.example.com/emu"),
                          PD_URI("http://www.example.com/lives-in"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2() o:%s\n", o.toString().c_str()));

    o = getObject( PD_URI("http://www.example.com/et"),
                   PD_URI("http://www.example.com/is-a"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2() (should be nothing) o:%s\n", o.toString().c_str()));

        
    POCol col = getArcsOut( PD_URI("http://www.example.com/foo") );
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2() subject foo has arcs... count:%d\n", (int)col.size()));
    for( POCol::iterator iter = col .begin(); iter != col.end(); ++iter )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2()   p:%s\n", iter->first.toString().c_str()));
        UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2()   o:%s\n", iter->second.toString().c_str()));
    }

    s = getSubject( PD_URI("http://www.example.com/lives-in"),
                    PD_Object("http://www.example.com/australia"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2() one creature living in aus:%s\n", s.toString().c_str()));

    PD_URIList ul = getSubjects( PD_URI("http://www.example.com/lives-in"),
                                 PD_Object("http://www.example.com/australia"));
    UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2() all of the creatures living in aus, count:%d\n",
                 (int)ul.size()));
    for( PD_URIList::iterator iter = ul.begin(); iter != ul.end(); ++iter )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::runMilestone2Test2()   creature:%s\n", iter->toString().c_str()));
    }
    
    
#endif    
}


void PD_DocumentRDF::dumpObjectMarkersFromDocument()
{
#ifdef DEBUG
    UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() doc:%p\n", m_doc));
    m_doc->dumpDoc("dumpObjectMarkersFromDocument", 0, 0);

    PD_Document*    doc = getDocument();
    pt_PieceTable*   pt = getPieceTable();
    PT_DocPosition curr = 0;
    PT_DocPosition eod = 0;
    pt->getBounds( true, eod );
    
    UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() current:%d end:%d\n", curr, eod ));

    for( ; curr < eod; ++curr )
    {
        pf_Frag* pf = 0;
        PT_BlockOffset boffset;
        pf_Frag_Strux* psdh;
        if( pt->getStruxOfTypeFromPosition( curr, PTX_Block, &psdh ) && psdh )
        {
            UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() current:%d end:%d have PTX_BLOCK \n",
                         curr, eod ));
	        PT_AttrPropIndex api = doc->getAPIFromSDH( psdh );
            const PP_AttrProp * AP = NULL;
            doc->getAttrProp(api,&AP);
            if( AP )
            {
                const char * v = NULL;
                if(AP->getAttribute("xml:id",v))
                {
                    UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() xmlid:%s \n",v));
                }
            }
        }
        

        
        if( pt->getFragFromPosition( curr, &pf, &boffset ) )
        {
            UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() current:%d end:%d frag type:%d len:%d \n", curr, eod, pf->getType(), pf->getLength() ));

            if(pf->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object* pOb = static_cast<pf_Frag_Object*>(pf);

                UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() po type:%d\n",
                             pOb->getObjectType() ));
        
                if(pOb->getObjectType() == PTO_RDFAnchor)
                {
                    const PP_AttrProp * pAP = NULL;
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);

                    const gchar * v = 0;
                    if(!pAP->getAttribute(PT_XMLID, v) || !v)
                    {
                    }
                    else
                    {
                        UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() xml:id:%s\n",v));
                    }
                    if(pAP->getAttribute("this-is-an-rdf-anchor", v) && v) {
                        UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() is-rdf-a:%s\n",v));
					}
                }
            }
            
        }
    }


    //////////////

//    curr = 420;
//    PD_RDFModelHandle h = getRDFAtPosition( curr );
    
#endif    
}


/**
 * Dump the model as debug messages
 */
void PD_DocumentRDF::dumpModel( const std::string& headerMsg )
{
    UT_DEBUG_ONLY_ARG(headerMsg);
  
#ifdef DEBUG    
    UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() doc:%p\n", m_doc));
    apDumpModel( getAP(), headerMsg );
#endif
}

void
PD_DocumentRDF::maybeSetDocumentDirty()
{
    if( m_doc )
        m_doc->forceDirty();
    
    incremenetVersion();
}


/**
 * Dump the model contained in the given AP as debug messages
 */
void
PD_DocumentRDF::apDumpModel( const PP_AttrProp* AP, const std::string& headerMsg )
{
	UT_DEBUG_ONLY_ARG(AP);
	UT_DEBUG_ONLY_ARG(headerMsg);

#ifdef DEBUG
    UT_DEBUGMSG(("PD_DocumentRDF::apDumpModel() ----------------------------------\n"));
    UT_DEBUGMSG(("PD_DocumentRDF::apDumpModel() %s\n", headerMsg.c_str()));
    UT_DEBUGMSG(("PD_DocumentRDF::apDumpModel() triple count:%ld\n", getTripleCount()));
    UT_DEBUGMSG(("PD_DocumentRDF::apDumpModel() ----------------------------------\n"));

    size_t count = AP->getPropertyCount();
    UT_DEBUGMSG(("PD_DocumentRDF::DUMPMODEL() API:%d COUNT:%ld\n", m_indexAP, (long)count));
    for( size_t i = 0; i < count; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        if( AP->getNthProperty( i, szName, szValue) )
        {
//            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() szName :%s\n", szName ));
//            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() szValue:%s\n", szValue ));
            
            POCol l = decodePOCol( szValue );
//            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() po list size:%d\n", (int)l.size() ));
            std::string subj = szName;
            for( POCol::iterator iter = l.begin(); iter!=l.end(); ++iter )
            {
                std::string pred = iter->first.toString();
                std::string obj  = iter->second.toString();
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() s:%s\n",
                             subj.c_str()));
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()     p:%s\n",
                             pred.c_str()));
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()     ot:%d o:%s\n",
                             iter->second.getObjectType(), obj.c_str()));
                if( iter->second.hasXSDType() )
                {
                    UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()  type:%s\n",
                                 iter->second.getXSDType().c_str()));
                }
            }
        }
    }
#endif
}

/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/


/**
 * A view on the given delegate model, restricted to showing only the
 * triples which match a given SPARQL query. For dynamicaly created
 * queries, a subclass might overload getSparql() and make the query
 * string at runtime.
 * 
 * Subclasses might decide to override createMutation() to adjust
 * things when changes are desired.
 */
class RDFModel_SPARQLLimited
    :
    public PD_RDFModelFromAP
{
protected:

    PD_DocumentRDFHandle m_rdf;
    PD_RDFModelHandle    m_delegate;
    std::string          m_sparql;
    
protected:
    virtual void update();
    
public:

    RDFModel_SPARQLLimited( PD_DocumentRDFHandle rdf,
                            PD_RDFModelHandle delegate )
        : PD_RDFModelFromAP( rdf->getDocument(), new PP_AttrProp() )
        , m_rdf( rdf )
        , m_delegate( delegate )
    {
    }
    virtual ~RDFModel_SPARQLLimited()
    {
    }
    virtual PD_DocumentRDFMutationHandle createMutation();    
    virtual std::string getSparql()
    {
        return m_sparql;
    }

    
    void setSparql( const std::string& s )
    {
        m_sparql = s;
    }
    
    virtual const PP_AttrProp* getAP(void)
    {
        update();
        return m_AP;
    }

};

PD_DocumentRDFMutationHandle
RDFModel_SPARQLLimited::createMutation()
{
    PD_DocumentRDFMutationHandle ret;
    return ret;
}

void
RDFModel_SPARQLLimited::update()
{
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() TOP\n" ));
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() our   version:%ld\n", getVersion() ));
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() their version:%ld\n", m_delegate->getVersion() ));

    if( getVersion() >= m_delegate->getVersion() )
    {
        UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() nothing to be done...\n" ));
        return;
    }
    
    PP_AttrProp* AP = new PP_AttrProp();
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() 1\n" ));
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() sparql:%s\n", getSparql().c_str() ));
    
    PD_RDFQuery q( m_rdf, m_delegate );
    PD_ResultBindings_t bindings = q.executeQuery( getSparql() );
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() 2\n" ));

    for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
    {
        std::map< std::string, std::string > d = *iter;
        
        PD_URI    s( d["s"] );
        PD_URI    p( d["p"] );

        int objectType = PD_Object::OBJECT_TYPE_URI;
        PD_Object dobj = m_delegate->getObject( s, p );
        UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() dobj.valid:%d dobj.type:%d dobj.str:%s \n",
                     dobj.isValid(),
                     dobj.getObjectType(),
                     dobj.toString().c_str() ));
        if( dobj.isValid() )
            objectType = dobj.getObjectType();
        
        PD_Object o( d["o"], objectType );
        

        POCol l;
        const gchar* szName = s.toString().c_str();
        const gchar* szValue = 0;
        if( AP->getProperty( szName, szValue) )
        {
            l = decodePOCol(szValue);
        }
        l.insert( std::make_pair( p, o ));
        std::string po = encodePOCol(l);
        AP->setProperty( szName, po.c_str() );    
        
        PD_RDFStatement st( s, p, o );
        UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() adding st:%s \n", st.toString().c_str() ));
    }    

    delete m_AP;
    m_AP = AP;
    m_version = m_delegate->getVersion();

//    dumpModel("after sparql query...");
}

/****************************************/
/****************************************/
/****************************************/


class RDFModel_XMLIDLimited
    :
    public RDFModel_SPARQLLimited
{
    std::string m_writeID;
    std::set< std::string > m_readIDList;
protected:
    virtual void update();
    
public:
    
    RDFModel_XMLIDLimited( PD_DocumentRDFHandle rdf,
                           PD_RDFModelHandle delegate,
                           const std::string& writeID,
                           const std::set< std::string >& readIDList )
        : RDFModel_SPARQLLimited( rdf, delegate )
        , m_writeID( writeID )
        , m_readIDList( readIDList )
    {
    }
    
    virtual ~RDFModel_XMLIDLimited()
    {
        UT_DEBUGMSG(("~RDFModel_XMLIDLimited()\n"));
    }

    virtual std::string getSparql();
    virtual PD_DocumentRDFMutationHandle createMutation();
};

void
RDFModel_XMLIDLimited::update()
{
    if( getVersion() >= m_delegate->getVersion() )
    {
        UT_DEBUGMSG(("RDFModel_XMLIDLimited::update() nothing to be done...\n" ));
        return;
    }

    std::set< std::string > xmlids;
    xmlids.insert( m_writeID );
    copy( m_readIDList.begin(), m_readIDList.end(), inserter( xmlids, xmlids.end() ) );

    if( xmlids.size() == 1 )
    {
        std::string xmlid = *(xmlids.begin());
        PP_AttrProp* AP = new PP_AttrProp();

        PD_URI     idref(  "http://docs.oasis-open.org/opendocument/meta/package/common#pkg:idref" );
        PD_Literal rdflink( xmlid );
        PD_URI s = m_delegate->getSubject( idref, rdflink );
        POCol polist = m_delegate->getArcsOut( s );

        const gchar* szName = s.toString().c_str();
        std::string po = encodePOCol( polist );
        AP->setProperty( szName, po.c_str() );    
        return;
    }
    
    RDFModel_SPARQLLimited::update();
}


std::string
RDFModel_XMLIDLimited::getSparql()
{
    UT_DEBUGMSG(("RDFModel_XMLIDLimited::getSparql()\n"));
    std::set< std::string > xmlids;
    xmlids.insert( m_writeID );
    copy( m_readIDList.begin(), m_readIDList.end(), inserter( xmlids, xmlids.end() ) );
    std::string sparql = PD_DocumentRDF::getSPARQL_LimitedToXMLIDList( xmlids );
    return sparql;
}


class ABI_EXPORT PD_RDFMutation_XMLIDLimited
    :
    public PD_DocumentRDFMutation
{
    PD_DocumentRDFMutationHandle m_delegate;
    std::string m_writeID;
    typedef std::set< std::string > m_cleanupSubjects_t;
    m_cleanupSubjects_t m_cleanupSubjects;
    
  public:
    PD_RDFMutation_XMLIDLimited( PD_DocumentRDF* rdf,
                                 PD_DocumentRDFMutationHandle delegate,
                                 const std::string& xmlid )
        : PD_DocumentRDFMutation( rdf )
        , m_delegate( delegate )
        , m_writeID( xmlid )
    {
    }
    
    virtual bool add( const PD_URI& s, const PD_URI& p, const PD_Object& o )
    {
        UT_DEBUGMSG(("XMLIDLimited::add() s:%s\n", s.toString().c_str() ));
        UT_DEBUGMSG(("XMLIDLimited::add() p:%s\n", p.toString().c_str() ));
        UT_DEBUGMSG(("XMLIDLimited::add() o:%s\n", o.toString().c_str() ));
        UT_DEBUGMSG(("XMLIDLimited::add() s:%s linking to xmlid:%s\n",
                     s.toString().c_str(), m_writeID.c_str() ));
        
        bool rc = true;
        rc &= m_delegate->add( s, p, o );
        if( !rc )
            return rc;

        
        PD_RDFStatement rdflink( s,
                                 PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"),
                                 PD_Literal(m_writeID) );
        UT_DEBUGMSG(("XMLIDLimited::add() testing document for existing rdflink\n" ));
        if( !m_rdf->contains( rdflink ))
        {
            UT_DEBUGMSG(("XMLIDLimited::add() document does not contain the rdflink\n" ));
            rc &= m_delegate->add( rdflink );
        }
        
        UT_DEBUGMSG(("XMLIDLimited::add() s:%s linking to xmlid:%s rc:%d\n",
                     s.toString().c_str(), m_writeID.c_str(), rc ));
        return rc;
        
    }
    virtual void remove( const PD_URI& s, const PD_URI& p, const PD_Object& o )
    {
        POCol po = m_rdf->getArcsOut( s );
        UT_DEBUGMSG(("XMLIDLimited::remove() subject count:%d\n", (int)po.size() ));
        
        m_delegate->remove( s, p, o );
        m_cleanupSubjects.insert( s.toString() );
    }
    virtual UT_Error commit()
    {
        UT_DEBUGMSG(("XMLIDLimited::commit()\n" ));
        UT_Error ret = m_delegate->commit();

        //
        // Check the subjects that were deleted to see if they are now only
        // involved in triples like
        // s pkg:idref ?xmlid
        // 
        for( m_cleanupSubjects_t::iterator iter = m_cleanupSubjects.begin();
             iter != m_cleanupSubjects.end(); ++iter )
        {
            std::string subj = *iter;
            
            std::stringstream sparql;
            sparql << "prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                   << "prefix foaf:  <http://xmlns.com/foaf/0.1/>  \n"
                   << "prefix pkg:   <http://docs.oasis-open.org/opendocument/meta/package/common#>  \n"
                   << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
                   << " \n"
                   << "select ?s ?p ?o ?rdflink  \n"
                   << "where {  \n"
                   << " ?s ?p ?o .  \n"
                   << " ?s pkg:idref ?rdflink .  \n"
                   << "   filter( str(?s) = \"" << subj << "\" ) . \n"
                   << "   filter( str(?p) != \"http://docs.oasis-open.org/opendocument/meta/package/common#idref\" ) \n"
                << "} \n";
            

            // std::list< std::string > xmlids;
            // xmlids.push_back( xmlid );
            // std::string sparql = PD_DocumentRDF::getSPARQL_LimitedToXMLIDList(
            //     xmlids,
            //     "filter( str(?p) != \"http://docs.oasis-open.org/opendocument/meta/package/common#idref\" ) " );
            PD_DocumentRDFHandle rdf = m_rdf->getDocument()->getDocumentRDF();
            PD_RDFQuery q( rdf, rdf );
            PD_ResultBindings_t bindings = q.executeQuery( sparql.str() );

            
            UT_DEBUGMSG(("XMLIDLimited::commit() subj:%s bindings.count:%d\n",
                         subj.c_str(), (int)bindings.size() ));
            
            if( bindings.empty() )
            {
                PD_URI s(subj);
                PD_URI idref("http://docs.oasis-open.org/opendocument/meta/package/common#idref");
                UT_DEBUGMSG(("XMLIDLimited::commit() can remove links for subj:%s\n", subj.c_str() ));

                PD_ObjectList ul = rdf->getObjects( subj, idref );
                PD_DocumentRDFMutationHandle m = rdf->createMutation();
                for( PD_ObjectList::iterator iter2 = ul.begin(); iter2 != ul.end(); ++iter2 )
                {
                    m->remove( s, idref, *iter2 );
                }
                m->commit();
            }
        }
        
        return ret;
    }
    
    virtual void rollback()
    {
        m_delegate->rollback();
    }
    
    
    
};


PD_DocumentRDFMutationHandle
RDFModel_XMLIDLimited::createMutation()
{
    UT_DEBUGMSG(("XMLIDLimited::createMutation()\n"));

    PD_DocumentRDFMutationHandle dmodel = m_delegate->createMutation();
    PD_DocumentRDFMutationHandle ret(
        new PD_RDFMutation_XMLIDLimited( dmodel->m_rdf,
                                         dmodel,
                                         m_writeID ));
    return ret;
}

    
PD_RDFModelHandle
PD_DocumentRDF::createRestrictedModelForXMLIDs( const std::string& writeID,
                                                const std::set< std::string >& xmlids )
{
    UT_DEBUGMSG(("createRestrictedModelForXMLIDs() writeID:%s xmlids.sz:%d\n",
                 writeID.c_str(), (int)xmlids.size() ));

    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFModelHandle  model = rdf;
    
    PD_RDFModelHandle ret(new RDFModel_XMLIDLimited( rdf, model, writeID, xmlids ));
    return ret;
}

PD_RDFModelHandle
PD_DocumentRDF::createRestrictedModelForXMLIDs( const std::set< std::string >& xmlids )
{
    std::string writeID = "";
    if( !xmlids.empty() )
        writeID = *(xmlids.begin());
    return createRestrictedModelForXMLIDs( writeID, xmlids );
}

PD_RDFSemanticItems
PD_DocumentRDF::getAllSemanticObjects( const std::string& classRestriction )
{
    PD_RDFSemanticItems ret;
    if( classRestriction.empty() || classRestriction == "Contact" )
    {
        PD_RDFContacts contacts = getContacts();
        copy( contacts.begin(), contacts.end(), back_inserter(ret));
    }
    
    if( classRestriction.empty() || classRestriction == "Event" )
    {
        PD_RDFEvents events = getEvents();
        copy( events.begin(), events.end(), back_inserter(ret));
    }
    
    if( classRestriction.empty() || classRestriction == "Location" )
    {
        PD_RDFLocations locations = getLocations();
        copy( locations.begin(), locations.end(), back_inserter(ret));
    }
    
    return ret;
}



PD_RDFSemanticItems
PD_DocumentRDF::getSemanticObjects( const std::set< std::string >& xmlids )
{
    PD_RDFSemanticItems ret;
    
    PD_RDFContacts contacts = getContacts();
    for( PD_RDFContacts::iterator ci = contacts.begin();
         ci != contacts.end(); ++ci )
    {
        PD_RDFContactHandle c = *ci;
        std::set< std::string > clist = c->getXMLIDs();
        std::set< std::string > tmp;
        std::set_intersection( xmlids.begin(), xmlids.end(),
                               clist.begin(), clist.end(),
                               inserter( tmp, tmp.end() ));
        if( !tmp.empty() )
        {
            ret.push_back( c );
        }
    }

    PD_RDFEvents events = getEvents();
    for( PD_RDFEvents::iterator ci = events.begin();
         ci != events.end(); ++ci )
    {
        PD_RDFEventHandle c = *ci;
        std::set< std::string > clist = c->getXMLIDs();
        std::set< std::string > tmp;
        std::set_intersection( xmlids.begin(), xmlids.end(),
                               clist.begin(), clist.end(),
                               inserter( tmp, tmp.end() ));
        if( !tmp.empty() )
        {
            ret.push_back( c );
        }
    }

    PD_RDFLocations locations = getLocations();
    for( PD_RDFLocations::iterator ci = locations.begin();
         ci != locations.end(); ++ci )
    {
        PD_RDFLocationHandle c = *ci;
        std::set< std::string > clist = c->getXMLIDs();
        std::set< std::string > tmp;
        std::set_intersection( xmlids.begin(), xmlids.end(),
                               clist.begin(), clist.end(),
                               inserter( tmp, tmp.end() ));
        if( !tmp.empty() )
        {
            ret.push_back( c );
        }
    }
    
    return ret;
}





void
PD_DocumentRDF::selectXMLIDs( const std::set< std::string >& xmlids, FV_View* pView ) const
{
    XAP_Frame* lff = XAP_App::getApp()->getLastFocussedFrame();
    if( !pView && lff )
        pView = static_cast<FV_View*>( lff->getCurrentView() );
    if( !pView )
        return;

    for( std::set< std::string >::const_iterator iter = xmlids.begin(); iter != xmlids.end(); ++iter )
    {
        const std::string& xmlid = *iter;
        std::pair< PT_DocPosition, PT_DocPosition > range = getIDRange( xmlid );
        UT_DEBUGMSG(("selectXMLIDs() id:%s begin:%d end:%d\n",
                     xmlid.c_str(), range.first, range.second ));
        if( range.first && range.second > range.first )
            pView->selectRange( range );
    }
}

void
PD_DocumentRDF::showEditorWindow( const PD_RDFSemanticItems & cl )
{
    if( !cl.empty() )
    {
        PD_RDFSemanticItems::const_iterator ci = cl.begin();
        PD_RDFSemanticItemHandle c = *ci;
        c->showEditorWindow( cl );
    }
}



PD_RDFContacts
PD_DocumentRDF::getContacts( PD_RDFModelHandle alternateModel )
{
    PD_RDFModelHandle m = alternateModel;
    if( !m )
        m = getDocument()->getDocumentRDF();

    PD_RDFContacts ret;
    std::stringstream sparqlQuery;
    sparqlQuery << "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                << "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
                << "prefix pkg: <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
                << "select distinct ?person ?name ?nick ?email ?homepage ?img ?phone \n"
                << "where { \n"
                << "    ?person rdf:type foaf:Person . \n"
                << "    ?person foaf:name ?name \n"
                << "    OPTIONAL { ?person foaf:phone ?phone } \n"
                << "    OPTIONAL { ?person foaf:mbox  ?email } \n"
                << "    OPTIONAL { ?person foaf:nick ?nick } \n"
                << "    OPTIONAL { ?person foaf:homepage ?homepage } \n"
                << "    OPTIONAL { ?person foaf:img ?img } \n"
                << "}\n";
    UT_DEBUGMSG(("getContacts() sparql:\n%s\n\n", sparqlQuery.str().c_str() ));

    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFQuery q( rdf, m );
    PD_ResultBindings_t bindings = q.executeQuery( sparqlQuery.str() );
    UT_DEBUGMSG(("getContacts() bindings.sz:%lu\n", (long unsigned)bindings.size() ));
    
    // uniqfilter is needed because redland might not honour the
    // DISTINCT sparql keyword
    std::set<std::string> uniqfilter;
    for( PD_ResultBindings_t::iterator it = bindings.begin(); it != bindings.end(); ++it )
    {
        std::string n = (*it)["name"];
        if (uniqfilter.count(n))
            continue;
        uniqfilter.insert(n);

        PD_RDFContact* newItem = PD_DocumentRDF::getSemanticItemFactory()->createContact( rdf, it );
        PD_RDFContactHandle h( newItem );
        ret.push_back( h );
    }
    
    return ret;
}


PD_RDFEvents
PD_DocumentRDF::getEvents( PD_RDFModelHandle alternateModel )
{
    PD_RDFModelHandle m = alternateModel;
    if( !m )
        m = getDocument()->getDocumentRDF();

    PD_RDFEvents ret;
    std::stringstream sparqlQuery;
    sparqlQuery << " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                << " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
                << " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
                << " select distinct ?ev ?uid ?dtstart ?dtend ?summary ?location ?description ?geo ?long ?lat \n"
                << " where {  \n"
                << "    ?ev rdf:type cal:Vevent . \n"
                << "    ?ev cal:uid      ?uid . \n"
                << "    ?ev cal:dtstart  ?dtstart . \n"
                << "    ?ev cal:dtend    ?dtend \n"
                << "    OPTIONAL { ?ev cal:summary  ?summary  } \n"
                << "    OPTIONAL { ?ev cal:location ?location } \n"
                << "    OPTIONAL { ?ev cal:description ?description } \n"
                << "    OPTIONAL {  \n"
                << "               ?ev cal:geo ?geo . \n"
                << "               ?geo rdf:first ?lat . \n"
                << "               ?geo rdf:rest ?joiner . \n"
                << "               ?joiner rdf:first ?long \n"
                << "              } \n"
                << "  } \n";
    
    UT_DEBUGMSG(("getEvents() sparql:\n%s\n\n", sparqlQuery.str().c_str() ));

    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFQuery q( rdf, m );
    PD_ResultBindings_t bindings = q.executeQuery( sparqlQuery.str() );
    UT_DEBUGMSG(("getEvents() bindings.sz:%lu\n", (long unsigned)bindings.size() ));
    
    // uniqfilter is needed because redland might not honour the
    // DISTINCT sparql keyword
    std::set<std::string> uniqfilter;
    for( PD_ResultBindings_t::iterator it = bindings.begin(); it != bindings.end(); ++it )
    {
        std::string n = (*it)["uid"];
        if (uniqfilter.count(n))
            continue;
        uniqfilter.insert(n);

        PD_RDFEvent* newItem = PD_DocumentRDF::getSemanticItemFactory()->createEvent( rdf, it );
        PD_RDFEventHandle h( newItem );
        ret.push_back( h );
    }
    
    return ret;
}


PD_RDFLocations&
PD_DocumentRDF::addLocations( PD_RDFLocations& ret,
                              bool isGeo84,
                              const std::string sparql,
                              PD_RDFModelHandle /*alternateModel*/ )
{
    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFQuery q( rdf, rdf );
    PD_ResultBindings_t bindings = q.executeQuery( sparql );
    UT_DEBUGMSG(("addLocations() bindings.sz:%lu sparql\n%s\n", (long unsigned)bindings.size(), sparql.c_str() ));
    std::set<std::string> uniqfilter;
    for( PD_ResultBindings_t::iterator it = bindings.begin(); it != bindings.end(); ++it )
    {
        std::string n = (*it)["lat"];
        if (uniqfilter.count(n))
            continue;
        uniqfilter.insert(n);
        UT_DEBUGMSG(("addLocations() n:%s\n", n.c_str() ));

#ifdef WITH_CHAMPLAIN
        PD_RDFLocation* newItem = PD_DocumentRDF::getSemanticItemFactory()->createLocation( rdf, it, isGeo84 );
        PD_RDFLocationHandle h( newItem );
        ret.push_back( h );
#else
    UT_UNUSED( isGeo84 );
#endif
    }
    return ret;
}


PD_RDFLocations
PD_DocumentRDF::getLocations( PD_RDFModelHandle alternateModel )
{
    PD_RDFLocations ret;
    addLocations( ret, false,
                  " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  \n"
                  " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
                  " prefix dc:   <http://purl.org/dc/elements/1.1/> \n"
                  " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
                  " select distinct ?geo ?long ?lat ?joiner ?desc \n"
                  " where {  \n"
                  "               ?ev cal:geo ?geo . \n"
                  "               ?geo rdf:first ?lat . \n"
                  "               ?geo rdf:rest ?joiner . \n"
                  "               ?joiner rdf:first ?long \n"
                  "               OPTIONAL { ?geo dc:title ?desc } \n"
                  "  } \n", alternateModel );
    UT_DEBUGMSG(( "getLocations(1) ret.size:%lu\n", (long unsigned)ret.size() ));
    
    addLocations( ret, true,
                  " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                  " prefix dc:   <http://purl.org/dc/elements/1.1/> \n"
                  " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
                  " prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
                  "  \n"
                  " select distinct ?geo ?long ?lat ?type ?desc \n"
                  " where {  \n"
                  "  \n"
                  "        ?geo geo84:lat  ?lat . \n"
                  "        ?geo geo84:long ?long \n"
                  "        OPTIONAL { ?geo rdf:type ?type } \n"
                  "        OPTIONAL { ?geo dc:title ?desc } \n"
                  "  \n"
                  " } \n", alternateModel );
    UT_DEBUGMSG(( "getLocations(2) ret.size:%lu\n", (long unsigned)ret.size() ));

    return ret;
}

/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/
/****************************************/

PD_DocumentRDFMutation::PD_DocumentRDFMutation( PD_DocumentRDF* rdf )
    : m_rdf(rdf)
    , m_rolledback(false)
    , m_committed(false)
    , m_handlingAbiCollabNotification( false )
    , m_pAP( 0 )
{
    m_pAP = m_rdf->getAP()->cloneWithReplacements(PP_NOPROPS, PP_NOPROPS, false);
    m_crRemoveAP = new PP_AttrProp();
    m_crAddAP    = new PP_AttrProp();

    UT_DEBUGMSG(("PD_DocumentRDF::ctor() this:%p rdf:%p\n", this, m_rdf));
}

PD_DocumentRDFMutation::~PD_DocumentRDFMutation()
{
    if( !m_committed )
        commit();
    
    if(m_pAP)
        delete m_pAP;
    if(m_crRemoveAP)
        delete m_crRemoveAP;
    if(m_crAddAP)
        delete m_crAddAP;
}

bool PD_DocumentRDFMutation::apAdd( PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    POCol l;
    const gchar* szName = s.toString().c_str();
    const gchar* szValue = 0;
	if( AP->getProperty( szName, szValue) )
    {
        l = decodePOCol(szValue);
    }

    // don't do this... it doesn't allow a (s,p) to have multiple objects at a global scale.
    // for( POCol::iterator iter = l.find( p ); iter != l.end(); iter = l.find( p ) )
    // {
    //     l.erase( iter );
    // }
    l.insert( std::make_pair( p, o ));
    std::string po = encodePOCol(l);
    return AP->setProperty( szName, po.c_str() );    
}

void PD_DocumentRDFMutation::apRemove( PP_AttrProp*& AP, const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    PP_AttrProp* newAP = new PP_AttrProp();
	size_t propCount = AP->getPropertyCount();
    for( size_t i = 0; i<propCount; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        
        if( !AP->getNthProperty( i, szName, szValue))
        {
            // failed to get old prop
            continue;
        }

        if( s.toString() == szName )
        {
            // drop this triple...
            POCol l = decodePOCol(szValue);
            std::pair< POCol::iterator, POCol::iterator > range
                = std::equal_range( l.begin(), l.end(), p );
            for( POCol::iterator iter = range.first; iter != range.second; )
            {
                if( iter->first == p && iter->second == o )
                {
                    POCol::iterator t = iter;
                    ++iter;
                    l.erase(t);
                    continue;
                }
                ++iter;
            }
            
            std::string po = encodePOCol(l);
            // commit() calls prune first, so this property
            // will disappear from the AP if it's list is empty
            if(l.empty())
                po = "";
            
            if( !newAP->setProperty( szName, po.c_str() ))
            {
                // FIXME: failed to copy prop
            }
            
            continue;
        }

        if( !newAP->setProperty( szName, szValue ))
        {
            // FIXME: failed to copy prop
        }
    }

    std::swap( AP, newAP );
    delete newAP;    
}


bool
PD_DocumentRDFMutation::add( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    UT_DEBUGMSG(("PD_DocumentRDFMutation::add(1) s:%s o:%s ot:%d\n",
                 s.toString().c_str(), o.toString().c_str(), o.getObjectType() ));
    // If it already exists and was not removed
    // then you can't add it again
    if( m_rdf->contains( s, p, o ) && !m_rdf->apContains( m_crRemoveAP, s, p, o ))
        return true;

    // If this mutation has added it but not already committed that addition
    // to the document then do not add it again either
    if( m_rdf->apContains( m_crAddAP, s, p, o ) && !m_rdf->apContains( m_crRemoveAP, s, p, o ))
        return true;

    UT_DEBUGMSG(("PD_DocumentRDFMutation::add(2) s:%s o:%s\n", s.toString().c_str(), o.toString().c_str() ));
    
    apAdd( m_pAP, s, p, o );
    apAdd( m_crAddAP, s, p, o );
    return true;
}

bool
PD_DocumentRDFMutation::add( const PD_URI& s, const PD_URI& p, const PD_Object& o, const PD_URI& /*context*/ )
{
    return add( s, p, o );
}


void
PD_DocumentRDFMutation::remove( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    UT_DEBUGMSG(("PD_DocumentRDFMutation::remove(1) s:%s p:%s o:%s ot:%d\n",
                 s.toString().c_str(), p.c_str(), o.toString().c_str(), o.getObjectType() ));
    
    apRemove( m_pAP, s, p, o );
    apRemove( m_crAddAP, s, p, o );
    apAdd( m_crRemoveAP, s, p, o );    
}

void
PD_DocumentRDFMutation::remove( const PD_URI& s, const PD_URI& p, const PD_URI& o )
{
    remove( s, p, PD_Object( o.toString() ));
}


bool
PD_DocumentRDFMutation::add( const PD_RDFStatement& st )
{
    return add( st.getSubject(), st.getPredicate(), st.getObject() );
}

PD_URI
PD_DocumentRDFMutation::createBNode()
{
    PD_Document* doc = m_rdf->getDocument();
    std::stringstream ss;
    ss << "uri:bnode" << doc->getUID( UT_UniqueId::Annotation );
    return PD_URI( ss.str() );
}

void
PD_DocumentRDFMutation::remove( const PD_RDFStatement& st )
{   
    remove( st.getSubject(), st.getPredicate(), st.getObject() );
}

void
PD_DocumentRDFMutation::remove( const std::list< PD_RDFStatement >& sl )
{
    for( std::list< PD_RDFStatement >::const_iterator si = sl.begin(); si!=sl.end(); ++si )
    {
        remove( *si );
    }
}

void
PD_DocumentRDFMutation::remove( const PD_URI& s, const PD_URI& p )
{
    PD_ObjectList objects = m_rdf->getObjects( s, p );
    std::list< PD_RDFStatement > removeList;
    for( PD_ObjectList::iterator it = objects.begin(); it != objects.end(); ++it )
    {
        PD_Object obj = *it;
        PD_RDFStatement st( s, p, obj );
        removeList.push_back( st );
    }
    remove( removeList );
}


int
PD_DocumentRDFMutation::add( PD_RDFModelHandle model )
{
    int count = 0;
    PD_RDFModelIterator iter = model->begin();
    PD_RDFModelIterator    e = model->end();
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;

        UT_DEBUGMSG(("PD_DocumentRDFMutation::add(submodel) ot:%d o:%s\n",
                     st.getObject().getObjectType(), st.getObject().toString().c_str()  ));
        
        if( add( st ) )
            ++count;
    }
    
    return count;
}


void
PD_DocumentRDFMutation::handleCollabEvent( gchar** szAtts, gchar** szProps )
{
    UT_DEBUGMSG(("PD_DocumentRDFMutation::handleCollabEvent (remote) rdf:%p\n", m_rdf));
    m_handlingAbiCollabNotification = true;
    
    PP_AttrProp* addAP    = new PP_AttrProp();
    PP_AttrProp* removeAP = new PP_AttrProp();
	addAP->setProperties( (const gchar**)szAtts );
	removeAP->setProperties( (const gchar**)szProps );
    handleAddAndRemove( addAP, removeAP );
    delete addAP;
    delete removeAP;
}

/**
 * This is where the triples in @add and @remove will actually modify
 * the DocumentRDF. This method is called by commit() locally and by
 * handleCollabEvent to handle remote change notificatioans during
 * collaborative sessions.
 *
 * First the current AttrProp is copied from the DocumentRDF,
 * filtering out the triples from the remove table as the copy is
 * performed. Then the new triples are added and the
 * piecetable/documentRDF is updated to use the new RDF APIndex.
 */
UT_Error
PD_DocumentRDFMutation::handleAddAndRemove( PP_AttrProp* add_, PP_AttrProp* remove_ )
{
//    UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove (general) rdf:%p\n", m_rdf));
//    m_rdf->apDumpModel( remove_, "remove from model" );
//    m_rdf->apDumpModel( add_, "add to model" );


    /*
     * remove all the triples we don't want first
     * doing this set wise should great less junk AP instances.
     */
    const PP_AttrProp* existingAP = m_rdf->getAP();
    PP_AttrProp* newAP = new PP_AttrProp();
	size_t propCount = existingAP->getPropertyCount();
    for( size_t i = 0; i<propCount; ++i )
    {
        const gchar * szExistingName = 0;
        const gchar * szExistingValue = 0;
        
        if( !existingAP->getNthProperty( i, szExistingName, szExistingValue))
        {
            UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove() failed to get prop:%ld\n", (long)i ));
            // failed to get old prop
            continue;
        }

        const gchar* szPropertiesToRemove = 0;
        if( remove_->getProperty( szExistingName, szPropertiesToRemove ))
        {
            POCol existingProps = decodePOCol(szExistingValue);
            POCol removeProps   = decodePOCol(szPropertiesToRemove);
//            UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove(1) n:%s exist.sz:%d rem.sz:%d\n",
//                         szExistingName, existingProps.size(), removeProps.size() ));
            for( POCol::iterator iter = removeProps.begin(); iter != removeProps.end(); ++iter )
            {
//                UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove(rem) %s\n", iter->first.c_str() ));
                std::pair< POCol::iterator, POCol::iterator > range
                    = std::equal_range( existingProps.begin(), existingProps.end(), iter->first );
                for( POCol::iterator t = range.first; t!=range.second; )
                {
                    if( t->second == iter->second )
                    {
                        POCol::iterator target = t;
                        t++;
                        existingProps.erase( target );
                        continue;
                    }
                    t++;
                }                
            }
            std::string po = encodePOCol(existingProps);
            // commit() calls prune first, so this property
            // will disappear from the AP if it's list is empty
            if(existingProps.empty())
                po = "";
//            UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove(2) n:%s exist.sz:%d rem.sz:%d\n",
//                         szExistingName, existingProps.size(), removeProps.size() ));
            if( !newAP->setProperty( szExistingName, po.c_str() ))
            {
                UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove() failed to set prop:%ld\n", (long)i ));
                // FIXME: failed to copy prop
            }            
        }
        else
        {
            UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove() copy prop:%s\n", szExistingName ));
            if( !newAP->setProperty( szExistingName, szExistingValue ))
            {
                // FIXME: failed to copy prop
            }
        }
    }

//    m_rdf->apDumpModel( newAP, "model after remove..." );
    
    // add all the new triples
	propCount = add_->getPropertyCount();
    for( size_t i = 0; i<propCount; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        
        if( !add_->getNthProperty( i, szName, szValue))
        {
            // FIXME: failed to get old prop
            continue;
        }
        PD_URI s(szName);
        POCol  c = decodePOCol(szValue);
        for( POCol::iterator iter = c.begin(); iter != c.end(); ++iter )
        {
            apAdd( newAP, s, iter->first, iter->second );
        }
    }

//    m_rdf->apDumpModel( newAP, "updated RDF model..." );
    UT_Error e = m_rdf->setAP( newAP );
    if( e != UT_OK )
    {
        UT_DEBUGMSG(("DocumentRDFMutation::handleAddAndRemove() failed to add new AttrPropIndex for RDF update\n"));
        // MIQ 2010 July: addIfUniqueAP() method states false == memory error of some kind.
        return e;
    }
    
//    m_rdf->apDumpModel( m_rdf->getAP(), "updated RDF model..." );
    return UT_OK;
}


/**
 * Commit the add() and remove() changes to the main document RDF.
 * This method also emits a change record detailing the update.
 *
 * Implementation: Note that at this stage we have m_crRemoveAP and
 * m_crAddAP which are the tripes to remove and add respectively.
 * These are also the triples that will be sent via abicollab to other
 * instances to add/remove so we might as well try to use the same
 * code to handle the mutation in both this and other abiword
 * instances.
 *
 * Note that you can only commit() once per PD_DocumentRDFMutation object.
 * So you should throw it away after a commit to avoid the temptation of
 * trying to add() more to it.
 */
UT_Error PD_DocumentRDFMutation::commit()
{
    bool success = false;
    
    UT_DEBUGMSG(("PD_DocumentRDF::commit(top1) this:%p rdf:%p\n", this, m_rdf));
    // UT_DEBUGMSG(("PD_DocumentRDF::commit(top2) m_rolledback:%d\n", m_rolledback));
    // UT_DEBUGMSG(("PD_DocumentRDF::commit(top3) rm.hasP:%d add.hasP:%d\n",
    //              m_crRemoveAP->hasProperties(), m_crAddAP->hasProperties()));
    // if( m_crAddAP->hasProperties() )
    // {
    //     UT_DEBUGMSG(("PD_DocumentRDF::commit(top3) add count:%d\n",
    //                  (int)m_crAddAP->getPropertyCount()));
    // }
    // m_rdf->apDumpModel( m_crAddAP, "xx add to model" );
    
    
    if(m_rolledback)
        return UT_OK;
    if( !m_crRemoveAP->hasProperties() && !m_crAddAP->hasProperties() )
        return UT_OK;
    if( m_handlingAbiCollabNotification )
        return UT_OK;
    if( m_committed )
        return UT_OK;
        
    UT_DEBUGMSG(("PD_DocumentRDF::commit(running) rdf:%p\n", m_rdf));
    m_pAP->prune();
    m_pAP->markReadOnly();
    PD_Document*    doc = m_rdf->getDocument();
    pt_PieceTable*   pt = m_rdf->getPieceTable();
    pt_VarSet& m_varset = pt->getVarSet();

    handleAddAndRemove( m_crAddAP, m_crRemoveAP );    
//    UT_DEBUGMSG(("PD_DocumentRDF::commit(sending CR) rdf:%p\n", m_rdf));

    if( !m_rdf->isStandAlone() )
    {
        
        //
        // Notify others about this change...
        //
        PP_AttrProp* crAP = new PP_AttrProp();
        crAP->setAttributes( m_crAddAP->getProperties() );
        crAP->setProperties( m_crRemoveAP->getProperties() );
        crAP->markReadOnly();
    
        PT_AttrPropIndex crAPI = 0;
        success = m_varset.addIfUniqueAP( crAP, &crAPI );
        // addIfUniqueAP() eats it
        crAP = 0;

        if( !success )
        {
            UT_DEBUGMSG(("DocumentRDFMutation::commit() failed to add new AttrPropIndex for RDF update\n"));
            // MIQ 2010 July: addIfUniqueAP() method states false == memory error of some kind.
            return UT_OUTOFMEM;
        }
    
        PT_DocPosition pos = 0;
        UT_uint32 iXID = 0;
        PX_ChangeRecord* pcr = new PX_ChangeRecord( PX_ChangeRecord::PXT_ChangeDocRDF,
                                                    pos, crAPI, iXID );
        doc->notifyListeners( 0, pcr );
        delete pcr;
//        UT_DEBUGMSG(("PD_DocumentRDF::commit(done) rdf:%p\n", m_rdf));
    }

    m_committed = true;
    m_rdf->maybeSetDocumentDirty();
    m_rdf->updateHaveSemItemsCache();
    
    return UT_OK;
}

/**
 * Forget the changes and do not automatically try to commit() them
 * when the PD_DocumentRDFMutation goes out of scope.
 */
void PD_DocumentRDFMutation::rollback()
{
    m_rolledback = true;
}


