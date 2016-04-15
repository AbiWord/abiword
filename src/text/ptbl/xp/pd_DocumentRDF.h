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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef PD_DOCUMENTRDF_H
#define PD_DOCUMENTRDF_H

#include <string>
#include <list>
#include <map>
#include <memory>
#include <set>
#include "ut_types.h"
#include "pt_Types.h"


class PD_Document;
class pt_PieceTable;
class PP_AttrProp;
class RDFModel_SPARQLLimited;
class pf_Frag;
class pf_Frag_Object;
class PD_RDFSemanticItem;
class FV_View;

class   PD_DocumentRDFMutation;
typedef std::shared_ptr<PD_DocumentRDFMutation> PD_DocumentRDFMutationHandle;
class   PD_RDFModel;
typedef std::shared_ptr<PD_RDFModel> PD_RDFModelHandle;
class   PD_DocumentRDF;
typedef std::shared_ptr<PD_DocumentRDF> PD_DocumentRDFHandle;


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
    virtual ~PD_URI() {}
    virtual std::string toString() const;
    int  length() const
    {
        return m_value.length();
    }
    bool isValid() const;
    bool operator==(const PD_URI& b) const;
    bool operator==(const std::string& b) const;
    bool operator<(const PD_URI& b) const;

    virtual bool read( std::istream& ss );
    virtual bool write( std::ostream& ss ) const;

    PD_URI prefixedToURI(const PD_RDFModelHandle & model) const;

    bool empty() const        { return m_value.empty(); }
    const char* c_str() const { return m_value.c_str(); }

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
    PD_Object( const PD_URI& u );
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
    typedef const PD_RDFModelIterator& self_constref;
    typedef PD_RDFModelIterator& self_reference;
    typedef PD_RDFModelIterator  self_type;

    PD_RDFModelIterator();
    ~PD_RDFModelIterator();
    PD_RDFModelIterator( PD_RDFModelHandle model, const PP_AttrProp* AP );

    self_reference operator++();
    bool operator==( self_constref other );
    PD_RDFModelIterator& operator=( const PD_RDFModelIterator& other );

    self_type operator++(int)
    {
        self_type result( *this );
        ++( *this );
        return result;
    }
    bool operator!=( self_constref other )
    {
        return !operator==(other);
    }
    reference operator*()
    {
        return m_current;
    }

    self_reference moveToNextSubject();
    void moveToNextSubjectReadPO();
    bool moveToNextSubjectHavePOCol();
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
    virtual ~PD_RDFModel() {}
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

    inline long size()  { return getTripleCount(); }
    inline bool empty() { return size() == 0; }

    long getVersion() const { return m_version; }

};

class   PD_RDFSemanticItem;
typedef std::shared_ptr<PD_RDFSemanticItem> PD_RDFSemanticItemHandle;
typedef std::list< PD_RDFSemanticItemHandle > PD_RDFSemanticItems;
class   PD_RDFContact;
typedef std::shared_ptr<PD_RDFContact >     PD_RDFContactHandle;
typedef std::list< PD_RDFContactHandle      > PD_RDFContacts;
class   PD_RDFEvent;
typedef std::shared_ptr<PD_RDFEvent >       PD_RDFEventHandle;
typedef std::list< PD_RDFEventHandle      >   PD_RDFEvents;
class   PD_RDFLocation;
typedef std::shared_ptr<PD_RDFLocation >       PD_RDFLocationHandle;
typedef std::list< PD_RDFLocationHandle      >   PD_RDFLocations;
class   PD_RDFSemanticStylesheet;
typedef std::shared_ptr<PD_RDFSemanticStylesheet > PD_RDFSemanticStylesheetHandle;
typedef std::list< PD_RDFSemanticStylesheetHandle  > PD_RDFSemanticStylesheets;
class   PD_RDFSemanticItemViewSite;
typedef std::shared_ptr<PD_RDFSemanticItemViewSite> PD_RDFSemanticItemViewSiteHandle;
typedef std::list< PD_RDFSemanticItemViewSiteHandle > PD_RDFSemanticItemViewSites;

