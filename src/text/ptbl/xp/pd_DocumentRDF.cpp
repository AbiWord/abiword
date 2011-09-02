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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "pd_DocumentRDF.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"
#include "ut_debugmsg.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "ut_std_string.h"

#include <sstream>
#include <set>

#include <iostream>
using std::cerr;
using std::endl;
using std::make_pair;
using std::string;

#include "config.h"
#include "pd_RDFQuery.h"


#define DEBUG_LOWLEVEL_IO    0

typedef std::map< std::string, std::string > stringmap_t;


static void dump( const std::string& msg, PD_RDFModelIterator begin, PD_RDFModelIterator end );


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


/**
 * Get an object which has the given subject and predicate. If there
 * are more than one such object, any one of those will be returned.
 * So you should only use this method when the RDF only allows a
 * single matching triple.
 */
PD_URI
PD_RDFModel::getObject( const PD_URI& s, const PD_URI& p )
{
    PD_URIList l = getObjects(s,p);
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

PD_URIList
PD_RDFModel::getObjects( const PD_URI& s, const PD_URI& p )
{
    PD_URIList ret;
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
            ret.insert( make_pair( st.getPredicate(), st.getObject() ));
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
    int colonLocation = prefixedstr.find(":");
    if( colonLocation != string::npos )
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

    if( DEBUG_LOWLEVEL_IO )
    {
        int loc = iss.tellg();
        UT_DEBUGMSG(("PD_DocumentRDF::readLengthPrefixedString() len:%d loc:%d\n", len,loc));
    }
    
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
static std::string createLengthPrefixedString( const PD_URI& u )
{
    return createLengthPrefixedString(u.toString());
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

int PD_URI::length() const
{
    return m_value.length();
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

PD_URI
PD_URI::prefixedToURI( PD_RDFModelHandle model ) const
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

bool operator<( PD_URI a, PD_URI b)
{
    return a.toString() < b.toString();
}

bool operator<( std::pair< PD_URI, PD_URI > a, PD_URI b)
{
    return a.first.toString() < b.toString();
}

bool operator<( PD_URI a, std::pair< PD_URI, PD_URI > b )
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
       << " o:" << m_object.toString() << " ";
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
        cerr << "ERROR: getAP() is not valid for a start-end position rdf model" << endl;
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
                std::string obj  = m_pocoliter->second.toString();
                m_current = PD_RDFStatement( m_subject, pred, PD_Object(obj) );
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
            std::string obj  = m_pocoliter->second.toString();
            m_current = PD_RDFStatement( m_subject, pred, PD_Object(obj) );
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

    virtual PD_URIList getObjects( const PD_URI& s, const PD_URI& p )
    {
        PD_URIList ret;
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apGetObjects( *iter, ret, s, p );
        return ret;
    }
    
    virtual PD_URI getObject( const PD_URI& s, const PD_URI& p )
    {
        PD_URIList l = getObjects(s,p);
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
        UT_DEBUGMSG(("PD_RDFModelFromStartEndPos::dumpModel() doc:%p\n", m_doc));
        for( m_APList_t::iterator iter = apBegin(); iter != apEnd(); ++iter )
            apDumpModel( *iter, headerMsg );
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
    cerr << "~PD_RDFModelIterator() model:" << m_model << endl;
}

PD_RDFModelIterator::PD_RDFModelIterator( PD_RDFModelHandle model, const PP_AttrProp* AP )
    : m_model( model )
    , m_AP( AP )
    , m_end( false )
    , m_apPropertyNumber( 0 )
{
    cerr << "PD_RDFModelIterator() model:" << model << endl;
    operator++();
}

void
PD_RDFModelIterator::setup_pocol()
{
    const gchar * szName  = 0;
    const gchar * szValue = 0;
    if( m_AP->getNthProperty( m_apPropertyNumber, szName, szValue) )
    {
//        UT_DEBUGMSG(("statement iter++/setup_pocol szName :%s\n", szName ));
//        UT_DEBUGMSG(("statement iter++/setup_pocol szValue:%s\n", szValue ));
        m_subject   = szName;
        m_pocol     = decodePOCol( szValue );
        if( m_pocol.empty() )
            return;
                
        m_pocoliter = m_pocol.begin();

        std::string pred = m_pocoliter->first.toString();
        std::string obj  = m_pocoliter->second.toString();
        m_current = PD_RDFStatement( m_subject, pred, PD_Object(obj) );
    }
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
    std::string obj  = m_pocoliter->second.toString();
    m_current = PD_RDFStatement( m_subject, pred, PD_Object(obj) );
    ++m_pocoliter;
    if( m_pocoliter == m_pocol.end() )
    {
        m_pocol.clear();
    }
            
    return *this;
}

bool
PD_RDFModelIterator::operator==( const self_reference other )
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
        cerr << "PD_RDFModelIterator op=() model:" << m_model << " r.model:" << r.m_model << endl;
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


PD_DocumentRDF::PD_DocumentRDF( PD_Document* doc )
    :
    m_doc( doc ),
    m_indexAP( 0 )
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
    for( string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
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


std::string
PD_DocumentRDF::getSPARQL_LimitedToXMLIDList( const std::list< std::string >& xmlids,
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
    for( std::list< std::string >::const_iterator iter = xmlids.begin();
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
PD_URIList PD_DocumentRDF::getObjects( const PD_URI& s, const PD_URI& p )
{
    PD_URIList ret;
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
PD_URIList&
PD_DocumentRDF::apGetObjects( const PP_AttrProp* AP, PD_URIList& ret, const PD_URI& s, const PD_URI& p )
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
 * dpindoing it in a loop.
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
    UT_DEBUGMSG(("docrdf::begin() ap:%p\n", getAP() ));

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
PD_DocumentRDF::getIDRange( const std::string& xmlid )
{
    std::pair< PT_DocPosition, PT_DocPosition > ret = make_pair( 0, 0 );
    PD_Document*    doc = getDocument();
    pf_Frag *	   iter = doc->getFragFromPosition(0);

    UT_DEBUGMSG(( "getIDRange() iter starting at:%p\n", iter ));
    
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
            return make_pair( pf->getPos(), epos );
        }
    }
    
    return ret;
}



std::list< std::string >&
PD_DocumentRDF::addRelevantIDsForPosition( std::list< std::string >& ret,
                                                                     PT_DocPosition pos )
{
    PD_Document*    doc = getDocument();
    pt_PieceTable*   pt = getPieceTable();
    PT_DocPosition curr = pos;
    
    UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForPosition() current:%d \n", curr ));

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
    for( ; curr > 0; --curr )
    {
        pf_Frag* pf = 0;
        PT_BlockOffset boffset;

        if( pt->getFragFromPosition( curr, &pf, &boffset ) )
        {
            UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForPosition() current:%d frag type:%d len:%d \n",
                         curr, pf->getType(), pf->getLength() ));

            if(pf->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object* pOb = static_cast<pf_Frag_Object*>(pf);
                const PP_AttrProp * pAP = NULL;

                UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForPosition() po type:%d\n",
                             pOb->getObjectType() ));

                if(pOb->getObjectType() == PTO_Bookmark)
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
                    const char* v = 0;
                    if( pAP->getAttribute(PT_XMLID, v) && v)
                    {
                        std::string xmlid = v;
                        bool isEnd = pAP->getAttribute("type", v) && v && !strcmp(v,"end");
                        
                        UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForPosition() isEnd:%d id:%s\n",
                                     isEnd, xmlid.c_str() ));
                        
                        if( isEnd )
                        {
                            m_ignoreIDSet.insert( xmlid );
                        }
                        else
                        {
                            if( !m_ignoreIDSet.count( xmlid ))
                                ret.push_back( xmlid );
                        }
                    }
                }
                
                if(pOb->getObjectType() == PTO_RDFAnchor)
                {
                    pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);

                    RDFAnchor a(pAP);
                    UT_DEBUGMSG(("PD_DocumentRDF::addRelevantIDsForPosition() isEnd:%d id:%s\n",
                                 a.isEnd(), a.getID().c_str() ));
                    
                    if( a.isEnd() )
                    {
                        m_ignoreIDSet.insert( a.getID() );
                    }
                    else
                    {
                        if( !m_ignoreIDSet.count( a.getID() ))
                            ret.push_back( a.getID() );
                    }
                }
            }
        }
    }

    //
    // xml:id attached to containing paragraph/header
    // <text:p> / <text:h>
    //
    PL_StruxFmtHandle psfh;
    if( pt->getStruxOfTypeFromPosition( pos, PTX_Block, &psfh ) && psfh )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() block pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psfh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() xmlid:%s \n",v));
                ret.push_back(v);
            }
        }
    }

    //
    // xml:id attached to containing table:table-cell
    //
    if( pt->getStruxOfTypeFromPosition( pos, PTX_SectionCell, &psfh ) && psfh )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() cell pos:%d\n", pos ));
        PT_AttrPropIndex api = doc->getAPIFromSDH( psfh );
        const PP_AttrProp * AP = NULL;
        doc->getAttrProp(api,&AP);
        if( AP )
        {
            const char * v = NULL;
            if(AP->getAttribute("xml:id", v))
            {
                UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() xmlid:%s \n",v));
                ret.push_back(v);

                if(AP->getAttribute("props", v))
                    UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() props:%s \n",v));

            }
        }
    }
    
    return ret;
}


