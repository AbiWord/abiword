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
#include "xap_EncodingManager.h"
#include "xap_App.h"
#include "ttftool.h"
#include "xap_UnixPSGenerate.h"
#include "xap_UnixFontManager.h"
#include "gr_UnixGraphics.h"

/* Xft face locker impl. */
XftFaceLocker::XftFaceLocker(XftFont * pFont):m_pFont(pFont)
{
	m_pFace = pFont ? XftLockFace(pFont) : NULL;
}

XftFaceLocker::~XftFaceLocker()
{
	if (m_pFont)
		XftUnlockFace(m_pFont);
}

XftFaceLocker::XftFaceLocker(const XftFaceLocker & other)
{
	m_pFont = other.m_pFont;
	m_pFace = m_pFont ? XftLockFace(m_pFont) : NULL;
}

XftFaceLocker & XftFaceLocker::operator=(const XftFaceLocker & other)
{
	if (&other != this)
	  {
		  if (m_pFont)
			  XftUnlockFace(m_pFont);

		  m_pFont = other.m_pFont;
		  m_pFace = XftLockFace(m_pFont);
	  }

	return *this;
}

/*******************************************************************/

XAP_UnixFontHandle::XAP_UnixFontHandle():m_font(NULL), m_size(0)
{
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFont * font, UT_uint32 size):m_font(font),
																			m_size (size)
{
	m_hashKey = m_font->getName();
}

XftFont *XAP_UnixFontHandle::getLayoutXftFont(void)
{
	return m_font ? m_font->getLayoutXftFont(m_size) : NULL;
}

XftFont *XAP_UnixFontHandle::getDeviceXftFont(UT_uint32 zoomPercentage)
{
	return m_font ? m_font->getDeviceXftFont(m_size, zoomPercentage) : NULL;
}

UT_sint32 XAP_UnixFontHandle::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
	UT_sint32 width;
	XftFaceLocker locker(m_font->getLayoutXftFont(GR_CharWidthsCache::CACHE_FONT_SIZE));
	FT_Face pFace = locker.getFace();

	FT_UInt glyph_index = FT_Get_Char_Index(pFace, cChar);
	FT_Error error =
		FT_Load_Glyph(pFace, glyph_index,
					FT_LOAD_LINEAR_DESIGN |
					FT_LOAD_IGNORE_TRANSFORM |
					FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
	if (error) {
		return 0;
	}

	width = pFace->glyph->linearHoriAdvance;
	return width;
}

/*******************************************************************/

XAP_UnixFont::XAP_UnixFont(XAP_UnixFontManager * pFM)
	: m_fontKey(NULL), 
	  m_name(NULL), 
	  m_style(STYLE_LAST),
	  m_xlfd(NULL), 
	  m_uniWidths(NULL),
	  m_fontfile(NULL), 
	  m_metricfile(NULL), 
	  m_PFFile(NULL),
	  m_PFB(false), 
	  m_bufpos(0), 
	  m_pEncodingTable(NULL),
	  m_iEncodingTableSize(0), 
	  m_fontType(FONT_TYPE_UNKNOWN),
	  m_bisCopy(false), 
	  m_pFontManager(pFM),
	  m_pXftFont(0)
{
}

XAP_UnixFont::XAP_UnixFont(const XAP_UnixFont & copy)
{
	//UT_DEBUGMSG(("XAP_UnixFont:: copy constructor\n"));

	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;

	m_fontfile = NULL;
	m_metricfile = NULL;
	m_uniWidths = NULL;

	m_PFFile = NULL;

	m_fontKey = NULL;

	openFileAs(copy.getFontfile(), copy.getMetricfile(), copy.getName(), copy.getXLFD(), copy.getStyle());

	m_pEncodingTable = NULL;
	m_iEncodingTableSize = 0;
	if (copy.getEncodingTable())
		loadEncodingFile();
	m_bisCopy = true;

	m_pFontManager = copy.m_pFontManager;

	/* copy the fonts in our cache */
	const UT_Vector & copyAllocFonts = copy.m_allocFonts;
	for (UT_uint32 i = 0; i < m_allocFonts.getItemCount(); ++i)
	  {
		  allocFont *p = static_cast<allocFont *>(copyAllocFonts.getNthItem(i));
		  insertFontInCache(p->pixelSize, XftFontCopy(GDK_DISPLAY(), p->xftFont));
	  }
}