// semantic stylesheets
#define RDF_SEMANTIC_STYLESHEET_NAME                         "name"
#define RDF_SEMANTIC_STYLESHEET_CONTACT_NAME                 RDF_SEMANTIC_STYLESHEET_NAME
#define RDF_SEMANTIC_STYLESHEET_CONTACT_NICK                 "nick"
#define RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_PHONE           "name, phone"
#define RDF_SEMANTIC_STYLESHEET_CONTACT_NICK_PHONE           "nick, phone"
#define RDF_SEMANTIC_STYLESHEET_CONTACT_NAME_HOMEPAGE_PHONE  "name, (homepage), phone"
#define RDF_SEMANTIC_STYLESHEET_EVENT_NAME                   RDF_SEMANTIC_STYLESHEET_NAME
#define RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY                "summary"
#define RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION       "summary, location"
#define RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_LOCATION_TIMES "summary, location, start date/time"
#define RDF_SEMANTIC_STYLESHEET_EVENT_SUMMARY_TIMES          "summary, start date/time"
#define RDF_SEMANTIC_STYLESHEET_LOCATION_NAME                RDF_SEMANTIC_STYLESHEET_NAME
#define RDF_SEMANTIC_STYLESHEET_LOCATION_NAME_LATLONG        "name, digital latitude, digital longitude"

/**
 * @short Base class for C++ objects which represent RDF at a higher level.
 * @author Ben Martin
 *
 */
class ABI_EXPORT PD_RDFSemanticItem
{
    PD_DocumentRDFHandle m_rdf;
    PD_URI               m_context;
  protected:
    std::string          m_name;
    PD_URI               m_linkingSubject;
    typedef std::list< std::map< std::string, std::string > > PD_ResultBindings_t;

    virtual std::string getImportFromFileName( const std::string& filename_const,
                                               std::list< std::pair< std::string, std::string> > types ) const;
    virtual std::list< std::pair< std::string, std::string> > getImportTypes() const;

    virtual std::string getExportToFileName( const std::string& filename_const,
                                             std::string defaultExtension,
                                             std::list< std::pair< std::string, std::string> > types ) const;
    virtual std::list< std::pair< std::string, std::string> > getExportTypes() const;
    virtual std::string getDefaultExtension() const;
    std::string getProperty( std::string subj, std::string pred, std::string defVal ) const;

  public:
    PD_RDFSemanticItem( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it );
    virtual ~PD_RDFSemanticItem();

    PD_DocumentRDFHandle getRDF() const;
    PD_DocumentRDFMutationHandle createMutation();

    std::string requestExportFileNameByDialog();


    /**
     * For an item like a contact, event, location, if there is a
     * subject, common#idref xml:id triple that can be used to link
     * into the document content, return that subject node here for
     * the common base class methods to use.
     *
     * For example, in the FOAF vocabulary the ?person node from the
     * SPARQL query fragment below will be the linkingSubject()
     * ?person rdf:type foaf:Person
     */
    virtual PD_URI linkingSubject() const;


    /**
     * Given a linking subject, find all the xmlids that are referenced by it
     * ie, return all ?xmlid from the form:
     *
     * ?linkingSubj pkg:idref ?xmlid
     *
     */
    static std::set< std::string > getXMLIDsForLinkingSubject( PD_DocumentRDFHandle rdf, const std::string& linkingSubj );


    /**
     * A simple description of the semantic item that can be shown to the user
     */
    virtual std::string name() const;
    virtual void        setName( const std::string& n );

    /**
     * Something that can be used on a notebook tab to describe this semantic item
     * eg, Contact, Event etc.
     */
    virtual std::string getDisplayLabel() const;


    /**
     * A Semantic Item can appear multiple times in a document. For
     * example, the person Frodo can be referenced 20 times in a short
     * book review. This method gives all the xmlid values where the
     * semantic item is referenced in the document.
     *
     * The list of xmlid values can in turn be used by
     * PD_DocumentRDF::getIDRange() and PD_DocumentRDF::getRDFForID()
     * to inspect or perform actions at the various places the
     * semanitc item appears in the document.
     */
    virtual std::set< std::string > getXMLIDs() const;

