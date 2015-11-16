/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#define DUMP_COPY_TEXT  1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_bytebuf.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"

#include "ie_exp.h"
#include "ie_exp_AbiWord_1.h"

#include "fl_DocLayout.h"
#include "pd_Document.h"

#include <gsf/gsf-output-impl.h>

static UT_GenericVector<IE_ExpSniffer *> m_sniffers(20);

/*****************************************************************/
/*****************************************************************/

#include "fv_View.h"
#include "xap_App.h"
#include "gr_Graphics.h"

class ABI_EXPORT IE_FieldUpdater
{
public:
	
	IE_FieldUpdater()
	{
		updatedFields_ = false;
	}

	void updateFields(PD_Document * pDoc)
	{
		if (updatedFields_)
			return;

		GR_Graphics * graphics = GR_Graphics::newNullGraphics();

		if (graphics)
			{
				FL_DocLayout * pDocLayout = new FL_DocLayout(pDoc, graphics);
				FV_View * printView = new FV_View(XAP_App::getApp(), 0, pDocLayout);
				
				printView->getLayout()->fillLayouts();
				printView->getLayout()->formatAll();
				printView->getLayout()->recalculateTOCFields();
				
				DELETEP(pDocLayout);
				DELETEP(printView);
				DELETEP(graphics);
				
				updatedFields_ = true;
			}
	}

private:

	bool updatedFields_;
};

/*****************************************************************/
/*****************************************************************/

IE_ExpSniffer::IE_ExpSniffer (const char * _name, bool canCopy)
	: m_name(_name),
	  m_type(IEFT_Bogus),
	  m_bCanCopy(canCopy)
{
}

IE_ExpSniffer::~IE_ExpSniffer ()
{
}

UT_UTF8String IE_ExpSniffer::getPreferredSuffix()
{
	const char * szDummy;
	const char * szSuffixes = 0;
	IEFileType ieftDummy;
	
	if (!getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
		return "";

    UT_String suffixes(szSuffixes);

    // semicolon-delimited list of suffixes
    size_t first_suffix_end = UT_String_findCh(suffixes, ';');
    if(first_suffix_end == (size_t)-1)
      first_suffix_end = suffixes.size();

    // strip off the '*'
    return UT_UTF8String(suffixes.substr(1, first_suffix_end - 1).c_str());
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp::registerExporter (IE_ExpSniffer * s)
{
	UT_sint32 ndx = 0;
	UT_Error err = m_sniffers.addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);

	s->setFileType(ndx+1);
}

void IE_Exp::unregisterExporter (IE_ExpSniffer * s)
{
	UT_uint32 ndx = 0;

	ndx = s->getFileType(); // 1:1 mapping

	m_sniffers.deleteNthItem (ndx-1);

	// Refactor the indexes
	IE_ExpSniffer * pSniffer = 0;
	UT_uint32 size  = m_sniffers.size();
	UT_uint32 i     = 0;
	for( i = ndx-1; i < size; i++)
	{
		pSniffer = m_sniffers.getNthItem(i);
		if (pSniffer)
        	pSniffer->setFileType(i+1);
	}
}

void IE_Exp::unregisterAllExporters ()
{
	IE_ExpSniffer * pSniffer = 0;
	UT_uint32 size = m_sniffers.size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		pSniffer = m_sniffers.getNthItem(i);
		DELETEP(pSniffer);
	}

	m_sniffers.clear();
}

/*****************************************************************/
/*****************************************************************/

IE_Exp::IE_Exp(PD_Document * pDocument, UT_Confidence_t fidelity)
	: m_error(false), m_pDocument(pDocument),
	  m_pDocRange (0), m_pByteBuf(0),
	  m_fp(0), m_bOwnsFp(false), m_fidelity(fidelity),
	  m_fieldUpdater(0)
{
	m_pDocument->invalidateCache();
}

IE_Exp::~IE_Exp()
{
	if (m_fp)
		_closeFile();

	DELETEP(m_fieldUpdater);
}

