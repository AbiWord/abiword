/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2011 Ben Martin
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

#include "pd_RDFQuery.h"
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

// RDF support
#include <redland.h>
#include <rasqal.h>
#include "pd_RDFSupportRed.h"

librdf_world* getWorld()
{
    static librdf_world* world = 0;
    if( !world )
    {
        world = librdf_new_world();
        librdf_world_open( world );
    }
    return world;
}

std::string tostr( librdf_node* n )
{
    if( !n )
        return "NULL";

    if( librdf_uri* u = librdf_node_get_uri( n ))
    {
        std::string s = (const char*)librdf_uri_as_string( u );
        return s;
    }
    
    std::string s = (const char*)librdf_node_to_string( n );
    return s;
}

librdf_statement* toRedland( const PD_RDFStatement& st )
{
    librdf_world* w = getWorld();
    librdf_statement* ret = librdf_new_statement_from_nodes(
        w,
        librdf_new_node_from_uri_string( w, (const unsigned char*)st.getSubject().toString().c_str() ),
        librdf_new_node_from_uri_string( w, (const unsigned char*)st.getPredicate().toString().c_str() ),
        librdf_new_node_from_uri_string( w, (const unsigned char*)st.getObject().toString().c_str() )
        );
    return ret;
}


static std::string tostr( librdf_statement* statement )
{
    std::stringstream ss;
    ss << "  subj:" << tostr( librdf_statement_get_subject( statement ) ) << endl;
    ss << "  pred:" << tostr( librdf_statement_get_predicate( statement ) ) << endl;
    ss << "   obj:" << tostr( librdf_statement_get_object( statement ) ) << endl;
    return ss.str();
}



/********************************************************************************/
/********************************************************************************/
/*** Redland internal storage class *********************************************/
/********************************************************************************/
/********************************************************************************/

struct abiwordContext
{
    librdf_storage*   storage;
    PD_RDFModelHandle m_model;
    int x;
    
    abiwordContext( librdf_storage* storage,
                    const char *name,
                    librdf_hash* options )
        : storage(storage)
        , x(0)
    {
        librdf_storage_set_instance( storage, this );

        if(librdf_hash_get_as_boolean(options, "x")>0)
            x = 1;
    }

    static abiwordContext* get( librdf_storage* storage )
    {
        if( !storage || !librdf_storage_get_instance(storage) )
        {
            cerr << "problem getting abiwordContext from RDF storage!" << endl;
            return 0;
        }
        
        abiwordContext* ret = (abiwordContext*)librdf_storage_get_instance(storage);
        return ret;
    }
    
    void setModel( PD_RDFModelHandle m )
    {
        m_model = m;
    }

    void dump( const std::string& s )
    {
        m_model->dumpModel( s );
    }
};


struct abiwordFindStreamContext
{
    librdf_storage*     storage;
    abiwordContext*     context;
    librdf_statement*   query;
    librdf_statement*   statement;
    librdf_node*        context_node;
    PD_RDFModelIterator m_iter;
    bool done;
    
    abiwordFindStreamContext( librdf_storage* storage,
                              abiwordContext* c,
                              librdf_statement* statement,
                              librdf_node* context_node )
        : storage( storage )
        , context(c)
        , done(false)
        , query(0)
        , statement(0)
        , context_node(0)
    {
        librdf_storage_add_reference(this->storage);
        if( statement )
            this->query = librdf_new_statement_from_statement( statement );
        if( context_node )
            this->context_node = librdf_new_node_from_node( context_node );

        cerr << "abiwordFindStreamContext() query..." << endl;
        cerr << "  subj:" << tostr( librdf_statement_get_subject( statement ) ) << endl;
        cerr << "  pred:" << tostr( librdf_statement_get_predicate( statement ) ) << endl;
        cerr << "   obj:" << tostr( librdf_statement_get_object( statement ) ) << endl;
        
    }

    ~abiwordFindStreamContext()
    {
        if(storage)
            librdf_storage_remove_reference(storage);

        if(query)
            librdf_free_statement(query);

        if(statement)
            librdf_free_statement(statement);

        if(context_node)
            librdf_free_node(context_node);
    }
    

    static abiwordFindStreamContext* get( void* context )
    {
        return static_cast<abiwordFindStreamContext*>(context);
    }
    