    /**
     * Create a Widget that can edit the SemanticItem. Note that the
     * widget will show the data and allow editing of it for the
     * SemanticItem, but to make changes permenant, the
     * updateFromEditorData() method must be used. A typical senario
     * is to add the widget from createEditor to a dialog and when the
     * user affirms the dialog call updateFromEditorData() to update
     * the document.
     *
     * @see updateFromEditorData()
     */
     virtual void* createEditor() = 0;

     /**
      * Create a top level dialog allowing the user to edit this semitem
      */
     virtual void showEditorWindow( const PD_RDFSemanticItemHandle & c );
     virtual void showEditorWindow( const PD_RDFSemanticItems & cl );

     /**
      * Update the SemanticItem from the edited dialog that was created using
      * createEditor.
      *
      * @see createEditor()
      */
     void updateFromEditorData();
     virtual void updateFromEditorData( PD_DocumentRDFMutationHandle m ) = 0;


    /**
     * Import the data in iss to the semnatic item. This is used for
     * D&D Drop events to create a new semnatic item. Subclasses
     * should set their internal state based on the data in 'iss' and
     * then call importFromDataComplete() at the end of the method to
     * update the RDF and insert the semantic item into the document.
     *
     * ??? maybe not ??? importFromDataComplete() calls also insert() which links the
     * ??? semanticItem with the document object m_rdf.
     */
     virtual void importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange = 0 ) = 0;

    /**
     * Export to a file in whatever format is the most useful for the
     * semantic item. Prompt for a filename if none is given.
     */
     virtual void exportToFile( const std::string& filename = "" ) const = 0;

     /**
     * Import from a file in whatever format is the most useful for the
     * semantic item. The default implemenation uses importFromData() to perform this import,
     * so all a subclass needs to do is override the getImportTypes() method to
     * let an import dialog know what types are OK.
     */
     virtual void importFromFile( const std::string& filename = "" );


    /**
     * Create a SemanticItem subclass using its name from
     * getClassNames(). Useful for menus and other places that want to
     * allow the user to create new SemanticItem Objects.
     */
     static PD_RDFSemanticItemHandle createSemanticItem( PD_DocumentRDFHandle rdf, const std::string& semanticClass );
     static PD_RDFSemanticItemHandle createSemanticItem( PD_DocumentRDFHandle rdf,
                                                         PD_ResultBindings_t::iterator it,
                                                         const std::string& semanticClass );

    /**
     * Gets a list of SemanticItem subclasses that can be created.
     * Any of the strings in the return value can be created using
     * createSemanticItem().
     *
     * @see createSemanticItem()
     */
     static std::list< std::string > getClassNames();


     /**
      * Insert a reference to this semantic item at the cursor location of pView
      *
      * This should only be called after creating a new sem item with
      * createSemanticItem().
      */
     std::pair< PT_DocPosition, PT_DocPosition > insert( PD_DocumentRDFMutationHandle m, FV_View* pView );
     std::pair< PT_DocPosition, PT_DocPosition > insert( FV_View* pView );


     virtual void setupStylesheetReplacementMapping( std::map< std::string, std::string >& m );


    /**
     * Unambiguiously find a stylesheet by its UUID. The sheet can
     * be either user or system as long as it has the uuid you want.
     */
     PD_RDFSemanticStylesheetHandle findStylesheetByUuid(const std::string &uuid) const;

    /**
     * Find a user/system stylesheet by name.
     * sheetType is one of TYPE_SYSTEM/TYPE_USER.
     * n is the name of the stylesheet you want.
     */
    PD_RDFSemanticStylesheetHandle findStylesheetByName(const std::string &sheetType, const std::string &n) const;
    /**
     * Find a user/system stylesheet by name.
     * ssl is either stylesheets() or userStylesheets()
     * n is the name of the stylesheet you want.
     */
    PD_RDFSemanticStylesheetHandle findStylesheetByName(const PD_RDFSemanticStylesheets& ssl, const std::string &n) const;

    /**
     * Get the default stylesheet for this subclass of Semantic Item.
     *
     * If you want to know which stylesheet is in use by a particular
     * reference to a semantic item, use KoRdfSemanticItemViewSite::stylesheet()
     *
     * @see KoRdfSemanticItemViewSite
     * @see KoRdfSemanticItemViewSite::stylesheet()
     */
    PD_RDFSemanticStylesheetHandle defaultStylesheet() const;

    /**
     * Get the system semantic stylesheets that are supported for this
     * particular semantic item subclass.
     */
    virtual PD_RDFSemanticStylesheets stylesheets() const = 0;
    virtual std::string className() const = 0;

    enum RelationType
    {
        RELATION_FOAF_KNOWS = 1
    };
    void relationAdd( PD_RDFSemanticItemHandle si, RelationType rt );
    PD_RDFSemanticItems relationFind( RelationType rt );

  protected:

     std::pair< PT_DocPosition, PT_DocPosition > insertTextWithXMLID( const std::string& textconst,
                                                                      const std::string& xmlid );

    /**
     * The importFromData() method can use this method to finish an
     * import. Text is also inserted into the document to show the
     * user the new semantic object. The semanticObjectAdded signal is
     * emitted so that UI elements have a chance to update themselves to
     * reflect the newly added SemanticItem in the document.
     *
     * This method uses createEditor() followed by
     * updateFromEditorData() to actually put the RDF triples into the
     * store. So a subclass does not have to explicitly handle the
     * import if it can present a GUI to edit itself. The GUI is not
     * shown to the user.
     *
     * Basically a subclass reads the data in importFromData and sets
     * local member variables from it. Then calls
     * importFromDataComplete() which makes an editor using the member
     * variables and commits the editor which in turn updates the RDF
     * for the document.
     */
     virtual void importFromDataComplete( std::istream& iss,
                                          PD_DocumentRDFHandle rdf,
                                          PD_DocumentRDFMutationHandle m,
                                          PD_DocumentRange * pDocRange = 0 );

     std::string bindingAsString( PD_ResultBindings_t::iterator& it, const std::string k );
     std::string optionalBindingAsString( PD_ResultBindings_t::iterator& it, const std::string k );

    PD_URI& handleSubjectDefaultArgument( PD_URI& subj );

    /**
     * Return the graph context that contains this SematicItem's Rdf
     * statements. Used by the updateTriple()s to remove and add
     * updated information. The default is the manifest.rdf file.
     */
    virtual PD_URI context() const;

    /**
     * Ensure the Rdf Type of the subject or linkingSubject() by
     * default is what you want After this method, the Rdf will have
     * the following: linkingSubject() rdf:type type
     */
    void setRDFType(PD_DocumentRDFMutationHandle m, const std::string& type, PD_URI subj = PD_URI());
    void setRDFType(const std::string& type, PD_URI subj = PD_URI());


    /**
     * When a subclass wants to update a triple in the RDF store
     * to reflect a change, for example, the phone number of a
     * contact, it should call here to set toModify = newValue.
     *
     * This is done both in the C++ objects and the Rdf model.
     * The RDF will be changed from
     * linkingSubject() predString toModify
     * to
     * linkingSubject() predString newValue
     *
     * Note that rounding errors and other serialization issues that
     * crop up are handled by these methods, so you should try very
     * hard not to directly update the PD_RDFModelHandle outside these
     * methods.
     *
     * Note that if you do not supply a mutationhandle the method to
     * create/commit a mutation for you.
     */
    void updateTriple( PD_DocumentRDFMutationHandle m, std::string& toModify, const std::string& newValue, const PD_URI& predString );
    void updateTriple( PD_DocumentRDFMutationHandle m, time_t&      toModify, time_t newValue, const PD_URI& predString );
    void updateTriple( PD_DocumentRDFMutationHandle m, double&      toModify, double newValue, const PD_URI& predString );
    void updateTriple( PD_DocumentRDFMutationHandle m, double&      toModify, double newValue, const PD_URI& predString, PD_URI linkingSubject  );

    void updateTriple( std::string& toModify, const std::string& newValue, const PD_URI& predString );
    void updateTriple( time_t&      toModify, time_t newValue, const PD_URI& predString );
    void updateTriple( double&      toModify, double newValue, const PD_URI& predString );

    /**
     * Create a bnode with a uuid
     */
    PD_URI createUUIDNode();


