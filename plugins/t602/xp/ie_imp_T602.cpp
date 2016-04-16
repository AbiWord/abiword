/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Petr Tomasek <tomasek@etf.cuni.cz>
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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
#include "pd_Document.h"
#include "ut_string_class.h"
#include "ut_std_string.h"
#include "xap_Prefs.h"
#include "ie_imp_T602.h"
#include "xap_App.h"
#include "xap_Module.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_t602_register
#define abi_plugin_unregister abipgn_t602_unregister
#define abi_plugin_supports_version abipgn_t602_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("T602")
#endif

#define X_CheckDocError(v) if ((!v)) { UT_DEBUGMSG(("X_CheckDocError: ie_imp_T602.cpp:%d\n", __LINE__)); return UT_IE_IMPORTERROR; }
#define X_CheckT602Error(v) if ((v != UT_OK)) { UT_DEBUGMSG(("X_CheckT602Error: ie_imp_T602.cpp:%d\n", __LINE__)); return UT_IE_IMPORTERROR; }

/****************************************************************************/

// completely generic code to allow this to be a plugin

static IE_Imp_T602_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_T602_Sniffer ();
	}

	UT_ASSERT (m_sniffer);

	mi->name    = "T602 Importer";
	mi->desc   = "Imports T602 documents into abiword. T602 was "
		"popular czech and slovak text editor in early nineties "
		"produced by Software602 (http://www.software602.cz/).";
	mi->version = ABI_VERSION_STRING;
	mi->author  = "Petr Tomasek <tomasek@etf.cuni.cz>";
	mi->usage   = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

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

/****************************************************************************/

