/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 Sean Young <sean@mess.org>
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2001 Dom Lachowicz 
 *               2010 Ingo Brueckl
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
 *
 * Documentation at http://www.msxnet.org/word2rtf/formats/write
 *
 * TODO: 
 *  - pictures: clean up / polish off code, add wmf
 *  - OLE: no support yet
 *  - headers/footers need proper positioning implemented
 *  - speed it up!
 */

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <set>
#include "ut_locale.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_MSWrite.h"
#include "ie_impexp_MSWrite.h"
#include "pd_Document.h"
#include "ut_growbuf.h"
#include "ut_string_class.h"
#include "xap_Module.h"
#include "fg_Graphic.h"
#include "ie_impGraphic.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_mswrite_register
#define abi_plugin_unregister abipgn_mswrite_unregister
#define abi_plugin_supports_version abipgn_mswrite_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("MsWrite")
#endif

/*****************************************************************/
/*****************************************************************/

// completely generic code to allow this to be a plugin

// we use a reference-counted sniffer
static IE_Imp_MSWrite_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_MSWrite_Sniffer ();
	}

	mi->name = "MSWrite Importer";
	mi->desc = "Import MSWrite Documents";
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
/*****************************************************************/

IE_Imp_MSWrite_Sniffer::IE_Imp_MSWrite_Sniffer () :
  IE_ImpSniffer("AbiMSWrite::MSWrite")
{
  // 
}

