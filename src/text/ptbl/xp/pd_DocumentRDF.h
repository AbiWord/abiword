/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2010 GPL. V2+ copyright to AbiSource B.V. 
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

#ifndef PD_DOCUMENTRDF_H
#define PD_DOCUMENTRDF_H

#include <string>
#include <list>
#include <map>
#include <set>
#include "ut_types.h"
#include "pt_Types.h"

#include <boost/shared_ptr.hpp>

class PD_Document;
class pt_PieceTable;
class PP_AttrProp;
class RDFModel_SPARQLLimited;
class pf_Frag;


class   PD_DocumentRDFMutation;
typedef boost::shared_ptr<PD_DocumentRDFMutation> PD_DocumentRDFMutationHandle;
class   PD_RDFModel;
typedef boost::shared_ptr<PD_RDFModel> PD_RDFModelHandle;
class   PD_DocumentRDF;
typedef boost::shared_ptr<PD_DocumentRDF> PD_DocumentRDFHandle;


/**
 * An RDF URI. This can be thought of as a std::string
 * like object, but explicit support for namespaces and
 * other RDF functionality might be added over time.
 * For example, foaf:name mapping to the foaf URI vocab.
 * 
 */
class ABI_EXPORT PD_URI
{
  protected:
    std::string m_value;
    
  public:
    PD_URI( const std::string& v = "" );
    virtual std::string toString() const;
    int  length() const;
    bool isValid() const;
    bool operator==(const PD_URI& b) const;
    bool operator==(const std::string& b) const;

    virtual bool read( std::istream& ss );
    virtual bool write( std::ostream& ss ) const;

    PD_URI prefixedToURI( PD_RDFModelHandle model ) const;
};

template <class ostream>
ostream& operator<<( ostream& ss, PD_URI& uri )
{
    ss << uri.toString();
    return ss;
}



/**
 * An RDF Object is either a URI, a bnode, or a Literal value. While inheritance
 * is not strictly correct for this relation, since the main features of
 * a PD_URI are to get a string value, if the Object is a literal it can
 * return it's literal value via toString().
 *
 * The main additions for an Object are the type information of the
 * literal and also the context if the Object forms part of a triple.
 *
 * There are two types for an Object, use getObjectType() to see if it
 * is a URI, LITERAL, BNODE etc. Use getXSDType() to get the XSD
 * schema type for the literal. While using getXSDType() for a URI or
 * BNODE will result in "", the object type is still needed to
 * differentiate properly between URI and BNodes and makes this
 * explicit.
 * 
 */
class ABI_EXPORT PD_Object : public PD_URI
{
  public:
    enum
    {
        OBJECT_TYPE_URI = 1,
        OBJECT_TYPE_LITERAL,
        OBJECT_TYPE_BNODE
    };
    
    PD_Object( const std::string& v = "" );
    PD_Object( const std::string& v, int objectType, const std::string& xsdtype = "" );

    std::string getXSDType() const;
    bool hasXSDType() const;
    int  getObjectType() const;
    bool isLiteral() const;
    bool isURI() const;
    bool isBNode() const;
    
    virtual bool read( std::istream& ss );
    virtual bool write( std::ostream& ss ) const;

  protected:
    std::string m_xsdType;
    std::string m_context;
    int m_objectType;
};
/**
 * Convenience class for creating Literal values. Note that object
 * slicing is OK here because there are no local member variables in
 * this class and the type is explicitly preserved in m_objectType in
 * the parent class.
 */
class ABI_EXPORT PD_Literal : public PD_Object
{
  public:
    PD_Literal( const std::string& v = "", const std::string& xsdtype = "" );
};

typedef std::list< PD_URI > PD_URIList;
// REQUIRES: ordered, same key can -> many different values
typedef std::multimap< PD_URI, PD_Object > POCol;
typedef std::list< PD_Object > PD_ObjectList;

struct PD_URIListCompare : public std::binary_function<PD_URI, PD_URI, bool> {
	bool operator()(PD_URI x, PD_URI y) { return x.toString() < y.toString(); }
};


/**
 * When iterating over the RDF triples it is nice to have a single
 * C++ object which represents the while triple.
 */