XAP_UnixFont::~XAP_UnixFont(void)
{
	xxx_UT_DEBUGMSG(("deleting font %x \n",this));
	FREEP(m_name);
	FREEP(m_fontfile);
	FREEP(m_metricfile);
	FREEP(m_xlfd);
	FREEP(m_fontKey);

	if (m_pFontManager)
		m_pFontManager->unregisterFont(this);

	//      UT_VECTOR_PURGEALL(allocFont *, m_allocFonts);
	for (UT_uint32 i = 0; i < m_allocFonts.getItemCount(); ++i)
	  {
		  allocFont *p = static_cast<allocFont *>(m_allocFonts.getNthItem(i));
		  XftFontClose(GDK_DISPLAY(), p->xftFont);
		  delete p;
	  }
	if (m_uniWidths)
	  {
		  delete[]m_uniWidths;
		  m_uniWidths = 0;
	  }

	_deleteEncodingTable();
}

bool XAP_UnixFont::openFileAs(const char *fontfile, const char *metricfile, const char* family, const char *xlfd,
							  XAP_UnixFont::style s)
{
	if (!fontfile || !metricfile || !xlfd)
		return false;

	FREEP(m_name);
	UT_cloneString(m_name, static_cast<const char *>(family));

	// save to memebers
	FREEP(m_fontfile);
	UT_cloneString(m_fontfile, fontfile);
	FREEP(m_metricfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;
	setXLFD(xlfd);

	// update our key so we can be identified
	_makeFontKey();
	size_t stFontFile = strlen(m_fontfile);

	m_fontType = FONT_TYPE_UNKNOWN;

	if (stFontFile > 4)
	{
		if (!UT_stricmp(m_fontfile + stFontFile - 4, ".ttf") || 
			!UT_stricmp(m_fontfile + stFontFile - 5, ".font"))
			m_fontType = FONT_TYPE_TTF;
		else if (!UT_stricmp(m_fontfile + stFontFile - 4, ".pfa"))
			m_fontType = FONT_TYPE_PFA;
		else if (!UT_stricmp(m_fontfile + stFontFile - 4, ".pfb"))
			m_fontType = FONT_TYPE_PFB;
	}

	if (m_fontType == FONT_TYPE_UNKNOWN)
		return false;

	return true;
}

void XAP_UnixFont::setName(const char *name)
{
	FREEP(m_name);
	UT_cloneString(m_name, name);
}

const char *XAP_UnixFont::getName(void) const
{
	return m_name;
}

void XAP_UnixFont::setStyle(XAP_UnixFont::style s)
{
	m_style = s;
}

XAP_UnixFont::style XAP_UnixFont::getStyle(void) const
{
	return m_style;
}

const char *XAP_UnixFont::getFontfile(void) const
{
	return m_fontfile;
}

const char *XAP_UnixFont::getMetricfile(void) const
{
	return m_metricfile;
}

void XAP_UnixFont::setXLFD(const char *xlfd)
{
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);
}

const char *XAP_UnixFont::getXLFD(void) const
{
	return m_xlfd;
}

/**
 * Calculates the width of a character in original font units
 */
