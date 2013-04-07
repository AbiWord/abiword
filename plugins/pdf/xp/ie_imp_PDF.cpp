/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2005 Dom Lachowicz <cinamod@hotmail.com>
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
#include <unistd.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ie_imp_Text.h"

#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-output-stdio.h>

#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_pdf_register
#define abi_plugin_unregister abipgn_pdf_unregister
#define abi_plugin_supports_version abipgn_pdf_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("PDF")
#endif

static const struct
{
	const char *conversion_program;
	const char *extension;
} pdf_conversion_programs[] = {
	{ "pdftoabw", ".abw" },
	{ "pdftotext", ".txt" }
};

static UT_Error temp_name (UT_String& out_filename)
{
	char *temporary_file = NULL;
	GError *err = NULL;
	gint tmp_fp = g_file_open_tmp ("XXXXXX", &temporary_file, &err);
	
	if (err) 
		{
			g_warning ("%s", err->message);
			g_error_free (err); err = NULL;
			return UT_ERROR;
		}

	out_filename = temporary_file;
	g_free (temporary_file);
	close(tmp_fp);
	return UT_OK;
}

class IE_Imp_PDF : public IE_Imp
{
public:

  IE_Imp_PDF (PD_Document * pDocument)
    : IE_Imp(pDocument)
  {
  }

  virtual ~IE_Imp_PDF ()
  {
  }
  
	UT_Error _runConversion(const UT_String& pdf_on_disk, const UT_String& output_on_disk, size_t which)
	{
		UT_Error rval = UT_ERROR;

		const char * pdftoabw_argv[4];
		
		int argc = 0;
		pdftoabw_argv[argc++] = pdf_conversion_programs[which].conversion_program;
		pdftoabw_argv[argc++] = pdf_on_disk.c_str ();
		pdftoabw_argv[argc++] = output_on_disk.c_str ();
		pdftoabw_argv[argc++] = NULL;
		
		// run conversion
		if (g_spawn_sync (NULL,
						  (gchar **)pdftoabw_argv,
						  NULL,
						  (GSpawnFlags)(G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
						  NULL,
						  NULL,
						  NULL,
						  NULL,
						  NULL,
						  NULL))
			{
				char * uri = UT_go_filename_to_uri (output_on_disk.c_str ());
				if (uri)
					{
						// import the document
						rval = IE_Imp::loadFile (getDoc (), uri, IE_Imp::fileTypeForSuffix (pdf_conversion_programs[which].extension));
						g_free (uri);
					}
			}

		return rval;
	}

  virtual UT_Error _loadFile(GsfInput * input)
  {
    UT_Error rval = UT_ERROR;

	UT_String pdf_on_disk, abw_on_disk;

	// create temporary file names
	rval = temp_name (pdf_on_disk);
	if (rval != UT_OK) return rval;

	rval = temp_name (abw_on_disk);
	if (rval != UT_OK) return rval;

	GsfOutput * output = gsf_output_stdio_new (pdf_on_disk.c_str (), NULL);
	if (output)
		{
			// copy input to disk
			gboolean copy_res = gsf_input_copy (input, output);

			gsf_output_close (output);
			g_object_unref (G_OBJECT (output));

			if (copy_res)
				{
					for (size_t i = 0; i < G_N_ELEMENTS(pdf_conversion_programs); i++)
						{
							if ((rval = _runConversion(pdf_on_disk, abw_on_disk, i)) == UT_OK)
								break;
						}
				}			
		}

	// remove temporary files
	remove(pdf_on_disk.c_str ());
	remove(abw_on_disk.c_str ());

    return rval;
  }

};

/*****************************************************************/
/*****************************************************************/

// supported suffixes
static IE_SuffixConfidence IE_Imp_PDF_Sniffer__SuffixConfidence[] = {
	{ "pdf", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

// supported mimetypes
static IE_MimeConfidence IE_Imp_PDF_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"application/pdf", 	UT_CONFIDENCE_PERFECT 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 				UT_CONFIDENCE_ZILCH 	}
};

class IE_Imp_PDF_Sniffer : public IE_ImpSniffer
{
public:

  IE_Imp_PDF_Sniffer()
    : IE_ImpSniffer("application/pdf", false)
  {
  }
	
  virtual ~IE_Imp_PDF_Sniffer()
  {
  }
  
  const IE_SuffixConfidence * getSuffixConfidence ()
  {
	return IE_Imp_PDF_Sniffer__SuffixConfidence;
  }

  const IE_MimeConfidence * getMimeConfidence ()
  {
	return IE_Imp_PDF_Sniffer__MimeConfidence;
  }

  virtual UT_Confidence_t recognizeContents (const char * szBuf,
					     UT_uint32 /*iNumbytes*/)
  {
    if (!strncmp (szBuf, "%PDF-", 5))
      return UT_CONFIDENCE_PERFECT;
    return UT_CONFIDENCE_ZILCH;
  }

  virtual bool getDlgLabels (const char ** pszDesc,
							 const char ** pszSuffixList,
							 IEFileType * ft)
  {
    *pszDesc = "PDF (.pdf)";
    *pszSuffixList = "*.pdf";
    *ft = getFileType();
    return true;
  }

  virtual UT_Error constructImporter (PD_Document * pDocument,
				      IE_Imp ** ppie)
  {
    *ppie = new IE_Imp_PDF(pDocument);
    return UT_OK;
  }
};

/*****************************************************************/
/* General plugin stuff                                          */
/*****************************************************************/

// we use a reference-counted sniffer
static IE_Imp_PDF_Sniffer * m_impSniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	for (size_t i = 0; i < G_N_ELEMENTS(pdf_conversion_programs); i++)
		{
			gchar * prog_path;

			prog_path = g_find_program_in_path (pdf_conversion_programs[i].conversion_program);
			if (prog_path)
				{
					// don't register the plugin if it can't find pdftoabw
					g_free (prog_path);
					
					if (!m_impSniffer)
						{
							m_impSniffer = new IE_Imp_PDF_Sniffer ();
						}
					
					mi->name    = "PDF Import Filter";
					mi->desc    = "Import Adobe PDF Documents";
					mi->version = ABI_VERSION_STRING;
					mi->author  = "Dom Lachowicz <cinamod@hotmail.com>";
					mi->usage   = "No Usage";
					
					IE_Imp::registerImporter (m_impSniffer);
					return 1;
				}
		}

	return 0;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
  mi->name = 0;
  mi->desc = 0;
  mi->version = 0;
  mi->author = 0;
  mi->usage = 0;
  
  if (m_impSniffer)
	  {
		  IE_Imp::unregisterImporter (m_impSniffer);
		  delete m_impSniffer;
		  m_impSniffer = 0;
	  }

  return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}