PD_RDFModelHandle PD_DocumentRDF::getRDFAtPosition( PT_DocPosition pos )
{
    PD_Document*    doc = getDocument();

    std::list< std::string > IDList;
    addRelevantIDsForPosition( IDList, pos );

    PP_AttrProp* AP = new PP_AttrProp();
    PD_RDFModelFromAP* retModel = new PD_RDFModelFromAP( doc, AP );
    PD_RDFModelHandle ret( retModel );

    if( !IDList.empty() )
    {
        PD_DocumentRDFMutationHandle m = retModel->createMutation();
        for( std::list< std::string >::iterator iter = IDList.begin();
             iter != IDList.end(); ++iter )
        {
            std::string xmlid = *iter;
            addRDFForID( xmlid, m );
        } 
        m->commit();
    }
    

    retModel->dumpModel("RDF result for xmlid");
    return ret;
    
    
    // if( pt->getStruxOfTypeFromPosition( pos, PTX_Block, &psfh ) && psfh )
    // {
    //     UT_DEBUGMSG(("PD_DocumentRDF::getRDFAtPosition() pos:%d\n", pos ));
    //     PT_AttrPropIndex api = doc->getAPIFromSDH( psfh );
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

    {
        // return an empty model
        PD_RDFModelHandle x( new PD_RDFModelFromAP( doc, new PP_AttrProp() ));
        return x;
    }
    
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

    retModel->dumpModel("RDF result for xmlid");
    return ret;
}








