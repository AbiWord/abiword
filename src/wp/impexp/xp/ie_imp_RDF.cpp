/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_iconv.h"
#include "ie_imp_RDF.h"
#include "pd_Document.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"


#include "ap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Encoding.h"
#include "ap_Prefs.h"

#include "pt_PieceTable.h"
#include "pf_Frag_Strux.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sstream>
#include <list>

#define IE_MIMETYPE_VCard			"text/x-vcard"
#define IE_MIMETYPE_Calendar        "text/calendar"

IE_Imp_RDF_Sniffer::IE_Imp_RDF_Sniffer( const char * n )
	: IE_ImpSniffer( n, true )
{
}

IE_Imp_RDF_Sniffer::~IE_Imp_RDF_Sniffer ()
{
}

/*!
  Check if buffer contains data meant for this importer.

 We don't attmpt to recognize since other filetypes (HTML) can
 use the same encodings a text file can.
 We also don't want to steal recognition when user wants to use
 the Encoded Text importer.
 */
UT_Confidence_t
IE_Imp_RDF_Sniffer::recognizeContents( const char * szBuf, UT_uint32 iNumbytes )
{
    UT_UNUSED( szBuf );
    UT_UNUSED( iNumbytes );
    return UT_CONFIDENCE_ZILCH;
}

/**********/
/**********/
/**********/

IE_Imp_RDF_VCard_Sniffer::IE_Imp_RDF_VCard_Sniffer()
    :
    IE_Imp_RDF_Sniffer( IE_MIMETYPE_VCard )
{
}

IE_Imp_RDF_VCard_Sniffer::~IE_Imp_RDF_VCard_Sniffer()
{
}

const IE_SuffixConfidence * IE_Imp_RDF_VCard_Sniffer::getSuffixConfidence ()
{
    static IE_SuffixConfidence ret[] = {
        { "vcf", 	UT_CONFIDENCE_PERFECT 	},
        { "vcard", 	UT_CONFIDENCE_PERFECT 	},
        { "", 	UT_CONFIDENCE_ZILCH 	}
    };
    return ret;
}

const IE_MimeConfidence * IE_Imp_RDF_VCard_Sniffer::getMimeConfidence ()
{
    static IE_MimeConfidence ret[] = {
        { IE_MIME_MATCH_FULL, 	IE_MIMETYPE_VCard, 	UT_CONFIDENCE_GOOD 	},
        { IE_MIME_MATCH_CLASS, 	"text", 			UT_CONFIDENCE_SOSO 	}, 
        { IE_MIME_MATCH_BOGUS, 	"", 				UT_CONFIDENCE_ZILCH }
    };
    return ret;
}