protected:
    /**
     * The updateTriple() methods all call remove() then add() to
     * perform their work. These lower level functions accept PD_URI
     * to remove/add. Note that corner cases like "double" values are
     * explicitly handled by these methods. For example, at times a
     * double will undergo some rounding during serialization, so you
     * can not just call mutation->remove() because you have to take
     * rounding errors into account for the value you are intending to
     * remove.
     */
    void updateTriple_remove( PD_DocumentRDFMutationHandle m,
                              const PD_URI& toModify,
                              const PD_URI& predString,
                              const PD_URI& explicitLinkingSubject );

    /**
     * After updateTriple() calls remove() it can set toModify to the
     * new value and call this method to add the new value to the RDF
     * store.
     */
    void updateTriple_add( PD_DocumentRDFMutationHandle m,
                           const PD_URI& toModify,
                           const PD_URI& predString,
                           const PD_URI& explicitLinkingSubject );

};



class ABI_EXPORT PD_RDFContact : public PD_RDFSemanticItem
{
  protected:
    std::string m_nick;
    std::string m_email;
    std::string m_homePage;
    std::string m_imageUrl;
    std::string m_phone;
    std::string m_jabberID;

    virtual std::list< std::pair< std::string, std::string> > getImportTypes() const;
    virtual std::list< std::pair< std::string, std::string> > getExportTypes() const;
    virtual std::string getDefaultExtension() const;