    int getNext()
    {
        cerr << "getNext() top..." << endl;
        if(statement)
        {
            librdf_free_statement(statement);
            statement = 0;
        }

        PD_RDFModelIterator e = context->m_model->end();
        if( m_iter == e )
        {
            cerr << "getNext() hit end()" << endl;
            done = 1;
            return -1;
        }

        for( ; m_iter != e; )
        {
            PD_RDFStatement st = *m_iter;
            ++m_iter;
            cerr << "getNext() testing statement...st:" << st.toString() << endl;

            librdf_statement* stred = toRedland( st );
            if( !query || librdf_statement_match( stred, query ) )
            {
                cerr << "getNext() statement matches..." << endl;
                cerr << " st:" << st.toString() << endl;
                statement = stred;
                break;
            }
            librdf_free_statement( stred );
        }
        
        return 0;
    }
    

    void setup( librdf_world* world )
    {
        cerr << "setup() top" << endl;

        // {
        //     PD_RDFModelIterator iter = context->m_model->begin();
        //     PD_RDFModelIterator  end = context->m_model->end();
        //     for( ; iter != end; ++iter )
        //     {
        //         PD_RDFStatement st = *iter;
        //         cerr << "setup(a)...st loop:" << st.toString() << endl;
        //     }
        // }
        
        // cerr << "setup() B............" << endl;

        // {
        //     PD_RDFModelIterator iter = context->m_model->begin();
        //     PD_RDFModelIterator  end = context->m_model->end();
        //     for( ; iter != end; ++iter )
        //     {
        //         PD_RDFStatement st = *iter;
        //         cerr << "setup(b)...st loop:" << st.toString() << endl;
        //     }
        // }
        
        // cerr << "setup() C............" << endl;

        // {
        //     m_iter = context->m_model->begin();
        //     PD_RDFModelIterator  end = context->m_model->end();
        //     for( ; m_iter != end; ++m_iter )
        //     {
        //         PD_RDFStatement st = *m_iter;
        //         cerr << "setup(c)...st loop:" << st.toString() << endl;
        //     }
        // }
        
        // cerr << "setup() D............" << endl;
        

        m_iter = context->m_model->begin();
        PD_RDFModelIterator e = context->m_model->end();
        cerr << "setup()...iter!=end:" << (m_iter != e) << endl;
        PD_RDFStatement st = *m_iter;
        cerr << "setup()...st1:" << st.toString() << endl;
    }
};


static int
abiword_storage_init( librdf_storage* storage,
                      const char *name,
                      librdf_hash* options )
{
    bool failed = true;
    
    if( name )
    {
        failed = false;
        abiwordContext* context = new abiwordContext( storage, name, options );
        librdf_storage_set_instance( storage, context );
    }
    
    if(options)
        librdf_free_hash(options);

    return failed;
}

static void
abiword_storage_terminate(librdf_storage* storage)
{
    abiwordContext* c = abiwordContext::get( storage );
    delete c;
}

static int
abiword_storage_open(librdf_storage* storage, librdf_model* model)
{
    abiwordContext* c = abiwordContext::get( storage );
    return 0;
}

static int
abiword_storage_close(librdf_storage* storage)
{
    abiwordContext* c = abiwordContext::get( storage );
    return 0;

}

static int
abiword_storage_size(librdf_storage* storage)
{
    abiwordContext* c = abiwordContext::get( storage );
    int statementCount = c->m_model->getTripleCount();
    return statementCount;
}


static int
abiword_storage_find_statements_end_of_stream( void* context )
{
    abiwordFindStreamContext* sc = abiwordFindStreamContext::get( context );
    cerr << "abiword_storage_find_statements_end_of_stream() ctx:" << sc << " done:" << sc->done << endl;
    
    if( sc->done )
        return 1;
  
    if( !sc->statement )
        sc->getNext();
    
    cerr << "abiword_storage_find_statements_end_of_stream(2) done:" << sc->done << endl;
    return sc->done;
}

static int
abiword_storage_find_statements_next_statement( void* context )
{
    abiwordFindStreamContext* sc = abiwordFindStreamContext::get( context );
    cerr << "abiword_storage_find_statements_next_statement() done:" << sc->done << endl;
    if( sc->done )
        return 1;
    return sc->getNext();
}

