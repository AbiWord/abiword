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
#include "ut_types.h"
#include "pt_Types.h"

#include <boost/shared_ptr.hpp>

class PD_Document;
class pt_PieceTable;
class PP_AttrProp;

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
    bool isValid() const;
    int length() const;
    bool operator==(const PD_URI& b) const;
    bool operator==(const std::string& b) const;

    virtual bool read( std::istream& ss );
    virtual bool write( std::ostream& ss ) const;
};

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
    int getObjectType() const;
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


class PD_DocumentRDFMutation;
typedef boost::shared_ptr<PD_DocumentRDFMutation> PD_DocumentRDFMutationHandle;
class PD_RDFModel;
typedef boost::shared_ptr<PD_RDFModel> PD_RDFModelHandle;
class PD_DocumentRDF;
typedef boost::shared_ptr<PD_DocumentRDF> PD_DocumentRDFHandle;

/**
 * An RDF Model is an abstract immutable collection of RDF triples.
 * You can query if the model contains a statement, and navigate
 * around by supplying one or more parts of a triple.
 *
 * Having the RDF Model abstract like this allows slices of the whole
 * RDF to be returned in various places. For example, to get all the
 * RDF that is explicitly "related" to a location in the document.
 */
class ABI_EXPORT PD_RDFModel
{
  public:    
    virtual PD_URIList getObjects( const PD_URI& s, const PD_URI& p ) = 0;
    virtual PD_URI     getObject( const PD_URI& s, const PD_URI& p ) = 0;
    virtual PD_URIList getSubjects( const PD_URI& p, const PD_Object& o ) = 0;
    virtual PD_URI     getSubject( const PD_URI& p, const PD_Object& o ) = 0;
    virtual PD_URIList getAllSubjects() = 0;
    virtual POCol      getArcsOut( const PD_URI& s ) = 0;
    virtual bool       contains( const PD_URI& s, const PD_URI& p, const PD_Object& o ) = 0;
    virtual bool       contains( const PD_URI& s, const PD_URI& p ) = 0;
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
    
  public:
    explicit PD_DocumentRDF( PD_Document* doc );
    virtual ~PD_DocumentRDF();

    UT_Error setupWithPieceTable();
    

    virtual PD_URIList getObjects( const PD_URI& s, const PD_URI& p );
    virtual PD_URI     getObject( const PD_URI& s, const PD_URI& p );
    virtual PD_URIList getSubjects( const PD_URI& p, const PD_Object& o );
    virtual PD_URI     getSubject( const PD_URI& p, const PD_Object& o );
    virtual PD_URIList getAllSubjects();
    virtual POCol      getArcsOut( const PD_URI& s );
    virtual bool       contains( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    virtual bool       contains( const PD_URI& s, const PD_URI& p );
    long getTripleCount();
    

    PD_DocumentRDFMutationHandle createMutation();
    void handleCollabEvent( gchar** szAtts, gchar** szProps );
         
    PD_RDFModelHandle getRDFAtPosition( PT_DocPosition pos );
    PD_RDFModelHandle getRDFForID( const std::string& xmlid );

    void runMilestone2Test();
    void runMilestone2Test2();
    void dumpObjectMarkersFromDocument();
    
    void dumpModel( const std::string& headerMsg = "dumpModel()" );
    
  private:
    PD_Document* m_doc;
	PT_AttrPropIndex m_indexAP;

    PD_Document*   getDocument(void) const;
    pt_PieceTable* getPieceTable(void) const;
    PD_URI front( const PD_URIList& l ) const;
    void setIndexAP( PT_AttrPropIndex idx );
    PT_AttrPropIndex getIndexAP(void) const;
    virtual const PP_AttrProp* getAP(void) const;
    virtual UT_Error setAP( PP_AttrProp* newAP );
    virtual bool isStandAlone() const;
    
    std::string combinePO(const PD_URI& p, const PD_Object& o );
    std::pair< PD_URI, PD_Object > splitPO( const std::string& po );
    POCol decodePOCol( const std::string& data );
    std::string encodePOCol( const POCol& l );

    void dumpModelFromAP( const PP_AttrProp* AP, const std::string& headerMsg );

    bool apContains( const PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void addRDFForID( const std::string& xmlid, PD_DocumentRDFMutationHandle& m );
    std::list< std::string >& addRelevantIDsForPosition( std::list< std::string >& ret,
                                                         PT_DocPosition pos );
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
    PD_DocumentRDF* m_rdf; ///< DocumentRDF we are changing
    bool m_rolledback;     ///< Should we rollback
    bool m_committed;      ///< Only commit() once.
    bool m_handlingAbiCollabNotification; ///< If we are handling a remote CR
    PP_AttrProp* m_pAP;        ///< AP that is changed incrementally (deprecated)
    PP_AttrProp* m_crRemoveAP; ///< Triples to remove during commit()
    PP_AttrProp* m_crAddAP;    ///< Triples to add during commit()
    
    PD_DocumentRDFMutation( PD_DocumentRDF* rdf );


    bool apAdd( PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void apRemove( PP_AttrProp*& AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    UT_Error handleAddAndRemove( PP_AttrProp* add, PP_AttrProp* remove );
    
  public:

    ~PD_DocumentRDFMutation();
    void handleCollabEvent( gchar** szAtts, gchar** szProps );

    /**
     * @return false of the triple could not be added.
     */
    bool add( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void remove( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    
    UT_Error commit();
    void rollback();
};



class ABI_EXPORT RDFAnchor
{
    bool m_isEnd;
    std::string m_xmlid;
public:
    RDFAnchor( const PP_AttrProp* pAP );
    bool isEnd();
    std::string getID();
};


#endif
