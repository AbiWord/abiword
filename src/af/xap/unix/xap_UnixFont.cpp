/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixFont.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"
#include "xap_App.h"
//#include "ut_AdobeEncoding.h"

//this one is for use with qsort
static int s_compareUniWidths(const void * w1, const void * w2)
{
	const uniWidth   * W1 = (const uniWidth   * ) w1;
	const uniWidth   * W2 = (const uniWidth   * ) w2;

	if(W1->ucs < W2->ucs)
		return -1;
	if(W1->ucs > W2->ucs)
		return 1;
	return 0;
}

//this one is for use with bsearch
static int s_compareUniWidthsChar(const void * c, const void * w)
{
	const UT_UCSChar * C = (const UT_UCSChar * ) c;
	const uniWidth   * W = (const uniWidth   * ) w;
	if(*C < W->ucs)
		return -1;
	if(*C > W->ucs)
		return 1;
	return 0;
}

#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

/*******************************************************************/

XAP_UnixFontHandle::XAP_UnixFontHandle()
  : m_font(NULL), m_size(0)
{
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFont * font, UT_uint32 size)
  : m_font(font), m_size(size)
{
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFontHandle & copy)
  : GR_Font(copy), m_font(copy.m_font), m_size(copy.m_size)
{
}

XAP_UnixFontHandle::~XAP_UnixFontHandle()
{
}

GdkFont * XAP_UnixFontHandle::getGdkFont(void)
{
	if (m_font)
		return m_font->getGdkFont(m_size);
	else
		return NULL;
}

UT_uint32 XAP_UnixFontHandle::getSize(void)
{
	return m_size;
}


void XAP_UnixFontHandle::explodeUnixFonts(XAP_UnixFont ** pSingleByte, XAP_UnixFont ** pMultiByte)
{
	if(m_font==NULL)
	{
		*pSingleByte = NULL;
		*pMultiByte = NULL;
		return;
	}

	if(m_font->is_CJK_font())
	{
		*pMultiByte = m_font;
		*pSingleByte = m_font->getMatchUnixFont();
	}
	else
	{
		*pSingleByte = m_font;
		*pMultiByte = m_font->getMatchUnixFont();
	}		
}

void XAP_UnixFontHandle::explodeGdkFonts(GdkFont* & non_cjk_one,GdkFont*& cjk_one)
{
	if(m_font->is_CJK_font())
	  {
		non_cjk_one=getMatchGdkFont();
		cjk_one=getGdkFont();
	  }
	else
	  {
		non_cjk_one=getGdkFont();
		cjk_one=getMatchGdkFont();
	  }
}


/*******************************************************************/		

XAP_UnixFont::XAP_UnixFont(void)
  :  m_fontKey(NULL), m_name(NULL), m_style(STYLE_LAST),
     m_xlfd(NULL), m_metricsData(NULL), m_uniWidths(NULL),
     m_fontfile(NULL), m_metricfile(NULL), m_PFFile(NULL),
     m_PFB(false), m_bufpos(0), m_pEncodingTable(NULL),
     m_iEncodingTableSize(0), m_is_cjk(false), m_fontType(FONT_TYPE_UNKNOWN),
     m_bisCopy(false)
{
	//UT_DEBUGMSG(("XAP_UnixFont:: constructor (void)\n"));	

	m_cjk_font_metric.ascent  = 0;
	m_cjk_font_metric.descent = 0;
	m_cjk_font_metric.width   = 0;
}

XAP_UnixFont::XAP_UnixFont(XAP_UnixFont & copy)
{
	//UT_DEBUGMSG(("XAP_UnixFont:: copy constructor\n"));

	m_is_cjk=copy.m_is_cjk;
	m_cjk_font_metric.ascent =copy.m_cjk_font_metric.ascent;
	m_cjk_font_metric.descent =copy.m_cjk_font_metric.descent;
	m_cjk_font_metric.width =copy.m_cjk_font_metric.width;
	
	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;

	m_fontfile = NULL;
	m_metricfile = NULL;
	m_uniWidths = NULL;
	
	m_metricsData = NULL;

	m_PFFile = NULL;

	m_fontKey = NULL;

	openFileAs(copy.getFontfile(),
			   copy.getMetricfile(),
			   copy.getXLFD(),
			   copy.getStyle());
	m_pEncodingTable = NULL;
	m_iEncodingTableSize = 0;
	if(copy.getEncodingTable())
		loadEncodingFile();
	m_bisCopy = true;
}