void PD_DocumentRDF::runMilestone2Test()
{
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
}

static void dump( const std::string& msg, PD_RDFModelIterator iter, PD_RDFModelIterator e )
{
    int count = 0;
    UT_DEBUGMSG(("dump(top) msg::%s\n", msg.c_str() ));
    for( ; iter != e; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        UT_DEBUGMSG((" st:%s\n", st.toString().c_str() ));
        ++count;
    }
    UT_DEBUGMSG(("dump(end) count:%d msg::%s\n", count, msg.c_str() ));
}

void PD_DocumentRDF::runPlay()
{
    UT_DEBUGMSG(("PD_DocumentRDF::runPlay() o:%s\n", "foo" ));

    int count = 0;
    typedef PD_RDFModelIterator iterator;
    dump( "whole model", begin(), end() );
    
    UT_DEBUGMSG(("runPlay() triple count:%d\n", count ));
    UT_DEBUGMSG(("runPlay()\n\n"));

    PD_RDFModelHandle m = getRDFForID( "wingb" );
    dump( "wingb", m->begin(), m->end() );
    
}


void PD_DocumentRDF::runMilestone2Test2()
{
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
    
    
    
}


void PD_DocumentRDF::dumpObjectMarkersFromDocument()
{
    UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() doc:%p\n", m_doc));
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
        PL_StruxFmtHandle psfh;

        if( pt->getStruxOfTypeFromPosition( curr, PTX_Block, &psfh ) && psfh )
        {
            UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() current:%d end:%d have PTX_BLOCK \n",
                         curr, eod ));
	        PT_AttrPropIndex api = doc->getAPIFromSDH( psfh );
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
                    if(pAP->getAttribute("this-is-an-rdf-anchor", v) && v)
                        UT_DEBUGMSG(("PD_DocumentRDF::dumpObjectMarkersFromDocument() is-rdf-a:%s\n",v));
                    
                }
            }
            
        }
    }


    //////////////

    curr = 420;
    PD_RDFModelHandle h = getRDFAtPosition( curr );
    
    
}


