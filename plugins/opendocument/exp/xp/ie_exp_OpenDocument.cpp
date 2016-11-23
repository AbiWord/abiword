/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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


// Class definition include
#include "ie_exp_OpenDocument.h"

// Internal includes
#include "ODe_AbiDocListener.h"
#include "ODe_AuxiliaryData.h"
#include "ODe_Common.h"
#include "ODe_DocumentData.h"
#include "ODe_HeadingSearcher_Listener.h"
#include "ODe_TOC_Listener.h"
#include "ODe_ManifestWriter.h"
#include "ODe_RDFWriter.h"
#include "ODe_Main_Listener.h"
#include "ODe_MetaDataWriter.h"
#include "ODe_ThumbnailsWriter.h"
#include "ODe_PicturesWriter.h"
#include "ODe_SettingsWriter.h"

// Abiword includes
#include <ut_assert.h>
#include <ut_locale.h>
#include <pd_Document.h>
#include <pd_DocumentRDF.h>
#include "ie_exp_DocRangeListener.h"
#include "pl_ListenerCoupleCloser.h"

// External includes
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-outfile-stdio.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-input-stdio.h>


#ifdef _WIN32
#include <io.h>
#endif
#include <glib.h>
#include <glib/gstdio.h>

/**
 * Constructor
 */
IE_Exp_OpenDocument::IE_Exp_OpenDocument (PD_Document * pDoc)
  : IE_Exp (pDoc), m_odt(0)
{
}


/**
 * Destructor
 */
IE_Exp_OpenDocument::~IE_Exp_OpenDocument()
{
}

GsfOutput* IE_Exp_OpenDocument::_openFile(const char *szFilename)
{
  GsfOutput *output = NULL;

  const std::string & prop = getProperty ("uncompressed");

  if (!prop.empty() && UT_parseBool (prop.c_str (), false))
    {
      char *filename = UT_go_filename_from_uri (szFilename);
      if (filename) 
	{
	  output = (GsfOutput*)gsf_outfile_stdio_new (filename, NULL);
	  g_free (filename);
	}
    }
  else
    {
      output = IE_Exp::_openFile (szFilename);
    }

  return output;
}

void IE_Exp_OpenDocument::setGSFOutput(GsfOutput * pBuf)
{
    m_odt = gsf_output_container(pBuf);
}

/*!
 * This method copies the selection defined by pDocRange to ODT format
 * placed in the ByteBuf bufODT
 */
UT_Error IE_Exp_OpenDocument::copyToBuffer(PD_DocumentRange * pDocRange, const UT_ByteBufPtr & bufODT)
{
    //
    // First export selected range to a tempory document
    //
    PD_Document * outDoc = new PD_Document();
    outDoc->createRawDocument();
    IE_Exp_DocRangeListener * pRangeListener = new IE_Exp_DocRangeListener(pDocRange,outDoc);
    UT_DEBUGMSG(("DocumentRange low %d High %d \n",pDocRange->m_pos1,pDocRange->m_pos2));
    PL_ListenerCoupleCloser* pCloser = new PL_ListenerCoupleCloser();
    pDocRange->m_pDoc->tellListenerSubset(pRangeListener,pDocRange,pCloser);
    if( pCloser)
        delete pCloser;
    
    //
    // Grab the RDF triples while we are copying...
    //
    if( PD_DocumentRDFHandle outrdf = outDoc->getDocumentRDF() )
    {

        std::set< std::string > xmlids;
        PD_DocumentRDFHandle inrdf = pDocRange->m_pDoc->getDocumentRDF();
        inrdf->addRelevantIDsForRange( xmlids, pDocRange );

        if( !xmlids.empty() )
        {
            UT_DEBUGMSG(("MIQ: ODF export creating restricted RDF model xmlids.sz:%ld \n",(long)xmlids.size()));
            PD_RDFModelHandle subm = inrdf->createRestrictedModelForXMLIDs( xmlids );
            PD_DocumentRDFMutationHandle m = outrdf->createMutation();
            m->add( subm );
            m->commit();
            subm->dumpModel("copied rdf triples subm");
            outrdf->dumpModel("copied rdf triples result");
        }
        
        // PD_DocumentRDFMutationHandle m = outrdf->createMutation();
        // m->add( PD_URI("http://www.example.com/foo"),
        //         PD_URI("http://www.example.com/bar"),
        //         PD_Literal("copyToBuffer path") );
        // m->commit();
    }
    outDoc->finishRawCreation();
    //
    // OK now we have a complete and valid document containing our selected 
    // content. We export this to an in memory GSF buffer
    //
    IE_Exp * pNewExp = NULL; 
    char *szTempFileName = NULL;
    GError *err = NULL;
    g_file_open_tmp ("XXXXXX", &szTempFileName, &err);
    GsfOutput * outBuf =  gsf_output_stdio_new (szTempFileName,&err);
    IEFileType ftODT = IE_Exp::fileTypeForMimetype("application/vnd.oasis.opendocument.text");
    UT_Error aerr = IE_Exp::constructExporter(outDoc,outBuf,
					     ftODT,&pNewExp);
    if(pNewExp == NULL)
    {
         return aerr;
    }
    aerr = pNewExp->writeFile(szTempFileName);
    if(aerr != UT_OK)
    {
	delete pNewExp;
	delete pRangeListener;
	UNREFP( outDoc);
	g_remove(szTempFileName);
	g_free (szTempFileName);
	return aerr;
    }
    //
    // File is closed at the end of the export. Open it again.
    //

    GsfInput *  fData = gsf_input_stdio_new(szTempFileName,&err);
    UT_DebugOnly<UT_sint32> siz = gsf_input_size(fData);
    const UT_Byte * pData = gsf_input_read(fData,gsf_input_size(fData),NULL);
    UT_DEBUGMSG(("Writing %d bytes to clipboard \n", (UT_sint32)siz));
    bufODT->append( pData, gsf_input_size(fData));
    
    delete pNewExp;
    delete pRangeListener;
    UNREFP( outDoc);
    g_remove(szTempFileName);
    g_free (szTempFileName);
    return aerr;
}

