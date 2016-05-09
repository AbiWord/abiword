/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1998-2000 Hubert Figuiere
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
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_ClarisWorks.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_clarisworks_register
#define abi_plugin_unregister abipgn_clarisworks_unregister
#define abi_plugin_supports_version abipgn_clarisworks_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("ClarisWorks")
#endif

/*****************************************************************/
/*****************************************************************/

// completely generic code to allow this to be a plugin

// we use a reference-counted sniffer
static IE_Imp_ClarisWorks_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_ClarisWorks_Sniffer ();
	}

	mi->name = "ClarisWorks Importer";
	mi->desc = "Import ClarisWorks Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Imp::unregisterImporter (m_sniffer);
	delete m_sniffer;
	m_sniffer = 0;

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}

/*****************************************************************/
/* Generic IO                                                    */
/*****************************************************************/


/*****************************************************************/
/*****************************************************************/

/*
  Import ClarisWorks/AppleWorks file
*/

enum {
    CW_HANDLED_VERSION = 5
};


/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(ies,exp)	do { if (((ies)=(exp)) != UT_OK) goto Cleanup; } while (0)


UT_Error IE_Imp_ClarisWorks::_loadFile (GsfInput * fp)
{
    UT_Error iestatus;
    
    X_CleanupIfError(iestatus,_writeHeader(fp));
    X_CleanupIfError(iestatus,_parseFile(fp));
    
    iestatus = UT_OK;

 Cleanup:
    return iestatus;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_ClarisWorks_Sniffer::IE_Imp_ClarisWorks_Sniffer () :
  IE_ImpSniffer("AbiClarisWorks::CWK")
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_ClarisWorks_Sniffer__SuffixConfidence[] = {
	{ "cwk", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_ClarisWorks_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_ClarisWorks_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_ClarisWorks_Sniffer::recognizeContents(const char * szBuf, 
												   UT_uint32 iNumbytes)
{
    if (iNumbytes >= 8) 
    {
        if ((szBuf[4] == 'B') && (szBuf[5] == 'O') && (szBuf [6] == 'B')
            && (szBuf [7] == 'O'))
        {
            if (szBuf [0] == CW_HANDLED_VERSION) 
            {
                return (UT_CONFIDENCE_PERFECT);
            }
            else 
            {
                UT_DEBUGMSG (("%s,%d: Mismatch version.\n",__FILE__,__LINE__));
                return (UT_CONFIDENCE_SOSO);
            }
        }
    
    }

    return(UT_CONFIDENCE_ZILCH);
}

UT_Error IE_Imp_ClarisWorks_Sniffer::constructImporter(PD_Document * pDocument,
													   IE_Imp ** ppie)
{
   IE_Imp_ClarisWorks * p = new IE_Imp_ClarisWorks(pDocument);
   *ppie = p;
   return UT_OK;
}

bool	IE_Imp_ClarisWorks_Sniffer::getDlgLabels(const char ** pszDesc,
												 const char ** pszSuffixList,
												 IEFileType * ft)
{
   *pszDesc = "ClarisWorks/AppleWorks 5 (.cwk)";
   *pszSuffixList = "*.cwk";
   *ft = getFileType();
   return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_ClarisWorks::~IE_Imp_ClarisWorks()
{
}

IE_Imp_ClarisWorks::IE_Imp_ClarisWorks(PD_Document * pDocument)
    : IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,ies)		do { bool b = (exp); if (!b) return (ies); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_ClarisWorks::_writeHeader(GsfInput * /* fp */)
{
    X_ReturnNoMemIfError(appendStrux(PTX_Section, PP_NOPROPS));
    
    return UT_OK;
}

UT_Error IE_Imp_ClarisWorks::_parseFile(GsfInput * fp)
{
    unsigned char buf [128];   // general purpose buffer (128 bytes, no more)
    UT_uint32 offset;
    UT_GrowBuf gbBlock(1024);
    bool bEmptyFile = true;
    unsigned char c;
    
    
    // read the final header to get the ETBL offset
    gsf_input_seek (fp, -24, G_SEEK_END);
    
	gsf_input_read (fp, 4, (guint8*)&buf);
   if (strncmp (reinterpret_cast<char *>(buf), "ETBL", 4) != 0) 
   {
       // ERROR !
       UT_WARNINGMSG(("ETBL marker not found!\n"));
   }
   gsf_input_read (fp, sizeof (offset), (guint8*)&offset);
   {
       if (offset >= gsf_input_tell (fp)) 
       {
           // ERROR again !
           UT_WARNINGMSG(("incorrect offset for ETBL struct !\n"));
       }
   }
   
   // moving to ETBL 
   gsf_input_seek (fp, offset, G_SEEK_SET);
   
   gsf_input_read (fp, 4, (guint8*)&buf);
   if (strncmp (reinterpret_cast<char *>(buf), "ETBL", 4) != 0) 
   {
       // ERROR !
       UT_WARNINGMSG(("ETBL marker from ETBL not found!\n"));
   }
   gsf_input_read (fp, sizeof (offset), (guint8*)&offset);
   {
       if (offset >= gsf_input_tell (fp)) 
       {
           // ERROR again !
           UT_WARNINGMSG(("incorrect offset for  struct !\n"));
       }
   }
   
   
   while (gsf_input_read(fp, 1, &c) != NULL)
   {
       switch (c)
       {
           
       case 0x0D:// ClarisWorks uses 0x0D as an EOL, like on the Macintosh
           
           // start a paragraph and emit any text that we
           // have accumulated.
           X_ReturnNoMemIfError(appendStrux(PTX_Block, PP_NOPROPS));
           bEmptyFile = false;
           if (gbBlock.getLength() > 0)
           {
               X_ReturnNoMemIfError(appendSpan(reinterpret_cast<const UT_UCSChar *>(gbBlock.getPointer(0)), gbBlock.getLength()));
               gbBlock.truncate(0);
           }
           break;
           
       default:
           
           // deal with plain character.
           // this cast is OK.  we have US-ASCII (actually Latin-1) character
           // data, so we can do this.
           
           // TODO consider scanning for UTF8...
           
           UT_GrowBufElement gbe = static_cast<UT_GrowBufElement>(static_cast<UT_UCSChar>(c));
           X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&gbe,1));
           break;
       }
   } 
   
   if (gbBlock.getLength() > 0 || bEmptyFile)
   {
       // if we have text left over (without final CR/LF),
       // or if we read an empty file,
       // create a paragraph and emit the text now.
       X_ReturnNoMemIfError(appendStrux(PTX_Block, PP_NOPROPS));
       if (gbBlock.getLength() > 0)
           X_ReturnNoMemIfError(appendSpan(reinterpret_cast<const UT_UCSChar *>(gbBlock.getPointer(0)), gbBlock.getLength()));
   }
   
   
   return UT_OK;
}



#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail


/*****************************************************************/
/*****************************************************************/
