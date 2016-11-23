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
#include <gsf/gsf-infile-zip.h>
#include <librevenge-stream/librevenge-stream.h>
#include "xap_Module.h"

using libwpg::WPGraphics;

ABI_PLUGIN_DECLARE("WPG")

class AbiWordPerfectGraphicsInputStream : public librevenge::RVNGInputStream
{
public:
	AbiWordPerfectGraphicsInputStream(GsfInput *input);
	~AbiWordPerfectGraphicsInputStream();

	virtual bool isStructured();
	virtual unsigned subStreamCount();
	virtual const char* subStreamName(unsigned);
	bool existsSubStream(const char*);
	virtual librevenge::RVNGInputStream* getSubStreamByName(const char*);
	virtual librevenge::RVNGInputStream* getSubStreamById(unsigned);
	virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
	virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType);
	virtual long tell();
	virtual bool isEnd();

private:

	GsfInput *m_input;
	GsfInfile *m_ole;
	std::map<unsigned, std::string> m_substreams;
};

AbiWordPerfectGraphicsInputStream::AbiWordPerfectGraphicsInputStream(GsfInput *input) :
	librevenge::RVNGInputStream(),
	m_input(input),
	m_ole(NULL),
	m_substreams()
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

int AbiWordPerfectGraphicsInputStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) 
{
	GSeekType gsfSeekType = G_SEEK_SET;
	switch(seekType)
	{
	case librevenge::RVNG_SEEK_CUR:
		gsfSeekType = G_SEEK_CUR;
		break;
	case librevenge::RVNG_SEEK_SET:
		gsfSeekType = G_SEEK_SET;
		break;
	case librevenge::RVNG_SEEK_END:
		gsfSeekType = G_SEEK_END;
		break;
	}

	return gsf_input_seek(m_input, offset, gsfSeekType);
}

bool AbiWordPerfectGraphicsInputStream::isStructured()
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 

	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		return true;

	return false;
}

unsigned AbiWordPerfectGraphicsInputStream::subStreamCount()
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			int numChildren = gsf_infile_num_children(m_ole);
			if (numChildren > 0)
				return numChildren;
			return 0;
		}
	
	return 0;
}

const char * AbiWordPerfectGraphicsInputStream::subStreamName(unsigned id)
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			if ((int)id >= gsf_infile_num_children(m_ole))
			{
				return 0;
			}
			std::map<unsigned, std::string>::iterator i = m_substreams.lower_bound(id);
			if (i == m_substreams.end() || m_substreams.key_comp()(id, i->first))
				{
					std::string name = gsf_infile_name_by_index(m_ole, (int)id);
					i = m_substreams.insert(i, std::map<unsigned, std::string>::value_type(id, name));
				}
			return i->second.c_str();
		}
	
	return 0;
}

bool AbiWordPerfectGraphicsInputStream::existsSubStream(const char * name)
{
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_name(m_ole, name);
			if (document) 
				{
					g_object_unref(G_OBJECT (document));
					return true;
				}
		}
	
	return false;
}

librevenge::RVNGInputStream * AbiWordPerfectGraphicsInputStream::getSubStreamByName(const char * name)
{
	librevenge::RVNGInputStream *documentStream = NULL;
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
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

librevenge::RVNGInputStream * AbiWordPerfectGraphicsInputStream::getSubStreamById(unsigned id)
{
	librevenge::RVNGInputStream *documentStream = NULL;
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_msole_new (m_input, NULL)); 
	
	if (!m_ole)
		m_ole = GSF_INFILE(gsf_infile_zip_new (m_input, NULL)); 
	
	if (m_ole)
		{
			GsfInput *document = gsf_infile_child_by_index(m_ole, (int)id);
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

bool AbiWordPerfectGraphicsInputStream::isEnd()
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


UT_Error IE_Imp_WordPerfectGraphics::importGraphic(GsfInput *input, FG_ConstGraphicPtr& pfg)
{
	AbiWordPerfectGraphicsInputStream gsfInput(input);
	librevenge::RVNGString svgOutput;
	librevenge::RVNGStringVector vec;
	librevenge::RVNGSVGDrawingGenerator generator(vec, "");

	if (!libwpg::WPGraphics::parse(&gsfInput, &generator) || vec.empty() || vec[0].empty())
	{
		return UT_ERROR;
	}

	svgOutput.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	svgOutput.append("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"");
	svgOutput.append(" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	svgOutput.append(vec[0]);
	svgOutput.append("\n");

	GsfInput * svgInput = gsf_input_memory_new((const guint8*)svgOutput.cstr(), svgOutput.len(), false);
	UT_Error result = IE_ImpGraphic::loadGraphic(svgInput, IE_ImpGraphic::fileTypeForSuffix(".svg"), pfg);
	g_object_unref(svgInput);
	return result;
}