UT_Error IE_Imp_RDF_VCard_Sniffer::constructImporter( PD_Document * pDocument,
                                                      IE_Imp ** ppie)
{
	IE_Imp_RDF* p = new IE_Imp_RDF_VCard( pDocument, false );
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_RDF_VCard_Sniffer::getDlgLabels( const char ** /*pszDesc*/,
                                             const char ** /*pszSuffixList*/,
                                             IEFileType * /*ft*/ )
{
    return false;
	// *pszDesc = "VCard (.vcf, .vcard)";
	// *pszSuffixList = "*.vcf; *.vcard";
	// *ft = getFileType();
	// return true;
}


/**********/
/**********/
/**********/

IE_Imp_RDF_Calendar_Sniffer::IE_Imp_RDF_Calendar_Sniffer()
    :
    IE_Imp_RDF_Sniffer( IE_MIMETYPE_Calendar )
{
}

IE_Imp_RDF_Calendar_Sniffer::~IE_Imp_RDF_Calendar_Sniffer()
{
}

const IE_SuffixConfidence * IE_Imp_RDF_Calendar_Sniffer::getSuffixConfidence ()
{
    static IE_SuffixConfidence ret[] = {
//        { "ical",  	    UT_CONFIDENCE_PERFECT 	},
//        { "ics",    	UT_CONFIDENCE_PERFECT 	},
        { "", 	UT_CONFIDENCE_ZILCH 	}
    };
    return ret;
}

const IE_MimeConfidence * IE_Imp_RDF_Calendar_Sniffer::getMimeConfidence ()
{
    static IE_MimeConfidence ret[] = {
        { IE_MIME_MATCH_FULL, 	IE_MIMETYPE_Calendar, UT_CONFIDENCE_GOOD 	},
        { IE_MIME_MATCH_CLASS, 	"text", 			UT_CONFIDENCE_SOSO 	}, 
        { IE_MIME_MATCH_BOGUS, 	"", 				UT_CONFIDENCE_ZILCH }
    };
    return ret;
}

UT_Error IE_Imp_RDF_Calendar_Sniffer::constructImporter( PD_Document * pDocument,
                                                      IE_Imp ** ppie)
{
	IE_Imp_RDF* p = new IE_Imp_RDF_Calendar( pDocument, false );
	*ppie = p;
	return UT_OK;
}

bool IE_Imp_RDF_Calendar_Sniffer::getDlgLabels( const char ** /*pszDesc*/,
												const char ** /*pszSuffixList*/,
												IEFileType * /*ft*/ )
{
    return false;
	// *pszDesc = "Calendar (.ical, .ics)";
	// *pszSuffixList = "*.ical; *.ics";
	// *ft = getFileType();
	// return true;
}



/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

IE_Imp_RDF::IE_Imp_RDF( PD_Document * pDocument, bool bEncoded )
  : IE_Imp(pDocument)
{
    UT_UNUSED( bEncoded );
}

IE_Imp_RDF::IE_Imp_RDF( PD_Document * pDocument, const char * encoding )
  : IE_Imp(pDocument)
{
    UT_UNUSED( encoding );
}

IE_Imp_RDF::~IE_Imp_RDF ()
{
}


UT_Error
IE_Imp_RDF::_loadFile( GsfInput * fp )
{
    UT_UNUSED( fp );
    return UT_ERROR;
}


bool
IE_Imp_RDF::pasteFromBuffer( PD_DocumentRange * pDocRange,
                             const unsigned char * pData, UT_uint32 lenData,
                             const char *szEncoding )
{
    UT_UNUSED(szEncoding);
    
	UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
	UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);

    std::stringstream ss;
    ss.write( (const char*)pData, lenData );
    UT_DEBUGMSG(("IE_Imp_RDF::pasteFromBuffer() have data:%s\n", ss.str().c_str() ));
	setClipboard (pDocRange->m_pos1);

    bool ret = pasteFromBufferSS( pDocRange, ss, szEncoding );
    
	return ret;
}

bool
IE_Imp_RDF::pasteFromBufferSS( PD_DocumentRange * /*pDocRange*/,
                               std::stringstream& /*ss*/,
                               const char * /*szEncoding*/ )
{
    UT_DEBUGMSG(("IE_Imp_RDF::pasteFromBufferSS() doing nothing...\n"));
    return true;
}

#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "pd_DocumentRDF.h"

std::pair< PT_DocPosition, PT_DocPosition >
IE_Imp_RDF::insertTextWithXMLID( const std::string& textconst,
                                 const std::string& xmlid )
{
    std::string text = " " + textconst + " ";
    PT_DocPosition startpos = getDocPos();
	// FIXME
    /*bool bRes =*/ appendSpan( text );
    PT_DocPosition endpos = getDocPos();
    startpos++;
    endpos--;
    
    XAP_Frame* lff = XAP_App::getApp()->getLastFocussedFrame();
    if(lff) 
    {
        FV_View * pView = static_cast<FV_View*>( lff->getCurrentView() );
        pView->selectRange( startpos, endpos );
        pView->cmdInsertXMLID( xmlid );
    }

    return std::make_pair( startpos, endpos );    
}



/**********/
/**********/
/**********/

IE_Imp_RDF_VCard::IE_Imp_RDF_VCard( PD_Document * pDocument, bool bEncoded )
    : IE_Imp_RDF( pDocument, bEncoded )
{
}

IE_Imp_RDF_VCard::IE_Imp_RDF_VCard( PD_Document * pDocument, const char * encoding )
    : IE_Imp_RDF( pDocument, encoding )
{
}