static void*
abiword_storage_find_statements_get_statement(void* context, int flags)
{
    abiwordFindStreamContext* sc = abiwordFindStreamContext::get( context );
    cerr << "abiword_storage_find_statements_get_statement() done:" << sc->done << " flags:" << flags << endl;

    switch(flags)
    {
        case LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT:
            cerr << "get_statement() result...." << tostr( sc->statement ) << endl;
            return sc->statement;
            
        default:
            cerr << "ERROR: Unknown iterator method flag:" << flags << endl;
            return NULL;
    }
}


static void
abiword_storage_find_statements_finished(void* context)
{
    abiwordFindStreamContext* sc = abiwordFindStreamContext::get( context );
    delete sc;
    cerr << "=== abiword_storage_find_statements_finished()" << endl << endl;
}


static librdf_stream*
abiword_storage_find_statements_with_context( librdf_storage* storage,
                                              librdf_statement* statement,
                                              librdf_node* context_node )
{
    cerr << "=== abiword_storage_find_statements()" << endl;
    cerr << "statement:" << statement << endl;

    if( statement )
    {
        cerr << "subj:" << tostr( librdf_statement_get_subject( statement ) ) << endl;
        cerr << "pred:" << tostr( librdf_statement_get_predicate( statement ) ) << endl;
        cerr << " obj:" << tostr( librdf_statement_get_object( statement ) ) << endl;
    }
    
    abiwordContext* c = abiwordContext::get( storage );
    abiwordFindStreamContext* sc = new abiwordFindStreamContext( storage, c, statement, context_node );
    sc->setup( librdf_storage_get_world(storage) );
    
    librdf_stream* stream = librdf_new_stream( librdf_storage_get_world(storage),
                                               (void*)sc,
                                               &abiword_storage_find_statements_end_of_stream,
                                               &abiword_storage_find_statements_next_statement,
                                               &abiword_storage_find_statements_get_statement,
                                               &abiword_storage_find_statements_finished);
    if(!stream)
    {
        abiword_storage_find_statements_finished((void*)sc);
        return NULL;
    }
    
    cerr << "abiword_storage_find_statements(done)" << endl;
    return stream;  
}


static librdf_stream*
abiword_storage_find_statements( librdf_storage* storage,
                                 librdf_statement* statement )
{
    return abiword_storage_find_statements_with_context( storage, statement, 0 );
}

static int
abiword_storage_contains_statement( librdf_storage* storage, 
                                    librdf_statement* query )
{
    abiwordContext* c = abiwordContext::get( storage );

    PD_RDFModelIterator iter = c->m_model->begin();
    PD_RDFModelIterator    e = c->m_model->end();
    
    for( ; iter != e; ++iter )
    {
        PD_RDFStatement st = *iter;
        librdf_statement* stred = toRedland( st );
        RedStatementHolder h(stred);
        
        if( librdf_statement_match( stred, query ) )
            return 1;
    }
    
    return 0;
}


static int
abiword_storage_context_add_statement( librdf_storage* storage,
                                       librdf_node* context_node,
                                       librdf_statement* statement )
{
    cerr << "abiword_storage_context_add_statement() FIXME!" << endl;
    abiwordContext* c = abiwordContext::get( storage );
}

static int
abiword_storage_add_statement( librdf_storage* storage,
                               librdf_statement* statement )
{
  if( abiword_storage_contains_statement( storage, statement ))
    return 0;

  return abiword_storage_context_add_statement(storage, NULL, statement);
}

static int
abiword_storage_add_statements( librdf_storage* storage,
                                librdf_stream* statement_stream )
{
    int rc = 1;
    
    for( ; !librdf_stream_end(statement_stream);
         librdf_stream_next(statement_stream))
    {
        librdf_statement* statement    = librdf_stream_get_object(statement_stream);
        librdf_node*      context_node = librdf_stream_get_context2(statement_stream);

        if(abiword_storage_contains_statement(storage, statement))
            continue;

        rc &= abiword_storage_context_add_statement(storage, context_node, statement);
    }

    return rc;
}


static librdf_stream*
abiword_storage_context_serialise( librdf_storage* storage,
                                   librdf_node* context_node ) 
{
    return abiword_storage_find_statements_with_context( storage, 0, context_node );
}

static librdf_stream*
abiword_storage_serialise(librdf_storage* storage)
{
    return abiword_storage_find_statements_with_context( storage, 0, 0 );
}