XAP_UnixFont::~XAP_UnixFont(void)
{
	
	FREEP(m_name);
	FREEP(m_fontfile);
	FREEP(m_metricfile);
	FREEP(m_xlfd);
	FREEP(m_fontKey);

	//	UT_VECTOR_PURGEALL(allocFont *, m_allocFonts);
	for(UT_uint32 i =0; i < m_allocFonts.getItemCount(); i++)
	{
		allocFont * p = (allocFont *) m_allocFonts.getNthItem(i);
		gdk_font_unref(p->gdkFont);
		delete p;
	}
	if(m_uniWidths)
	{
		delete [] m_uniWidths;
		m_uniWidths = 0;
	}
	
	_deleteEncodingTable();

	// leave GdkFont * alone
	// but not ABIFontInfo *!

	if (m_metricsData != NULL)
	{
		if (m_metricsData->cmi)
		{
			FREEP(m_metricsData->cmi->name);
			Ligature * l = m_metricsData->cmi->ligs;
			Ligature * ln = l ? l->next : NULL;
			while (ln)
			{
				FREEP(l->succ);
				FREEP(l->lig);
				FREEP(l);
				l = ln;
				ln = ln->next;
			}
			FREEP(l);
		}
		FREEP(m_metricsData->cmi);
		FREEP(m_metricsData->cwi);
		if (m_metricsData->gfi)
		{
			FREEP(m_metricsData->gfi->afmVersion);
			FREEP(m_metricsData->gfi->fontName);
			FREEP(m_metricsData->gfi->encodingScheme);
			FREEP(m_metricsData->gfi->fullName);
			FREEP(m_metricsData->gfi->familyName);
			FREEP(m_metricsData->gfi->weight);
			FREEP(m_metricsData->gfi->version);
			FREEP(m_metricsData->gfi->notice);
		}
		FREEP(m_metricsData->gfi);
		FREEP(m_metricsData->tkd);
		if (m_metricsData->pkd)
		{
			int pos = 0;
			while (pos < m_metricsData->numOfPairs)
			{
				FREEP(m_metricsData->pkd[pos].name1);
				FREEP(m_metricsData->pkd[pos].name2);
				pos++;
			}
			FREEP(m_metricsData->pkd);
		}
		FREEP(m_metricsData->pkd);
		if (m_metricsData->ccd)
		{
			int pos = 0;
			while (pos < m_metricsData->numOfComps)
			{
				FREEP(m_metricsData->ccd[pos].ccName);
				int jpos = 0;
				while (jpos<m_metricsData->ccd[pos].numOfPieces)
					FREEP(m_metricsData->ccd[pos].pieces[jpos++].pccName);
				pos++;
			}
		}
		FREEP(m_metricsData->ccd);
		FREEP(m_metricsData);
	}
}

bool XAP_UnixFont::openFileAs(const char * fontfile,
			      const char * metricfile,
			      const char * xlfd,
			      XAP_UnixFont::style s)
{
	// test all our data to make sure we can continue
	if (!fontfile)
		return false;
	if (!metricfile)
		return false;
	if (!xlfd)
		return false;

	struct stat buf;
	
	if (!m_is_cjk) //HJ's patch had this logic
	{
	    int err = stat(fontfile, &buf);
	    UT_ASSERT(err == 0 || err == -1);

	    if (! (err == 0 || S_ISREG(buf.st_mode)) )
		return false;
	
	    err = stat(metricfile, &buf);
	    UT_ASSERT(err == 0 || err == -1);

	    if (! (err == 0 || S_ISREG(buf.st_mode)) )
		return false;
	};
	
	// strip our proper face name out of the XLFD
	char * newxlfd;
	UT_cloneString(newxlfd, xlfd);

	// run past the first field (foundry)
	strtok(newxlfd, "-");
	// save the second to a member
	FREEP(m_name);
	UT_cloneString(m_name, strtok(NULL, "-"));
	
	FREEP(newxlfd);
	
	// save to memebers
	FREEP(m_fontfile);
	UT_cloneString(m_fontfile, fontfile);
	FREEP(m_metricfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);

	// update our key so we can be identified
	_makeFontKey();

	if(strstr(m_fontfile, ".ttf"))
		m_fontType = FONT_TYPE_TTF;
	else if(strstr(m_fontfile, ".pfa"))
		m_fontType = FONT_TYPE_PFA;
	else if(strstr(m_fontfile, ".pfb"))
		m_fontType = FONT_TYPE_PFB;
	else
	{
		m_fontType = FONT_TYPE_UNKNOWN;
		if(!m_is_cjk)	/* cjk has different fonts.dir format. */
		  {
		    return false;
		  }
	}

	return true;
}