UT_uint16 XAP_UnixFont::getCharWidth(UT_UCSChar c) const
{
	/* don't care about the pixel size */
	UT_sint32 width = static_cast<UT_sint32>(m_cw.getWidth(c));
	if (width == GR_CW_UNKNOWN || width == GR_UNKNOWN_BYTE)
	  {
		  XftFaceLocker locker(getLayoutXftFont(12));
		  FT_Face pFace = locker.getFace();

		  FT_UInt glyph_index = FT_Get_Char_Index(pFace, c);
		  FT_Error error =
			  FT_Load_Glyph(pFace, glyph_index,
					FT_LOAD_LINEAR_DESIGN |
					FT_LOAD_IGNORE_TRANSFORM |
					FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
		  if (error)
			  return 0;

		  width = pFace->glyph->linearHoriAdvance;
		  m_cw.setWidth(c, width);
	  }
	return static_cast<UT_uint16>(width);
}

void XAP_UnixFont::_deleteEncodingTable()
{
	if (m_pEncodingTable)
	  {
		  for (UT_uint32 i = 0; i < m_iEncodingTableSize; i++)
			  FREEP(m_pEncodingTable[i].adb);
		  delete[]m_pEncodingTable;
		  m_pEncodingTable = 0;
		  m_iEncodingTableSize = 0;
	  }
}


static int s_compare(const void *a, const void *b)
{
	const encoding_pair *pair1 = static_cast<const encoding_pair *>(a);
	const encoding_pair *pair2 = static_cast<const encoding_pair *>(b);

	return UT_strcmp(pair1->adb, pair2->adb);
}

const encoding_pair *XAP_UnixFont::loadEncodingFile()
{
	if (!m_pXftFont)
		return 0;

	if (m_pEncodingTable)
		return m_pEncodingTable;

	XftFaceLocker locker(m_pXftFont);
	FT_Face face = locker.getFace();

	// it should be "if (!FT_Has_PS_Glyph_Names(face))", but I don't have this function in my freetype version...
	if (!FT_HAS_GLYPH_NAMES(face))
		return 0;

	UT_Vector coverage;
	getCoverage(coverage);

	size_t nb_glyphs = 0;
	for (size_t i = 0; i < coverage.size(); i += 2)
		nb_glyphs += reinterpret_cast<size_t>(coverage[i + 1]);

	m_pEncodingTable = new encoding_pair[nb_glyphs];

	size_t idx = 0;
	for (size_t i = 0; i < coverage.size(); i += 2)
	  {
		  UT_UCSChar c1 = static_cast<UT_UCSChar>(reinterpret_cast<UT_uint32>(coverage[i]));
		  UT_UCSChar c2 =
			  static_cast<UT_UCSChar>(static_cast<UT_uint32>(c1)) +
			  static_cast<UT_UCSChar>(reinterpret_cast<UT_uint32>(coverage[i + 1]));
		  for (UT_UCSChar c = c1; c < c2; ++c)
		    {
			    FT_UInt glyph_idx = FT_Get_Char_Index(face, c);
			    char *glyph_name = static_cast<char *>(malloc(256));

			    FT_Get_Glyph_Name(face, glyph_idx, glyph_name,
					      256);
			    m_pEncodingTable[idx].ucs = c;
			    m_pEncodingTable[idx].adb = glyph_name;
			    ++idx;
		    }
	  }

	qsort(m_pEncodingTable, nb_glyphs, sizeof(encoding_pair), s_compare);
	m_iEncodingTableSize = nb_glyphs;

	return m_pEncodingTable;
}

bool XAP_UnixFont::_createPsSupportFiles()
{
	if (!is_PS_font())
		return false;

	char fontfile[100];
	char pfa2util[100];
	char cmdline[400];
	char buff[256];

	strcpy(pfa2util, XAP_App::getApp()->getAbiSuiteLibDir());
	strcat(pfa2util, "/bin/pfa2afm");

	strcpy(fontfile, m_fontfile);
	char *dot = strrchr(fontfile, '.');
	*(dot + 1) = 0;

	/*      now open a pipe to the pfa2afm program, and examine the output for
	   presence of error messages
	 */
	sprintf(cmdline, "%s -p %s -a %safm", pfa2util, m_fontfile, fontfile);

	UT_DEBUGMSG(("XAP_UnixFont::_createPsSupportFiles: running pfa2afm\n\t%s", cmdline));
	FILE *p = popen(cmdline, "r");
	if (!p)
	  {
		  UT_DEBUGMSG(("XAP_UnixFont::_createPsSupportFiles: unable to run pfa2afm\n"));
		  return false;
	  }

	while (!feof(p))
	  {
		  fgets(buff, 256, p);
		  if (strstr(buff, "Error"))
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
	if (!is_TTF_font())
		return false;
	char fontfile[100];
	char buff[256];
	const char *enc;
	const char *libdir = XAP_App::getApp()->getAbiSuiteLibDir();
	const char latin1[] = "ISO-8859-1";

	if (XAP_EncodingManager::get_instance()->isUnicodeLocale())
		enc = latin1;
	else
		enc = XAP_EncodingManager::get_instance()->
			getNativeEncodingName();

	char *dot = strrchr(m_fontfile, '.');
	strcpy(fontfile, m_fontfile);
	fontfile[dot - m_fontfile + 1] = 0;

	char *cmdline =
		new char[strlen(libdir) + 4 * strlen(fontfile) + strlen(enc) +
			 50];
	/*      now open a pipe to the ttftool program, and examine the output for
	   presence of error messages
	 */
	sprintf(cmdline,
		"%s/bin/ttftool -f %sttf -a %safm -u %su2g -p %st42 -e %s",
		libdir, fontfile, fontfile, fontfile, fontfile, enc);

	UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: \n\t%s",
		     cmdline));
	FILE *p = popen(cmdline, "r");
	if (!p)
	  {
		  UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: unable to run ttftool\n"));
		  return false;
	  }

	while (!feof(p))
	  {
		  fgets(buff, 256, p);
		  if (strstr(buff, "Error"))
		    {
			    UT_DEBUGMSG(("XAP_UnixFont::_createTtfSupportFiles: ttftool error:\n%s\n", buff));
			    return false;
		    }
	  }
	pclose(p);

	delete[]cmdline;
	return true;
}