/*****************************************************************/
/*****************************************************************/

void IE_Exp::populateFields()
{
	if(isCopying())
		return;

	if (!m_fieldUpdater)
		m_fieldUpdater = new IE_FieldUpdater;

	m_fieldUpdater->updateFields (getDoc ());
}

void IE_Exp::setProps (const char * props)
{
	m_props_map.clear();
	UT_parse_properties(props, m_props_map);
}

GsfOutput* IE_Exp::_openFile(const char *szFilename)
{
	return UT_go_file_create(szFilename, NULL);
}

GsfOutput* IE_Exp::openFile(const char * szFilename)
{
	UT_return_val_if_fail(!m_fp, NULL);
	UT_return_val_if_fail(szFilename, NULL);

	m_szFileName = szFilename;

	GsfOutput* file = _openFile(szFilename);
	if (file) {
		gsf_output_set_name (file, szFilename);
	}

	return file;
}

UT_uint32 IE_Exp::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	if(!pBytes || !length)
	  return 0;

	if (!gsf_output_write(m_fp, length, pBytes))
		return 0;
	return length;
}

bool IE_Exp::_writeBytes(const UT_Byte * sz)
{
	size_t length = strlen(reinterpret_cast<const char *>(sz));
	return (_writeBytes(sz,length)==static_cast<UT_uint32>(length));
}

bool IE_Exp::_closeFile(void)
{
	if (m_fp && m_bOwnsFp) {
		gboolean res = TRUE;

		if(!gsf_output_is_closed(m_fp))
			res = gsf_output_close(m_fp);

		g_object_unref(G_OBJECT(m_fp));
		m_fp = 0;

		if (!res) {
			// then remove the unwritten file
			(void)UT_go_file_remove (m_szFileName.c_str(), NULL);
		}

		return (res == TRUE);
	}
	return true;
}