void XAP_UnixFont::setName(const char * name)
{
	FREEP(m_name);
	UT_cloneString(m_name, name);
}

const char * XAP_UnixFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_name;
}

void XAP_UnixFont::setStyle(XAP_UnixFont::style s)
{
	m_style = s;
}

XAP_UnixFont::style XAP_UnixFont::getStyle(void)
{
	ASSERT_MEMBERS;
	return m_style;
}

const char * XAP_UnixFont::getFontfile(void)
{
	ASSERT_MEMBERS;
	
	return m_fontfile;
}

const char * XAP_UnixFont::getMetricfile(void)
{
	ASSERT_MEMBERS;
	return m_metricfile;
}

void XAP_UnixFont::setXLFD(const char * xlfd)
{
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);
}

const char * XAP_UnixFont::getXLFD(void)
{
	ASSERT_MEMBERS;
	return m_xlfd;
}

UT_uint16 XAP_UnixFont::getCharWidth(UT_UCSChar c)
{
	if(!m_uniWidths || !m_metricsData)
		getMetricsData();

	uniWidth * w = (uniWidth *) bsearch(&c, m_uniWidths, m_metricsData->numOfChars, sizeof(uniWidth), s_compareUniWidthsChar);
	if(w)
		return w->width;
	else
		return 0;
}

bool	XAP_UnixFont::_getMetricsDataFromX(void)
{
	UT_DEBUGMSG(("Attempting to generate font metrix from X server info\n"));
	UT_DEBUGMSG(("This feature is not implemented (yet)\n"));
	return (false);
}

void XAP_UnixFont::_deleteEncodingTable()
{
	if(m_pEncodingTable)
	{
		for(UT_uint32 i = 0; i < m_iEncodingTableSize; i++)
			FREEP(m_pEncodingTable[i].adb);
		delete[] m_pEncodingTable;
		m_pEncodingTable = 0;
		m_iEncodingTableSize = 0;
	}
}

const encoding_pair * XAP_UnixFont::loadEncodingFile()
{
	char * encfile = new char[strlen(m_fontfile)+3]; //3 to be on the safe side, in case extension is < 3
	strcpy(encfile, m_fontfile);
	char * dot = strrchr(encfile, '.');
	if(!dot)
	  {
	    delete [] encfile;
	    return NULL;
	  }
	*(dot+1) = 'u';
	*(dot+2) = '2';
	*(dot+3) = 'g';
	*(dot+4) = 0;
	
	const encoding_pair * ep = loadEncodingFile(encfile);
	delete [] encfile;
	return ep;
}