// supported suffixes
static IE_SuffixConfidence const IE_Imp_MSWrite_Sniffer__SuffixConfidence[] = {
	{ "wri", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_MSWrite_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_MSWrite_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_Imp_MSWrite_Sniffer::recognizeContents(const char * szBuf, 
											   UT_uint32 iNumbytes)
{
    if ( iNumbytes > 8 )
    {
        if ( (szBuf[0] == static_cast<char>(0x31) || szBuf[0] == static_cast<char>(0x32)) && 
             szBuf[1] == static_cast<char>(0xbe) &&
             szBuf[2] == static_cast<char>(0) && szBuf[3] == static_cast<char>(0) &&
             szBuf[4] == static_cast<char>(0) && szBuf[5] == static_cast<char>(0xab) )
        {
            return(UT_CONFIDENCE_PERFECT);
        }
    }
    return(UT_CONFIDENCE_ZILCH);
}

UT_Error IE_Imp_MSWrite_Sniffer::constructImporter(PD_Document * pDocument,
												   IE_Imp ** ppie)
{
    IE_Imp_MSWrite * p = new IE_Imp_MSWrite(pDocument);
    *ppie = p;
    return UT_OK;
}

bool	IE_Imp_MSWrite_Sniffer::getDlgLabels(const char ** pszDesc,
											 const char ** pszSuffixList,
											 IEFileType * ft)
{
    *pszDesc = "Microsoft Write (.wri)";
    *pszSuffixList = "*.wri";
    *ft = getFileType();
    return true;
}

/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(ies,exp)	do { if (((ies)=(exp)) != UT_OK) goto Cleanup; } while (0)
#define X_ReturnIfFail(exp,ies)		do { bool b = (exp); if (!b) return (ies); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

/*****************************************************************/
/*****************************************************************/

/*
 * all the font stuff
 */

void IE_Imp_MSWrite::free_ffntb () 
{
    for (UT_uint32 i=0; i < wri_fonts_count; i++) {
      wri_fonts[i].name.~basic_string();
    }
    free(wri_fonts);
    wri_fonts = NULL;
}

int IE_Imp_MSWrite::read_ffntb () 
{
    int	page, pnMac, font_count, cbFfn;
    unsigned char byt[2], ffid;
    char *ffn;
    struct wri_font *wri_fonts_tmp;
	
    /* if the page is the same as pnMac, there are no fonts */
    page = wri_struct_value (write_file_header, "pnFfntb");
    pnMac = wri_struct_value (write_file_header, "pnMac");
    if (page == pnMac) {
		return 0;
    }
	
    if (gsf_input_seek (mFile, page++ * 0x80, G_SEEK_SET) ) {
		perror ("wri_file");
		return 1;
    }
	
    /* the first two bytes are the number of fonts */
    if (!gsf_input_read (mFile, 2, byt)) {
		perror ("wri_file");
		return 1;
    }
    wri_fonts_count = byt[0] + 256 * byt[1];
	
    font_count = 0;
    wri_fonts = NULL;

	std::set<std::string> raw_fonts;
	
    while (true) {
		if (!gsf_input_read (mFile, 2, byt)) {
			perror ("wri_file");
			return 1;
		}
		cbFfn = byt[0] + 256 * byt[1];
		if (cbFfn == 0) {
			break;
		}
		if (cbFfn == 0xffff) {
    	    if (gsf_input_seek (mFile, page++ * 0x80, G_SEEK_SET)) {
				perror ("wri_file");
				return 1;
			}
			continue;
		}
		wri_fonts_tmp  = (struct wri_font*) 
			realloc (static_cast<void*>(wri_fonts), (font_count + 1) * sizeof (wri_font));
		if (!wri_fonts_tmp) {
			UT_DEBUGMSG(("Out of memory!\n"));
			free_ffntb ();
		}
		wri_fonts = static_cast<struct wri_font*>(wri_fonts_tmp);
		
		/* this is the font family identifier; this can either be
		   FF_DONTCARE, FF_ROMAN, FF_SWISS, FF_MODERN, FF_SCRIPT,
		   FF_DECORATIVE; these are defined in <windows.h>, but 
		   I don't know what to do with them */
		if (!gsf_input_read (mFile, 1, &ffid)) {
			perror ("wri_file");
			return 1;
		}
		wri_fonts[font_count].ffid = ffid;
		cbFfn--; 
		ffn = static_cast<char*>(malloc (cbFfn));
		/* we've read the first byte, so we take one of cbFfn */
		if (!gsf_input_read (mFile, cbFfn, (guint8*)ffn)) {
			perror ("wri_file");
			return 1;
		}
		char *trname;
		wri_fonts[font_count].codepage=get_codepage(ffn,&trname);
		if (wri_fonts[font_count].codepage) {
			strcpy(ffn,trname);
			raw_fonts.insert(trname);
		}
		memset(&(wri_fonts[font_count].name),0,sizeof(std::string));
		wri_fonts[font_count].name = ffn;
		font_count++;

		free(ffn);
    }
	for (int i=0; i<font_count; i++) {
		std::set<std::string>::iterator pos;
		if (!wri_fonts[i].codepage) {
			pos = raw_fonts.find(wri_fonts[i].name);
			if (pos != raw_fonts.end())
				wri_fonts[i].codepage="CP1252";
		}
	}
    if (static_cast<unsigned>(font_count) != wri_fonts_count) {
		wri_fonts_count = font_count;
		UT_DEBUGMSG(("write file lied about number of fonts\n"));
    }
    return 0;
}

static struct cst_data {
	const char *suffix;
	const char *cpid;
} const cp_suf_tbl[]={
	{"\x04 cyr","CP1251"},
	{"\x03 ce","CP1250"},
	{NULL,NULL}
};

void inline IE_Imp_MSWrite::translate_char (char ch, UT_UCS4String &buf)
{
	UT_UCS4Char uch=ch;

	lf = false;

	switch (ch)
	{
	case 9:
		buf += UCS_TAB;
		break;

	case 12:
		buf += UCS_FF;
		break;

	case 10: /* line feed */
		lf = true;
	case 13: /* carriage return */
	case 31: /* soft hyphen */
		break;
	default:
		if (ch & 0x80) {
			charconv.mbtowc(uch, ch);
		}
		buf += uch;
	}
}

const char *IE_Imp_MSWrite::get_codepage(char *facename, char **newname) const
{
	static char facebuf[40];
	const cst_data *p=cp_suf_tbl;	
	int l=strlen(facename);	
	while (p->suffix) {
		if (*p->suffix < l) {
			if (!g_ascii_strcasecmp(&p->suffix[1],&facename[l-*p->suffix])) {
				if (newname) {
					strncpy(facebuf,facename,l-*p->suffix);
					facebuf[l-*p->suffix]=0;
					*newname=facebuf;
				}
				return p->cpid;
			}
		}
		p++;
	}
	return 0;
}

void IE_Imp_MSWrite::set_codepage(const char *charset)
{
	charconv.setInCharset(charset);
}

/* the section property */
int IE_Imp_MSWrite::read_sep ()
{
  int page, pnSetb, cch;
  int yaMac, yaTop, dyaText, yaBot, xaMac, xaLeft, dxaText, xaRight;
  unsigned char sep_page[0x80];
  UT_String propBuffer;

  page = wri_struct_value(write_file_header, "pnSep");
  pnSetb = wri_struct_value(write_file_header, "pnSetb");

  /* default values */
  yaMac = 15840;
  yaTop = 1440;
  dyaText = 12960;

  xaMac = 12240;
  xaLeft = 1800;
  dxaText = 8640;

  int firstpage = -1;

  if (page != pnSetb)
  {
    gsf_input_seek(mFile, page * 0x80, G_SEEK_SET);
    gsf_input_read(mFile, 0x80, sep_page);
    cch = *sep_page;

    if (cch >= 4) yaMac = READ_WORD(sep_page + 3);
    if (cch >= 6) xaMac = READ_WORD(sep_page + 5);
    if (cch >= 8) firstpage = READ_WORD(sep_page + 7);
    if (cch >= 10) yaTop = READ_WORD(sep_page + 9);
    if (cch >= 12) dyaText = READ_WORD(sep_page + 11);
    if (cch >= 14) xaLeft = READ_WORD(sep_page + 13);
    if (cch >= 16) dxaText = READ_WORD(sep_page + 15);
  }

  yaBot = yaMac - yaTop - dyaText;
  xaRight = xaMac - xaLeft - dxaText;

  UT_LocaleTransactor lt(LC_NUMERIC, "C");
  UT_String_sprintf(propBuffer, "page-margin-right:%.4fin; "
                                "page-margin-left:%.4fin; "
                                "page-margin-top:%.4fin; "
                                "page-margin-bottom:%.4fin",
                                static_cast<float>(xaRight) / 1440.0,
                                static_cast<float>(xaLeft) / 1440.0,
                                static_cast<float>(yaTop) / 1440.0,
                                static_cast<float>(yaBot) / 1440.0);

  // We assume each document has header and footer for
  // the sake of simplicity

  const gchar *propsArray[7];
  propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
  propsArray[1] = propBuffer.c_str();
  propsArray[2] = PT_HEADER_ATTRIBUTE_NAME;
  propsArray[3] = "0";
  propsArray[4] = PT_FOOTER_ATTRIBUTE_NAME;
  propsArray[5] = "1";
  propsArray[6] = NULL;

  appendStrux(PTX_Section, propsArray);

  return 0;
}

/* the paragraph information */
int IE_Imp_MSWrite::read_pap (int cStart, int cLimit) 
{
    int page, cfod, fod;
    int fcFirst, fcLim, fcMac, n;
    unsigned char pap_page[0x80];
    static const char *text_align[] = { "left", "center", "right", "justify" };

    UT_String propBuffer;
    UT_String tempBuffer;
    UT_String lastProp;
    lastProp.clear();
	
    fcMac = wri_struct_value (write_file_header, "fcMac");
    page = wri_struct_value (write_file_header, "pnPara");
    fcFirst = 0x80;
	
    while (true) {
		gsf_input_seek (mFile, page++ * 0x80, G_SEEK_SET);
		gsf_input_read (mFile, 0x80, pap_page);
		cfod = pap_page[0x7f];
		n = READ_DWORD (pap_page);
		if (n != fcFirst) {
			UT_DEBUGMSG(("fcFirst wrong, trying to continue...\n"));
		}
		for (fod=0;fod<cfod;fod++) {
			int jc, fGraphics, dyaLine, bfProp, cch, header, rhcPage;
			int tab_count, tabs[14], jcTab[14], dxaRight, dxaLeft, dxaLeft1;
			
            /* read FOD */	
			fcLim = READ_DWORD (pap_page + 4 + fod * 6);
			bfProp = READ_WORD (pap_page + 8 + fod * 6);
			
			if (cStart < fcLim) {	// do not set format if we're skipping this fod
				/* default values */
				jc = 0;
				dyaLine = 240;
				fGraphics = 0;
				header = rhcPage = 0;
				tab_count = 0;
				dxaLeft = dxaRight = dxaLeft1 = 0;
				
				if ((bfProp != 0xffff) && (bfProp + (cch = pap_page[bfProp + 4]) < 0x80)) {
					if (cch >= 2) {
						jc = pap_page[bfProp + 6]  & 3;
					}
					if (cch >= 12) {
						dyaLine = pap_page[bfProp + 15] + 
							pap_page[bfProp + 16] * 256;
					}
					if (dyaLine < 240) dyaLine = 240;
					if (cch >= 17) {
						fGraphics = pap_page[bfProp + 21] & 0x10;
						header = pap_page[bfProp + 21] & 6;
						rhcPage = pap_page[bfProp + 21] & 1;
					}
					if (cch >= 6) {
						dxaRight = pap_page[bfProp + 9] + 
							pap_page[bfProp + 10] * 256;
						if (dxaRight & 0x8000) 
							dxaRight = -0x10000 + dxaRight;
					}
					if (cch >= 8) {
						dxaLeft = pap_page[bfProp + 11] + 
							pap_page[bfProp + 12] * 256;
						if (dxaLeft & 0x8000) 
							dxaLeft = -0x10000 + dxaLeft;
					}
					if (cch >= 10) {
						dxaLeft1 = pap_page[bfProp + 13] + 
							pap_page[bfProp + 14] * 256;
						if (dxaLeft1 & 0x8000) 
							dxaLeft1 = -0x10000 + dxaLeft1;
					}

					// TODO: dyaBefore, dyaAfter are used for header/footer
					
					for (n=0;n<14;n++) {
						if (cch >= (4 * (n + 1) + 26) ) {
							tabs[tab_count] = pap_page[bfProp + n * 4 + 27] +
								256 * pap_page[bfProp + n * 4 + 28];
							jcTab[tab_count] = (pap_page[bfProp + n * 4 + 29] & 3);
							tab_count++;
						}
					}
				}
			}

			if (fcFirst >= cLimit) return 0;
			
			if (fcFirst >= cStart) {
				if (!m_insubdoc && header) {
					add_subdoc((rhcPage)? 1 : 0,
						fcFirst,fcLim);
				} else {
					UT_LocaleTransactor lt (LC_NUMERIC, "C");
					UT_String_sprintf (propBuffer, "text-align:%s; line-height:%.1f",
							   text_align[jc], static_cast<float>(dyaLine) / 240.0);

					/* tabs */
					if (tab_count) {
						propBuffer += "; tabstops:";
						for (n=0; n < tab_count; n++) {
						  UT_String_sprintf (tempBuffer, "%.4fin/%c0", static_cast<float>(tabs[n]) / 1440.0,
									 jcTab[n] ? 'D' : 'L');
							propBuffer += tempBuffer;
							if (n != (tab_count - 1)) {
								propBuffer += ",";
							}
						}
					}
					
					/* indentation */
					if (dxaLeft1) {
						UT_String_sprintf (tempBuffer, "; text-indent:%.4fin", 
								 static_cast<float>(dxaLeft1) / 1440.0);
						propBuffer += tempBuffer;
					}
					if (dxaLeft) {
						UT_String_sprintf (tempBuffer, "; margin-left:%.4fin", 
								   static_cast<float>(dxaLeft) / 1440.0);
						propBuffer += tempBuffer;
					}
					if (dxaRight) {
						UT_String_sprintf (tempBuffer, "; margin-right:%.4fin", 
								 static_cast<float>(dxaRight) / 1440.0);
						propBuffer += tempBuffer;
					}

					// end of formatting

					if (lf || (strcmp(propBuffer.c_str(), lastProp.c_str()) != 0)) {
					// only if fod has changed or last char was line feed
						const gchar* propsArray[3];
						propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
						propsArray[1] = propBuffer.c_str();
						propsArray[2] = NULL;
					
						appendStrux (PTX_Block, propsArray);
						lastProp = propBuffer;
					}
					
					if (fGraphics) {
						read_pic(fcFirst, fcLim - fcFirst);
					} else {
						read_char (fcFirst, fcLim - 1);
					}

				}
			}
			
			fcFirst = fcLim;
			if (fcLim >= fcMac) 
				return 0;
		}
    }
}

/* the character information stuff */
int IE_Imp_MSWrite::read_char (int fcFirst2, int fcLim2) {
    int page, cfod, fcFirst, n, fod ;
    int fcLim, fcMac;
    unsigned char char_page[0x80];

	UT_String propBuffer;
	UT_String tempBuffer;
	
	const char *oldcp = "";

    fcMac = wri_struct_value (write_file_header, "fcMac");
    page = (fcMac + 127) / 128;
    fcFirst = 0x80;
	
    while (true) {
		gsf_input_seek (mFile, page++ * 0x80, G_SEEK_SET);
		gsf_input_read (mFile, 0x80, char_page);
		cfod = char_page[0x7f];
		n = READ_DWORD (char_page);
		if (n != fcFirst) {
			UT_DEBUGMSG(("fcFirst wrong, trying to continue...\n"));
		}
		for (fod=0;fod<cfod;fod++) {
			int cch, bfProp, ftc, hps, bold, italic, underline, hpsPos;
			
			fcLim = READ_DWORD (char_page + 4 + fod * 6);
			bfProp = READ_WORD ((char_page + 8 + fod * 6));
			
			/* default values */
			ftc = 0;
			hps = 24;
			bold = italic = underline = hpsPos = 0;
			
			if ((bfProp != 0xffff) && (bfProp + (cch = char_page[bfProp + 4]) < 0x80)) {
				
				if (cch >= 2) 
					ftc = char_page[bfProp + 6] / 4;
				if (cch >= 5) 
					ftc |= (char_page[bfProp + 9] & 3) * 64;
				if (cch >= 3)
					hps = char_page[bfProp + 7];
				if (cch >= 2) 
					bold = char_page[bfProp + 6] & 1;
				if (cch >= 2) 
					italic = char_page[bfProp + 6] & 2;
				if (cch >= 4) 
					underline = char_page[bfProp + 8] & 1;
				if (cch >= 6) 
					hpsPos = char_page[bfProp + 10];
			}
			
			if (static_cast<unsigned>(ftc) >= wri_fonts_count) {
				ftc = wri_fonts_count - 1;
			}
			
			if ((fcLim >= fcFirst2) && (fcFirst <= fcLim2))  {
				mCharBuf.clear ();
				UT_LocaleTransactor lt (LC_NUMERIC, "C");
				
				UT_String_sprintf (propBuffer, "font-weight:%s", bold ? "bold" : "normal");
				if (hps != 24) {
					UT_String_sprintf(tempBuffer, "; font-size:%dpt", hps / 2);
					propBuffer += tempBuffer;
				}
				if (italic)  {
					propBuffer += "; font-style:italic";
				}
				if (underline) {
					propBuffer += "; text-decoration:underline";
				}
				if (hpsPos) {
					UT_String_sprintf (tempBuffer, "; text-position:%s",
							   hpsPos < 128 ? "superscript" : "subscript");
					propBuffer += tempBuffer;
				}
				if (wri_fonts_count) {
					UT_String_sprintf (tempBuffer, "; font-family:%s", wri_fonts[ftc].name.c_str());
					propBuffer += tempBuffer;
				}
				const char *needcp = wri_fonts[ftc].codepage ? 
					wri_fonts[ftc].codepage : default_cp.c_str();
				if (needcp != oldcp /*sic!*/) {
					set_codepage((char*)needcp);
					oldcp = needcp;
				}

				const gchar* propsArray[3];

				const char *pText = (const char*) mTextBuf.getPointer(0) - 0x80;
				//const char *pLimit = pText + mTextBuf.getLength();
				UT_uint32   nLimit = 0x80 + mTextBuf.getLength();

				while (fcFirst2 >= fcFirst) {
					if ((fcFirst2 >= fcLim) || (fcFirst2 > fcLim2) ||
						(fcFirst2 >= nLimit)) {
						break;
					}
					if (pText[fcFirst2] == 0x1) {
						propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
						propsArray[1] = propBuffer.c_str();
						propsArray[2] = NULL;
						appendFmt (propsArray);
						if (mCharBuf.size() > 0) {
							appendSpan (reinterpret_cast<const UT_UCSChar *>(mCharBuf.ucs4_str()), mCharBuf.size());
						}
						const gchar *atts[3] = {"type","page_number",NULL};
						appendObject(PTO_Field,atts,propsArray);
						mCharBuf.clear();
					} else
						translate_char (pText[fcFirst2], mCharBuf);
					fcFirst2++;
				}
				
				if (mCharBuf.size() > 0) {
					propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
					propsArray[1] = propBuffer.c_str();
					propsArray[2] = NULL;

					appendFmt (propsArray);
					appendSpan (reinterpret_cast<const UT_UCSChar *>(mCharBuf.ucs4_str()), mCharBuf.size());
				}
				/*else { // can occur on empty paragraphs
					UT_DEBUGMSG (("Hub: Ignoring 0 length span\n"));
				}*/
			}
			
			fcFirst=fcLim;
			if (fcMac == fcLim) {
				return 0;
			}
			if (fcFirst > fcLim2) {
				return 0;
			}
		}
    }
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_MSWrite::_loadFile (GsfInput * input)
{
    mFile = (GsfInput *)g_object_ref (G_OBJECT (input));
    if (!mFile)
    {
        return UT_ERROR;
    }
    
    UT_Error iestatus;
    
    X_CleanupIfError(iestatus, _parseFile());
    
    iestatus = UT_OK;
    
 Cleanup:
    g_object_unref(G_OBJECT(mFile));
    return iestatus;
}

/*****************************************************************/
/*****************************************************************/


/* the file header */
static const struct wri_struct WRITE_FILE_HEADER[] = {
	/* value, data, size, type, name */		/* word no. */
	{ 0,    NULL,   2,  CT_VALUE, "wIdent" },       /* 0 */
	{ 0,    NULL,   2,  CT_VALUE, "dty" },          /* 1 */
	{ 0,    NULL,   2,  CT_VALUE, "wTool" },        /* 2 */
	{ 0,    NULL,   2,  CT_VALUE, "reserved1" },    /* 3 */
	{ 0,    NULL,   2,  CT_VALUE, "reserved2" },    /* 4 */
	{ 0,    NULL,   2,  CT_VALUE, "reserved3" },    /* 5 */
	{ 0,    NULL,   2,  CT_VALUE, "reserved4" },    /* 6 */
	{ 0,    NULL,   4,  CT_VALUE, "fcMac" },        /* 7-8 */
	{ 0,    NULL,   2,  CT_VALUE, "pnPara" },       /* 9 */
	{ 0,    NULL,   2,  CT_VALUE, "pnFntb" },       /* 10 */
	{ 0,    NULL,   2,  CT_VALUE, "pnSep" },        /* 11 */
	{ 0,    NULL,   2,  CT_VALUE, "pnSetb" },       /* 12 */
	{ 0,    NULL,   2,  CT_VALUE, "pnPgtb" },       /* 13 */
	{ 0,    NULL,   2,  CT_VALUE, "pnFfntb" },      /* 14 */
	{ 0,    NULL,   64, CT_IGNORE, "szSsht" },      /* 15-47 */
	{ 0,    NULL,   2,  CT_VALUE, "pnMac" },        /* 48 */
	{ 0,    NULL,   0,  CT_IGNORE, NULL }           /* EOF */
};

#define WRITE_PICTURE_HEADER 40   // common size of both header types

static const struct wri_struct WRITE_PICTURE[] = {

	/* value, data, size, type, name */		/* word no. */
	/* METAFILEPICT structure */
	{ 0,	NULL, 	2,  CT_VALUE, "mm" },		/* 0 */
	{ 0, 	NULL,	2,  CT_VALUE, "xExt" },		/* 1 */
	{ 0,	NULL, 	2,  CT_VALUE, "yExt" },		/* 2 */
	{ 0,	NULL,	2,  CT_IGNORE, "hMF" },		/* 3 */
	/* end of METAFILEPICT */
	{ 0,	NULL,	2,  CT_VALUE, "dxaOffset" },	/* 4 */
		{ 0,	NULL,	2,  CT_VALUE, "dxaSize" },	/* 5 */
	{ 0,	NULL,	2,  CT_VALUE, "dyaSize" },	/* 6 */
	{ 0,	NULL,	2,  CT_VALUE, "cbOldSize" },	/* 7 */
	/* BITMAP structure */
	{ 0,	NULL,	2,  CT_VALUE, "bmType" },	/* 8 */
	{ 0, 	NULL,	2,  CT_VALUE, "bmWidth" },	/* 9 */
	{ 0, 	NULL,	2,  CT_VALUE, "bmHeight" },	/* 10 */
	{ 0,	NULL,	2,  CT_VALUE, "bmWidthBytes" },	/* 11 */
	{ 0,	NULL,	1,  CT_VALUE, "bmPlanes" },	/* 12.5 */
	{ 0,	NULL,   1,  CT_VALUE, "bmBitsPixel" },	/* 12 */
	{ 0,	NULL,	4,  CT_VALUE, "bmBits" },	/* 13-14 */
	/* end of BITMAP structure */
	{ 0,	NULL,	2,  CT_VALUE, "cbHeader" },	/* 15 */
	{ 0,	NULL,	4,  CT_VALUE, "cbSize" },	/* 16-17 */
	{ 0,	NULL,	2,  CT_VALUE, "mx" },		/* 18 */
	{ 0,	NULL,	2,  CT_VALUE, "my" },		/* 19 */
	{ 0,    NULL,   0,  CT_IGNORE, NULL }           /* EOF */
};

/* OLE stuff */
static const struct wri_struct WRITE_OLE_PICTURE[] = {
	/* value, data, size, type, name */		/* word no. */
	{ 0,	NULL, 	2,  CT_VALUE, "mm" },		/* 0 */
	{ 0, 	NULL,	4,  CT_IGNORE, "not_used" },	/* 1-2 */
	{ 0,	NULL, 	2,  CT_VALUE, "objectType" },	/* 3 */
	{ 0,	NULL,	2,  CT_VALUE, "dxaOffset" },	/* 4 */
	{ 0,	NULL,	2,  CT_VALUE, "dxaSize" },	/* 5 */
	{ 0,	NULL,	2,  CT_VALUE, "dyaSize" },	/* 6 */
	{ 0,	NULL,	2,  CT_IGNORE, "not_used2" },	/* 7 */
	{ 0,	NULL,	4,  CT_VALUE, "dwDataSize" },	/* 8-9 */
	{ 0, 	NULL,	4,  CT_IGNORE, "not_used3" },	/* 10-11 */
	{ 0, 	NULL,	4,  CT_VALUE, "dwObjNum" },	/* 12-13 */
	{ 0,	NULL,	2,  CT_VALUE, "not_used4" },	/* 14 */
	{ 0,	NULL,	2,  CT_VALUE, "cbHeader" },	/* 15 */
	{ 0,	NULL,   4,  CT_IGNORE, "not_used5" },	/* 16-17 */
	{ 0,	NULL,	2,  CT_VALUE, "mx" },		/* 18 */
	{ 0,	NULL,	2,  CT_VALUE, "my" },		/* 19 */
	{ 0,    NULL,   0,  CT_IGNORE, NULL }           /* EOF */
};

// OLE1.0 structure offsets
#define OLE_Version         0
#define OLE_FormatID        4
#define OLE_ClassNameLength 8
#define OLE_ClassNameString 12   // variable length
// offsets relative to empty ClassNameString
#define OLE_TopicNameLength 12
#define OLE_TopicNameString 16
// offsets relative to empty ClassNameString and TopicNameString
#define OLE_ItemNameLength  16
#define OLE_ItemNameString  20
// offsets relative to empty ClassNameString and TopicNameString and ItemNameString
#define OLE_ObjDataSize     20
#define OLE_Object          24
#define OLE_MF_Reserved     24
#define OLE_MF_Object       32

// Bitmap16 structure offsets
#define BM16_Type       0
#define BM16_Width      2
#define BM16_Height     4
#define BM16_WidthBytes 6
#define BM16_Planes     8
#define BM16_BitsPixel  9
#define BM16_Bits       10


IE_Imp_MSWrite::~IE_Imp_MSWrite()
{
  free_wri_struct (write_file_header);
  free(write_file_header);
  free(write_ole_picture);
  free(write_picture);
}

IE_Imp_MSWrite::IE_Imp_MSWrite(PD_Document * pDocument)
  : IE_Imp(pDocument), mFile(0), wri_fonts_count(0),
    wri_fonts(0), pic_nr(0), default_cp("CP1252"),
	lf(false)
{
	set_codepage((char*)default_cp.c_str());

	static const doc_range s_docrange_0 = {0,0};
	m_header = m_footer = s_docrange_0;

	write_file_header = static_cast<struct wri_struct*>(malloc (sizeof (WRITE_FILE_HEADER)));
	memcpy (write_file_header, WRITE_FILE_HEADER, sizeof (WRITE_FILE_HEADER));

	write_ole_picture = static_cast<struct wri_struct*>(malloc (sizeof (WRITE_OLE_PICTURE)));
	memcpy (write_ole_picture, WRITE_OLE_PICTURE, sizeof (WRITE_OLE_PICTURE));

	write_picture = static_cast<struct wri_struct*>(malloc (sizeof (WRITE_PICTURE)));
	memcpy (write_picture, WRITE_PICTURE, sizeof (WRITE_PICTURE));
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_MSWrite::_parseFile()
{
    int x, size;
    UT_Byte *thetext;
	
    if (read_wri_struct (write_file_header, mFile) ) {
		return UT_ERROR;
    }
	
    x = wri_struct_value (write_file_header, "wIdent");
    if (x == 0137062) {
		/* It's okay, but expect OLE objects */
    } else if (x != 0137061) {
		UT_DEBUGMSG(("Not a write file!\n"));
		return UT_ERROR;
    }
    if (wri_struct_value (write_file_header, "wTool") != 0125400) {
		UT_DEBUGMSG(("Not a write file!\n"));
		return UT_ERROR;
    }
    size = wri_struct_value (write_file_header, "fcMac") - 0x80;
    thetext = static_cast<UT_Byte*>(malloc (size));
    if (!thetext) {
		UT_DEBUGMSG(("Out of memory!\n"));
		return UT_ERROR;
    }
    if (gsf_input_seek (mFile, 0x80, G_SEEK_SET) ) {
		UT_DEBUGMSG(("Seek error!\n"));
		return UT_ERROR;
    }
    gsf_input_read (mFile, size, thetext);
    read_ffntb ();
	mTextBuf.truncate(0);
    mTextBuf.append(thetext, size);
    free(thetext);
    read_sep();
	m_insubdoc = false;
    read_pap (0x80, 0x80 + size); // ugly...
	
	UT_ASSERT(getDoc());

	m_insubdoc = true;
	_append_hdrftr(0);
	if (m_header.start > 0) {
		read_pap (m_header.start, m_header.end);
	}
	_append_hdrftr(1);
	if (m_footer.start > 0) {
		read_pap (m_footer.start, m_footer.end);
	}

    free_ffntb ();

    return UT_OK;
}

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
#endif

 /*****************************************************************/
 /*****************************************************************/
 
int IE_Imp_MSWrite::read_pic (int from, int size)
{
	unsigned char page[0x80];
	int mm;
	struct wri_struct *write_pic = NULL;
	int cbHeader, cbSize, bitsppixel, colorPaletteLen = 0;
	int bmw = 0, bmh = 0, chunk, filler, written;
	IEGraphicFileType iegft = IEGFT_Unknown;
	int objectType, ole_offset, formatID, objLen, bhSize, bppOff;
	const char *msg = NULL, *className = "";
	UT_ByteBuf pBB;
	FG_Graphic *pfg = NULL;
	int dxaOffset, dxaSize, dyaSize, mx, my;
	UT_String propBuffer, dataID;

	// structs without alignment
#pragma pack(push)
#pragma pack(1)
	struct
	{
		char bfType[2];
		uint32_t bfSize;
		uint32_t bfReserved;
		uint32_t bfOffBits;
	} bmpheader;
	struct
	{
		uint32_t header_sz;
		uint32_t width;
		uint32_t height;
		uint16_t nplanes;
		uint16_t bitspp;
		uint32_t compress_type;
		uint32_t bmp_bytesz;
		uint32_t hres;
		uint32_t vres;
		uint32_t ncolors;
		uint32_t nimpcolors;
	} bmpinfo;
#pragma pack(pop)

	// 8 bit colormap (plus one extra color only in the 4 bit colormap)
	const UT_Byte bgr_palette[257][4] =
	{
		{0x00, 0x00, 0x00, 0x00},
		{0x40, 0x00, 0x00, 0x00},
		{0x80, 0x00, 0x00, 0x00},
		{0xff, 0x00, 0x00, 0x00},
		{0x00, 0x20, 0x00, 0x00},
		{0x40, 0x20, 0x00, 0x00},
		{0x80, 0x20, 0x00, 0x00},
		{0xff, 0x20, 0x00, 0x00},
		{0x00, 0x40, 0x00, 0x00},
		{0x40, 0x40, 0x00, 0x00},
		{0x80, 0x40, 0x00, 0x00},
		{0xff, 0x40, 0x00, 0x00},
		{0x00, 0x60, 0x00, 0x00},
		{0x40, 0x60, 0x00, 0x00},
		{0x80, 0x60, 0x00, 0x00},
		{0xff, 0x60, 0x00, 0x00},
		{0x00, 0x80, 0x00, 0x00},
		{0x40, 0x80, 0x00, 0x00},
		{0x80, 0x80, 0x00, 0x00},
		{0xff, 0x80, 0x00, 0x00},
		{0x00, 0xa0, 0x00, 0x00},
		{0x40, 0xa0, 0x00, 0x00},
		{0x80, 0xa0, 0x00, 0x00},
		{0xff, 0xa0, 0x00, 0x00},
		{0x00, 0xc0, 0x00, 0x00},
		{0x40, 0xc0, 0x00, 0x00},
		{0x80, 0xc0, 0x00, 0x00},
		{0xff, 0xc0, 0x00, 0x00},
		{0x00, 0xff, 0x00, 0x00},
		{0x40, 0xff, 0x00, 0x00},
		{0x80, 0xff, 0x00, 0x00},
		{0xff, 0xff, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x00},
		{0x40, 0x00, 0x20, 0x00},
		{0x80, 0x00, 0x20, 0x00},
		{0xff, 0x00, 0x20, 0x00},
		{0x00, 0x20, 0x20, 0x00},
		{0x40, 0x20, 0x20, 0x00},
		{0x80, 0x20, 0x20, 0x00},
		{0xff, 0x20, 0x20, 0x00},
		{0x00, 0x40, 0x20, 0x00},
		{0x40, 0x40, 0x20, 0x00},
		{0x80, 0x40, 0x20, 0x00},
		{0xff, 0x40, 0x20, 0x00},
		{0x00, 0x60, 0x20, 0x00},
		{0x40, 0x60, 0x20, 0x00},
		{0x80, 0x60, 0x20, 0x00},
		{0xff, 0x60, 0x20, 0x00},
		{0x00, 0x80, 0x20, 0x00},
		{0x40, 0x80, 0x20, 0x00},
		{0x80, 0x80, 0x20, 0x00},
		{0xff, 0x80, 0x20, 0x00},
		{0x00, 0xa0, 0x20, 0x00},
		{0x40, 0xa0, 0x20, 0x00},
		{0x80, 0xa0, 0x20, 0x00},
		{0xff, 0xa0, 0x20, 0x00},
		{0x00, 0xc0, 0x20, 0x00},
		{0x40, 0xc0, 0x20, 0x00},
		{0x80, 0xc0, 0x20, 0x00},
		{0xff, 0xc0, 0x20, 0x00},
		{0x00, 0xff, 0x20, 0x00},
		{0x40, 0xff, 0x20, 0x00},
		{0x80, 0xff, 0x20, 0x00},
		{0xff, 0xff, 0x20, 0x00},
		{0x00, 0x00, 0x40, 0x00},
		{0x40, 0x00, 0x40, 0x00},
		{0x80, 0x00, 0x40, 0x00},
		{0xff, 0x00, 0x40, 0x00},
		{0x00, 0x20, 0x40, 0x00},
		{0x40, 0x20, 0x40, 0x00},
		{0x80, 0x20, 0x40, 0x00},
		{0xff, 0x20, 0x40, 0x00},
		{0x00, 0x40, 0x40, 0x00},
		{0x40, 0x40, 0x40, 0x00},
		{0x80, 0x40, 0x40, 0x00},
		{0xff, 0x40, 0x40, 0x00},
		{0x00, 0x60, 0x40, 0x00},
		{0x40, 0x60, 0x40, 0x00},
		{0x80, 0x60, 0x40, 0x00},
		{0xff, 0x60, 0x40, 0x00},
		{0x00, 0x80, 0x40, 0x00},
		{0x40, 0x80, 0x40, 0x00},
		{0x80, 0x80, 0x40, 0x00},
		{0xff, 0x80, 0x40, 0x00},
		{0x00, 0xa0, 0x40, 0x00},
		{0x40, 0xa0, 0x40, 0x00},
		{0x80, 0xa0, 0x40, 0x00},
		{0xff, 0xa0, 0x40, 0x00},
		{0x00, 0xc0, 0x40, 0x00},
		{0x40, 0xc0, 0x40, 0x00},
		{0x80, 0xc0, 0x40, 0x00},
		{0xff, 0xc0, 0x40, 0x00},
		{0x00, 0xff, 0x40, 0x00},
		{0x40, 0xff, 0x40, 0x00},
		{0x80, 0xff, 0x40, 0x00},
		{0xff, 0xff, 0x40, 0x00},
		{0x00, 0x00, 0x60, 0x00},
		{0x40, 0x00, 0x60, 0x00},
		{0x80, 0x00, 0x60, 0x00},
		{0xff, 0x00, 0x60, 0x00},
		{0x00, 0x20, 0x60, 0x00},
		{0x40, 0x20, 0x60, 0x00},
		{0x80, 0x20, 0x60, 0x00},
		{0xff, 0x20, 0x60, 0x00},
		{0x00, 0x40, 0x60, 0x00},
		{0x40, 0x40, 0x60, 0x00},
		{0x80, 0x40, 0x60, 0x00},
		{0xff, 0x40, 0x60, 0x00},
		{0x00, 0x60, 0x60, 0x00},
		{0x40, 0x60, 0x60, 0x00},
		{0x80, 0x60, 0x60, 0x00},
		{0xff, 0x60, 0x60, 0x00},
		{0x00, 0x80, 0x60, 0x00},
		{0x40, 0x80, 0x60, 0x00},
		{0x80, 0x80, 0x60, 0x00},
		{0xff, 0x80, 0x60, 0x00},
		{0x00, 0xa0, 0x60, 0x00},
		{0x40, 0xa0, 0x60, 0x00},
		{0x80, 0xa0, 0x60, 0x00},
		{0xff, 0xa0, 0x60, 0x00},
		{0x00, 0xc0, 0x60, 0x00},
		{0x40, 0xc0, 0x60, 0x00},
		{0x80, 0xc0, 0x60, 0x00},
		{0xff, 0xc0, 0x60, 0x00},
		{0x00, 0xff, 0x60, 0x00},
		{0x40, 0xff, 0x60, 0x00},
		{0x80, 0xff, 0x60, 0x00},
		{0xff, 0xff, 0x60, 0x00},
		{0x00, 0x00, 0x80, 0x00},
		{0x40, 0x00, 0x80, 0x00},
		{0x80, 0x00, 0x80, 0x00},
		{0xff, 0x00, 0x80, 0x00},
		{0x00, 0x20, 0x80, 0x00},
		{0x40, 0x20, 0x80, 0x00},
		{0x80, 0x20, 0x80, 0x00},
		{0xff, 0x20, 0x80, 0x00},
		{0x00, 0x40, 0x80, 0x00},
		{0x40, 0x40, 0x80, 0x00},
		{0x80, 0x40, 0x80, 0x00},
		{0xff, 0x40, 0x80, 0x00},
		{0x00, 0x60, 0x80, 0x00},
		{0x40, 0x60, 0x80, 0x00},
		{0x80, 0x60, 0x80, 0x00},
		{0xff, 0x60, 0x80, 0x00},
		{0x00, 0x80, 0x80, 0x00},
		{0x40, 0x80, 0x80, 0x00},
		{0x80, 0x80, 0x80, 0x00},
		{0xff, 0x80, 0x80, 0x00},
		{0x00, 0xa0, 0x80, 0x00},
		{0x40, 0xa0, 0x80, 0x00},
		{0x80, 0xa0, 0x80, 0x00},
		{0xff, 0xa0, 0x80, 0x00},
		{0x00, 0xc0, 0x80, 0x00},
		{0x40, 0xc0, 0x80, 0x00},
		{0x80, 0xc0, 0x80, 0x00},
		{0xff, 0xc0, 0x80, 0x00},
		{0x00, 0xff, 0x80, 0x00},
		{0x40, 0xff, 0x80, 0x00},
		{0x80, 0xff, 0x80, 0x00},
		{0xff, 0xff, 0x80, 0x00},
		{0x00, 0x00, 0xa0, 0x00},
		{0x40, 0x00, 0xa0, 0x00},
		{0x80, 0x00, 0xa0, 0x00},
		{0xff, 0x00, 0xa0, 0x00},
		{0x00, 0x20, 0xa0, 0x00},
		{0x40, 0x20, 0xa0, 0x00},
		{0x80, 0x20, 0xa0, 0x00},
		{0xff, 0x20, 0xa0, 0x00},
		{0x00, 0x40, 0xa0, 0x00},
		{0x40, 0x40, 0xa0, 0x00},
		{0x80, 0x40, 0xa0, 0x00},
		{0xff, 0x40, 0xa0, 0x00},
		{0x00, 0x60, 0xa0, 0x00},
		{0x40, 0x60, 0xa0, 0x00},
		{0x80, 0x60, 0xa0, 0x00},
		{0xff, 0x60, 0xa0, 0x00},
		{0x00, 0x80, 0xa0, 0x00},
		{0x40, 0x80, 0xa0, 0x00},
		{0x80, 0x80, 0xa0, 0x00},
		{0xff, 0x80, 0xa0, 0x00},
		{0x00, 0xa0, 0xa0, 0x00},
		{0x40, 0xa0, 0xa0, 0x00},
		{0x80, 0xa0, 0xa0, 0x00},
		{0xff, 0xa0, 0xa0, 0x00},
		{0x00, 0xc0, 0xa0, 0x00},
		{0x40, 0xc0, 0xa0, 0x00},
		{0x80, 0xc0, 0xa0, 0x00},
		{0xff, 0xc0, 0xa0, 0x00},
		{0x00, 0xff, 0xa0, 0x00},
		{0x40, 0xff, 0xa0, 0x00},
		{0x80, 0xff, 0xa0, 0x00},
		{0xff, 0xff, 0xa0, 0x00},
		{0x00, 0x00, 0xc0, 0x00},
		{0x40, 0x00, 0xc0, 0x00},
		{0x80, 0x00, 0xc0, 0x00},
		{0xff, 0x00, 0xc0, 0x00},
		{0x00, 0x20, 0xc0, 0x00},
		{0x40, 0x20, 0xc0, 0x00},
		{0x80, 0x20, 0xc0, 0x00},
		{0xff, 0x20, 0xc0, 0x00},
		{0x00, 0x40, 0xc0, 0x00},
		{0x40, 0x40, 0xc0, 0x00},
		{0x80, 0x40, 0xc0, 0x00},
		{0xff, 0x40, 0xc0, 0x00},
		{0x00, 0x60, 0xc0, 0x00},
		{0x40, 0x60, 0xc0, 0x00},
		{0x80, 0x60, 0xc0, 0x00},
		{0xff, 0x60, 0xc0, 0x00},
		{0x00, 0x80, 0xc0, 0x00},
		{0x40, 0x80, 0xc0, 0x00},
		{0x80, 0x80, 0xc0, 0x00},
		{0xff, 0x80, 0xc0, 0x00},
		{0x00, 0xa0, 0xc0, 0x00},
		{0x40, 0xa0, 0xc0, 0x00},
		{0x80, 0xa0, 0xc0, 0x00},
		{0xff, 0xa0, 0xc0, 0x00},
		{0x00, 0xc0, 0xc0, 0x00},
		{0x40, 0xc0, 0xc0, 0x00},
		{0x80, 0xc0, 0xc0, 0x00},
		{0xff, 0xc0, 0xc0, 0x00},
		{0x00, 0xff, 0xc0, 0x00},
		{0x40, 0xff, 0xc0, 0x00},
		{0x80, 0xff, 0xc0, 0x00},
		{0xff, 0xff, 0xc0, 0x00},
		{0x00, 0x00, 0xff, 0x00},
		{0x40, 0x00, 0xff, 0x00},
		{0x80, 0x00, 0xff, 0x00},
		{0xff, 0x00, 0xff, 0x00},
		{0x00, 0x20, 0xff, 0x00},
		{0x40, 0x20, 0xff, 0x00},
		{0x80, 0x20, 0xff, 0x00},
		{0xff, 0x20, 0xff, 0x00},
		{0x00, 0x40, 0xff, 0x00},
		{0x40, 0x40, 0xff, 0x00},
		{0x80, 0x40, 0xff, 0x00},
		{0xff, 0x40, 0xff, 0x00},
		{0x00, 0x60, 0xff, 0x00},
		{0x40, 0x60, 0xff, 0x00},
		{0x80, 0x60, 0xff, 0x00},
		{0xff, 0x60, 0xff, 0x00},
		{0x00, 0x80, 0xff, 0x00},
		{0x40, 0x80, 0xff, 0x00},
		{0x80, 0x80, 0xff, 0x00},
		{0xff, 0x80, 0xff, 0x00},
		{0x00, 0xa0, 0xff, 0x00},
		{0x40, 0xa0, 0xff, 0x00},
		{0x80, 0xa0, 0xff, 0x00},
		{0xff, 0xa0, 0xff, 0x00},
		{0x00, 0xc0, 0xff, 0x00},
		{0x40, 0xc0, 0xff, 0x00},
		{0x80, 0xc0, 0xff, 0x00},
		{0xff, 0xc0, 0xff, 0x00},
		{0x00, 0xff, 0xff, 0x00},
		{0x40, 0xff, 0xff, 0x00},
		{0x80, 0xff, 0xff, 0x00},
		{0xff, 0xff, 0xff, 0x00},
		{0xc0, 0xc0, 0xc0, 0x00}   // a color only in the 4 bit colormap
	};
	// indices for the 4 bit colormap
	int const c16idx[16] = {  0, 128, 16, 144, 2, 130, 18, 256,
		146, 224, 28, 252, 3, 227, 31, 255 };


	if (size < WRITE_PICTURE_HEADER + OLE_ClassNameString)
	{
		UT_DEBUGMSG(("Size error 1\n"));
		return 1;
    }

	// prepare bmp file header
	memset(&bmpheader, 0, sizeof(bmpheader));
	bmpheader.bfType[0] = 'B';
	bmpheader.bfType[1] = 'M';
	memset(&bmpinfo, 0, sizeof(bmpinfo));

	gsf_input_seek(mFile, from, G_SEEK_SET);
	gsf_input_read(mFile, sizeof(page), page);

	mm = READ_WORD(page);

	switch (mm)
	{
	case 0x88:   /* wmf file */
	case 0xe3:   /* bitmap */

		write_pic = write_picture;
		read_wri_struct_mem(write_pic, page);

		cbHeader = wri_struct_value(write_pic, "cbHeader");
		cbSize = wri_struct_value(write_pic, "cbSize");

		if (size < cbHeader + cbSize)
		{
			msg = "Size error 2\n";
			break;
    }

		if (mm == 0xe3)   // bitmap needs header
		{
			if ((bitsppixel = wri_struct_value(write_pic, "bmBitsPixel")) < 16)
				colorPaletteLen = (1 << bitsppixel) << 2;

			if (colorPaletteLen != 8)
			{
				msg = "Color palette error\n";
				break;
        }

			// make a bitmap file header

			WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + sizeof(bmpinfo) + colorPaletteLen);
			pBB.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

			bmw = wri_struct_value(write_pic, "bmWidth");
			bmh = wri_struct_value(write_pic, "bmHeight");
			if (bmh & 0x8000) bmh = -0x10000 + bmh;

			WRITE_DWORD(bmpinfo.header_sz, sizeof(bmpinfo));
			WRITE_DWORD(bmpinfo.width, bmw);
			WRITE_DWORD(bmpinfo.height, -bmh);
			WRITE_WORD(bmpinfo.nplanes, wri_struct_value(write_pic, "bmPlanes"));
			WRITE_WORD(bmpinfo.bitspp, bitsppixel);
			pBB.append(reinterpret_cast<UT_Byte *>(&bmpinfo), sizeof(bmpinfo));

			// add monochrome colormap
			pBB.append(bgr_palette[0], colorPaletteLen >> 1);
			pBB.append(bgr_palette[255], colorPaletteLen >> 1);

			chunk = wri_struct_value(write_pic, "bmWidthBytes");
			filler = (4 - (chunk & 3)) & 3;
        }
		else
		{
			iegft = IEGFT_WMF;
			chunk = cbSize;
			filler = 0;
    }


		written = 0;

		while (written < cbSize)
		{
			// append data
			pBB.append(mTextBuf.getPointer(from - 0x80 + cbHeader + written), chunk);
			for (int i = 0; i < filler; i++) pBB.append(reinterpret_cast<const UT_Byte *>("\x00"), 1);
			written += chunk;
    }

		break;

	case 0xe4:   /* ole object */	
		write_pic = write_ole_picture;
		read_wri_struct_mem(write_pic, page);

		objectType = wri_struct_value(write_pic, "objectType");
		cbHeader = wri_struct_value(write_pic, "cbHeader");

		if ((ole_offset = READ_DWORD(page + cbHeader + OLE_ClassNameLength)) > 0)
			className = reinterpret_cast<const char *>(page + cbHeader + OLE_ClassNameString);

		if ((formatID = READ_DWORD(page + cbHeader + OLE_FormatID)) == 1 ||
			formatID == 2)
		{
			if (size < cbHeader + OLE_TopicNameString + ole_offset)
			{
				msg = "Size error 3\n";
				break;
		}

			ole_offset += READ_DWORD(page + cbHeader + OLE_TopicNameLength + ole_offset);
			if (size < cbHeader + OLE_ItemNameString + ole_offset)
			{
				msg = "Size error 4\n";
				break;
		}

			ole_offset += READ_DWORD(page + cbHeader + OLE_ItemNameLength + ole_offset);
        }
		objLen = READ_DWORD(page + cbHeader + OLE_ObjDataSize + ole_offset);

		if (strcmp(className, "METAFILEPICT") == 0)
		{
			iegft = IEGFT_WMF;
			ole_offset += OLE_MF_Object - OLE_Object;
			objLen -= OLE_MF_Object - OLE_Object;
			objectType = 2;   // we can go embedded now
		}
		if (size <= cbHeader + OLE_Object + ole_offset ||
			cbHeader + OLE_Object + ole_offset > 0x80 ||
			size < cbHeader + OLE_Object + ole_offset + objLen)
		{
			msg = "Size error 5\n";
			break;
		}

		// static
		if (objectType == 1)
		{
			if (strcmp(className, "DIB") == 0)
			{
				bhSize = READ_DWORD(page + cbHeader + OLE_Object + ole_offset);
				bppOff = (bhSize == 12 ? 10 : 14);   // different header info types

				if ((bitsppixel = READ_WORD(page + cbHeader + OLE_Object + ole_offset + bppOff)) < 16)
					colorPaletteLen = (1 << bitsppixel) << 2;

				// make a bitmap file header
				WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + bhSize + colorPaletteLen);
				pBB.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

				objectType = 2;   // we can go embedded now

			} else if (strcmp(className, "BITMAP") == 0) {
				if ((bitsppixel = *(page + cbHeader + OLE_Object + ole_offset + BM16_BitsPixel)) < 16)
					colorPaletteLen = (1 << bitsppixel) << 2;

				// make a bitmap file header

				WRITE_DWORD(bmpheader.bfOffBits, sizeof(bmpheader) + sizeof(bmpinfo) + colorPaletteLen);
				pBB.append(reinterpret_cast<UT_Byte *>(&bmpheader), sizeof(bmpheader));

				bmh = READ_WORD(page + cbHeader + OLE_Object + ole_offset + BM16_Height);
				if (bmh & 0x8000) bmh = -0x10000 + bmh;

				WRITE_DWORD(bmpinfo.header_sz, sizeof(bmpinfo));
				WRITE_DWORD(bmpinfo.width, READ_WORD(page + cbHeader + OLE_Object + ole_offset + BM16_Width));
				WRITE_DWORD(bmpinfo.height, bmh);
				WRITE_WORD(bmpinfo.nplanes, *(page + cbHeader + OLE_Object + ole_offset + BM16_Planes));
				WRITE_WORD(bmpinfo.bitspp, bitsppixel);
				pBB.append(reinterpret_cast<UT_Byte *>(&bmpinfo), sizeof(bmpinfo));

				// add corresponding colormap
				switch (bitsppixel)
				{
				case 1:
					pBB.append(bgr_palette[0], colorPaletteLen >> 1);
					pBB.append(bgr_palette[255], colorPaletteLen >> 1);
					break;

				case 4:
					for (int i = 0; i < 16; i++)
						pBB.append(bgr_palette[c16idx[i]], 4);
					break;

				case 8:
					pBB.append(bgr_palette[0], colorPaletteLen);
					break;
				}

				ole_offset += BM16_Bits;
				objectType = 2;   // we can go embedded now
			}
		}

		// embedded
		if (objectType == 2)
			pBB.append(mTextBuf.getPointer(from - 0x80 + cbHeader + OLE_Object + ole_offset), objLen);

		break;
	}

	// let's see...
	if (pBB.getLength())
	{
		// ...whether it's a picture
		if ((IE_ImpGraphic::loadGraphic(pBB, iegft, &pfg) != UT_OK) || !pfg)
		{
			msg = "Picture load error or no picture\n";
		}
		else
		{
			const gchar* propsArray[5];

			dxaOffset = wri_struct_value(write_pic, "dxaOffset");

			if (dxaOffset & 0x8000) dxaOffset = -0x10000 + dxaOffset;

			if (dxaOffset)
			{
				UT_String_sprintf(propBuffer, "margin-left:%.4fin",
					static_cast<float>(dxaOffset) / 1440.0);

				propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
				propsArray[1] = propBuffer.c_str();
				propsArray[2] = NULL;

				appendStrux(PTX_Block, propsArray);
			}

			dxaSize = wri_struct_value(write_pic, "dxaSize");
			if (dxaSize == 0) dxaSize = wri_struct_value(write_pic, "xExt");

			dyaSize = wri_struct_value(write_pic, "dyaSize");
			if (dyaSize == 0) dyaSize = wri_struct_value(write_pic, "yExt");

			if (dxaSize & 0x8000) dxaSize = -0x10000 + dxaSize;
			if (dyaSize & 0x8000) dyaSize = -0x10000 + dyaSize;

			mx = wri_struct_value(write_pic, "mx");
			my = wri_struct_value(write_pic, "my");

			// 0xe3 picture sizes aren't reliable, so we recalculate them
			// (1 pixel = 15 twips)
			if (mm == 0xe3)
			{
				dxaSize = bmw * 15;
				dyaSize = (bmh < 0 ? -bmh : bmh) * 15;
			}

			UT_String_sprintf(propBuffer, "width:%.4fin; height:%.4fin",
				static_cast<float>(mx) / 1000.0 *
				static_cast<float>(dxaSize) / 1440.0,
				static_cast<float>(my) / 1000.0 *
				static_cast<float>(dyaSize) / 1440.0);

			UT_String_sprintf(dataID, "image%u", ++pic_nr);

			propsArray[0] = PT_PROPS_ATTRIBUTE_NAME;
			propsArray[1] = propBuffer.c_str();;
			propsArray[2] = "dataid";
			propsArray[3] = dataID.c_str();
			propsArray[4] = NULL;

			appendObject(PTO_Image, propsArray);
			getDoc()->createDataItem(dataID.c_str(), false, pfg->getBuffer(), pfg->getMimeType(), NULL);
		}
	}

	DELETEP(pfg);

	if (write_pic) free_wri_struct(write_pic);

	if (msg)
	{
		UT_DEBUGMSG((msg));
	}

	return (msg ? 1 : 0);
}

void IE_Imp_MSWrite::add_subdoc(int type, unsigned int from, unsigned int to)
{
	doc_range *r;
	if (type == 0) r=&m_header;
	else if (type == 1) r=&m_footer;
	else return;

	if (r->end == r->start) {
		r->start = from;
		r->end = to;
	} else if (r->end == from) {
		r->end = to;
	} else 
		UT_DEBUGMSG(("Not contigious range [%d;%d), was [%d;%d)\n",from,to,r->start,r->end));
}

void IE_Imp_MSWrite::_append_hdrftr(int type)
{
	const gchar *attribs[8];

	attribs[0]= PT_ID_ATTRIBUTE_NAME;
	attribs[1]= type ? "1" : "0";
	attribs[2]= PT_TYPE_ATTRIBUTE_NAME;
	attribs[3]= type ? "footer" : "header";
	attribs[4]=NULL;

	appendStrux(PTX_Section,attribs);
}