IE_Imp_RDF_VCard::~IE_Imp_RDF_VCard()
{
}


// #ifdef WITH_EVOLUTION_DATA_SERVER
// extern "C" {
//   #include <libebook/e-book.h>
// };

// static std::string get( EVCard* c, const char* v )
// {
//     EVCardAttribute* a = e_vcard_get_attribute( c, v );

//     if( a && e_vcard_attribute_is_single_valued(a) )
//     {
//         return e_vcard_attribute_get_value(a);
//     }
//     return "";
// }

// static void addFoafProp( PD_DocumentRDFMutationHandle m,
//                          EVCard* c,
//                          const char* vckey,
//                          const PD_URI& uuidnode,
//                          const std::string& predend )
// {
//     PD_URI pred("http://xmlns.com/foaf/0.1/" + predend );
//     std::string objdata = get( c, vckey );
//     if( !objdata.empty() )
//     {
//         m->add( uuidnode, pred, PD_Literal( objdata ));
//     }
// }

// #endif


bool
IE_Imp_RDF_VCard::pasteFromBufferSS( PD_DocumentRange * pDocRange,
                                     std::stringstream& inputss,
                                     const char * szEncoding )
{
#ifndef WITH_EVOLUTION_DATA_SERVER
	UT_UNUSED(pDocRange);
	UT_UNUSED(inputss);
	UT_UNUSED(szEncoding);
    UT_DEBUGMSG(("can not parse vcards!\n"));
    return true;
#else

    UT_DEBUGMSG(("trying to get card for data:%s\n",inputss.str().c_str() ));

    PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
    PD_RDFSemanticItemHandle obj = PD_RDFSemanticItem::createSemanticItem( rdf, "Contact" );
    obj->importFromData( inputss, rdf, pDocRange );
    
    // if( EVCard* c = e_vcard_new_from_string( inputss.str().c_str() ) )
    // {
    //     std::string textrep = "";
    //     typedef std::list< char* > charplist_t;
    //     charplist_t textreplist;
    //     textreplist.push_back( EVC_EMAIL );
    //     textreplist.push_back( EVC_FN );
    //     textreplist.push_back( EVC_NICKNAME );
    //     textreplist.push_back( EVC_UID );
    //     for( charplist_t::iterator iter = textreplist.begin();
    //          iter != textreplist.end(); ++iter )
    //     {
    //         textrep = get( c, *iter );
    //         if( !textrep.empty() )
    //             break;
    //     }
    //     UT_DEBUGMSG(("have card!\n"));

    //     PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
        
    //     std::string fn    = get( c, EVC_FN );
    //     std::string uid   = get( c, EVC_UID );
    //     std::string xmlid = rdf->makeLegalXMLID( fn + "_" + uid );
    //     std::string email = get( c, EVC_EMAIL );

    //     std::pair< PT_DocPosition, PT_DocPosition > se = insertTextWithXMLID( textrep, xmlid );
    //     PT_DocPosition startpos = se.first;
    //     PT_DocPosition   endpos = se.second;
        
    //     std::string uuid = "http://abicollab.net/rdf/foaf#" + xmlid;
    //     PD_URI uuidnode(uuid);
    //     PD_DocumentRDFMutationHandle m = rdf->createMutation();
    //     m->add( PD_URI(uuid),
    //             PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"),
    //             PD_Literal( xmlid ) );
    //     m->add( PD_URI(uuid),
    //             PD_URI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"),
    //             PD_Object("http://xmlns.com/foaf/0.1/Person") );
    //     addFoafProp( m, c, EVC_TEL,      uuidnode, "phone" );
    //     addFoafProp( m, c, EVC_NICKNAME, uuidnode, "nick" );
    //     addFoafProp( m, c, EVC_FN,       uuidnode, "name" );
    //     addFoafProp( m, c, EVC_N,        uuidnode, "givenName" );
    //     addFoafProp( m, c, EVC_X_JABBER, uuidnode, "jabberID" );
            
    //     m->commit();
    // }

    return true;

#endif
}

/**********/
/**********/
/**********/