bool XAP_UnixFont::embedInto(ps_Generate & ps)
{
	if (openPFA())
	  {
		  signed char ch = 0;

		  while ((ch = getPFAChar()) != EOF)
			  if (!ps.writeBytes(reinterpret_cast<UT_Byte *>(&ch), 1))
			    {
				    closePFA();
				    return false;
			    }

		  closePFA();
	  }
	else
	  {
		  // PFA file doesn't exists.  Maybe a TrueType font not converted to type 42.
		  // We generate the type 42 in the fly, and we embed it in the postscript file.
		  // TODO: Check for errors, and return false on error
		  create_type42(m_fontfile, ps.getFileHandle());
	  }

	return true;
}

bool XAP_UnixFont::openPFA(void)
{
	int peeked;
	const char *extension = NULL;
	size_t sFontfile;

	UT_ASSERT(m_fontfile);
	sFontfile = strlen(m_fontfile);
	if (sFontfile > 3)
		extension = m_fontfile + sFontfile - 3;

	UT_DEBUGMSG(("UnixFont::openPFA: opening file %s\n", m_fontfile));
	if (extension &&
	    (UT_stricmp(extension, "pfa") == 0 ||
	     UT_stricmp(extension, "pfb") == 0))
		m_PFFile = fopen(m_fontfile, "r");
	else
		m_PFFile = 0;

	if (!m_PFFile)
		return false;

	/* Check to see if it's a binary or ascii PF file */
	peeked = fgetc(m_PFFile);
	ungetc(peeked, m_PFFile);
	m_PFB = peeked == PFB_MARKER;
	m_bufpos = 0;

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
	int type, in, got, length;
	static char *hex = "0123456789abcdef";

	if (m_buffer.getLength() > m_bufpos)
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
		  else		// type == PFB_BINARY
		    {
			    hexp = hexbuf;
			    for (inp = inbuf; inp - inbuf < got; inp += 1)
			      {
				      *hexp++ = hex[(*inp >> 4) & 0xf];
				      *hexp++ = hex[*inp & 0xf];
			      }
			    m_buffer.append(hexbuf, 2 * got);
		    }
		  length -= got;
	  }

	m_bufpos = 1;
	return m_buffer.getPointer(0)[0];
}