void IE_Exp::_abortFile(void)
{
	// abort the write
    UT_DEBUGMSG(("aborting file"));
	if (m_fp)
    {
        _closeFile();

        // then remove the unwanted file
        (void)UT_go_file_remove (m_szFileName.c_str(), NULL);
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

PD_Document * IE_Exp::getDoc () const
{
  return m_pDocument;
}

PD_DocumentRange * IE_Exp::getDocRange() const
{
  return m_pDocRange;
}

UT_Error IE_Exp::writeFile(GsfOutput * fp)
{
	UT_return_val_if_fail(m_pDocument, UT_IE_COULDNOTWRITE);
	UT_return_val_if_fail(fp, UT_IE_COULDNOTWRITE);

	m_fp = fp;

	m_szFileName = gsf_output_name(fp);
	return _writeDocument();
}

UT_Error IE_Exp::writeFile(const char * szFilename)
{
	UT_return_val_if_fail(m_pDocument, UT_IE_COULDNOTWRITE);
	UT_return_val_if_fail(szFilename && *szFilename, UT_IE_COULDNOTWRITE);

	m_bCancelled = false;

	if (!(m_fp = openFile(szFilename)))
		return m_bCancelled ? UT_SAVE_CANCELLED : UT_IE_COULDNOTWRITE;

	m_bOwnsFp = true;

	UT_Error error = _writeDocument();

	if (UT_OK == error)
		error = (_closeFile() ? UT_OK : UT_IE_COULDNOTWRITE);
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return error;
}

UT_Error IE_Exp::copyToBuffer(PD_DocumentRange * pDocRange, UT_ByteBuf * pBuf)
{
	// copy selected range of the document into the provided
	// byte buffer.  (this will be given to the clipboard later)

	UT_return_val_if_fail(m_pDocument == pDocRange->m_pDoc, UT_ERROR);
	
	m_pDocRange = pDocRange;
	m_pByteBuf = pBuf;

	UT_Error err = _writeDocument();

  	// write trailing zero to byte buffer (not required for file)
  	write("",1);

#if (DEBUG && DUMP_COPY_TEXT)
	printf("Text from copy is... \n");
	printf("%s",reinterpret_cast<const char *>(m_pByteBuf->getPointer(0)));
	printf("\n");
#endif
	return err;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Exp::write(const char * sz)
{
	write(sz, strlen(sz));
}

/*! 
 * This method deletes the last char in the output buffer. 
 * The value of the character at the last position is returned.
 */
char IE_Exp::rewindChar(void)
{
	UT_uint32 len = m_pByteBuf->getLength();
	const char * pchr = reinterpret_cast<const char *>(m_pByteBuf->getPointer(len-1));
	char chr = *pchr;
	m_pByteBuf->del(len-1,1);
	return chr;
}

void IE_Exp::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;

	if (!sz || !length)
		return;

	if (m_pByteBuf)
		m_error |= (m_pByteBuf->append(reinterpret_cast<const UT_Byte *>(sz),length) != true);
	else
		m_error |= (_writeBytes(reinterpret_cast<const UT_Byte *>(sz),length) != length);
	
	return;
}

/*****************************************************************/
/*****************************************************************/

/*! 
  Find the filetype for the given mimetype.
 \param szMimetype File mimetype

 Returns IEFT_AbiWord_1 if no exporter knows this mimetype.
 Note that more than one exporter may support a mimetype.
 We return the first one we find.
 This function should closely resemble IE_Exp::fileTypeForMimetype()
*/
IEFileType IE_Exp::fileTypeForMimetype(const char * szMimetype)
{
	if (!szMimetype)
	{
		UT_DEBUGMSG(("fileTypeForMimetype() no mimetype specified, defaulting to suffix .abw\n"));
		return IE_Exp::fileTypeForSuffix(".abw");
	}

	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a mimetype match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = m_sniffers.getNthItem(k);
		UT_return_val_if_fail (s, IEFT_Unknown);
		if (s->supportsMIME(szMimetype) == UT_CONFIDENCE_PERFECT)
		{
			for (UT_uint32 a = 0; a < nrElements; a++)
			{
				if (s->supportsFileType(static_cast<IEFileType>(a+1)))
					return static_cast<IEFileType>(a+1);
			}

			// UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// Hm... an exporter has registered for the given mimetype,
			// but refuses to support any file type we request.
			
			// bug 9548 -- do not return native format
			// return IE_Exp::fileTypeForMimetype(".abw");
			return IEFT_Unknown;
		}
	}

	// bug 9548 -- returning type IEFT_Unknown causes Save to fail and brings up Save As dlg
	// return IE_Exp::fileTypeForMimetype(".abw");
	return IEFT_Unknown;
}

/*! 
  Find the filetype for the given suffix.
 \param szSuffix File suffix

 Returns IEFT_AbiWord_1 if no exporter knows this suffix.
 Note that more than one exporter may support a suffix.
 We return the first one we find.
 This function should closely resemble IE_Exp::fileTypeForSuffix()
*/
IEFileType IE_Exp::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
	{
		UT_DEBUGMSG(("fileTypeForSuffix() no suffix specified, defaulting to suffix .abw\n"));
		return IE_Exp::fileTypeForSuffix(".abw");
	}

	// to alert us to bugs like 9571
	UT_ASSERT_HARMLESS( *szSuffix == '.' );
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = m_sniffers.getNthItem(k);
		UT_return_val_if_fail (s, IEFT_Unknown);
		if (s->recognizeSuffix(szSuffix))
		{
			for (UT_uint32 a = 0; a < nrElements; a++)
			{
				if (s->supportsFileType(static_cast<IEFileType>(a+1)))
					return static_cast<IEFileType>(a+1);
			}

			// UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// Hm... an exporter has registered for the given suffix,
			// but refuses to support any file type we request.
			
			// bug 9548 -- do not return native format
			// return IE_Exp::fileTypeForSuffix(".abw");
			return IEFT_Unknown;
		}
	}

	// bug 9548 -- returning type IEFT_Unknown causes Save to fail and brings up Save As dlg
	// return IE_Exp::fileTypeForSuffix(".abw");
	return IEFT_Unknown;
}