const encoding_pair * XAP_UnixFont::loadEncodingFile(char * encfile)
{
	if(m_pEncodingTable)
	{
		//UT_DEBUGMSG(("UnixFont: font table already exists\n"));
		return m_pEncodingTable;
	}
	
	if(!encfile)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	xxx_UT_DEBUGMSG(("Loading encoding file %s \n",encfile));
	
	FILE * ef = fopen(encfile, "r");
	if(!ef)
	{
		//if the file does not exist, we will load a generic one
		//get the path from m_fontfile
		char * full_name = new char[strlen(XAP_App::getApp()->getAbiSuiteLibDir())+30];
		strcpy(full_name, m_fontfile);

		char * slash = strrchr(full_name, '/');
		if(slash)
			*(slash+1) = 0;
		
		//first we look for a locale specific file locale.u2g in the same directory as this font
		strcat(full_name, "locale.u2g");
		UT_DEBUGMSG(("UnixFont: trying to open locale specific encoding file [%s]\n", full_name));

		ef = fopen(full_name, "r");

		//if there is no locale.u2g, we will open one of the default Adobe encodings
		if(!ef)
		{
			strcpy(full_name, XAP_App::getApp()->getAbiSuiteLibDir());

			//if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
				//unicode locale, use the complete table
				strcat(full_name, "/fonts/adobe-full.u2g");
			//else
			//	strcat(full_name, "/fonts/adobe-short.u2g");
			
			UT_DEBUGMSG(("UnixFont: loading generic encoding file [%s]\n", full_name));
		
			ef = fopen(full_name, "r");
		
			if(!ef)
			{
				UT_DEBUGMSG(("UnixFont: could not load default encoding file [%s].\n", full_name));
				char msg[300];
				sprintf(msg, "AbiWord could not find required encoding file %s.", full_name);
				messageBoxOK(msg);
				delete[] full_name;
				return 0;
			}
		}
		delete[] full_name;
	}

	char buff[128];
	//skip comments and empty lines
	while(fgets(buff,128,ef))
		if((*buff != '#')&&(*buff != '\n'))
			break;
			
	m_iEncodingTableSize = atoi(buff);
	if(!m_iEncodingTableSize)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	UT_DEBUGMSG(("encoding file contains %d entries\n", m_iEncodingTableSize));
	
	UT_uint32 ucs;
	
	m_pEncodingTable = new encoding_pair[m_iEncodingTableSize];
	
	for(UT_uint32 i = 0; i < m_iEncodingTableSize; i++)
	{
		if(!fgets(buff,128,ef))
		{
			for(UT_uint32 j = 0; j < i; j++)
				FREEP(m_pEncodingTable[j].adb);
			delete[] m_pEncodingTable;
			m_pEncodingTable = 0;
			m_iEncodingTableSize = 0;
			UT_DEBUGMSG(("Invalid encoding file\n"));
			fclose(ef);
			return NULL;
		}
		
		if((*buff != '#')&&(*buff != '\n'))
		{
			buff[strlen(buff)-1]= 0; //remove '\n'
			char * comma = strchr(buff, ',');
			if(!comma)
			{
				for(UT_uint32 j = 0; j < i; j++)
					FREEP(m_pEncodingTable[j].adb);
				delete[] m_pEncodingTable;
				m_pEncodingTable = 0;
				m_iEncodingTableSize = 0;
				UT_DEBUGMSG(("Invalid encoding file\n"));
				fclose(ef);
				return NULL;
			}
			
			*comma = 0;
			
			sscanf(comma + 1, "0x%x", &ucs);
			m_pEncodingTable[i].ucs = (UT_UCSChar) ucs;
			m_pEncodingTable[i].adb = UT_strdup(buff);
			//UT_DEBUGMSG(("encoding pair: %s, 0x%x\n",m_pEncodingTable[i].adb, m_pEncodingTable[i].ucs));
		}
		else
			i--;
	}
	
	fclose(ef);
	return m_pEncodingTable;
};

bool XAP_UnixFont::_createPsSupportFiles()
{
	if(!is_PS_font())
		return false;

	char fontfile[100];
	char pfa2util[100];
	char cmdline[400];
	char buff[256];
	
	strcpy(pfa2util, XAP_App::getApp()->getAbiSuiteLibDir());
	strcat(pfa2util, "/bin/pfa2afm");
	
	strcpy(fontfile, m_fontfile);
	char * dot   = strrchr(fontfile, '.');
	*(dot+1) = 0;

	/*	now open a pipe to the pfa2afm program, and examine the output for
		presence of error messages
	*/
	sprintf(cmdline, "%s -p %s -a %safm", pfa2util, m_fontfile, fontfile);
	
	UT_DEBUGMSG(("XAP_UnixFont::_createPsSupportFiles: running pfa2afm\n\t%s", cmdline));
	FILE * p = popen(cmdline, "r");
	if(!p)
	{
		UT_DEBUGMSG(("XAP_UnixFont::_createPsSupportFiles: unable to run pfa2afm\n"));
		return false;
	}
	
	while(!feof(p))
	{
		fgets(buff, 256, p);
		if(strstr(buff, "Error"))
		{
			UT_DEBUGMSG(("XAP_UnixFont::_createPsSupportFiles: pfa2afm error:\n%s\n", buff));
			return false;
		}
	}
	pclose(p);

	return true;
}
bool XAP_UnixFont::_createTtfSupportFiles()
{
	if(!is_TTF_font())
		return false;
	char fontfile[100];
	char buff[256];
	const char *enc;
	const char *libdir = XAP_App::getApp()->getAbiSuiteLibDir();
	const char latin1[]= "ISO-8859-1";
	
	if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
		enc = latin1;
	else
	 	enc = XAP_EncodingManager::get_instance()->getNativeEncodingName();
	
	char * dot   = strrchr(m_fontfile, '.');
	strcpy(fontfile, m_fontfile);
	fontfile[dot - m_fontfile + 1] = 0;

	char * cmdline = new char[strlen(libdir) + 4*strlen(fontfile)+ strlen(enc) + 50 ];
	/*	now open a pipe to the ttftool program, and examine the output for
		presence of error messages
	*/
	sprintf(cmdline, "%s/bin/ttftool -f %sttf -a %safm -u %su2g -p %st42 -e %s", libdir, fontfile, fontfile, fontfile, fontfile,enc);
	
	UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: \n\t%s", cmdline));
	FILE * p = popen(cmdline, "r");
	if(!p)
	{
		UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: unable to run ttftool\n"));
		return false;
	}
	
	while(!feof(p))
	{
		fgets(buff, 256, p);
		if(strstr(buff, "Error"))
		{
			UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: ttftool error:\n%s\n", buff));
			return false;
		}
	}
	pclose(p);

	delete[] cmdline;
	return true;
}