const char *XAP_UnixFont::getFontKey(void)
{
  	return m_fontKey;
}

XftFont *XAP_UnixFont::getLayoutXftFont(UT_uint32 pixelsize) const
{
	XftFont *pXftFont = getFontFromCache(pixelsize, true, 100);
	if (pXftFont)
		return pXftFont;

	// ideally we'd get the layout resolution font back here, but we don't.
	fetchXftFont(pixelsize);
	return getFontFromCache(pixelsize, true, 100);
}

// returns different fonts depending on the zoom percentage
XftFont *XAP_UnixFont::getDeviceXftFont(UT_uint32 pixelsize, UT_uint32 zoomPercentage) const
{
	XftFont *pXftFont = getFontFromCache(pixelsize*zoomPercentage/100, false, zoomPercentage);
	if (pXftFont)
		return pXftFont;

	fetchXftFont(pixelsize*zoomPercentage/100);
	return getFontFromCache(pixelsize*zoomPercentage/100, false, zoomPercentage);
}

XftFont *XAP_UnixFont::getFontFromCache(UT_uint32 pixelsize, bool /*bIsLayout*/, UT_uint32 /*zoomPercentage*/) const
{
	UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	allocFont *entry;

	while (l < count)
	  {
		  entry = static_cast<allocFont *>(m_allocFonts.getNthItem(l));
		  if (entry && entry->pixelSize == pixelsize)
			  return entry->xftFont;
		  l++;
	  }

	return NULL;
}

void XAP_UnixFont::insertFontInCache(UT_uint32 pixelsize, XftFont * pXftFont) const
{
	allocFont *entry = new allocFont;
	entry->pixelSize = pixelsize;
	entry->xftFont = pXftFont;

	m_pXftFont = pXftFont;
	m_allocFonts.push_back(static_cast<void *>(entry));
}

/*! Loads pixelsize into cache - both layout & device versions. */
void XAP_UnixFont::fetchXftFont(UT_uint32 pixelsize) const
{
	XftFont * pXftFont;

	FcPattern *fp = XftNameParse(m_xlfd);
	FcResult result;
	FcPattern *layout_fp = XftFontMatch(GDK_DISPLAY(),
										DefaultScreen(GDK_DISPLAY()), fp,
										&result);
	FcPatternDestroy(fp);
	// just in case they already exist...
	FcPatternDel(layout_fp, FC_PIXEL_SIZE);
	FcPatternDel(layout_fp, FC_DPI);
	FcPatternAddInteger(layout_fp, FC_PIXEL_SIZE, pixelsize);

// This doesn't seem to actually do anything.  Boo!
	FcPatternAddDouble(layout_fp, FC_DPI, 72.0);
	pXftFont = XftFontOpenPattern(GDK_DISPLAY(), layout_fp);

	// That means that we should should be 100% sure that,
	// at this point, the font exists in the system
	UT_ASSERT(pXftFont);
	
	insertFontInCache(pixelsize, pXftFont);
}

void XAP_UnixFont::_makeFontKey()
{
	// if we already have a key, free it
	FREEP(m_fontKey);

	char *copy;
	UT_cloneString(copy, m_name);
	UT_upperString(copy);
	
	UT_String key = UT_String_sprintf("%s@%d", copy, m_style);
	FREEP(copy);

	// point member our way
	UT_cloneString(m_fontKey, key.c_str());
}