/**
 * Dump the model as debug messages
 */
void PD_DocumentRDF::dumpModel( const std::string& headerMsg )
{
    UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() doc:%p\n", m_doc));
    apDumpModel( getAP(), headerMsg );
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
            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() szName :%s\n", szName ));
            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() szValue:%s\n", szValue ));
            
            POCol l = decodePOCol( szValue );
            UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() po list size:%d\n", (int)l.size() ));
            std::string subj = szName;
            for( POCol::iterator iter = l.begin(); iter!=l.end(); ++iter )
            {
                std::string pred = iter->first.toString();
                std::string obj  = iter->second.toString();
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel() s:%s\n",
                             subj.c_str()));
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()     p:%s\n",
                             pred.c_str()));
                UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()     o:%s\n",
                             obj.c_str()));
                if( iter->second.hasXSDType() )
                {
                    UT_DEBUGMSG(("PD_DocumentRDF::dumpModel()  type:%s\n",
                                 iter->second.getXSDType().c_str()));
                }
            }
        }
    }
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
    void update();
    
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
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() our   version:%d\n", getVersion() ));
    UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() their version:%d\n", m_delegate->getVersion() ));

    if( getVersion() >= m_delegate->getVersion() )
    {
        UT_DEBUGMSG(("RDFModel_SPARQLLimited::update() nothing to be done...\n" ));
        return;
    }
    
    PP_AttrProp* AP = new PP_AttrProp();
    PD_RDFQuery q( m_rdf, m_delegate );
    PD_ResultBindings_t bindings = q.executeQuery( getSparql() );

    for( PD_ResultBindings_t::iterator iter = bindings.begin(); iter != bindings.end(); ++iter )
    {
        std::map< std::string, std::string > d = *iter;
        
        PD_URI    s( d["s"] );
        PD_URI    p( d["p"] );
        PD_Object o( d["o"] );

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
}

/****************************************/
/****************************************/
/****************************************/