IE_Imp_RDF_Calendar::IE_Imp_RDF_Calendar( PD_Document * pDocument, bool bEncoded )
    : IE_Imp_RDF( pDocument, bEncoded )
{
}

IE_Imp_RDF_Calendar::IE_Imp_RDF_Calendar( PD_Document * pDocument, const char * encoding )
    : IE_Imp_RDF( pDocument, encoding )
{
}

IE_Imp_RDF_Calendar::~IE_Imp_RDF_Calendar()
{
}

#ifdef WITH_LIBICAL
extern "C" {
  #include <libical/ical.h>
};

#if 0
static std::string tostr( time_t v )
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}


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

// static void addFoafProp( PD_DocumentRDFMutationHandle m,
//                          ECalendar* c,
//                          const char* vckey,
//                          const PD_URI& uuidnode,
//                          const std::string& predend )
// {
//     PD_URI pred("http://xmlns.com/foaf/0.1/" + predend );
//     std::string objdata = get( c, vckey );
//     if( !objdata.empty() )
//     {
//         m->add( uuidnode, pred, PD_Literal( objdata ));
//     }
// }

#endif


bool
IE_Imp_RDF_Calendar::pasteFromBufferSS( PD_DocumentRange * pDocRange,
                                        std::stringstream& inputss,
                                        const char * szEncoding )
{
	UT_UNUSED(szEncoding);
#ifndef WITH_LIBICAL
	UT_UNUSED(pDocRange);
	UT_UNUSED(inputss);
    UT_DEBUGMSG(("can not parse calendars!\n"));
    return true;
#else

    
    UT_DEBUGMSG(("trying to get calendar for data:%s\n",inputss.str().c_str() ));
    PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
    PD_RDFSemanticItemHandle obj = PD_RDFSemanticItem::createSemanticItem( rdf, "Event" );
    obj->importFromData( inputss, rdf, pDocRange );

    
    // if( icalcomponent* c = icalcomponent_new_from_string( inputss.str().c_str() ) )
    // {
    //     const char* desc = icalcomponent_get_description( c );
    //     const char* loc  = icalcomponent_get_location( c );
    //     const char* summary = icalcomponent_get_summary( c );
    //     const char* uid  = icalcomponent_get_uid( c );
    //     struct icaltimetype dtstart = icalcomponent_get_dtstart( c );
    //     struct icaltimetype dtend   = icalcomponent_get_dtend( c );

    //     std::string textrep;
    //     std::string xmlid;
    //     if( summary )
    //     {
    //         xmlid += (std::string)summary + "_";
    //         textrep = summary;
    //     }
    //     xmlid += uid;
    //     PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
    //     xmlid = rdf->makeLegalXMLID( xmlid );
    //     if( textrep.empty() )
    //     {
    //         if( desc ) 
    //             textrep = desc;
    //         else
    //             textrep = uid;
    //     }

    //     std::pair< PT_DocPosition, PT_DocPosition > se = insertTextWithXMLID( textrep, xmlid );
    //     PT_DocPosition startpos = se.first;
    //     PT_DocPosition   endpos = se.second;

    //     std::string predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    //     std::string uuid = "http://abicollab.net/rdf/cal#" + xmlid;
    //     PD_URI uuidnode(uuid);
    //     PD_DocumentRDFMutationHandle m = rdf->createMutation();
    //     m->add( PD_URI(uuid),
    //             PD_URI("http://docs.oasis-open.org/opendocument/meta/package/common#idref"),
    //             PD_Literal( xmlid ) );
    //     m->add( PD_URI(uuid),
    //             PD_URI("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"),
    //             PD_Object(predBase + "Vevent") );

    //     addCalPropSZ( m, uuidnode, "summary",     summary );
    //     addCalPropSZ( m, uuidnode, "location",    loc );
    //     addCalPropSZ( m, uuidnode, "uid",         uid );
    //     addCalPropSZ( m, uuidnode, "description", desc );

    //     addCalProp( m, uuidnode, "dtstart", tostr(icaltime_as_timet( dtstart )));
    //     addCalProp( m, uuidnode, "dtend",   tostr(icaltime_as_timet( dtend )));
        
    //     m->commit();
        
    // }
    
    return true;

#endif
}