IEFileType IE_Exp::fileTypeForSuffixes(const char * suffixList)
{
	IEFileType ieft = IEFT_Unknown;
	if (!suffixList)
		return ieft;

	UT_String utSuffix (suffixList);
	const size_t len = strlen(suffixList);
	size_t i = 0;

	while (true)
		{
			while (i < len && suffixList[i] != '.')
				i++;

			// will never have all-space extension

			const size_t start = i;
			while (i < len && suffixList[i] != ';')
				i++;

			if (i <= len) {
				UT_String suffix (utSuffix.substr(start, i-start).c_str());
				UT_DEBUGMSG(("DOM: suffix: %s\n", suffix.c_str()));
				
				ieft = fileTypeForSuffix (suffix.c_str());
				if (ieft != IEFT_Unknown || i == len)
					return ieft;

				i++;
			}
		}
	return ieft;
}

/*! 
  Find the filetype for the given filetype description.
 \param szDescription Filetype description

 Returns IEFT_Unknown if no importer has this description.
 This function should closely resemble IE_Exp::fileTypeForDescription()
*/
IEFileType IE_Exp::fileTypeForDescription(const char * szDescription)
{
	IEFileType ieft = IEFT_Unknown;

	if (!szDescription)
		return ieft;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * pSniffer = m_sniffers.getNthItem(k);

		const char * szDummy;
		const char * szDescription2 = 0;

		if (pSniffer->getDlgLabels(&szDescription2,&szDummy,&ieft))
		{
			if (!strcmp(szDescription,szDescription2))
				return ieft;
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}

	return ieft;
}

/*! 
  Find the filetype sniffer for the given filetype.
 \param ieft Filetype

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Exp::snifferForFileType()
*/
IE_ExpSniffer * IE_Exp::snifferForFileType(IEFileType ieft)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getExporterCount();

	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = m_sniffers.getNthItem(k);
		if (s->supportsFileType(ieft))
			return s;
	}

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the suffixes for the given filetype.
 \param szSuffix File suffix

 Returns 0 if no exporter knows this filetype.
 This function should closely resemble IE_Exp::suffixesForFileType()