  public:
    PD_RDFContact( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it );
    virtual ~PD_RDFContact();

    virtual void importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange = 0 );
    virtual void exportToFile( const std::string& filename = "" ) const;
    virtual std::string getDisplayLabel() const;
    virtual void setupStylesheetReplacementMapping( std::map< std::string, std::string >& m );
    virtual PD_RDFSemanticStylesheets stylesheets() const;
    virtual std::string className() const;
};


class ABI_EXPORT PD_RDFEvent : public PD_RDFSemanticItem
{
  protected:
    std::string m_uid;
    std::string m_summary;
    std::string m_location;
    std::string m_desc;
    time_t      m_dtstart;
    time_t      m_dtend;

    virtual std::list< std::pair< std::string, std::string> > getImportTypes() const;
    virtual std::list< std::pair< std::string, std::string> > getExportTypes() const;
    virtual std::string getDefaultExtension() const;

  public:
    PD_RDFEvent( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it );
    virtual ~PD_RDFEvent();

    virtual void importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange = 0 );
    virtual void exportToFile( const std::string& filename = "" ) const;
    virtual std::string getDisplayLabel() const;
    virtual void setupStylesheetReplacementMapping( std::map< std::string, std::string >& m );
    virtual PD_RDFSemanticStylesheets stylesheets() const;
    virtual std::string className() const;
};


class ABI_EXPORT PD_RDFLocation : public PD_RDFSemanticItem
{
  protected:
    std::string m_uid;
    std::string m_desc;
    double      m_dlat;
    double      m_dlong;
    PD_Object   m_joiner;
    bool        m_isGeo84;

    virtual std::list< std::pair< std::string, std::string> > getImportTypes() const;
    virtual std::list< std::pair< std::string, std::string> > getExportTypes() const;
    virtual std::string getDefaultExtension() const;

  public:
    PD_RDFLocation( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it, bool isGeo84 = false );
    virtual ~PD_RDFLocation();

    virtual void importFromData( std::istream& iss, PD_DocumentRDFHandle rdf, PD_DocumentRange * pDocRange = 0 );
    virtual void exportToFile( const std::string& filename = "" ) const;
    virtual std::string getDisplayLabel() const;
    virtual std::set< std::string > getXMLIDs() const;
    virtual void setupStylesheetReplacementMapping( std::map< std::string, std::string >& m );
    virtual PD_RDFSemanticStylesheets stylesheets() const;
    virtual std::string className() const;
};


/**
 * @short A stylesheet that knows how to format a given PD_RDFSemanticItem
 *
 * If you are looking to apply a stylesheet you should use the PD_RDFSemanticItemViewSite
 * class. For example:
 *
 * PD_RDFSemanticItemViewSite vs( h_semitem, xmlid );
 * vs.applyStylesheet( pView, h_stylesheet );
 *
 * @author Ben Martin
 * @see PD_DocumentRDF
 * @see PD_RDFSemanticItem
 */