/**
 * This writes out our AbiWord file as an OpenOffice
 * compound document
 */
UT_Error IE_Exp_OpenDocument::_writeDocument(void)
{
	ODe_DocumentData docData(getDoc());
	ODe_AuxiliaryData auxData;
	ODe_AbiDocListener* pAbiDocListener = NULL;
	ODe_AbiDocListenerImpl* pAbiDocListenerImpl = NULL;
    
	UT_return_val_if_fail (getFp(), UT_ERROR);

    PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
    auxData.m_additionalRDF = rdf->createScratchModel();
    
	const std::string & prop = getProperty ("uncompressed");
	
	if (!prop.empty() && UT_parseBool (prop.c_str (), false))
	  {
	    m_odt = GSF_OUTFILE(g_object_ref(G_OBJECT(getFp())));
	  }
	else
	  {
	    GError* error = NULL;
	    m_odt = GSF_OUTFILE (gsf_outfile_zip_new (getFp(), &error));
	    
	    if (error)
	      {
		UT_DEBUGMSG(("Error writing odt file: %s\n", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	      }
	  }

	UT_return_val_if_fail(m_odt, UT_ERROR);

	// Needed to ensure that all *printf writes numbers correctly,
	// like "45.56mm" instead of "45,56mm".
	UT_LocaleTransactor numericLocale (LC_NUMERIC, "C");
	{
		GsfOutput * mimetype = gsf_outfile_new_child_full (m_odt, "mimetype", FALSE, "compression-level", 0, (void*)0);
		if (!mimetype)
		{
			ODe_gsf_output_close(GSF_OUTPUT(m_odt));
			return UT_ERROR;
		}

		ODe_gsf_output_write(mimetype,
				39 /*39 == strlen("application/vnd.oasis.opendocument.text")*/,
				(const guint8 *)"application/vnd.oasis.opendocument.text");

		ODe_gsf_output_close(mimetype);
    }

	if (!ODe_MetaDataWriter::writeMetaData(getDoc(), m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}

	if (!ODe_ThumbnailsWriter::writeThumbnails(getDoc(), m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}

    if (!ODe_SettingsWriter::writeSettings(getDoc(), m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
	return UT_ERROR;
	}

	if (!ODe_PicturesWriter::writePictures(getDoc(), m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}


	if (!ODe_ManifestWriter::writeManifest(getDoc(), m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}


    // Gather all paragraph style names used by heading paragraphs
    // (ie. all paragraph styles that are used to build up TOCs).

    pAbiDocListenerImpl = new ODe_HeadingSearcher_Listener(docData.m_styles, auxData);
    pAbiDocListener = new ODe_AbiDocListener(getDoc(),
                                             pAbiDocListenerImpl, false);

	if (!getDoc()->tellListener(static_cast<PL_Listener *>(pAbiDocListener)))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}
    pAbiDocListener->finished();
    
    DELETEP(pAbiDocListener);
    DELETEP(pAbiDocListenerImpl);

    // Now that we have all paragraph styles that build up the TOCs in the 
    // document (if any), we can build up the TOC bodies. We do this because
    // OpenOffice.org requires the TOC bodies to be present and filled
    // when initially opening the document. Without it, it will show 
    // an empty TOC until the user regenerates it, which is not that pretty.
    // Annoyingly we have to build up the TOC ourselves during export, as
    // it doesn't exist within AbiWord's PieceTable. Until that changes, this
    // is the best we can do.

    if (auxData.m_pTOCContents) {
        pAbiDocListenerImpl = new ODe_TOC_Listener(auxData);
        pAbiDocListener = new ODe_AbiDocListener(getDoc(),
                                                 pAbiDocListenerImpl, false);

	    if (!getDoc()->tellListener(static_cast<PL_Listener *>(pAbiDocListener)))
	    {
		    ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		    return UT_ERROR;
	    }
        pAbiDocListener->finished();
        
        DELETEP(pAbiDocListener);
        DELETEP(pAbiDocListenerImpl);
    }

    
    // Gather document content and styles

    if (!docData.doPreListeningWork()) {
      ODe_gsf_output_close(GSF_OUTPUT(m_odt));
      return UT_ERROR;
    }

    pAbiDocListenerImpl = new ODe_Main_Listener(docData, auxData);
    pAbiDocListener = new ODe_AbiDocListener(getDoc(),
                                             pAbiDocListenerImpl, false);

	if (!getDoc()->tellListener(static_cast<PL_Listener *>(pAbiDocListener)))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}
	pAbiDocListener->finished();
    
	DELETEP(pAbiDocListener);
	DELETEP(pAbiDocListenerImpl);
    
	if (!docData.doPostListeningWork())
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}

    // Write RDF.
    if (!ODe_RDFWriter::writeRDF(getDoc(), m_odt, auxData.m_additionalRDF ))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}
    
    // Write content and styles
        
	if (!docData.writeStylesXML(m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}
	if (!docData.writeContentXML(m_odt))
	{
		ODe_gsf_output_close(GSF_OUTPUT(m_odt));
		return UT_ERROR;
	}

	ODe_gsf_output_close(GSF_OUTPUT(m_odt));
	return UT_OK;
}