ABIFontInfo * XAP_UnixFont::getMetricsData(void)
{
	if (m_metricsData)
		return m_metricsData;

	UT_ASSERT(m_metricfile);
	
	// open up the metrics file, which should have been proven to
	// exist earlier in the construction of this object.
	FILE * fp = fopen(m_metricfile, "r");

	char message[1024];
	
	/*	if Afm file is not found, try to generate afm and u2g file
		from the font
	*/
	if (fp == NULL)
	{
		if(_createTtfSupportFiles() || _createPsSupportFiles())
			fp = fopen(m_metricfile, "r");
	}
	
	if (fp == NULL)
	{
		if(!_getMetricsDataFromX())	
		{
			g_snprintf(message, 1024,
				   "The font metrics file [%s] could\n"
				   "not be opened for parsing, nor was\n"
				   "it possible to retrieve the needed\n"
				   "information from the X server; this\n"
				   "is a fatal error and AbiWord will\n"
				   "terminate.",
				   m_metricfile);
			messageBoxOK(message);
			return NULL;
		}
		else
			return(m_metricsData);
	}
    else
	{
		//UT_DEBUGMSG(("Found afm file for the font\n"));
		// call down to the Adobe code
		int result = abi_parseFile(fp, &m_metricsData, P_GM);
		switch (result)
		{
			case parseFileRes_parseError:
				g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font metrics file\n"
				   "[%s].\n"
				   "These errors were not fatal, but print metrics may be incorrect.",
				   m_metricfile);
				messageBoxOK(message);
				break;
			case parseFileRes_earlyEOF:
				g_snprintf(message, 1024,
				   "AbiWord encountered a premature End of File (EOF) while parsing\n"
				   "the font metrics file [%s].\n"
				   "Printing cannot continue.",
				   m_metricfile);
				messageBoxOK(message);
				free (m_metricsData);
				m_metricsData = NULL;
				break;
			case parseFileRes_storageProblem:
				// if we got here, either the metrics file is broken (like it's
				// saying it has 209384098278942398743982 kerning lines coming, and
				// we know we can't allocate that), or we really did run out of memory.
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				free (m_metricsData);
				m_metricsData = NULL;
				break;
			default:
				// everything is peachy
				break;
		}

		// Need to mangle the cmi[i].wx variables to work right with Unicode
		UT_DEBUGMSG(("PS font file:          %s %d\n", m_fontfile, this));
		UT_DEBUGMSG(("PS font metrics file:  %s\n", m_metricfile));
		UT_DEBUGMSG(("PS font encoding type: %s\n", m_metricsData->gfi->encodingScheme));
		UT_DEBUGMSG(("PS font char count: %d\n", m_metricsData->numOfChars));
		m_uniWidths = new uniWidth[m_metricsData->numOfChars];
		memset (m_uniWidths, 0, m_metricsData->numOfChars * sizeof(uniWidth)); // Clear array
		UT_AdobeEncoding *ae = 0;

		if (loadEncodingFile() && !XAP_EncodingManager::get_instance()->cjk_locale())
		{
			/*
				map glyphs by name.
			*/
			UT_DEBUGMSG(("Creating AdobeEncoding with %d entries\n", m_iEncodingTableSize));
			ae = new UT_AdobeEncoding(m_pEncodingTable, m_iEncodingTableSize);
    		int numfound = 0;
			
			for (UT_sint32 i=0; i < m_metricsData->numOfChars; ++i)
       		{
				UT_UCSChar unicode = ae->adobeToUcs(m_metricsData->cmi[i].name);
				if (unicode > 0)
				{
					m_uniWidths[numfound].ucs = unicode;
					m_uniWidths[numfound].width = m_metricsData->cmi[i].wx;
					++numfound;
				}

				// AbiWord does the adobe glyph -> unicode mapping
				// with a tiny bit of bogosity.  It assumes that the
				// relation is one-to-one.
				// This isn't necessarily so; read 4c (double mappings) at
			// http://partners.adobe.com/asn/developer/type/unicodegn.html#4
				// Fortunately, we can insert a gross hack!

				if (unicode == 0x20) // gross hack: space
				{
					m_uniWidths[numfound].ucs = 0xa0;
					m_uniWidths[numfound].width = m_metricsData->cmi[i].wx;
					++numfound;
				}
			}

			// we want to set m_metricsData->numOfChars to the number of chars which we found;
			// this is typically smaller than the afm file indicated (often the font contains
			// alternative glyphs, names of which use vendor-defined names; we are not able to
			// translate these into UCS-2 values).

			m_metricsData->numOfChars = numfound;
			
			//OK, now we have the widths, we can get rid off the encoding table
			// (there is a chance that we will have to immediately recreate the
			// encoding table in the PSFont, but I would rather do that, than
			// was left with the table left around when not needed.)
			delete ae;
			_deleteEncodingTable();
		}
		else
		{
			/*	
				either CJK or we did not succeed in loading an encoding file
				we have to assume that order of glyphs in font is correct.
			*/
			
           int numfound = 0;
           for (UT_sint32 i=0; i < m_metricsData->numOfChars; ++i)
           {     
				UT_UCSChar ucs = XAP_EncodingManager::get_instance()->try_nativeToU(m_metricsData->cmi[i].code);
				if(ucs)
				{
					m_uniWidths[numfound].ucs   = ucs;
					m_uniWidths[numfound].width = m_metricsData->cmi[i].wx;
					++numfound;
				}
           }
		   m_metricsData->numOfChars = numfound;

       	}
	
		//Now we sort the m_uniWidths array
		qsort(m_uniWidths, m_metricsData->numOfChars, sizeof(uniWidth), s_compareUniWidths);
		
		fclose(fp);
		
		UT_ASSERT(m_metricsData);
		UT_ASSERT(m_metricsData->gfi);
		return m_metricsData;
	}
}