void abiword_storage_factory( librdf_storage_factory* f )
{
    cerr << "abiword_storage_factory()" << endl;
    cerr << "factory->name:" << f->name << endl;

    f->version               = LIBRDF_STORAGE_INTERFACE_VERSION;
    f->init                  = abiword_storage_init;
    f->terminate             = abiword_storage_terminate;
    f->open                  = abiword_storage_open;
    f->close                 = abiword_storage_close;
    f->size                  = abiword_storage_size;
    f->find_statements       = abiword_storage_find_statements;
    f->serialise             = abiword_storage_serialise;
    f->context_add_statement = abiword_storage_context_add_statement;
    f->context_serialise     = abiword_storage_context_serialise;
    f->add_statement         = abiword_storage_add_statement;
    f->add_statements        = abiword_storage_add_statements;
    f->contains_statement    = abiword_storage_contains_statement;
}





/********************************************************************************/
/********************************************************************************/
/*** Public class interface *****************************************************/
/********************************************************************************/
/********************************************************************************/

static void
ensureStorageIsRegistered()
{
    static bool v = true;
    if( v )
    {
        v = false;
    
        int rc = librdf_storage_register_factory( getWorld(),
                                                  "abiword", "abiword",
                                                  abiword_storage_factory );
    }
}


static librdf_model* getRedlandModel( PD_RDFModelHandle abimodel )
{
    const char *storage_name   = "abiword";
    const char *name           = "abiword";
    const char *options_string = "";

    ensureStorageIsRegistered();
    
    librdf_storage* storage = librdf_new_storage( getWorld(),
                                                  storage_name, name,
                                                  options_string );
    cerr << "getRedlandModel() storage:" << storage << endl;
    if( !storage )
    {
        return 0;
    }
    abiwordContext* ac = abiwordContext::get( storage );
    ac->setModel( abimodel );
    cerr << "getRedlandModel(2) storage:" << storage << " abimodel:" << abimodel << endl;
    
    librdf_model* model = 0;
    int rc = librdf_storage_open( storage, model );
    model = librdf_new_model( getWorld(), storage, NULL );
    
    cerr << "getRedlandModel(3) storage:" << storage << " model:" << model << endl;
    return model;
}

/**
 * Perpare to execute a SPARQL query on the submodel 'm' of the whole
 * document RDF 'rdf'. If you want to execute the query against all
 * the RDF for a document omit the submodel PD_RDFModelHandle
 * parameter 'm'.
 */
PD_RDFQuery::PD_RDFQuery( PD_DocumentRDFHandle rdf, PD_RDFModelHandle m )
    : m_rdf(rdf)
    , m_model(m)
{
    if( !m_model )
    {
        m_model = m_rdf;
    }
}

PD_RDFQuery::~PD_RDFQuery()
{
}

PD_ResultBindings_t
PD_RDFQuery::executeQuery( const std::string& sparql_query_string )
{
    PD_ResultBindings_t ret;

    if( m_model->empty() )
    {
        // The redland backend assumes there are 1+ triples
        // to avoid the edge case where there is nothing to query
        return ret;
    }
    
    librdf_model* rdfmodel = getRedlandModel( m_model );
    librdf_uri*   base_uri = 0;
    librdf_query* query = librdf_new_query( getWorld(),
                                            "sparql", 0,
                                            (unsigned char*)sparql_query_string.c_str(),
                                            base_uri );
    librdf_query_results* results = librdf_query_execute( query, rdfmodel );
    if( !results )
    {
        return ret;
    }
    
    

    // convert redland results into our model format.
    for( ; !librdf_query_results_finished( results );
         librdf_query_results_next( results ))
    {
        cerr << "have query result, loop..." << endl;
        
        std::map< std::string, std::string > x;
        const char ** names = 0;
        librdf_node** values = 0;
        int bc = librdf_query_results_get_bindings_count( results );
        if( !bc )
            continue;
        cerr << "have query result, bc:" << bc << endl;
        
        values = (librdf_node**)calloc( bc+1, sizeof(librdf_node*));
        if( !librdf_query_results_get_bindings( results, &names, values ) )
        {
            const char * name  = names[0];
            librdf_node* value = values[0];
            cerr << "initial  name:" << name << endl;

            for( int i = 0; name; ++i, name = names[i], value = values[i] )
            {
                cerr << "i:" << i << " name:" << name << endl;
                x.insert( make_pair( name, tostr( value )));
                librdf_free_node( value );
            }
        }
        free(values);

        ret.push_back(x);
    }
    
    return ret;
}