IE_Imp_T602_Sniffer::IE_Imp_T602_Sniffer () :
  IE_ImpSniffer("AbiT602::T602")
{
  // 
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_T602_Sniffer__SuffixConfidence[] = {
	{ "602", 	UT_CONFIDENCE_PERFECT 	},
	{ "t602", 	UT_CONFIDENCE_PERFECT 	},
	{ "txt", 	UT_CONFIDENCE_POOR 		},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_T602_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_T602_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_T602_Sniffer::recognizeContents (const char * szBuf, 
						     UT_uint32 iNumbytes)
{
  if ((iNumbytes>3) && (!strncmp (szBuf,"@CT ",4)))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;   
}

UT_Error IE_Imp_T602_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_T602 * p = new IE_Imp_T602(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_T602_Sniffer::getDlgLabels (const char ** pszDesc,
											const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "T602 (.602,.txt)";
	*pszSuffixList = "*.602; *.txt";
	*ft = getFileType();
	return true;
}

/****************************************************************************/

IE_Imp_T602::IE_Imp_T602(PD_Document * pDocument)
  : IE_Imp (pDocument), m_importFile(NULL), m_charset(1), m_family("Courier"),
    m_basefamily("Courier"), m_softcr(1), m_basesize( 10 ), 
    m_size(10), m_lmargin ( "1.0000in" ), m_rmargin ( "1.0000in" ), 
    m_bold(0), m_italic(0), m_underline(0),
    m_tpos(0), m_big(0), m_color("000000"), m_sfont(0), m_eol(true),
    m_lheight(1), m_footer(0), m_header(0), m_fhc(1), m_writeheader(true)
{
}

IE_Imp_T602::~IE_Imp_T602() 
{
}

/****************************************************************************/

/* This is ugly hack. Since iconv doesn't know the
 * keybcs2 and koi8cs encodings, we have to provide them ourselves :-(
 * (The tables were done using 'recode' and some scripting)
 */ 

// charset table keybcs2 -> ucs2
static const UT_uint16 keybcs22ucs[]= {
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
0x010c, 0x00fc, 0x00e9, 0x010f, 0x00e4, 0x010e, 0x0164, 0x010d,
0x011b, 0x011a, 0x0139, 0x00cd, 0x013e, 0x013a, 0x00c4, 0x00c1,
0x00c9, 0x017e, 0x017d, 0x00f4, 0x00f6, 0x00d3, 0x016f, 0x00da,
0x00fd, 0x00d6, 0x00dc, 0x0160, 0x013d, 0x00dd, 0x0158, 0x0165,
0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x0148, 0x0147, 0x016e, 0x00d4,
0x0161, 0x0159, 0x0155, 0x0154, 0x00bc, 0x00a7, 0x00ab, 0x00bb,
0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
0x03b1, 0x03b2, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x03bc, 0x03c4,
0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x2205, 0x03b5, 0x2229,
0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
0x2218, 0x00b7, 0x2219, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0
};

// charset table cp852 -> ucs2
static const UT_uint16 cp8522ucs[]= {
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x016f, 0x0107, 0x00e7,
0x0142, 0x00eb, 0x0150, 0x0151, 0x00ee, 0x0179, 0x00c4, 0x0106,
0x00c9, 0x0139, 0x013a, 0x00f4, 0x00f6, 0x013d, 0x013e, 0x015a,
0x015b, 0x00d6, 0x00dc, 0x0164, 0x0165, 0x0141, 0x00d7, 0x010d,
0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x0104, 0x0105, 0x017d, 0x017e,
0x0118, 0x0119, 0x00ac, 0x017a, 0x010c, 0x015f, 0x00ab, 0x00bb,
0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x011a,
0x015e, 0x2563, 0x2551, 0x2557, 0x255d, 0x017b, 0x017c, 0x2510,
0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x0102, 0x0103,
0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4,
0x0111, 0x0110, 0x010e, 0x00cb, 0x010f, 0x0147, 0x00cd, 0x00ce,
0x011b, 0x2518, 0x250c, 0x2588, 0x2584, 0x0162, 0x016e, 0x2580,
0x00d3, 0x00df, 0x00d4, 0x0143, 0x0144, 0x0148, 0x0160, 0x0161,
0x0154, 0x00da, 0x0155, 0x0170, 0x00fd, 0x00dd, 0x0163, 0x00b4,
0x00ad, 0x02dd, 0x02db, 0x02c7, 0x02d8, 0x00a7, 0x00f7, 0x00b8,
0x00b0, 0x00a8, 0x02d9, 0x0171, 0x0158, 0x0159, 0x25a0, 0x00a0
};

// charset table koi8cs -> ucs2
static const UT_uint16 koi8cs2ucs[]= {
0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
0x0020, 0x0021, 0x0022, 0x0023, 0x00a4, 0x0025, 0x0026, 0x0027,
0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
0x00a0, 0x0104, 0x02d8, 0x0141, 0x0024, 0x013d, 0x015a, 0x00a7,
0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7,
0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9
};

UT_uint16 IE_Imp_T602::_conv(unsigned char c)
{

  switch (m_charset)
    {
    case 0:
      return keybcs22ucs[c];
      break;
    case 1:
      return cp8522ucs[c];
      break;
    case 2:
      return koi8cs2ucs[c];
      break;
    }
  return 0;
}

bool IE_Imp_T602::_getbyte(unsigned char &c)
{
	UT_ASSERT(m_importFile);
	return (gsf_input_read (m_importFile, 1, &c) != NULL);
}

UT_Error IE_Imp_T602::_writeheader()
{
	X_CheckT602Error(_writeSP())
	X_CheckT602Error(_writePP())
	X_CheckT602Error(_writeTP())
	m_writeheader=false;
	return UT_OK;
}

UT_Error IE_Imp_T602::_writeTP()
{
  UT_DEBUGMSG(("T602: Append text properties\n"));
  UT_String buff;
  const gchar* pps[3];
  UT_String_sprintf(buff,"font-family: %s; font-size: %dpt; color:%s; font-weight: %s; "
		    "font-style: %s; text-decoration: %s; text-position: %s",
		    m_family.c_str(), m_size, m_color.c_str(),
		    m_bold ? "bold" : "normal",
		    m_italic ? "italic" : "normal",
		    m_underline ? "underline" : "none",
		    (m_tpos==1) ? "subscript": 
		    (m_tpos==2 ? "superscript" : "none"));
  
  UT_DEBUGMSG(("T602: text-prop:\"%s\"]\n",buff.c_str()));
  pps[0]="props";
  pps[1]=buff.c_str();
  pps[2]=NULL;
  X_CheckDocError(appendFmt(pps))
  return UT_OK;
}

UT_Error IE_Imp_T602::_writePP()
{
  UT_DEBUGMSG(("T602: Append paragraph properties\n"));
  std::string buff;

  // Don't put %1.1f here!! Locales could fuck it...
  buff = UT_std_string_sprintf("line-height: %d.%d",
                               (m_lheight+1)/2,((m_lheight+1)%2)*5);

  UT_DEBUGMSG(("T602: par-prop:\"%s\"]\n",buff.c_str()));

  const PP_PropertyVector pps = {
      "props", buff
  };
  X_CheckDocError(appendStrux(PTX_Block, pps))
  return UT_OK;
}

UT_Error IE_Imp_T602::_writeSP()
{
  UT_DEBUGMSG(("T602: Append section\n"));
  std::string bf1, bf2;

  std::string buff = UT_std_string_sprintf("page-margin-left: %s; page-margin-right: %s",
		  m_lmargin.c_str(),
		  m_rmargin.c_str());
  PP_PropertyVector sps = {
      "props", buff
  };

  if (!m_footer && !m_header)
    {
      X_CheckDocError(appendStrux(PTX_Section,sps))
    }
  else
    {
  if (m_header)
  {
      sps.push_back("header");
      bf1 = UT_std_string_sprintf("%d", m_header);
      sps.push_back(bf1);
  }
  if (m_footer)
  {
      sps.push_back("footer");
      bf2 = UT_std_string_sprintf("%d", m_footer);
      sps.push_back(bf2);
  }
  X_CheckDocError(appendStrux(PTX_Section, sps))
    }
  return UT_OK;
}

UT_Error IE_Imp_T602::_write_fh(UT_String & fh, UT_uint32 id, bool hea)
{
  UT_DEBUGMSG(("T602: Append footer/header section\n"));
  std::string bf1;
  int i = 0;
  bool slash = false;

  bf1 = UT_std_string_sprintf("%d", id);
  const PP_PropertyVector fhps = {
      "id", bf1,
      "type", (hea ? "header" : "footer")
  };
  X_CheckDocError(appendStrux(PTX_Section,fhps))
  X_CheckT602Error(_writePP())
  X_CheckT602Error(_writeTP())

  // Page-numbers: prepare text properties...
  std::string buff = UT_std_string_sprintf("font-family: %s; font-size: %dpt; color:%s; font-weight: %s; "
		    "font-style: %s; text-decoration: %s; text-position: %s",
		    m_family.c_str(), m_size, m_color.c_str(),
		    m_bold ? "bold" : "normal",
		    m_italic ? "italic" : "normal",
		    m_underline ? "underline" : "none",
		    (m_tpos==1) ? "subscript": 
		    (m_tpos==2 ? "superscript" : "none"));
  
  UT_DEBUGMSG(("T602: page-numbers: text-prop:\"%s\"\n",buff.c_str()));

  const PP_PropertyVector fps = {
      "type", "page_number",
      "props", buff
  };

  for (i=0; fh[i]!='\0'; i++)
  {
     if ((fh[i]=='\\') && (!slash)) 
       {
	 slash=true;
       }
     else if ((fh[i]=='#') && (!slash))
        {
	  X_CheckDocError(appendObject(PTO_Field,fps))
          slash=false;
	}
     else
	{
        X_CheckT602Error(_inschar(fh[i], false))
	slash=false;
	}
  }
  return UT_OK;
}

UT_Error IE_Imp_T602::_ins(UT_uint16 c)
{
  UT_UCSChar ch = static_cast<UT_UCSChar >(c);
  X_CheckDocError(appendSpan(&ch,1))
  return UT_OK;
}


UT_Error IE_Imp_T602::_dotcom(unsigned char ch)
{
  unsigned char c;
  char buff[1024];
  int i;
  int x;

  i=0;
  while (_getbyte(c) && (c!=0x0d) && (c!=0x8d) && (i<1023))
  {
	 if ((c!=0x0a) && (c!=0x1a)) buff[i]=c;
	 i++;
  }

// buffer too small, throw it out !
  if ((c!=0x0d) && (c!=0x8d) && (c!=0x1a))
  {
          if (m_writeheader) X_CheckT602Error(_writeheader())
	  if (ch=='.') X_CheckT602Error(_inschar('.', false))
	  while (_getbyte(c) && (c!=0x0d) && (c!=0x8d))
		  if ((ch=='.') && (c!=0x0a) && (c!=0x1a))
			  X_CheckT602Error(_inschar(c, false))
  }
  else
  { // O.K. let's parse the .dot command...
    buff[i]='\0';
    UT_DEBUGMSG(("Dot command: \"%s\"\n",static_cast<char *>(buff)));
    if (!strncmp(buff,"CT ",3))
    {
      m_charset=atoi(buff+3);
      UT_DEBUGMSG(("-->Charset %d\n",m_charset));
    }
    else if (!strncmp(buff,"PA",2))
    {
      UT_DEBUGMSG(("-->Page break\n"));
      if (m_writeheader) X_CheckT602Error(_writeheader())
      X_CheckT602Error(_ins(UCS_FF))
    }
    else if (!strncmp(buff,"LH ",3))
    {
      x=atoi(buff+3);
      if (x==6) m_lheight=1;
      else m_lheight=6-x;
      UT_DEBUGMSG(("-->Line height %1.1f\n",static_cast<float>(m_lheight+1)/2));
      m_writeheader=true; // FIXME m_writePP ???? 
    }
    else if (!strncmp(buff,"PI ",3))
    {
      UT_DEBUGMSG(("T602: Images not supported!! [%s]\n",buff+3));
    }
    else if (!strncmp(buff,"IX ",3))
    {
      UT_DEBUGMSG(("T602: Indexes not supported!! [%s]\n",buff+3));
    }
    else if (!strncmp(buff,"KA ",3))
    {
      UT_DEBUGMSG(("T602: Chapters not supported!!\n"));
    }
    else if (!strncmp(buff,"HE ",3))
    {
      UT_DEBUGMSG(("-->Header: \"%s\"\n",buff+3));
      if (buff[3]=='0')
       {
        m_header=0;
	m_writeheader= true;
       }
      else
       {
        m_fhc++;
	m_header=m_fhc;
	m_hbuff = buff+3;
	m_writeheader= true;
       }
    }
     else if (!strncmp(buff,"FO ",3))
    {
      UT_DEBUGMSG(("-->Footer: \"%s\"\n",buff+3));
      if (buff[3]=='0')
       {
        m_footer=0;
        m_writeheader= true;
       }
      else
       {
        m_fhc++;
	m_footer=m_fhc;
	m_fbuff = buff+3;
	m_writeheader= true;
       }
    }

  }
  m_eol=true;
  return UT_OK;
}


UT_Error IE_Imp_T602::_inschar(unsigned char c, bool eol)
{
switch (c)
    {
//- Text Properties:
//Bold
     case 0x02:
	m_bold ^=1;
	X_CheckT602Error(_writeTP())
	break;
//Italic
     case 0x04:
	m_italic ^=1;
	X_CheckT602Error(_writeTP())
	break;
//Underline
     case 0x13:
	m_underline ^=1;
	X_CheckT602Error(_writeTP())
	break;
//Subscript
     case 0x16:
	m_tpos ^=1;
	X_CheckT602Error(_writeTP())
	break;
//Superscript
     case 0x14:
	m_tpos ^=2;
	X_CheckT602Error(_writeTP())
	break;
//Wide
     case 0x0f:
	m_big ^=1;
	if (m_big & 1)
	  m_size=static_cast<int>(1.5*m_basesize);
	else
	  m_size=m_basesize;
	X_CheckT602Error(_writeTP())
	break;
//Tall
     case 0x10:
	m_big ^=2;
	if (m_big & 2)
	  m_size=static_cast<int>(1.2*m_basesize);
	else
	  m_size=m_basesize;
	X_CheckT602Error(_writeTP())
	break;
//Big
     case 0x1d:
	m_big ^=4;
	if (m_big & 4)
	  { m_size=2*m_basesize; m_bold=1; }
	else
	  { m_size=m_basesize; m_bold=0; }
	X_CheckT602Error(_writeTP())
	break;

//- Extra text Properties:
//Elite
     case 0x01:
	m_sfont ^=1;
	if (m_sfont & 1)
	  { m_size=static_cast<int>(0.8*m_basesize); m_family="Arial"; 
		  /* FIXME? -> .profile?*/ }
	else
	  { m_size=m_basesize; m_family=m_basefamily; }
	X_CheckT602Error(_writeTP())
	break;
//Condens
     case 0x03:
	m_sfont ^=2;
	if (m_sfont & 2)
	  m_size=static_cast<int>(.7*m_basesize);
	else
	  m_size=m_basesize;
	X_CheckT602Error(_writeTP())
	break;
//User 1
     case 0x11:
	m_sfont ^=4;
	if (m_sfont & 4)
	  m_color="ff0000";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;
//User 2
     case 0x12:
	m_sfont ^=8;
	if (m_sfont & 8)
	  m_color="ee00ff";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;
//User 3
     case 0x15:
	m_sfont ^=16;
	if (m_sfont & 16)
	  m_color="ffaa00";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;
//User 4
     case 0x17:
	m_sfont ^=32;
	if (m_sfont & 32)
	  m_color="1100ff";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;
//User 5
     case 0x18:
	m_sfont ^=64;
	if (m_sfont & 64)
	  m_color="bbbb00";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;
//User 6
     case 0x19:
	m_sfont ^=128;
	if (m_sfont & 128)
	  m_color="00bb00";
	else
	  m_color="000000";
	X_CheckT602Error(_writeTP())
	break;

//-- Hacks
//Soft hyphen. FIXME: We're putting there simple '-'...
     case 0xad:
	X_CheckT602Error(_ins(_conv('-')))
    	m_eol=false;
	break;
//Non breaking space. FIXME: how to do this in abiword?
     case 0xfe:
	X_CheckT602Error(_ins(_conv(' ')))
    	m_eol=false;
	break;

//-- Ignore
     case 0x0a:
//	m_eol=true;
     case 0x1a:
	break;

//Line breaks (cr/soft-cr)
     case 0x0d:
	m_eol=true;
	X_CheckDocError(appendStrux(PTX_Block, PP_NOPROPS))
	break;
    case 0x8d:  // FIXME dat moznost volby ?
      if (m_softcr)
	{
	  m_eol=true;
	  X_CheckDocError(appendStrux(PTX_Block, PP_NOPROPS))
	    }
      else
	X_CheckT602Error(_ins(_conv(' ')))
      break;
//Line commands
     case '.':
     case '@':
	if (!eol)
	  {
	  X_CheckT602Error(_ins(_conv(c)))
	  }
	else
	  {	
	  X_CheckT602Error(_dotcom(c))
	  }
	break;
//Normal character
     default:
    	m_eol=false;
	X_CheckT602Error(_ins(_conv(c)))
	break;
    }
return UT_OK;
}


UT_Error IE_Imp_T602::_loadFile(GsfInput * input)
{
   UT_Error error;  
   unsigned char c;
   
   m_importFile = (GsfInput *)g_object_ref (G_OBJECT (input));
   
   error=UT_OK;
	   
   while (_getbyte(c))
    {
    if (m_eol && m_writeheader && (c!='.') && (c!='@') && 
	(c!=0x0a) && (c!=0x1a))
   		X_CheckT602Error(_writeheader())
    X_CheckT602Error(_inschar(c, m_eol))
    }
   
// Abiword handles footer/header only once for the whole file(?) :-(
// FIXME: can we do something with it?

   if (m_footer)
     {
       X_CheckT602Error(_write_fh(m_fbuff, m_footer, false));
     }
   if (m_header)
     {
       X_CheckT602Error(_write_fh(m_hbuff, m_header, true));
     }

   g_object_unref(G_OBJECT(m_importFile));
   return error;
}