bool XAP_UnixFont::openPFA(void)
{
	ASSERT_MEMBERS;
	int peeked;

	/*	If this is a ttf font rather than a pfa/pfb font, we will try to
		open a .t42 file for the font which should have been generated
		when the font was first used; this file contains Type 42 PS font
		corresponding to our ttf font
	*/	
	char* pfafile = 0;
	bool  delete_pfa = false;
	if(is_TTF_font())
	{
		pfafile = new char[strlen(m_fontfile)+1];
		strcpy(pfafile, m_fontfile);
		delete_pfa = true;
		char* dot = strrchr(pfafile, '.');
		if(dot)
		{
			*(dot+2) = '4';
			*(dot+3) = '2';
		}
		else
			return false;
	}
	else
	{
		pfafile = m_fontfile;
	}
	
	UT_DEBUGMSG(("UnixFont::openPFA: opening file %s\n", pfafile));
	m_PFFile = fopen(pfafile, "r");

	if (!m_PFFile)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "Font data file [%s] can not be opened for reading.\n", m_fontfile);
		messageBoxOK(message);
		return false;
	}

	/* Check to see if it's a binary or ascii PF file */
	peeked = fgetc(m_PFFile);
	ungetc(peeked, m_PFFile);
	m_PFB = peeked == PFB_MARKER;
	m_bufpos = 0;
	if(delete_pfa && pfafile)
		delete[] pfafile;
	
	//UT_DEBUGMSG(("UnixFont::openPFA: about to return\n"));
	return true;
}

bool XAP_UnixFont::closePFA(void)
{
	if (m_PFFile)
	{
		fclose(m_PFFile);
		return true;
	}
	return false;
}

char XAP_UnixFont::getPFAChar(void)
{
	UT_ASSERT(m_PFFile);
	return m_PFB ? _getPFBChar() : fgetc(m_PFFile);
}