class ABI_EXPORT PD_RDFSemanticStylesheet
{
    std::string m_uuid;
    std::string m_name;
    std::string m_templateString;
    std::string m_type;
    bool        m_isMutable;
protected:

    // Restrict who can make us
    friend class PD_RDFSemanticItem;
    friend class PD_RDFLocation;
    friend class PD_RDFContact;
    friend class PD_RDFEvent;
    friend class PD_RDFSemanticItemViewSite;

    PD_RDFSemanticStylesheet( const std::string &uuid,
                              const std::string &name,
                              const std::string &templateString,
                              const std::string &type = "System",
                              bool isMutable = false);

    /**
     * Only called from PD_RDFSemanticItemViewSite, this method
     * actually applies the stylesheet to a specific reference to a
     * semantic item in the document.
     */
    void format( PD_RDFSemanticItemHandle sob, FV_View* pView, const std::string& xmlid = "" );

public:

    virtual ~PD_RDFSemanticStylesheet();
    static std::string stylesheetTypeSystem();
    static std::string stylesheetTypeUser();

    std::string uuid() const;
    std::string name() const;
    std::string templateString() const;
    std::string type() const;
    bool isMutable() const;
};


/**
 * @short Handling a specific reference to a semantic item in the document text.
 * @author Ben Martin
 *
 * There can be many references to a single PD_RDFSemanticItem in a
 * document. One can consider PD_RDFSemanticItem to be the model and
 * this class the view/controller (delegate).
 *
 * For example:
 * foaf pkg:idref frodo1
 * foaf pkg:idref frodo2
 *
 * the foaf/contact data is the PD_RDFSemanticItem (model) and each
 * xml:id, the frodo1 and frodo2 above, are PD_RDFSemanticItemViewSite instances.
 * This class allows different stylesheets to offer different
 * formatting for each presentation of the same PD_RDFSemanticItem
 * (model).
 */
class ABI_EXPORT PD_RDFSemanticItemViewSite
{
    std::string m_xmlid;
    PD_RDFSemanticItemHandle m_semItem;

  public:
    /**
     * Performing actions on a specific reference to a semantic item in the document.
     */
    PD_RDFSemanticItemViewSite( PD_RDFSemanticItemHandle si, const std::string &xmlid );
    PD_RDFSemanticItemViewSite( PD_RDFSemanticItemHandle si, PT_DocPosition pos );
    ~PD_RDFSemanticItemViewSite();

    /**
     * The stylesheet that has been set for this view site
     */
    PD_RDFSemanticStylesheetHandle stylesheet() const;

    /**
     * If there is a stylesheet set for this view site it is forgotten
     * and the reference can be freely edited by the user
     */
    void disassociateStylesheet();

    /**
     * Apply a stylesheet for the semantic item.
     * Note that the setting is rememebered for this site too.
     *
     * The application can be done separately using the setStylesheetWithoutReflow()
     * and reflowUsingCurrentStylesheet() methods. Performing the stylesheet
     * application in two parts is convenient if you are applying a stylesheet to many
     * semantic items at once, or to all the locations in the document which reference
     * a single semanti item.
     *
     * @see setStylesheetWithoutReflow()
     * @see reflowUsingCurrentStylesheet()
     */
    void applyStylesheet( FV_View* pView, PD_RDFSemanticStylesheetHandle ss );

    /**
     * Remember that a specific stylesheet should be applied for this
     * semantic item. No reflow of the document is performed and thus
     * no layout or user visible changes occur.
     *
     * @see applyStylesheet()
     */
    void setStylesheetWithoutReflow( PD_RDFSemanticStylesheetHandle ss );

    /**
     * Reflow the text that shows the user this semantic item in the
     * document.
     *
     * @see applyStylesheet()
     */
    void reflowUsingCurrentStylesheet( FV_View* pView );

    /**
     * Select this view of the semantic item in the document.
     */
    void select( FV_View* pView );

private:

    /**
     * This is similar to the linkingSubject() used by
     * PD_RDFSemanticItem in that you have:
     *
     * linkingSubject() common#idref xml:id
     *
     * but metadata about the site at xml:id is stored as properties
     * off the PD_RDFSemanticItemViewSite::linkingSubject() subject.
     *
     * The difference between this linkingSubject() and
     * PD_RDFSemanticItem is that this is for RDF describing a single
     * xml:id site in the document, the
     * PD_RDFSemanticItem::linkingSubject() is the model for the
     * semanitc item itself which can be referenced by many xml:id
     * sites.
     */
    PD_URI linkingSubject() const;

    /**
     * Convenience method to get a specific property from the Rdf
     * model. There should be either zero or only one value for X in
     * the triple:
     *
     * linkingSubject(), prop, X
     *
     * if the property does not exist defval is returned.
     */
    std::string getProperty(const std::string &prop, const std::string &defval) const;
    /**
     * Set the single value for the Rdf predicate prop.
     * @see getProperty()
     */
    void setProperty(const std::string &prop, const std::string &v);

    /**
     * Convenience method to select a range in the document view
     */
    void selectRange( FV_View* pView, std::pair< PT_DocPosition, PT_DocPosition > range );

};



/**
 * Insert a reference to a semenatic item
 */
ABI_EXPORT std::pair< PT_DocPosition, PT_DocPosition > runInsertReferenceDialog( FV_View* pView );
ABI_EXPORT void runSemanticStylesheetsDialog( FV_View* pView );


class ABI_EXPORT PD_SemanticItemFactory
{
  public:
    typedef std::list< std::map< std::string, std::string > > PD_ResultBindings_t;
	virtual ~PD_SemanticItemFactory() {}
    virtual PD_RDFContact*  createContact( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it ) = 0;
    virtual PD_RDFEvent*    createEvent( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it ) = 0;
    virtual PD_RDFLocation* createLocation( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it,
                                            bool isGeo84 = false ) = 0;
};

class ABI_EXPORT PD_RDFDialogs
{
  public:
	virtual ~PD_RDFDialogs() {}
    virtual void runSemanticStylesheetsDialog( FV_View* pView ) = 0;
    virtual std::pair< PT_DocPosition, PT_DocPosition > runInsertReferenceDialog( FV_View* pView ) = 0;
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
    friend class PD_RDFSemanticItem;

  public:
    explicit PD_DocumentRDF( PD_Document* doc );
    virtual ~PD_DocumentRDF();

    UT_Error setupWithPieceTable();

    /**
     * Does this document have any semantic items like contacts, events etc.
     */
    bool haveSemItems() const;
    
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

    std::list< pf_Frag_Object* > getObjectsInScopeOfTypesForRange(
        std::set< PTObjectType > objectTypes,
        std::pair< PT_DocPosition, PT_DocPosition > range );
    std::set< std::string >& addXMLIDsForObjects( std::set< std::string >& ret, std::list< pf_Frag_Object* > objectList );
    PT_DocPosition addXMLIDsForBlockAndTableCellForPosition( std::set< std::string >& col, PT_DocPosition pos );


    void addRDFForID( const std::string& xmlid, PD_DocumentRDFMutationHandle& m );
    std::set< std::string >& addRelevantIDsForPosition( std::set< std::string >& ret,
                                                        PT_DocPosition pos );
    std::set< std::string >& addRelevantIDsForRange( std::set< std::string >& ret,
                                                      PD_DocumentRange* range );
    std::set< std::string >& addRelevantIDsForRange( std::set< std::string >& ret,
                                                      std::pair< PT_DocPosition, PT_DocPosition > range );

    std::set< std::string >& getAllIDs( std::set< std::string >& ret );
    std::pair< PT_DocPosition, PT_DocPosition > getIDRange( const std::string& xmlid ) const;


    PD_RDFModelHandle createRestrictedModelForXMLIDs( const std::string& writeID,
                                                      const std::set< std::string >& xmlids );
    PD_RDFModelHandle createRestrictedModelForXMLIDs( const std::set< std::string >& xmlids );

    virtual void maybeSetDocumentDirty();

    // testing methods...
    void runMilestone2Test();
    void runMilestone2Test2();
    void dumpObjectMarkersFromDocument();
    void runPlay();