class RDFModel_XMLIDLimited
    :
    public RDFModel_SPARQLLimited
{
    std::string m_writeID;
    std::list< std::string > m_readIDList;
    
public:
    
    RDFModel_XMLIDLimited( PD_DocumentRDFHandle rdf,
                           PD_RDFModelHandle delegate,
                           const std::string& writeID,
                           const std::list< std::string >& readIDList )
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


std::string
RDFModel_XMLIDLimited::getSparql()
{
    std::list< std::string > xmlids;
    xmlids.push_back( m_writeID );
    copy( m_readIDList.begin(), m_readIDList.end(), back_inserter( xmlids ) );
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
        if( !m_rdf->contains( rdflink ))
        {
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
                   << "prefix fqoaf: <http://xmlns.com/foaf/0.1/>  \n"
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

                PD_URIList ul = rdf->getObjects( subj, idref );
                PD_DocumentRDFMutationHandle m = rdf->createMutation();
                for( PD_URIList::iterator iter = ul.begin(); iter != ul.end(); ++iter )
                {
                    m->remove( s, idref, *iter );
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
                                                const std::list< std::string >& xmlids )
{
    UT_DEBUGMSG(("createRestrictedModelForXMLIDs() writeID:%s xmlids.sz:%d\n",
                 writeID.c_str(), (int)xmlids.size() ));

    PD_DocumentRDFHandle rdf = getDocument()->getDocumentRDF();
    PD_RDFModelHandle  model = rdf;
    
    PD_RDFModelHandle ret(new RDFModel_XMLIDLimited( rdf, model, writeID, xmlids ));
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
    m_pAP = m_rdf->getAP()->cloneWithReplacements( 0, 0, false );
    m_crRemoveAP = new PP_AttrProp();
    m_crAddAP    = new PP_AttrProp();
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


bool PD_DocumentRDFMutation::add( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
    // If it already exists and was not removed
    // then you can't add it again
    if( m_rdf->contains( s, p, o ) && !m_rdf->apContains( m_crRemoveAP, s, p, o ))
        return false;
    
    apAdd( m_pAP, s, p, o );
    apAdd( m_crAddAP, s, p, o );
    apRemove( m_crRemoveAP, s, p, o );
    return true;
}

void
PD_DocumentRDFMutation::remove( const PD_URI& s, const PD_URI& p, const PD_Object& o )
{
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

void
PD_DocumentRDFMutation::remove( const PD_RDFStatement& st )
{
    remove( st.getSubject(), st.getPredicate(), st.getObject() );
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
PD_DocumentRDFMutation::handleAddAndRemove( PP_AttrProp* add, PP_AttrProp* remove )
{
    UT_DEBUGMSG(("PD_DocumentRDFMutation::handleAddAndRemove (general) rdf:%p\n", m_rdf));
    m_rdf->apDumpModel( remove, "remove from model" );
    m_rdf->apDumpModel( add, "add to model" );


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
            // failed to get old prop
            continue;
        }

        const gchar* szPropertiesToRemove = 0;
        if( remove->getProperty( szExistingName, szPropertiesToRemove ))
        {
            POCol existingProps = decodePOCol(szExistingValue);
            POCol removeProps   = decodePOCol(szPropertiesToRemove);
            for( POCol::iterator iter = removeProps.begin(); iter != removeProps.end(); ++iter )
            {
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
            if( !newAP->setProperty( szExistingName, po.c_str() ))
            {
                // FIXME: failed to copy prop
            }            
        }
        else
        {
            if( !newAP->setProperty( szExistingName, szExistingValue ))
            {
                // FIXME: failed to copy prop
            }
        }
    }
    
    // add all the new triples
	propCount = add->getPropertyCount();
    for( size_t i = 0; i<propCount; ++i )
    {
        const gchar * szName = 0;
        const gchar * szValue = 0;
        
        if( !add->getNthProperty( i, szName, szValue))
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

    UT_Error e = m_rdf->setAP( newAP );
    if( e != UT_OK )
    {
        UT_DEBUGMSG(("DocumentRDFMutation::handleAddAndRemove() failed to add new AttrPropIndex for RDF update\n"));
        // MIQ 2010 July: addIfUniqueAP() method states false == memory error of some kind.
        return e;
    }
    
    m_rdf->apDumpModel( m_rdf->getAP(), "updated RDF model..." );
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
    
    UT_DEBUGMSG(("PD_DocumentRDF::commit(top1) rdf:%p\n", m_rdf));
    UT_DEBUGMSG(("PD_DocumentRDF::commit(top2) m_rolledback:%d\n", m_rolledback));
    UT_DEBUGMSG(("PD_DocumentRDF::commit(top3) rm.hasP:%d add.hasP:%d\n",
                 m_crRemoveAP->hasProperties(), m_crAddAP->hasProperties()));
    if( m_crAddAP->hasProperties() )
    {
        UT_DEBUGMSG(("PD_DocumentRDF::commit(top3) add count:%d\n",
                     (int)m_crAddAP->getPropertyCount()));
    }
    
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
    UT_DEBUGMSG(("PD_DocumentRDF::commit(sending CR) rdf:%p\n", m_rdf));

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
        UT_DEBUGMSG(("PD_DocumentRDF::commit(done) rdf:%p\n", m_rdf));
    }

    m_committed = true;
    m_rdf->maybeSetDocumentDirty();
    
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