char XAP_UnixFont::_getPFBChar(void)
{
	char message[1024];
	UT_Byte inbuf[BUFSIZ], hexbuf[2 * BUFSIZ], *inp, *hexp;
	int  type, in, got, length;
	static char *hex = "0123456789abcdef";

	if (m_buffer.getLength() > m_bufpos )
	{
		return m_buffer.getPointer(m_bufpos++)[0];
	}

	// Read a new block into the buffer.
	m_buffer.truncate(0);
	in = fgetc(m_PFFile);
	type = fgetc(m_PFFile);

	if (in != PFB_MARKER
	    || (type != PFB_ASCII && type != PFB_BINARY && type != PFB_DONE))
	{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
		messageBoxOK(message);
		return EOF;
	}

	if (type == PFB_DONE)
	{
		/*
		 * If there's data after this, the file is corrupt and we want
		 * to ignore it.
		 */
		ungetc(PFB_DONE, m_PFFile);
		return EOF;
	}

	length = fgetc(m_PFFile) & 0xFF;
	length |= (fgetc(m_PFFile) & 0XFF) << 8;
	length |= (fgetc(m_PFFile) & 0XFF) << 16;
	length |= (fgetc(m_PFFile) & 0XFF) << 24;
	if (feof(m_PFFile))
	{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
		messageBoxOK(message);
		return EOF;
	}

	while (length > 0)
	{
		in = (length > BUFSIZ ? BUFSIZ : length);
		got = fread(inbuf, 1, in, m_PFFile);
		if (in != got)
		{
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font data file\n"
				   "[%s].\n"
				   "These errors were not fatal, but printing may be incorrect.",
				   m_fontfile);
			messageBoxOK(message);
			length = got;
		}
		if (type == PFB_ASCII)
			m_buffer.append(inbuf, got);
		else // type == PFB_BINARY
		{
			hexp = hexbuf;
			for (inp = inbuf; inp - inbuf < got; inp += 1)
			{
				  *hexp++ = hex[(*inp >> 4) & 0xf];
				  *hexp++ = hex[*inp & 0xf];
			}
			m_buffer.append(hexbuf, 2 * got);
		}
		length -= got ;
	}

	m_bufpos = 1;
	return m_buffer.getPointer(0)[0];
}

const char * XAP_UnixFont::getFontKey(void)
{
	ASSERT_MEMBERS;
	return m_fontKey;
}
/*!
 * returns true if the requested pixelsize is in the cache.
\params pixelsize: This of the font.
\returns true if found
*/
bool XAP_UnixFont::isSizeInCache(UT_uint32 pixelsize)
{
	UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	allocFont * entry;
	while (l < count)
	{
		entry = (allocFont *) m_allocFonts.getNthItem(l);
		if (entry && entry->pixelSize == pixelsize)
			return true;
		else
			l++;
	}
    return false;
}

GdkFont * XAP_UnixFont::getGdkFont(UT_uint32 pixelsize)
{
	// this might return NULL, but that means a font at a certain
	// size couldn't be found
    UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	xxx_UT_DEBUGMSG(("There are %d allocated fonts for %s \n",count,m_name));
	allocFont * entry;
        char buf[1000];

 	bool bFontNotFound = false;
		
	while (l < count)
	{
		entry = (allocFont *) m_allocFonts.getNthItem(l);
		if (entry && entry->pixelSize == pixelsize)
			return entry->gdkFont;
		else
			l++;
	}


	GdkFont * gdkfont = NULL;
	
	// If the font is really, really small (an EXTREMELY low Zoom can trigger this) some
	// fonts will be calculated to 0 height.  Bump it up to 2 since the user obviously
	// doesn't care about readability anyway.  :)
	if (pixelsize < 2)
		pixelsize = 2;

	// create a real object around that string
	XAP_UnixFontXLFD myXLFD(m_xlfd);

	// Must set a pixel size, or a point size, but we're getting
	// automunged pixel sizes appropriate for our resolution from
	// the layout engine, so use pixel sizes.  They're not really
	// much more accurate this way, but they're more consistent
	// with how the layout engine wants fonts.
	myXLFD.setPixelSize(pixelsize);

	// TODO  add any other special requests, like for a specific encoding
	// TODO  or registry, or resolution here

	char * newxlfd = myXLFD.getXLFD();
	char* requested_lfd = newxlfd;

	if(!is_CJK_font())
	{
		UT_DEBUGMSG(("Loading gdkfont [%s]\n", newxlfd));
		gdkfont = gdk_font_load(newxlfd);

	 	if (!gdkfont)
		{
			free(newxlfd);
			newxlfd = myXLFD.getFallbackXLFD();
			requested_lfd  = newxlfd;
			gdkfont = gdk_font_load(newxlfd);
			UT_ASSERT(0);
   		}
   		
	}
	else
	{
		char* noncjkXLFD;
  		{
			int s;
			switch(m_style)
			{
				case XAP_UnixFont::STYLE_NORMAL:
					s=0;
					break;
				case XAP_UnixFont::STYLE_BOLD:
					s=1;
					break;
				case XAP_UnixFont::STYLE_ITALIC:
					s=2;
					break;
				case XAP_UnixFont::STYLE_BOLD_ITALIC:
					s=3;
					break;
				default:
				s=0;
			}
			XAP_UnixFont *pMatchUnixFont= s_defaultNonCJKFont[s];
			XAP_UnixFontXLFD non_cjk_lfd(pMatchUnixFont->m_xlfd);
			non_cjk_lfd.setPixelSize(pixelsize);
			noncjkXLFD = non_cjk_lfd.getXLFD();
		}
		sprintf(buf,"%s,%s",newxlfd,noncjkXLFD);
		free(noncjkXLFD);
		gdkfont = gdk_fontset_load(buf);
		requested_lfd = buf;
	}

	if (!gdkfont)
	{
		UT_String message("AbiWord could not load the following font or "
						  "fontset from the X Window System display server:\n[");
		message += requested_lfd;
		message += "]\n\n"
			"This error could be the result of an incomplete AbiSuite installation,\n"
			"an incompatibility with your X Window System display server,\n"
			"or a problem communicating with a remote font server.\n"
			"\n"
			"Often this error is the result of invoking AbiWord directly instead of through\n"
			"its wrapper shell script.  The script dynamically adds the AbiSuite font directory\n"
			"to your X Window System display server font path before running the executable.\n"
			"\n"
			"IMPORTANT: If you're a XFree86 4.x user, try adding:\n"
			"Load  \"type1\"\n"
			"to the \"Module\" section of the XF86Config-4 file (usually located at /etc/X11)\n"
			"\n"
			"Please visit http://www.abisource.com/ for more information.";
		messageBoxOK(message.c_str());
		
		// we do not have to quit here, just continue ...
#if 0		
		exit(1);
#else
		bFontNotFound = true;
#endif
	}

	free(newxlfd);
	
	if(bFontNotFound)
		return NULL;
		
	allocFont * item = new allocFont;
	item->pixelSize = pixelsize;
	item->gdkFont = gdkfont;
	xxx_UT_DEBUGMSG(("SEVIOR: Allocated font of pixel size %d \n",pixelsize));
	m_allocFonts.addItem((void *) item);

	return gdkfont;
}