*/
const char * IE_Exp::suffixesForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szSuffixes = 0;
	IEFileType ieftDummy;

	IE_ExpSniffer * pSniffer = snifferForFileType(ieft);

	UT_return_val_if_fail (pSniffer != NULL, 0);

	if (pSniffer->getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
	{
		return szSuffixes;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// The passed in filetype is invalid.
	return 0;
}

/*! 
  Find the preferred suffix for the given filetype.
 \param szSuffix File suffix

 Returns "" if no exporter knows this filetype.
*/
UT_UTF8String IE_Exp::preferredSuffixForFileType(IEFileType ieft)
{
	IE_ExpSniffer * pSniffer = snifferForFileType(ieft);

	UT_return_val_if_fail (pSniffer != NULL, "");

	return pSniffer->getPreferredSuffix();
}

/*! 
  Find the description for the given filetype.
 \param ieft Numerical "export filetype" ID

 Returns 0 if filetype doesn't exist.
 This function should closely resemble IE_Exp::descriptionForFileType()
*/
const char * IE_Exp::descriptionForFileType(IEFileType ieft)
{
	const char * szDummy;
	const char * szDescription = 0;
	IEFileType ieftDummy;

	IE_ExpSniffer * pSniffer = snifferForFileType(ieft);

	if (pSniffer->getDlgLabels(&szDescription,&szDummy,&ieftDummy))
	{
		return szDescription;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// The passed in filetype is invalid.
	return 0;
}

UT_Error IE_Exp::constructExporter(PD_Document * pDocument,
								   GsfOutput * output,
								   IEFileType ieft,
								   IE_Exp ** ppie,
								   IEFileType * pieft)
{
	UT_return_val_if_fail(output != NULL, UT_ERROR);

	return constructExporter(pDocument, gsf_output_name(output), ieft, ppie, pieft);
}

/*! 
  Construct an exporter of the right type.
 \param pDocument Document
 \param szFilename Name of file - optional
 \param ieft Desired filetype - pass IEFT_Unknown for best guess
 \param ppie Pointer to return importer in
 \param pieft Pointer to fill in actual filetype

 Caller is responsible for deleting the exporter object
 when finished with it.
 This function should closely match IE_Imp::contructImporter()
*/
UT_Error IE_Exp::constructExporter(PD_Document * pDocument,
								   const char * szFilename,
								   IEFileType ieft,
								   IE_Exp ** ppie,
								   IEFileType * pieft)
{
	UT_return_val_if_fail(pDocument, UT_ERROR);
	UT_return_val_if_fail(ieft != IEFT_Unknown || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ieft != IEFT_Bogus || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ppie, UT_ERROR);

	// no filter will support IEFT_Unknown, so we detect from the
	// suffix of the filename, the real exporter to use and assign
	// that back to ieft.
	if ( (ieft == IEFT_Unknown || ieft == IEFT_Bogus) && szFilename && *szFilename)
	{
		ieft = IE_Exp::fileTypeForSuffix(UT_pathSuffix(szFilename).c_str());
	}

	UT_return_val_if_fail (ieft != IEFT_Unknown, UT_ERROR);
	UT_return_val_if_fail (ieft != IEFT_Bogus, UT_ERROR);

   	// let the caller know what kind of exporter they're getting
   	if (pieft != NULL) 
		*pieft = ieft;
   
	// use the exporter for the specified file type
	UT_uint32 nrElements = getExporterCount ();
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_ExpSniffer * s = m_sniffers.getNthItem (k);
		if (s->supportsFileType(ieft))
		{
			return s->constructExporter (pDocument, ppie);
		}
	}

	// if we got here, no registered exporter handles the
	// type of file we're supposed to be writing.
	// assume it is our format and try to write it.
	// if that fails, just give up.
	*ppie = new IE_Exp_AbiWord_1(pDocument);
	if (pieft != NULL) 
		*pieft = IE_Exp::fileTypeForSuffix(".abw");
 	return ((*ppie) ? UT_OK : UT_IE_NOMEMORY);
}

bool IE_Exp::enumerateDlgLabels(UT_uint32 ndx,
								const char ** pszDesc,
								const char ** pszSuffixList,
								IEFileType * ft)
{

	if (ndx < getExporterCount())
	{
		IE_ExpSniffer * s = m_sniffers.getNthItem (ndx);
		UT_return_val_if_fail(s, false);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

UT_uint32 IE_Exp::getExporterCount(void)
{
	return m_sniffers.size();
}


UT_Error IE_Exp::writeBufferToFile(const UT_ByteBuf * pByteBuf,
                                   const std::string & imagedir,
                                   const std::string & filename)
{
    UT_go_directory_create(imagedir.c_str(), NULL);

    std::string path = imagedir + "/" + filename;

    GError * error = NULL;
	GsfOutput * out = UT_go_file_create (path.c_str (), &error);
	if (out)
	{
		gsf_output_write (out, pByteBuf->getLength (), (const guint8*)pByteBuf->getPointer (0));
		gsf_output_close (out);
		g_object_unref (G_OBJECT (out));
	}
    else {
        UT_DEBUGMSG(("Couldn't write file '%s': %s\n", path.c_str(), error->message));
        g_error_free(error);
        return UT_ERROR;
    }
    return UT_OK;
}

std::string IE_Exp::getProperty (const std::string & key) const
{
	std::map<std::string, std::string>::const_iterator iter
		= m_props_map.find(key);
	if (iter == m_props_map.end()) {
		return "";
	}
	return iter->second;
}
