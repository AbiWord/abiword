/* AbiWord
 * Copyright (C) 2006 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2006 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2006 Dominic Lachowicz <domlachowicz@gmail.com>
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

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_wpg_register
#define abi_plugin_unregister abipgn_wpg_unregister
#define abi_plugin_supports_version abipgn_wpg_supports_version
#endif

#include "ie_impGraphic_WPG.h"
#include <gsf/gsf-utils.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile-msole.h>
#include <libwpd-stream/libwpd-stream.h>
#include "xap_Module.h"

using libwpg::WPGraphics;

ABI_PLUGIN_DECLARE("WPG")

class AbiWordPerfectGraphicsInputStream : public WPXInputStream
{
public:
	AbiWordPerfectGraphicsInputStream(GsfInput *input);
	~AbiWordPerfectGraphicsInputStream();

	virtual bool isOLEStream();
	virtual WPXInputStream * getDocumentOLEStream();
	virtual WPXInputStream * getDocumentOLEStream(const char * name);
	virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
	virtual int seek(long offset, WPX_SEEK_TYPE seekType);
	virtual long tell();
	virtual bool atEOS();

private:

	GsfInput *m_input;
	GsfInfile *m_ole;
};

AbiWordPerfectGraphicsInputStream::AbiWordPerfectGraphicsInputStream(GsfInput *input) :
	WPXInputStream(),
	m_input(input),
	m_ole(NULL)
{
	g_object_ref(G_OBJECT(input));
}

AbiWordPerfectGraphicsInputStream::~AbiWordPerfectGraphicsInputStream()
{
	if (m_ole)
		g_object_unref(G_OBJECT(m_ole));

	g_object_unref(G_OBJECT(m_input));
}

const unsigned char * AbiWordPerfectGraphicsInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
	const unsigned char *buf = gsf_input_read(m_input, numBytes, NULL);

	if (buf == NULL)
		numBytesRead = 0;
	else
		numBytesRead = numBytes;

	return buf;
}

int AbiWordPerfectGraphicsInputStream::seek(long offset, WPX_SEEK_TYPE seekType) 
{
	GSeekType gsfSeekType = G_SEEK_SET;
	switch(seekType)
	{
	case WPX_SEEK_CUR:
		gsfSeekType = G_SEEK_CUR;
		break;
	case WPX_SEEK_SET:
		gsfSeekType = G_SEEK_SET;
		break;
	}

	return gsf_input_seek(m_input, offset, gsfSeekType);
}

bool AbiWordPerfectGraphicsInputStream::isOLEStream()
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 

	if (m_ole != NULL)
		return true;

	return false;
}

WPXInputStream * AbiWordPerfectGraphicsInputStream::getDocumentOLEStream()
{
	return getDocumentOLEStream("PerfectOffice_MAIN");
}

WPXInputStream * AbiWordPerfectGraphicsInputStream::getDocumentOLEStream(const char * name)
{
	WPXInputStream *documentStream = NULL;
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_name(m_ole, name);
			if (document) 
				{
					documentStream = new AbiWordPerfectGraphicsInputStream(document);
					g_object_unref(G_OBJECT (document)); // the only reference should be encapsulated within the new stream
				}
		}
	
	return documentStream;
}

long AbiWordPerfectGraphicsInputStream::tell()
{
	return gsf_input_tell(m_input);
}

bool AbiWordPerfectGraphicsInputStream::atEOS()
{
	return gsf_input_eof(m_input);
}

static IE_Imp_WordPerfectGraphics_Sniffer * m_ImpSniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	if (!m_ImpSniffer)
	{
		m_ImpSniffer = new IE_Imp_WordPerfectGraphics_Sniffer ();
	}

	UT_ASSERT (m_ImpSniffer);

	mi->name    = "WordPerfect(tm) Graphics Importer";
	mi->desc    = "Import WordPerfect(tm) Graphics";
	mi->version = ABI_VERSION_STRING;
	mi->author  = "Marc Maurer";
	mi->usage   = "No Usage";

	IE_ImpGraphic::registerImporter (m_ImpSniffer);
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

	UT_ASSERT (m_ImpSniffer);

	IE_ImpGraphic::unregisterImporter (m_ImpSniffer);
	delete m_ImpSniffer;
	m_ImpSniffer = 0;
	
	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}

// supported suffixes 
static IE_SuffixConfidence IE_WordPerfectGraphics_Sniffer__SuffixConfidence[] = {                                                                               
	{ "wpg",        UT_CONFIDENCE_PERFECT   },
	{ "",         UT_CONFIDENCE_ZILCH     }
};

IE_Imp_WordPerfectGraphics_Sniffer::IE_Imp_WordPerfectGraphics_Sniffer()
{
}

IE_Imp_WordPerfectGraphics_Sniffer::~IE_Imp_WordPerfectGraphics_Sniffer()
{
}

const IE_SuffixConfidence * IE_Imp_WordPerfectGraphics_Sniffer::getSuffixConfidence()
{
	return IE_WordPerfectGraphics_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_WordPerfectGraphics_Sniffer::recognizeContents(GsfInput * input)
{
	AbiWordPerfectGraphicsInputStream gsfInput(input);
	if (WPGraphics::isSupported(&gsfInput))
		return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

bool IE_Imp_WordPerfectGraphics_Sniffer::getDlgLabels (const char ** szDesc,
                        const char ** szSuffixList,
                        IEGraphicFileType *ft)
{
	*szDesc = "WordPerfect(tm) Graphics Images (.wpg)";
	*szSuffixList = "*.wpg";
	*ft = getType ();
	return true;
}

UT_Error IE_Imp_WordPerfectGraphics_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_Imp_WordPerfectGraphics();
	if (*ppieg == NULL)                                                                                                                     
		return UT_IE_NOMEMORY;
	return UT_OK;   
}


UT_Error IE_Imp_WordPerfectGraphics::importGraphic(GsfInput *input, FG_Graphic **ppfg)
{
	AbiWordPerfectGraphicsInputStream gsfInput(input);
	WPXString svgOutput;
	if (WPGraphics::generateSVG(&gsfInput, svgOutput))
	{
		GsfInput * svgInput = gsf_input_memory_new((const guint8*)svgOutput.cstr(), svgOutput.len(), false);
		UT_Error result = IE_ImpGraphic::loadGraphic(svgInput, IE_ImpGraphic::fileTypeForSuffix(".svg"), ppfg);
		g_object_unref(svgInput);
		return result;
	}
	return UT_ERROR;
}