void XAP_UnixFont::_makeFontKey()
{
	ASSERT_MEMBERS;

	// if we already have a key, free it
	FREEP(m_fontKey);
	
	// allocate enough to combine name, seperator, style, and NULL into key.
	// this won't work if we have styles that require two digits in decimal.
	char * key = (char *) calloc(strlen(m_name) + 1 + 1 + 1, sizeof(char));
	UT_ASSERT(key);

	char * copy;
	UT_cloneString(copy, m_name);
	UT_upperString(copy);
	
	sprintf(key, "%s@%d", copy, m_style);

	FREEP(copy);
	
	// point member our way
	m_fontKey = key;
}

XAP_UnixFont *XAP_UnixFont::s_defaultNonCJKFont[4];
XAP_UnixFont *XAP_UnixFont::s_defaultCJKFont[4];

GdkFont * XAP_UnixFont::getMatchGdkFont(UT_uint32 size)
{
  if (!XAP_EncodingManager::get_instance()->cjk_locale())
      return getGdkFont(size);
  int s;
  switch(m_style)
	{
	case XAP_UnixFont::STYLE_NORMAL:
	  s=0;
	  break;
	case XAP_UnixFont::STYLE_BOLD:
	  s=1;
	  break;
	case XAP_UnixFont::STYLE_ITALIC:
	  s=2;
	  break;
	case XAP_UnixFont::STYLE_BOLD_ITALIC:
	  s=3;
	  break;
	default:
	  s=0;
	}
  XAP_UnixFont *pMatchUnixFont=is_CJK_font()? s_defaultNonCJKFont[s]: s_defaultCJKFont[s];
  return pMatchUnixFont ? pMatchUnixFont->getGdkFont(size) : getGdkFont(size);
}


XAP_UnixFont * XAP_UnixFont::getMatchUnixFont(void)
{
  if (!XAP_EncodingManager::get_instance()->cjk_locale())
      return this;
  int s;
  switch(m_style)
  {
      case XAP_UnixFont::STYLE_NORMAL:
		  s=0;
		  break;
      case XAP_UnixFont::STYLE_BOLD:
		  s=1;
		  break;
	  case XAP_UnixFont::STYLE_ITALIC:
		  s=2;
		  break;
	  case XAP_UnixFont::STYLE_BOLD_ITALIC:
		  s=3;
		  break;
	  default:
		  s=0;
  }
  XAP_UnixFont *pMatchUnixFont= NULL;
  if(is_CJK_font())
  {
	  pMatchUnixFont = s_defaultNonCJKFont[s];
  }
  else
  {
	  pMatchUnixFont = s_defaultCJKFont[s];
  }
  if(pMatchUnixFont != NULL)
  {
	  return pMatchUnixFont;
  }
  else
  {
	  return this;
  }
}