    static std::string getSPARQL_LimitedToXMLIDList( const std::set< std::string >& xmlids,
                                                     const std::string& extraPreds = "" );

    std::string makeLegalXMLID( const std::string& s );
    void relinkRDFToNewXMLID( const std::string& oldxmlid, const std::string& newxmlid, bool deepCopyRDF = false );

    PD_RDFModelHandle createScratchModel();

    static PD_URI getManifestURI();

    PD_RDFSemanticItems getAllSemanticObjects( const std::string& classRestriction = "" );
    PD_RDFSemanticItems getSemanticObjects( const std::set< std::string >& xmlids );
    PD_RDFContacts  getContacts( PD_RDFModelHandle alternateModel = PD_RDFModelHandle((PD_RDFModel*)0) );
    PD_RDFEvents    getEvents( PD_RDFModelHandle alternateModel = PD_RDFModelHandle((PD_RDFModel*)0) );
    PD_RDFLocations getLocations( PD_RDFModelHandle alternateModel = PD_RDFModelHandle((PD_RDFModel*)0) );
    void selectXMLIDs( const std::set< std::string >& xmlids, FV_View* pView = 0 ) const;


    void showEditorWindow( const PD_RDFSemanticItems& cl );


    // GTK, win32, osx, whatever backends can call this method to allow the correct
    // subclasses to be made for the runtime environment.
    static void setSemanticItemFactory( PD_SemanticItemFactory* f );
    static void setRDFDialogs( PD_RDFDialogs* d );

    static PD_SemanticItemFactory *getSemanticItemFactory();
    static PD_RDFDialogs *getRDFDialogs();

  protected:
    PD_Document* m_doc;
  private:
    static PD_SemanticItemFactory *s_SemanticItemFactory;
    static PD_RDFDialogs *s_RDFDialogs;
	PT_AttrPropIndex m_indexAP;
    bool m_haveSemItems;
    
    PD_Document*   getDocument(void) const;
    pt_PieceTable* getPieceTable(void) const;
    void setIndexAP( PT_AttrPropIndex idx );
    PT_AttrPropIndex getIndexAP(void) const;
    virtual const PP_AttrProp* getAP(void);
    virtual UT_Error setAP( PP_AttrProp* newAP );
    virtual bool isStandAlone() const;

    std::set< std::string >& priv_addRelevantIDsForPosition( std::set< std::string >& ret,
                                                             PT_DocPosition pos,
                                                             PT_DocPosition searchBackThisFar = 0 );

  protected:
    PD_ObjectList& apGetObjects(     const PP_AttrProp* AP, PD_ObjectList& ret, const PD_URI& s, const PD_URI& p );
    PD_URIList&    apGetSubjects(    const PP_AttrProp* AP, PD_URIList& ret,    const PD_URI& p, const PD_Object& o );
    PD_URIList&    apGetAllSubjects( const PP_AttrProp* AP, PD_URIList& ret );
    POCol&         apGetArcsOut(     const PP_AttrProp* AP, POCol& ret, const PD_URI& s );
    bool           apContains(       const PP_AttrProp* AP, const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void           apDumpModel(      const PP_AttrProp* AP, const std::string& headerMsg );

    void updateHaveSemItemsCache();
    
  private:
    PD_RDFLocations& addLocations( PD_RDFLocations& ret,
                                   bool isGeo84,
                                   const std::string sparql,
                                   PD_RDFModelHandle alternateModel );
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
    virtual bool add( const PD_URI& s, const PD_URI& p, const PD_Object& o, const PD_URI& context );
    virtual void remove( const PD_URI& s, const PD_URI& p, const PD_Object& o );
    void remove( const PD_URI& s, const PD_URI& p, const PD_URI& o );
    bool add( const PD_RDFStatement& st );
    void remove( const PD_RDFStatement& st );
    int add( PD_RDFModelHandle model );
    void remove( const std::list< PD_RDFStatement >& sl );
    void remove( const PD_URI& s, const PD_URI& p );

    PD_URI createBNode();

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
    RDFAnchor( PD_Document* pDoc, PT_AttrPropIndex api );
    RDFAnchor( PD_Document* doc, pf_Frag* pf );
    bool isEnd();
    std::string getID();
};


#endif