class ABI_EXPORT PD_RDFStatement
{
    PD_URI    m_subject;
    PD_URI    m_predicate;
    PD_Object m_object;
    bool      m_isValid;
  public:
    PD_RDFStatement();
    PD_RDFStatement( PD_RDFModelHandle model, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    PD_RDFStatement( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    PD_RDFStatement( const std::string& s, const std::string& p, const PD_Object& o );
    PD_RDFStatement( const std::string& s, const std::string& p, const PD_Literal& o );
    const PD_URI&    getSubject() const;
    const PD_URI&    getPredicate() const;
    const PD_Object& getObject() const;
    bool             isValid() const;
    std::string      toString() const;

    PD_RDFStatement uriToPrefixed( PD_RDFModelHandle model ) const;
    PD_RDFStatement prefixedToURI( PD_RDFModelHandle model ) const;

    bool operator==(const PD_RDFStatement& b) const;
    
};

template <class ostream>
ostream& operator<<( ostream& ss, const PD_RDFStatement& st )
{
    ss << st.toString();
    return ss;
}




class ABI_EXPORT PD_RDFModelIterator
    :
    public std::iterator< std::forward_iterator_tag, PD_RDFStatement >
{
    PD_RDFModelHandle   m_model;
    const PP_AttrProp*  m_AP;
    bool                m_end;
    size_t              m_apPropertyNumber;
    std::string         m_subject;
    POCol               m_pocol;
    POCol::iterator     m_pocoliter;
    PD_RDFStatement     m_current;

    void setup_pocol();
    
  public:
    typedef PD_RDFModelIterator& self_reference;
    typedef PD_RDFModelIterator  self_type;
    
    PD_RDFModelIterator();
    ~PD_RDFModelIterator();
    PD_RDFModelIterator( PD_RDFModelHandle model, const PP_AttrProp* AP );
    
    self_reference operator++();
    bool operator==( const self_reference other );
    PD_RDFModelIterator& operator=( const PD_RDFModelIterator& other );

    self_type operator++(int)
    {
        self_type result( *this );
        ++( *this );
        return result;
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


/**
 * An RDF Model is an abstract immutable collection of RDF triples.
 * You can query if the model contains a statement, and navigate
 * around by supplying one or more parts of a triple.
 *
 * Having the RDF Model abstract like this allows slices of the whole
 * RDF to be returned in various places. For example, to get all the
 * RDF that is explicitly "related" to a location in the document.
 *
 * To make implementing custom models simpler many methods have
 * defaults which rely on other methods. For example, the getObject()
 * method simply returns the first object from the getObjects()
 * method. Many of the default implementations rely on begin() and
 * end() and simply loop over all the RDF to find the result. Compare
 * the linear performance of the default to the contant time
 * performance which might be possible for contains(s,p,o) if the
 * model uses more knowledge.
 * 
 * The uriToPrefixed() and prefixedToURI() rely on getUriToPrefix()
 * returning a suitable prefix map.
 *
 * Methods with default implementations remain virtual because a
 * subclass might have a more efficient method of finding the result
 * than the default implementation.
 */
class ABI_EXPORT PD_RDFModel
{
  protected:
    long m_version;
    PD_URI    front( const PD_URIList& l    ) const;
    PD_Object front( const PD_ObjectList& l ) const;
    PD_RDFModel();
    void incremenetVersion();
    
  public:    
    virtual PD_ObjectList getObjects( const PD_URI& s, const PD_URI& p );
    virtual PD_Object     getObject( const PD_URI& s, const PD_URI& p );
    virtual PD_URIList getSubjects( const PD_URI& p, const PD_Object& o );
    virtual PD_URI     getSubject( const PD_URI& p, const PD_Object& o );
    virtual PD_URIList getAllSubjects();
    virtual POCol      getArcsOut( const PD_URI& s );
    virtual bool       contains( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    virtual bool       contains( const PD_URI& s, const PD_URI& p );
    virtual bool       contains( const PD_RDFStatement& st );
    virtual long       getTripleCount();

    virtual PD_RDFModelIterator begin() = 0;
    virtual PD_RDFModelIterator end() = 0;

    typedef std::map< std::string, std::string > stringmap_t;
    typedef stringmap_t uriToPrefix_t;
    virtual uriToPrefix_t& getUriToPrefix();

    
    virtual void dumpModel( const std::string& headerMsg = "dumpModel()" );

    virtual PD_DocumentRDFMutationHandle createMutation() = 0;
    virtual std::string uriToPrefixed( const std::string& uri );
    virtual std::string prefixedToURI( const std::string& prefixed );
    
    inline long size() { return getTripleCount(); }

    long getVersion() const { return m_version; }
    
};


/**
 * The RDF associated with a document.
 *
 * The RDFModel interface can be used to query all the RDF for this
 * document. Use createMutation() to create an object which allows
 * you to update the RDF in batch.
 *
 * To find the RDF which is associated with an element use
 * getRDFAtPosition() and getRDFForID() which return a submodel.
 * 
 */
class ABI_EXPORT PD_DocumentRDF : public PD_RDFModel
{
    friend class PD_DocumentRDFMutation;
    friend class RDFModel_SPARQLLimited;
    friend class PD_RDFMutation_XMLIDLimited;
    
  public:
    explicit PD_DocumentRDF( PD_Document* doc );
    virtual ~PD_DocumentRDF();

    UT_Error setupWithPieceTable();
    
    // PD_RDFModel methods...
    virtual PD_ObjectList getObjects( const PD_URI& s, const PD_URI& p );
    virtual PD_URIList getSubjects( const PD_URI& p, const PD_Object& o );
    virtual PD_URIList getAllSubjects();
    virtual POCol      getArcsOut( const PD_URI& s );
    virtual bool       contains( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    virtual bool       contains( const PD_RDFStatement& st );
    virtual long       getTripleCount();
    virtual PD_RDFModelIterator begin();
    virtual PD_RDFModelIterator end();
    virtual void dumpModel( const std::string& headerMsg = "dumpModel()" );
    virtual PD_DocumentRDFMutationHandle createMutation();

    
    void handleCollabEvent( gchar** szAtts, gchar** szProps );
         
    PD_RDFModelHandle getRDFAtPosition( PT_DocPosition pos );
    PD_RDFModelHandle getRDFForID( const std::string& xmlid );

    void addRDFForID( const std::string& xmlid, PD_DocumentRDFMutationHandle& m );
    std::list< std::string >& addRelevantIDsForPosition( std::list< std::string >& ret,
                                                         PT_DocPosition pos );
    std::list< std::string >& addRelevantIDsForRange( std::list< std::string >& ret,
                                                      PD_DocumentRange* range );
    std::list< std::string >& addRelevantIDsForRange( std::list< std::string >& ret,
                                                      std::pair< PT_DocPosition, PT_DocPosition > range );
    
    std::set< std::string >& getAllIDs( std::set< std::string >& ret );
    std::pair< PT_DocPosition, PT_DocPosition > getIDRange( const std::string& xmlid );


    PD_RDFModelHandle createRestrictedModelForXMLIDs( const std::string& writeID,
                                                      const std::list< std::string >& xmlids );
    PD_RDFModelHandle createRestrictedModelForXMLIDs( const std::list< std::string >& xmlids );

    virtual void maybeSetDocumentDirty();
    
    // testing methods...
    void runMilestone2Test();
    void runMilestone2Test2();
    void dumpObjectMarkersFromDocument();
    void runPlay();

    static std::string getSPARQL_LimitedToXMLIDList( const std::list< std::string >& xmlids,
                                                     const std::string& extraPreds = "" );

    std::string makeLegalXMLID( const std::string& s );
    void relinkRDFToNewXMLID( const std::string& oldxmlid, const std::string& newxmlid, bool deepCopyRDF = false );

  protected:
    PD_Document* m_doc;
  private:
	PT_AttrPropIndex m_indexAP;

    PD_Document*   getDocument(void) const;
    pt_PieceTable* getPieceTable(void) const;
    void setIndexAP( PT_AttrPropIndex idx );
    PT_AttrPropIndex getIndexAP(void) const;
    virtual const PP_AttrProp* getAP(void);
    virtual UT_Error setAP( PP_AttrProp* newAP );
    virtual bool isStandAlone() const;
    
  protected:
    PD_ObjectList& apGetObjects(     const PP_AttrProp* AP, PD_ObjectList& ret, const PD_URI& s, const PD_URI& p );
    PD_URIList&    apGetSubjects(    const PP_AttrProp* AP, PD_URIList& ret,    const PD_URI& p, const PD_Object& o );
    PD_URIList&    apGetAllSubjects( const PP_AttrProp* AP, PD_URIList& ret );
    POCol&         apGetArcsOut(     const PP_AttrProp* AP, POCol& ret, const PD_URI& s );
    bool           apContains(       const PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void           apDumpModel(      const PP_AttrProp* AP, const std::string& headerMsg );
    
};


/**
 * Changes to the document RDF are handled with this class. Changes
 * made to the mutation are not reflected in the document RDF right
 * away but must be committed first. You can only have a single commit
 * or rollback for any instance of PD_DocumentRDFMutation. If you
 * commit and want to make another change to the document RDF, create
 * another PD_DocumentRDFMutation object.
 *
 * Because changing the document RDF uses the piece table and AttrProp
 * values, it is (relatively) expensive to mutate RDF. Using this
 * class a collection of changes can be built up and performed in a
 * single operation.
 *
 * By default the object will commit() when it is destroyed. Thus the
 * normal usage pattern is to get a PD_DocumentRDFMutationHandle smart
 * pointer from the DocumentRDF, call add() and/or remove() and then
 * let the PD_DocumentRDFMutationHandle run out of scope to commit
 * your changes. Of course, use rollback() if you want to abandon your
 * mutations.
 *
 * This class also handles notifying the local AbiCollab of updates
 * and any RDF change requests that come in from other abiword
 * instances.
 *
 * On an implementation note: the public API add() and remove()
 * maintain two internal AttrProp tables; the add and remove
 * collections. These internal AttrProp tables are modified using
 * apAdd() and apRemove(). A single add() will use apAdd() to add the
 * change to the add collection and apRemove() to remove it from the
 * remove collection. Thus having the ap prefixed methods is very
 * useful for implementing the public API.
 *
 * When commit() is called, the add/remove changes are made permanent
 * using handleAddAndRemove(). The add/remove are then combined into a
 * single AttrProp and sent via a Change Record CR. AbiCollab will
 * propergate the CR to other abiword instances and the collab code
 * knows to send that AttrProp back to the RDF code through
 * handleCollabEvent(). Inside of handleCollabEvent() a speical flag
 * is set so that we commit the changes but do not propergate them
 * again via a CR (thus avoiding a never ending loop).
 */
class ABI_EXPORT PD_DocumentRDFMutation
{
    friend class PD_DocumentRDF;
    friend class RDFModel_XMLIDLimited;

  protected:
    PD_DocumentRDF* m_rdf; ///< DocumentRDF we are changing
    bool m_rolledback;     ///< Should we rollback
    bool m_committed;      ///< Only commit() once.
    bool m_handlingAbiCollabNotification; ///< If we are handling a remote CR
    PP_AttrProp* m_pAP;        ///< AP that is changed incrementally (deprecated)
    PP_AttrProp* m_crRemoveAP; ///< Triples to remove during commit()
    PP_AttrProp* m_crAddAP;    ///< Triples to add during commit()
    


    bool apAdd( PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void apRemove( PP_AttrProp*& AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    UT_Error handleAddAndRemove( PP_AttrProp* add, PP_AttrProp* remove );

    PD_DocumentRDFMutation( PD_DocumentRDF* rdf );
    
  public:

    virtual ~PD_DocumentRDFMutation();
    virtual void handleCollabEvent( gchar** szAtts, gchar** szProps );

    /**
     * @return false of the triple could not be added.
     */
    virtual bool add( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    virtual void remove( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void remove( const PD_URI& s, const PD_URI& p, const PD_URI& o );
    bool add( const PD_RDFStatement& st );
    void remove( const PD_RDFStatement& st );
    int add( PD_RDFModelHandle model );
        
    virtual UT_Error commit();
    virtual void rollback();
};



class ABI_EXPORT RDFAnchor
{
    bool m_isEnd;
    std::string m_xmlid;

    void setup( const PP_AttrProp* pAP );
    
public:
    RDFAnchor( const PP_AttrProp* pAP );
    RDFAnchor( PD_Document* doc, pf_Frag* pf );
    bool isEnd();
    std::string getID();
};


#endif