void XAP_UnixFont::getCoverage(UT_Vector & coverage)
{
	FcChar32 coverage_map[FC_CHARSET_MAP_SIZE];
	FcChar32 next;
	const FcChar32 invalid = (FcChar32) - 1;
	FcChar32 base_range = invalid;
	coverage.clear();

	int i;

	for (FcChar32 ucs4 =
	     FcCharSetFirstPage(m_pXftFont->charset, coverage_map, &next);
	     ucs4 != FC_CHARSET_DONE;
	     ucs4 =
	     FcCharSetNextPage(m_pXftFont->charset, coverage_map, &next))
	  {
		  for (i = 0; i < FC_CHARSET_MAP_SIZE; i++)
		    {
			    FcChar32 bits = coverage_map[i];
			    FcChar32 base = ucs4 + (i << 5);
			    int b = 0;

			    if (base_range != invalid && bits == 0xFFFFFFFF)
				    continue;

			    while (bits)
			      {
				      if (bits & 1)
					{
						if (base_range == invalid)
							base_range = base + b;
					}
				      else
					{
						if (base_range != invalid)
						  {
							  coverage.
								  push_back(reinterpret_cast<void *>(base_range));
							  coverage.
								  push_back(reinterpret_cast<void *>(base + b - base_range));
							  base_range =
								  invalid;
						  }
					}

				      bits >>= 1;
				      b++;
			      }

			    if (b < 32 && base_range != invalid)
			      {
				      coverage.push_back(reinterpret_cast<void *>(base_range));
				      coverage.
					      push_back(reinterpret_cast<void *>(base + b -
								 base_range));
				      base_range = invalid;
			      }
		    }
	  }
}

inline static float fontPoints2float(UT_uint32 iSize, FT_Face pFace,
				     UT_uint32 iFontPoints)
{
	return iFontPoints * iSize * 1.0 / pFace->units_per_EM;
}

float XAP_UnixFont::getAscender(UT_uint32 iSize) const
{
	// in fact, we don't care about the pixelsize of the font, as we're going to use font units here
	XftFaceLocker locker(getLayoutXftFont(12));
	FT_Face pFace = locker.getFace();
	FT_Short ascender = pFace->ascender;
	float fAscender = fontPoints2float(iSize, pFace, ascender);
	xxx_UT_DEBUGMSG(("XAP_UnixFont::getAscender(%u) -> %f\n", iSize,
			 fAscender));

	return fAscender;
}

float XAP_UnixFont::getDescender(UT_uint32 iSize) const
{
	// in fact, we don't care about the pixelsize of the font, as we're going to use font units here
	XftFaceLocker locker(getLayoutXftFont(12));
	FT_Face pFace = locker.getFace();
	FT_Short descender = -pFace->descender;
	float fDescender = fontPoints2float(iSize, pFace, descender);
	xxx_UT_DEBUGMSG(("XAP_UnixFont::getDescender(%u) -> %f\n", iSize,
			 fDescender));

	return fDescender;
}

float XAP_UnixFont::measureUnRemappedChar(const UT_UCSChar c, UT_uint32 iSize) const
{
	// in fact, we don't care about the pixelsize of the font, as we're going to use font units here
	XftFaceLocker locker(getLayoutXftFont(12));
	float width =
		fontPoints2float(iSize, locker.getFace(), getCharWidth(c));
	xxx_UT_DEBUGMSG(("XAP_UnixFont::measureUnRemappedChar(%c, %u) -> %f\n", static_cast<char>(c), iSize, width));
	return width;
}

UT_String XAP_UnixFont::getPostscriptName() const
{
	// in fact, we don't care about the pixelsize of the font, as we're going to extract the postscript name
	XftFaceLocker locker(getLayoutXftFont(12));
	const char *pszName = FT_Get_Postscript_Name(locker.getFace());
	return (pszName == NULL ? "" : pszName);
}

void XAP_UnixFont::setFontManager(XAP_UnixFontManager * pFm)
{
	m_pFontManager = pFm;
}
