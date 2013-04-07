/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include "gr_Win32USPGraphics.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ut_Win32OS.h"

extern "C"  UT_uint16    wvLangToLIDConverter(const char * lang);
extern "C"  const char * wvLIDToLangConverter(UT_uint16);

HINSTANCE GR_Win32USPGraphics::s_hUniscribe = NULL;
UT_uint32 GR_Win32USPGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_Win32USPGraphics::s_Version;
const SCRIPT_PROPERTIES ** GR_Win32USPGraphics::s_ppScriptProperties = NULL;
int GR_Win32USPGraphics::s_iMaxScript = 0;
UT_UTF8String GR_Win32USPGraphics::s_sDescription;
UT_UTF8String GR_Win32USPGraphics::s_sUSPVersion;


tScriptItemize       GR_Win32USPGraphics::fScriptItemize       = NULL;
tScriptShape         GR_Win32USPGraphics::fScriptShape         = NULL;
tScriptFreeCache     GR_Win32USPGraphics::fScriptFreeCache     = NULL;
tScriptStringOut     GR_Win32USPGraphics::fScriptStringOut     = NULL;
tScriptStringAnalyse GR_Win32USPGraphics::fScriptStringAnalyse = NULL;
tScriptStringFree    GR_Win32USPGraphics::fScriptStringFree    = NULL;
tScriptTextOut       GR_Win32USPGraphics::fScriptTextOut       = NULL;
tScriptPlace         GR_Win32USPGraphics::fScriptPlace         = NULL;
tScriptJustify       GR_Win32USPGraphics::fScriptJustify       = NULL;
tScriptCPtoX         GR_Win32USPGraphics::fScriptCPtoX         = NULL;
tScriptXtoCP         GR_Win32USPGraphics::fScriptXtoCP         = NULL;
tScriptBreak         GR_Win32USPGraphics::fScriptBreak         = NULL;
tScriptIsComplex     GR_Win32USPGraphics::fScriptIsComplex     = NULL;
tScriptGetProperties GR_Win32USPGraphics::fScriptGetProperties = NULL;
tScriptRecordDigitSubstitution GR_Win32USPGraphics::fScriptRecordDigitSubstitution = NULL;

// some macros to ease logging of critical exceptions
// all of these macros log the file name and line where the exception occured, plus some
// additional information

// log single string
#define LOG_USP_EXCPT(msg)                                                      \
{                                                                               \
    UT_String __s;                                                              \
    UT_String_sprintf(__s, "%s (%d): %s",__FILE__, __LINE__, msg);              \
	UT_DEBUGMSG(("%s",__s.c_str()));                                            \
    if(XAP_App::getApp()->getPrefs())                                           \
    {                                                                           \
	    XAP_App::getApp()->getPrefs()->log("USPGraphics", __s.c_str());         \
    }                                                                           \
}

// log string and a numerical parameter ("msg [0xpar]")
#define LOG_USP_EXCPT_X(msg, par)                                               \
{                                                                               \
    UT_String __s;                                                              \
    UT_String_sprintf(__s, "%s (%d): %s [0x%x]",__FILE__, __LINE__, msg, par);  \
	UT_DEBUGMSG(("%s",__s.c_str()));                                            \
    if(XAP_App::getApp()->getPrefs())                                           \
    {                                                                           \
	    XAP_App::getApp()->getPrefs()->log("USPGraphics", __s.c_str());         \
    }                                                                           \
}

// log message consisting of two strings ("msg [str]")
#define LOG_USP_EXCPT_S(msg, str)                                               \
{                                                                               \
    UT_String __s;                                                              \
    UT_String_sprintf(__s, "%s (%d): %s [%s]",__FILE__, __LINE__, msg, str);    \
	UT_DEBUGMSG(("%s",__s.c_str()));                                            \
    if(XAP_App::getApp()->getPrefs())                                           \
    {                                                                           \
	    XAP_App::getApp()->getPrefs()->log("USPGraphics", __s.c_str());         \
    }                                                                           \
}

// log string, with additional string and numerical parameters ("msg [str, 0xpar]")
#define LOG_USP_EXCPT_SX(msg, str, par)                                                  \
{                                                                                        \
    UT_String __s;                                                                       \
    UT_String_sprintf(__s, "%s (%d): %s [%s, 0x%x]",__FILE__, __LINE__, msg, str, par);  \
	UT_DEBUGMSG(("%s",__s.c_str()));                                                     \
    if(XAP_App::getApp()->getPrefs())                                                    \
    {                                                                                    \
	    XAP_App::getApp()->getPrefs()->log("USPGraphics", __s.c_str());                  \
    }                                                                                    \
}

#define GR_WIN32_USP_FONT_SCALING 20
class ABI_EXPORT GR_Win32USPItem: public GR_Item
{
	friend class GR_Win32USPGraphics;

  public:
	virtual ~GR_Win32USPItem(){};
	
	virtual GR_ScriptType getType() const {return (GR_ScriptType) m_si.a.eScript;}
	virtual GR_Item *     makeCopy() const {return new GR_Win32USPItem(m_si);} // make a copy of this item
	virtual GRRI_Type     getClassId() const {return GRRI_WIN32_UNISCRIBE;}

	bool isRTL() const {return m_si.a.fRTL != 0;}
	
  protected:
	GR_Win32USPItem(SCRIPT_ITEM si):m_si(si){};
	GR_Win32USPItem(GR_ScriptType t){ m_si.a.eScript = (WORD)t;};

	SCRIPT_ITEM m_si;
};

class ABI_EXPORT GR_Win32USPRenderInfo : public GR_RenderInfo
{
  public:
	GR_Win32USPRenderInfo(GR_ScriptType type):
		GR_RenderInfo(type),
	    m_pIndices(NULL),
	    m_pVisAttr(NULL),
		m_iIndicesSize(0),
		m_pClust(NULL),
		m_iClustSize(0),
		m_iIndicesCount(0),
		m_iCharCount(0),
		m_pGoffsets(NULL),
	    m_pAdvances(NULL),
		m_pJustify(NULL),
		m_iZoom(100),
		m_eJustification(SCRIPT_JUSTIFY_NONE),
		m_bRejustify(true),
		m_bShapingFailed(false),
		m_bNeedsReshaping(true),
		m_hdc(NULL)
	{
		s_iInstanceCount++;
		if(s_iInstanceCount == 1)
		{
			allocStaticBuffers(200);
		}
		
	};
	
	virtual ~GR_Win32USPRenderInfo()
	    {
			delete [] m_pIndices;  delete [] m_pVisAttr;
			delete [] m_pClust;    delete [] m_pGoffsets;
			delete [] m_pAdvances; delete [] m_pJustify;

			s_iInstanceCount--;
			if(!s_iInstanceCount)
			{
				delete [] s_pAdvances; s_pAdvances = NULL;
				delete [] s_pJustifiedAdvances; s_pJustifiedAdvances = NULL;
				delete [] s_pJustify;  s_pJustify  = NULL;
				delete [] s_pLogAttr;  s_pLogAttr  = NULL;
				delete [] s_pChars;    s_pChars    = NULL;
				delete [] s_pGoffsets; s_pGoffsets = NULL;
				s_iAdvancesSize = 0;

				s_pOwnerDraw = NULL;
				s_pOwnerCP   = NULL;
				s_pOwnerChar = NULL;
			}
		}

	virtual GRRI_Type getType() const {return GRRI_WIN32_UNISCRIBE;}
	virtual bool      append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool      split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool      cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool      isJustified() const;
	
	inline bool       allocStaticBuffers(UT_uint32 iSize)
	    {
			s_iAdvancesSize = 0;
			if(s_pAdvances) { delete [] s_pAdvances; s_pAdvances = NULL;}
			s_pAdvances = new int [iSize];

			if(s_pJustifiedAdvances) { delete [] s_pJustifiedAdvances; s_pJustifiedAdvances = NULL;}
			s_pJustifiedAdvances = new int [iSize];

			if(s_pJustify) { delete [] s_pJustify; s_pJustify = NULL; }
			s_pJustify  = new int [iSize];

			if(s_pLogAttr) { delete [] s_pLogAttr; s_pLogAttr = NULL; }
			s_pLogAttr  = new SCRIPT_LOGATTR[iSize]; // log attr. correspont to characters, not glyphs, but since there are
												   // always at least as many glyphs, this is OK
			if(s_pChars) { delete [] s_pChars; s_pChars = NULL; }
			s_pChars    = new WCHAR[iSize];

			if(s_pGoffsets) { delete [] s_pGoffsets; s_pGoffsets = NULL; }
			s_pGoffsets  = new GOFFSET[iSize];
			
			UT_return_val_if_fail(s_pAdvances && s_pJustifiedAdvances && s_pJustify &&
								  s_pLogAttr && s_pChars && s_pGoffsets, false);

			s_iAdvancesSize = iSize;
			
			s_pOwnerDraw = NULL;
			s_pOwnerCP = NULL;
			s_pOwnerChar = NULL;
			
			return true;
	    }

  public:

	WORD *           m_pIndices;
	SCRIPT_VISATTR * m_pVisAttr;
	UT_uint32        m_iIndicesSize;

	WORD *           m_pClust;
	UT_uint32        m_iClustSize;

	UT_uint32        m_iIndicesCount;
	UT_uint32        m_iCharCount;

	GOFFSET *        m_pGoffsets;  // according to the docs this should be a single structure, but it needs to be an array
	int *            m_pAdvances;
	int *            m_pJustify;
	ABC              m_ABC;
	UT_uint32        m_iZoom;

	SCRIPT_JUSTIFY   m_eJustification;

	bool             m_bRejustify;
	bool             m_bShapingFailed;
	bool             m_bNeedsReshaping;
	HDC              m_hdc;
	
	
	static int *     s_pAdvances;            // in device units, used for drawing
	static int *     s_pJustifiedAdvances;   // in logical units, used for mapping x to CP
	static UT_uint32 s_iInstanceCount;
	static UT_uint32 s_iAdvancesSize;
	static int *     s_pJustify;
	static SCRIPT_LOGATTR * s_pLogAttr;
	static WCHAR *   s_pChars;
	static GOFFSET * s_pGoffsets;
	
	// static buffer ownerhip: we use static buffers to store various positioning information; the
	// following members identify the instance that last modified these buffers
	
	static GR_RenderInfo * s_pOwnerDraw; // buffers used for drawing
	static GR_RenderInfo * s_pOwnerCP;   // buffers used for xy <--> CP translation
	static GR_RenderInfo * s_pOwnerChar; // buffers that store raw unicode text (used by
										 // _scriptBreak, etc.)
};

int *           GR_Win32USPRenderInfo::s_pAdvances          = NULL;
int *           GR_Win32USPRenderInfo::s_pJustifiedAdvances = NULL;
int *           GR_Win32USPRenderInfo::s_pJustify           = NULL;
UT_uint32       GR_Win32USPRenderInfo::s_iInstanceCount     = 0;
UT_uint32       GR_Win32USPRenderInfo::s_iAdvancesSize      = 0;
SCRIPT_LOGATTR *GR_Win32USPRenderInfo::s_pLogAttr           = NULL;
WCHAR *         GR_Win32USPRenderInfo::s_pChars             = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerDraw         = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerCP           = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerChar         = NULL;
GOFFSET *       GR_Win32USPRenderInfo::s_pGoffsets          = NULL;


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, HWND hwnd)
	:GR_Win32Graphics(hdc, hwnd),
	 m_bConstructorSucceeded(false)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		return;
	}

	m_bConstructorSucceeded = true;
}


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, const DOCINFOW * pDI, HGLOBAL hDevMode)
	:GR_Win32Graphics(hdc, pDI, hDevMode),
	 m_bConstructorSucceeded(false)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		return;
	}

	m_bConstructorSucceeded = true;
}

GR_Win32Font * GR_Win32USPGraphics::_newFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC)
{
	return GR_Win32USPFont::newFont(lf, fPoints, hdc, printHDC);
}

#define loadUSPFunction(name)                           \
f##name = (t##name)GetProcAddress(s_hUniscribe, #name); \
if(!f##name)                                            \
{                                                       \
	LOG_USP_EXCPT(#name)                                \
	return false;                                       \
}

#define logScript(iId)                                                                   \
{                                                                                        \
    {                                                                                    \
	    UT_String s;                                                                     \
		char _buff[100];                                                                 \
        GetLocaleInfoA(MAKELCID((WORD)s_ppScriptProperties[iId]->langid, SORT_DEFAULT),  \
				      LOCALE_SENGLANGUAGE,                                               \
					  _buff,                                                             \
					  100);                                                              \
		                                                                                 \
		s += _buff;                                                                      \
                                                                                         \
		GetLocaleInfoA(MAKELCID((WORD)s_ppScriptProperties[iId]->langid, SORT_DEFAULT),  \
				      LOCALE_SENGCOUNTRY,                                                \
					  _buff,                                                             \
					  100);                                                              \
                                                                                         \
		s += " (";                                                                       \
		s += _buff;                                                                      \
		s += ")";                                                                        \
		                                                                                 \
		UT_DEBUGMSG(("Uniscribe %s\n", s.c_str()));                                      \
	    if(XAP_App::getApp()->getPrefs())                                                \
	    {                                                                                \
		   XAP_App::getApp()->getPrefs()->log("gr_Win32USPGraphics", s.c_str());         \
	    }                                                                                \
    }                                                                                    \
}

bool GR_Win32USPGraphics::_constructorCommonCode()
{
	// try to load Uniscribe
	s_iInstanceCount++;
	
	if(s_iInstanceCount == 1)
	{
		s_sDescription = "Uniscribe-based graphics";
		s_Version.set(0,1,0,0);
		
		s_hUniscribe = LoadLibraryW(L"usp10.dll");

		if(!s_hUniscribe)
		{
			LOG_USP_EXCPT("could not load usp10.dll")
			return false;
		}
		
#if 1 //def DEBUG
		wchar_t FileName[250];
		if(GetModuleFileNameW(s_hUniscribe,&FileName[0],250))
		{
			DWORD dummy;
			DWORD iSize = GetFileVersionInfoSizeW(FileName,&dummy);

			if(iSize)
			{
				char * pBuff = (char*)g_try_malloc(iSize);
				if(pBuff && GetFileVersionInfoW(FileName, 0, iSize, pBuff))
				{
					LPVOID buff2;
					UINT   buff2size;
					
					if(VerQueryValueW(pBuff,L"\\",
									 &buff2,
									 &buff2size))
					{
						VS_FIXEDFILEINFO * pFix = (VS_FIXEDFILEINFO *) buff2;
						UT_uint32 iV1 = (pFix->dwFileVersionMS & 0xffff0000) >> 16;
						UT_uint32 iV2 = pFix->dwFileVersionMS & 0x0000ffff;
						UT_uint32 iV3 = (pFix->dwFileVersionLS & 0xffff0000) >> 16;
						UT_uint32 iV4 = pFix->dwFileVersionLS & 0x0000ffff;
							
						UT_DEBUGMSG(("GR_Win32USPGraphics: Uniscribe version %d.%d.%d.%d\n",
									 iV1, iV2, iV3, iV4));

						UT_String s;
						UT_String_sprintf(s, "usp10.dll: %d.%d.%d.%d", iV1, iV2, iV3, iV4);

						if(XAP_App::getApp()->getPrefs())
						{
							XAP_App::getApp()->getPrefs()->log("gr_Win32USPGraphics", s.c_str()); 
						}

						s_sUSPVersion = s.c_str();
					}
				}
				g_free(pBuff);
			}
		}
#endif

		// now we load the functions we need
		loadUSPFunction(ScriptItemize);
		loadUSPFunction(ScriptShape);
		loadUSPFunction(ScriptFreeCache);
		loadUSPFunction(ScriptStringOut);
		loadUSPFunction(ScriptStringAnalyse);
		loadUSPFunction(ScriptStringFree);
		loadUSPFunction(ScriptTextOut);
		loadUSPFunction(ScriptPlace);
		loadUSPFunction(ScriptJustify);
		loadUSPFunction(ScriptCPtoX);
		loadUSPFunction(ScriptXtoCP);
		loadUSPFunction(ScriptBreak);
		loadUSPFunction(ScriptIsComplex);
		loadUSPFunction(ScriptRecordDigitSubstitution);
		loadUSPFunction(ScriptGetProperties);
		
		HRESULT hRes = fScriptGetProperties(&s_ppScriptProperties, & s_iMaxScript);
		if(hRes)
		{
			LOG_USP_EXCPT_X("no script properties", hRes)
			return false;
		}
#ifdef DEBUG
		for(UT_sint32 i = 0; i < s_iMaxScript; ++i)
		{
			logScript(i);
		}
#endif
	}
	else // we are not the first instance, USP should be loaded
	{
		if(!s_hUniscribe)
		{
			LOG_USP_EXCPT("no USP HINST")
			return false;
		}
	}
	
	return true;
}
#undef loadUSPFunction

GR_Win32USPGraphics::~GR_Win32USPGraphics()
{
	// have to delete all fonts before we unload usp10.dll
	// ~GR_Win32Graphics() from deleting it
	delete m_pFontGUI;
	m_pFontGUI = NULL;

	_destroyFonts();
	
	s_iInstanceCount--;
	
	if(!s_iInstanceCount)
	{
		if(s_hUniscribe)
		{
			FreeLibrary(s_hUniscribe);
			s_hUniscribe = NULL;
		}
	}
}

const char *    GR_Win32USPGraphics::graphicsDescriptor()
{
	return s_sDescription.utf8_str();
}

const char *    GR_Win32USPGraphics::getUSPVersion()
{
	return s_sUSPVersion.utf8_str();
}

GR_Graphics *   GR_Win32USPGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_WIN32, NULL);
	
	GR_Win32AllocInfo &AI = (GR_Win32AllocInfo&)info;

	GR_Win32USPGraphics * pG = NULL;

	if(AI.m_pDocInfo)
	{
		// printer graphics required
		pG = new GR_Win32USPGraphics(AI.m_hdc, AI.m_pDocInfo, AI.m_hDevMode);
	}
	else
	{
		// screen graphics required
		pG = new GR_Win32USPGraphics(AI.m_hdc, AI.m_hwnd);
	}

	if(pG->m_bConstructorSucceeded)
		return pG;
	else
		return NULL;
}

UT_uint32 GR_Win32USPGraphics::getFontHeight(GR_Font * fnt)
{
	UT_return_val_if_fail( fnt, 0 );
	GR_Font * pOldFont = m_pFont;
	setFont(fnt);
	
	UT_uint32 i = getFontHeight();
	if(pOldFont)
		setFont(pOldFont);
	return i;
}

UT_uint32 GR_Win32USPGraphics::getFontHeight()
{
	UT_return_val_if_fail( m_pFont, 0 );

	if(getPrintDC() != m_pFont->getYHDC())
	{
		_setupFontOnDC((GR_Win32USPFont*)m_pFont, false);
	}
	
	return (UT_uint32)(m_pFont->getHeight(m_hdc, getPrintDC()));
}

UT_uint32 GR_Win32USPGraphics::getFontAscent(GR_Font* fnt)
{
	UT_return_val_if_fail( fnt, 0 );
	GR_Font * pOldFont = m_pFont;
	setFont(fnt);
	
	UT_uint32 i = getFontAscent();
	if(pOldFont)
		setFont(pOldFont);
	return i;
}

UT_uint32 GR_Win32USPGraphics::getFontAscent()
{
	UT_return_val_if_fail( m_pFont, 0 );

	if(getPrintDC() != m_pFont->getYHDC())
	{
		_setupFontOnDC((GR_Win32USPFont*)m_pFont, false);
	}
	
	return (UT_uint32)(m_pFont->getAscent(m_hdc, getPrintDC()));
}

UT_uint32 GR_Win32USPGraphics::getFontDescent(GR_Font* fnt)
{
	UT_return_val_if_fail( fnt, 0 );
	GR_Font * pOldFont = m_pFont;
	setFont(fnt);
	
	UT_uint32 i = getFontDescent();
	if(pOldFont)
		setFont(pOldFont);
	return i;
}

UT_uint32 GR_Win32USPGraphics::getFontDescent()
{
	UT_return_val_if_fail( m_pFont, 0 );

	if(getPrintDC() != m_pFont->getYHDC())
	{
		_setupFontOnDC((GR_Win32USPFont*)m_pFont, false);
	}
	
	return (UT_uint32)(m_pFont->getDescent(m_hdc, getPrintDC()));
}

/*!
    The USP library maintains an internal font cache, which allows it to avoid accessing
    the DC for things like measuring fonts, etc. This means that often it is not necessary
    to have the font selected into the DC, and as SelectObject() is very time consuming,
    we will not be calling it from here at all, instead, we will only note which font is
    being set and call SelectObject() from inside the functions that access the DC only if
    that is necessary.
*/
void GR_Win32USPGraphics::setFont(const GR_Font* pFont)
{
	m_pFont = const_cast<GR_Win32Font *>(static_cast<const GR_Win32Font *>(pFont));

	UT_ASSERT_HARMLESS(pFont);	// TODO should we allow pFont == NULL?

	if(pFont)
		m_iFontAllocNo = pFont->getAllocNumber();
	else
		m_iFontAllocNo = 0;
}


void GR_Win32USPGraphics::_setupFontOnDC(GR_Win32USPFont *pFont, bool bZoomMe)
{
	UT_return_if_fail( pFont );

	UT_uint32 zoom = 0;
	UT_uint32 pixels = 0;
	HDC hdc = m_hdc;

	if(pFont->isFontGUI() || m_bPrint)
	{
		zoom = 100;
		pixels = pFont->getUnscaledHeight();
	}
	else if (bZoomMe)
	{
		// this branch gets only executed for screen draw
		// Win32 GDI uses a bigger font size for screen operations than the user request;
		// this is a real pain as it makes doing WYSIWIG near impossible. One of the
		// consequences of this is that if we do layout using printer metrics (in order to
		// achieve true WYSIWIG), the letters, particularly, the descending parts, can be
		// drawn outwith the line rectangle (and so leave pixel dirt, etc.)
		// In order to avoid that, we actually request a slightly smaller font from the
		// system if the physical pixel size drops below certain (heuristically determined) value.
		zoom = getZoomPercentage();
		double dZoom = (double)zoom;
		double dSizeFactor = dZoom * (double)pFont->getUnscaledHeight()/(double)getDeviceResolution();
		// 10 pt font on 96dpi is 13px
		if(dSizeFactor <= 14.0)
		{
			// The numerical constant is heuristic
			// 2% decrease on a 96 dpi screen works out 1 pixel adjustment
			dZoom /= 1.02;
		}
		
		pixels = (UT_uint32)((double)pFont->getUnscaledHeight()* dZoom/100.0) ;
	}
	else if(getPrintDC())
	{
		// we are using the printer dc for measuring, so we do not do any scaling
		zoom = 100;
		pixels = (int)(pFont->getPointSize() * (double)m_nPrintLogPixelsY / 72.0 + 0.5);
		hdc = getPrintDC();
	}
	else
	{
		// neither GUI font, nor to be zoomed -- we will actually scale the font to a big
		// size, so that our measurements are no affected by hinting; the scaling factor
		// is fairly arbitrary and is set near the top of this file
		zoom = 100;
		pixels = pFont->getUnscaledHeight() * GR_WIN32_USP_FONT_SCALING;
	}
	
	HFONT hFont = pFont->getFontFromCache(pixels, false, zoom);
	if (!hFont)
	{
		pFont->fetchFont(pixels);
		hFont = pFont->getFontFromCache(pixels, false, zoom);
	}

	bool bAllocNoMismatch = false;
	if(hdc == m_hdc && pFont->getAllocNumber() != m_iDCFontAllocNo)
		bAllocNoMismatch = true;

	if(!bAllocNoMismatch && hdc == getPrintDC() && pFont->getAllocNumber() != m_iPrintDCFontAllocNo)
		bAllocNoMismatch = true;
	
	if(bAllocNoMismatch || (HFONT) GetCurrentObject(hdc, OBJ_FONT) != hFont)
	{
		if(NULL == SelectObject(hdc, hFont))
		{
			DWORD e = GetLastError();
			LPVOID lpMsgBuf;
 
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						  NULL,
						  e,
						  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						  (LPWSTR) &lpMsgBuf,
						  0,
						  NULL);

			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			LOGFONTW lf;
			if(GetObjectW(hFont, sizeof(LOGFONTW), &lf))
			{
				// this assumes that the lfFaceName is a char string; it could be wchar in
				// fact but it seems to work elsewhere in the win32 graphics class
				LOG_USP_EXCPT_SX((char*)lpMsgBuf, lf.lfFaceName, lf.lfHeight)
			}
			else
			{
				LOG_USP_EXCPT("Could not select font into DC")
			}

			LocalFree(lpMsgBuf );
		}

		// remember which font we loaded
		// NB: when printing both of these are true (m_hdc == printDC)
		if(hdc == m_hdc)
			m_iDCFontAllocNo = pFont->getAllocNumber();

		if(hdc == getPrintDC())
			m_iPrintDCFontAllocNo = pFont->getAllocNumber();
			
	}
}


#define GRWIN32USP_CHARBUFF_SIZE 100
#define GRWIN32USP_ITEMBUFF_SIZE 20
bool GR_Win32USPGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	static WCHAR wcInChars[GRWIN32USP_CHARBUFF_SIZE];
	static SCRIPT_ITEM Items[GRWIN32USP_ITEMBUFF_SIZE];

	WCHAR *pInChars = &wcInChars[0];
	SCRIPT_ITEM * pItems = &Items[0];
	bool bDeleteChars = false;
	bool bDeleteItems = false;

	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	UT_uint32 iPosStart = text.getPosition();
	UT_uint32 iPosEnd   = text.getUpperLimit();
	UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);

	UT_uint32 iLen = iPosEnd - iPosStart + 1; // including iPosEnd

	if(iLen > GRWIN32USP_CHARBUFF_SIZE)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::itemize: text buffer too small (iLen %d)\n", iLen));
		pInChars = new WCHAR[iLen];
		UT_return_val_if_fail(pInChars,false);
		bDeleteChars = true;
	}

	UT_uint32 i;
	for(i = 0; i < iLen; ++i, ++text)
	{
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
		pInChars[i] = (WCHAR)text.getChar();
	}
	
	int       iItemCount;
	UT_uint16 iLid = wvLangToLIDConverter(I.getLang());
	
	SCRIPT_STATE ss;
	ss.uBidiLevel = I.getEmbedingLevel();
	ss.fOverrideDirection = I.getDirOverride() == FRIBIDI_TYPE_UNSET ? 0 : 1;
	ss.fInhibitSymSwap = 0;
	ss.fCharShape = 1;
	ss.fDigitSubstitute = 1;
	ss.fInhibitLigate = 0;
	ss.fDisplayZWG = I.getShowControlChars();
	ss.fArabicNumContext = 0;
	ss.fGcpClusters = 0;
	ss.fReserved = 0;
	ss.fEngineReserved = 0;

	SCRIPT_CONTROL sc;
	sc.uDefaultLanguage = iLid; 
	sc.fContextDigits = 1; 
	sc.fInvertPreBoundDir = 0; 
	sc.fInvertPostBoundDir = 0; 
	sc.fLinkStringBefore = 0; 
	sc.fLinkStringAfter = 0; 
	sc.fNeutralOverride = 0; 
	sc.fNumericOverride = 0; 
	sc.fLegacyBidiClass = 0; 
	sc.fReserved = 0; 

		
	HRESULT hRes = fScriptItemize(pInChars, iLen, GRWIN32USP_ITEMBUFF_SIZE, &sc, &ss, pItems, &iItemCount);
	if(hRes)
	{
		UT_return_val_if_fail(hRes == E_OUTOFMEMORY, false);
		UT_uint32 iItemBuffSize = GRWIN32USP_ITEMBUFF_SIZE;
		UT_DEBUGMSG(("GR_Win32USPGraphics::itemize: item buffer too small (len %d)\n", iItemBuffSize));
		
		do
		{
			iItemBuffSize *= 2;
			
			if(bDeleteItems)
				delete [] pItems;
			
			pItems = new SCRIPT_ITEM[iItemBuffSize];
			UT_return_val_if_fail(pItems, false);
			bDeleteItems = true;

			hRes = fScriptItemize(pInChars, iLen, iItemBuffSize, /*sc*/NULL, &ss, pItems, &iItemCount);
			
		}while(hRes == E_OUTOFMEMORY);

		UT_return_val_if_fail(hRes == 0, false);
	}
	
	// now we process the ouptut
	for(i = 0; i < (UT_uint32)iItemCount; ++i)
	{
		GR_Win32USPItem * pI = new GR_Win32USPItem(pItems[i]);
		UT_return_val_if_fail(pI, false);

		I.addItem(pItems[i].iCharPos, pI);
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_Win32USPItem(GRScriptType_Void));


	if(bDeleteItems)
		delete [] pItems;

	if(bDeleteChars)
		delete [] pInChars;
	
	return true;
}

bool GR_Win32USPGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	UT_return_val_if_fail(si.m_pItem && si.m_pItem->getClassId() == GRRI_WIN32_UNISCRIBE && si.m_pFont, false);
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)si.m_pItem;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)si.m_pFont;

	if(!ri)
	{
		ri = new GR_Win32USPRenderInfo((GR_ScriptType)pItem->m_si.a.eScript);
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_WIN32_UNISCRIBE, false);
	}

	GR_Win32USPRenderInfo * RI = (GR_Win32USPRenderInfo *)ri;

		
	if(RI->m_iClustSize < (UT_uint32)si.m_iLength)
	{
		delete [] RI->m_pClust;
		RI->m_pClust = new WORD[si.m_iLength];
		UT_return_val_if_fail(RI->m_pClust, false);

		RI->m_iClustSize = si.m_iLength;
	}

	// remove any justification information -- it will have to be recalculated
	delete[] RI->m_pJustify; RI->m_pJustify = NULL;
	
	// to save time we will use a reasonably sized static buffer and
	// will only allocate one on heap if the static one is too small.
	static WCHAR wcInChars[GRWIN32USP_CHARBUFF_SIZE]; 
	WCHAR *pInChars = &wcInChars[0];
	bool bDeleteChars = false;      // using static buffer

	if(si.m_iLength > GRWIN32USP_CHARBUFF_SIZE)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::shape: char buffer too small (len %d)\n", si.m_iLength));
		pInChars = new WCHAR[si.m_iLength];
		UT_return_val_if_fail(pInChars,false);

		bDeleteChars = true; // data on heap; cleanup later
	}

	UT_uint32 i;
	for(i = 0; i < (UT_uint32)si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		pInChars[i] = (WCHAR)si.m_Text.getChar();
	}

	// the problem with the glyph buffer is that we do no know how big
	// it needs to be, and what is worse, we will only find out by
	// trial and error; we will use a static buffer of size twice the
	// character count as the smallest buffer; this should mean that
	// most of the time we will succeed on first attempt (however, if the
	// buffer in the RI is bigger, we will use it instead)
	static WORD wGlyphs[2 * GRWIN32USP_CHARBUFF_SIZE];
	UT_uint32 iGlyphBuffSize = GRWIN32USP_CHARBUFF_SIZE *2;
	WORD *pGlyphs = &wGlyphs[0];
	
	static SCRIPT_VISATTR va[GRWIN32USP_CHARBUFF_SIZE *2];
	SCRIPT_VISATTR * pVa = &va[0];
	
	bool bCopyGlyphs = true;     // glyphs not in the RI
	bool bDeleteGlyphs = false;  // glyphs not in dynamically
								 // allocated memory
	
	if(GRWIN32USP_CHARBUFF_SIZE *2 < RI->m_iIndicesSize)
	{
		// use the bigger buffer in RI
		pGlyphs = RI->m_pIndices;
		pVa     = RI->m_pVisAttr;
		bCopyGlyphs = false; // glyphs directly in RI
		bDeleteGlyphs = true; // glyphs on heap
		iGlyphBuffSize = RI->m_iIndicesSize;
	}
	
	int iGlyphCount = 0;

	HDC hdc = 0;
	if(*(pFont->getScriptCache()) == NULL)
	{
		// need to make sure that the HDC has the correct font set
		// so that ScriptShape measures it correctly for the cache
		// we do not want to scale the font for this
		_setupFontOnDC(pFont, false);
		hdc = m_printHDC ? m_printHDC : m_hdc;
	}

	RI->m_bShapingFailed = false;
	RI->m_bNeedsReshaping = true;
	
	// we need to make sure that the analysis embeding level is in sync with si.m_iVisDir
	pItem->m_si.a.fRTL = si.m_iVisDir == UT_BIDI_RTL ? 1 : 0;
	HRESULT hRes = fScriptShape(hdc, pFont->getScriptCache(), pInChars, si.m_iLength,
								iGlyphBuffSize, & pItem->m_si.a, pGlyphs,
								RI->m_pClust, pVa, &iGlyphCount);

	if( hRes == E_PENDING)
	{
		UT_ASSERT_HARMLESS( hdc == 0 );

		_setupFontOnDC(pFont, false);
		hdc = m_printHDC ? m_printHDC : m_hdc;
		hRes = fScriptShape(hdc, pFont->getScriptCache(), pInChars, si.m_iLength,
							iGlyphBuffSize, & pItem->m_si.a, pGlyphs,
							RI->m_pClust, pVa, &iGlyphCount);
	}

	if(hRes == E_OUTOFMEMORY)
	{
		// glyph buffer too small ...
		do
		{
			// try twice the buffer size
			iGlyphBuffSize *= 2;
			if(bDeleteGlyphs)
			{
				delete [] pGlyphs;
				delete [] pVa;
				if(RI->m_pAdvances) {delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;}
				if(RI->m_pGoffsets) {delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;}
				if(RI->m_pJustify)  {delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;}
			}

			bCopyGlyphs = true; // glyphs not in RI
			
			pGlyphs = new WORD[iGlyphBuffSize];
			UT_return_val_if_fail(pGlyphs, false);
			pVa = new SCRIPT_VISATTR[iGlyphBuffSize];
			UT_return_val_if_fail(pVa, false);
			
			bDeleteGlyphs = true; // glyphs in dynamically alloc. memory

			hRes = fScriptShape(hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
							   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);
			
		}while(hRes == E_OUTOFMEMORY);

		UT_return_val_if_fail(hRes == 0, false);
	}
	else if(hRes)
	{
		// Some kind of other problem; this happens, for example, when the font cannot be used by
		// the shaping engine. For example, the Syriac text in World.abw fails unless it is
		// formatted with a suitable otf font (e.g. Estrangelo Edessa from Bet Marduk)
		// What we try to do as last resort is to disable shaping, and try
		// to call the function again. This will result in some or all of the glyphs in the string
		// being mapped to the missing glyph.
		
		UT_DEBUGMSG(("gr_Win32USPGraphics::shape: ScriptShape failed (hRes 0x%04x\n); disabling shaping\n"));

		// we only disable shaping temporarily, because if the font changes later, we need to try again
		RI->m_bShapingFailed = true;
		WORD eScript = pItem->m_si.a.eScript;
		pItem->m_si.a.eScript = GRScriptType_Undefined;

		hRes = fScriptShape(hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
						   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);

		pItem->m_si.a.eScript = eScript;
		UT_return_val_if_fail(hRes == 0, false);
	}
	
	if(bDeleteGlyphs && bCopyGlyphs)
	{
		// glyphs are in dynamically allocated memory, so we just need
		// to set the pointers
		RI->m_iIndicesSize = iGlyphBuffSize;
		RI->m_pIndices = pGlyphs;
		RI->m_pVisAttr = pVa;

		if(RI->m_pAdvances) {delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;}
		if(RI->m_pGoffsets) {delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;}
		if(RI->m_pJustify)  {delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;}
	}
	else if(!bDeleteGlyphs && bCopyGlyphs)
	{
		// glyphs are in a static buffer, we need to (possibly) g_try_realloc and copy

		// only g_try_realloc if necessary
		if((UT_sint32)RI->m_iIndicesSize < iGlyphCount)
		{
			delete [] RI->m_pIndices;
			delete [] RI->m_pVisAttr;

			RI->m_pIndices = new WORD [iGlyphCount];
			RI->m_pVisAttr = new SCRIPT_VISATTR [iGlyphCount];

			// we also have to delete the other related arrays we do
			// not need just yet to ensure that the size of all the
			// arrays will remain in sync
			if(RI->m_pAdvances) {delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;}
			if(RI->m_pGoffsets) {delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;}
			if(RI->m_pJustify)  {delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;}
			
			UT_return_val_if_fail(RI->m_pIndices && RI->m_pVisAttr, false);
		
			RI->m_iIndicesSize = iGlyphCount;
		}
		
		memcpy(RI->m_pIndices, pGlyphs, iGlyphCount * sizeof(WORD));
		memcpy(RI->m_pVisAttr, pVa, iGlyphCount * sizeof(SCRIPT_VISATTR));
	}
	else if (bDeleteGlyphs && !bCopyGlyphs)
	{
		// glyphs are already in the RI, just need to set the correct
		// size for the buffers
		RI->m_iIndicesSize = iGlyphBuffSize;

		if(RI->m_pAdvances) {delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;}
		if(RI->m_pGoffsets) {delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;}
		if(RI->m_pJustify)  {delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;}
	}
	else
	{
		// !bDeleteGlyphs && !bCopyGlyphs
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;
	RI->m_iIndicesCount = iGlyphCount;
	RI->m_pItem = si.m_pItem;
	RI->m_pFont = si.m_pFont;
	RI->m_iCharCount = si.m_iLength;
	RI->m_bNeedsReshaping = false;
	
	// once we implement the GR_Win32USPRenderInfo::append(), etc., we
	// should enable this; until then we need to treat everything as
	// complex and have it refreshed on merges, etc.
#if 0
	// work out shaping result
	DWORD dFlags = SIC_COMPLEX;

	if(si.m_iVisDir == UT_BIDI_RTL)
		dFlags |= SIC_NEUTRAL;

	SCRIPT_DIGITSUBSTITUTE sds;
	if(S_OK == fScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &sds))
	{
		if(sds.DigitSubstitute != SCRIPT_DIGITSUBSTITUTE_NONE)
			dFlags |= SIC_ASCIIDIGIT;
	}

	HRESULT hShape = fScriptIsComplex(pInChars, si.m_iLength, dFlags);
	if(hShape == S_OK)
		RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
	else
		RI->m_eShapingResult = GRSR_None;
#else
	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
#endif
	
	if(bDeleteChars)
	{
		delete [] pInChars;
	}

	if(RI->s_pOwnerChar == RI)
	{
		// this might not be strictly necessary, but it is safer to do so
		// no, this is necessary
		RI->s_pOwnerChar = NULL;
	}

	if(RI->s_pOwnerDraw == RI)
	{
		RI->s_pOwnerDraw = NULL;
	}
	
	return true;
}

UT_sint32 GR_Win32USPGraphics::getTextWidth(GR_RenderInfo & ri)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	GR_Win32USPFont * pFont = (GR_Win32USPFont*)RI.m_pFont;

	// NB -- do not have to check for zoom as the internal metrics is always for 100%
	if(pFont->getPrintDC() != getPrintDC() || RI.m_hdc != getPrintDC())
	{
		measureRenderedCharWidths(ri);
	}
	

	
#if 0
	// as neat as this is, we cannot use it, because due to the hinting errors, this is a
	// different number than our sum of character widths
	if(!RI.m_pJustify && ri.m_iOffset == 0 && ri.m_iLength == (UT_sint32)RI.m_iCharCount)
		return (RI.m_ABC.abcA + RI.m_ABC.abcB + RI.m_ABC.abcC);
#endif
	
	UT_return_val_if_fail(ri.m_iOffset + ri.m_iLength <= (UT_sint32)RI.m_iCharCount, 0);
	
	UT_sint32 iWidth = 0;
	GR_Win32USPItem & I = (GR_Win32USPItem &)*ri.m_pItem;
	bool bReverse = I.m_si.a.fRTL != 0;

	for(UT_sint32 i = ri.m_iOffset; i < ri.m_iLength + ri.m_iOffset; ++i)
	{
		if(!bReverse)
		{
			UT_uint32 iMax = RI.m_iIndicesCount;
			
			if(i < (UT_sint32)RI.m_iCharCount - 1)
				iMax = RI.m_pClust[i+1];

			for(UT_uint32 j = RI.m_pClust[i]; j < iMax; ++j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
		else
		{
			UT_sint32 iMin = -1;
			
			// The offset is a logical offset, and clusters are in logical order.  Indices, however,
			// are in visual order, and the clusters reference glyph indices, so that clust[i] >
			// clust[i+1]

			if(i < (UT_sint32)RI.m_iCharCount - 1)
				iMin = RI.m_pClust[i+1];

			for(UT_sint32 j = (UT_sint32)RI.m_pClust[i]; j > iMin; --j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
	}

	return iWidth;
}

void GR_Win32USPGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	// since we internally store widths in layout units, we need to
	// scale them down to device
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	UT_uint32 iZoom = getZoomPercentage();
	GR_Win32USPFont * pFont = (GR_Win32USPFont*)RI.m_pFont;

#if 0 //def DEBUG
	UT_uint32 iPoints = (UT_uint32)pFont->getPointSize();
#endif
	
	if(iZoom == RI.m_iZoom && RI.s_pOwnerDraw == & ri && pFont->getPrintDC() == getPrintDC())
	{
		// the buffer is up-to-date
		return;
	}

	// NB -- do not have to check for zoom as the internal metrics is always for 100%
	if(pFont->getPrintDC() != getPrintDC())
	{
		// this happens when we change printer
		// we need to recalculate the widths ...
		measureRenderedCharWidths(ri);
	}
	
	if(RI.s_iAdvancesSize < RI.m_iIndicesCount)
	{
		UT_return_if_fail(RI.allocStaticBuffers(RI.m_iIndicesCount));
	}

	UT_sint32 iWidth = 0;
	UT_sint32 iNextAdvance = 0;
	UT_sint32 iAdvance = 0;

	UT_sint32 iWidthJ = 0;
	UT_sint32 iNextAdvanceJ = 0;
	UT_sint32 iAdvanceJ = 0;
	
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		iWidth += RI.m_pAdvances[i];
		iNextAdvance = (UT_sint32)((double)_tduX(iWidth) * m_fXYRatio);
		RI.s_pAdvances[i] = iNextAdvance - iAdvance;
		iAdvance = iNextAdvance;

		// I use tdu here, not _tduX, _tduY, because the goffsets are adjustments relative
		// to the x,y offsets of the character
		RI.s_pGoffsets[i].du = (long)((double)tdu(RI.m_pGoffsets[i].du) * m_fXYRatio);
		RI.s_pGoffsets[i].dv = tdu(RI.m_pGoffsets[i].dv);
		
		
		if(RI.m_pJustify)
		{
			iWidthJ += RI.m_pAdvances[i] + RI.m_pJustify[i];
			iNextAdvanceJ = (UT_sint32)((double)_tduX(iWidthJ) * m_fXYRatio);
			RI.s_pJustify[i] = iNextAdvanceJ - iAdvanceJ;
			iAdvanceJ = iNextAdvanceJ;
		}
	}

	// there are two situations in which we need to recalculate the static positioning
	// buffers, (a) the buffer is not ours, or, (b) we have last calculated its contents
	// at different zomm level. Furthermore, if we detect a zoom change, we need to
	// invalidate the screen ascent value for the current font
	if(RI.m_iZoom != iZoom)
		pFont->setScreenAscent(0);
	
	RI.m_iZoom = iZoom;
	RI.s_pOwnerDraw = &ri;
}

/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_Win32USPGraphics::renderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)RI.m_pFont;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont);

	UT_sint32 xoff = (UT_sint32)((double)_tduX(RI.m_xoff) * m_fXYRatio);

	// we will deal with yoff later
	if(RI.m_iLength == 0)
		return;
	
	UT_return_if_fail(RI.m_iOffset + RI.m_iLength <= (UT_sint32)RI.m_iCharCount);

	UT_uint32 iGlyphCount = RI.m_iIndicesCount;
	UT_uint32 iGlyphOffset = 0;

	GR_Win32USPItem & I = (GR_Win32USPItem &)*ri.m_pItem;
	bool bReverse = I.m_si.a.fRTL != 0;

	// need logical offset to access cluster information
	UT_uint32 iOffset = bReverse ? RI.m_iCharCount - RI.m_iOffset - 1 : RI.m_iOffset;

	if(RI.m_iOffset != 0)
	{
		// we need to work out glyph offset
		iGlyphOffset = RI.m_pClust[iOffset];
	}

	if(RI.m_iOffset + RI.m_iLength == (UT_sint32)RI.m_iCharCount)
	{
		// drawing from the offset to the end
		iGlyphCount -= iGlyphOffset;
	}
	else
	{
		// work out glyph length
		iOffset = bReverse ? RI.m_iCharCount - (RI.m_iOffset + RI.m_iLength) - 1 : RI.m_iOffset + RI.m_iLength;
		UT_uint32 iOffsetEnd = RI.m_pClust[iOffset];
		iGlyphCount = iOffsetEnd - iGlyphOffset;
	}

	if(RI.m_bInvalidateFontCache)
	{
		fScriptFreeCache(pFont->getScriptCache());
		*(pFont->getScriptCache()) = NULL;
		RI.m_bInvalidateFontCache = false;
	}

	// need to make sure that the HDC has the correct font set
	// we scale this font
	_setupFontOnDC(pFont, true);

	// Now deal with y offset: RI.m_yoff is the top of the run, based on the ascent of the
	// font; if we are drawing on screen and using printer metrics to do the layout, the
	// ascent of the screen font can be smaller/greater and so we need to adjust the yoff
	// accordingly
	UT_sint32 yoff;

	if(m_bPrint)
	{
		yoff = _tduY(RI.m_yoff);
	}
	else
	{
		UT_sint32 iAscentScreen = pFont->getScreenAscent();

		if(!iAscentScreen)
		{
			TEXTMETRICW tm;
			memset(&tm, 0, sizeof(tm));

			GetTextMetricsW(m_hdc, &tm);
			iAscentScreen = (UT_sint32)((double)tm.tmAscent*(double)getResolution()*100.0/
										((double)getDeviceResolution()*(double)getZoomPercentage()));
		
			pFont->setScreenAscent(iAscentScreen);
		}

		UT_sint32 iAscentPrint = pFont->getTextMetric().tmAscent;
		yoff = _tduY(RI.m_yoff + iAscentPrint - iAscentScreen);
	}
	
	int * pJustify = RI.m_pJustify && RI.m_bRejustify ? RI.s_pJustify + iGlyphOffset : NULL;
	
	// not sure how expensive SetBkMode is, but GetBkMode() should not
	// be ...
	if(GetBkMode(m_hdc) != TRANSPARENT)
	{
		SetBkMode(m_hdc, TRANSPARENT); // this is necessary
	}
	
	UINT dFlags = 0;

	// need to disapble shaping if call to ScriptShape failed ...
	WORD eScript = pItem->m_si.a.eScript;
	if(RI.m_bShapingFailed)
		pItem->m_si.a.eScript = GRScriptType_Undefined;

	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	
	HRESULT hRes = fScriptTextOut(m_hdc, pFont->getScriptCache(), xoff, yoff,
								  dFlags, /*option flags*/
								  NULL, /*not sure about this*/
								  & pItem->m_si.a,
								  NULL, 0, /*reserved*/
								  RI.m_pIndices  + iGlyphOffset,
								  iGlyphCount,
								  RI.s_pAdvances + iGlyphOffset,
								  pJustify,
								  RI.s_pGoffsets + iGlyphOffset);

	pItem->m_si.a.eScript = eScript;
	//RI.m_bRejustify = false; -- the docs are misleading; rejustification is always needed

	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptTextOut failed", hRes)
	}
}

void GR_Win32USPGraphics::setPrintDC(HDC dc)
{
	// only do this for screen graphics
	if(!m_bPrint && dc != m_printHDC)
	{
		// since this is a printer DC we have to delete it once not needed
		// the win32 graphics does not delete the printer dc, which is a bug in the pure
		// win32 graphics class, but exactly what we need here.
		if(m_printHDC && m_printHDC != m_defPrintHDC)
			DeleteDC(m_printHDC);
		
		m_printHDC = dc;

		if(getPrintDC())
		{
			m_nPrintLogPixelsY = GetDeviceCaps(getPrintDC(), LOGPIXELSY);
			int nLogPixelsX = GetDeviceCaps(getPrintDC(), LOGPIXELSX);
			if(m_nPrintLogPixelsY)
			{
				m_fXYRatioPrint = (double)nLogPixelsX / (double) m_nPrintLogPixelsY;
			}
			else
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				m_fXYRatioPrint = 1;
			}
		}
		if(m_printHDC)
		{
			// now make our views to rebuild themselves ...
			XAP_App * pApp = XAP_App::getApp();
			UT_return_if_fail( pApp );

			UT_uint32 iFrameCount = pApp->getFrameCount();
			for(UT_uint32 i = 0; i < iFrameCount; i++)
			{
				XAP_Frame * pFrame = pApp->getFrame(i);
				if(!pFrame)
					continue;

				AV_View * pView = pFrame->getCurrentView();
				if(pView)
				{
					GR_Graphics * pG = pView->getGraphics();

					if(pG == this)
						pView->fontMetricsChange();
				}
			}
		}
	}
}



void GR_Win32USPGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pFont);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)RI.m_pFont;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_if_fail(pFont && pItem );

	if(!RI.m_pAdvances)
		RI.m_pAdvances = new int[RI.m_iIndicesSize];

	if(!RI.m_pGoffsets)
		RI.m_pGoffsets = new GOFFSET[RI.m_iIndicesSize];

	UT_uint32 iZoom = getZoomPercentage();
	
#if 0 //def DEBUG
	UT_uint32 iPoints = (UT_uint32)pFont->getPointSize();
#endif

	bool bFontSetUpOnDC = false;
	
	// the script cache is always containing data for 100% zoom, we scale widths manually later
	// but we need to refresh it if the printer changed
	//if(iZoom != RI.m_iZoom)
	if(pFont->getPrintDC() != getPrintDC())
	{
		// the zoom factor has changed; make sure we invalidate the cache
		if(*(pFont->getScriptCache()) != NULL)
		{
			fScriptFreeCache(pFont->getScriptCache());
			*(pFont->getScriptCache()) = NULL;
		}

		if(getPrintDC())
		{
			// we also need to remeasure the font metrics
			// and scale it down for the screen
			_setupFontOnDC(pFont, false);
			bFontSetUpOnDC = true;
			
			TEXTMETRICW tm;
			memset(&tm, 0, sizeof(tm));

			GetTextMetricsW(getPrintDC(), &tm);

#if 0 //def DEBUG
			HDC printHDC = UT_GetDefaultPrinterDC();
			TEXTMETRIC tm2 = { 0 };
			GetTextMetrics(printHDC, &tm2);
			DeleteDC(printHDC);
#endif
			pFont->setHeight(MulDiv(tm.tmHeight, getResolution(), m_nPrintLogPixelsY));
			pFont->setAscent(MulDiv(tm.tmAscent, getResolution(), m_nPrintLogPixelsY));
			pFont->setDescent(MulDiv(tm.tmDescent, getResolution(), m_nPrintLogPixelsY));
		}
	}

	// store the print DC used to measure the font in the font ...
	pFont->setPrintDC(getPrintDC());
	
	HDC hdc = 0; // this is the hdc to use if all fails
	HDC hdc1 = 0; // this is the hdc to use in the first pass (null if we have script cache)
	if(*(pFont->getScriptCache()) == NULL)
	{
		// need to make sure that the HDC has the correct font set
		// we do not scale the font by zoom
		if(!bFontSetUpOnDC)
		{
			_setupFontOnDC(pFont, false);
			bFontSetUpOnDC = true;
		}
		
		hdc = m_printHDC ? m_printHDC : m_hdc;

		// we remember the hdc for which we measured so we can remeasure when hdc changes
		RI.m_hdc = hdc;
		hdc1 = hdc;
	}
	else
	{
		hdc = pFont->getPrintDC() ? pFont->getPrintDC() : m_hdc;
		hdc1 = 0;
		RI.m_hdc = hdc;
	}
	

	
	// need to disapble shaping if call to ScriptShape failed ...
	WORD eScript = pItem->m_si.a.eScript;
	if(RI.m_bShapingFailed)
		pItem->m_si.a.eScript = GRScriptType_Undefined;
	
	HRESULT hRes = fScriptPlace(hdc1, pFont->getScriptCache(), RI.m_pIndices,
								RI.m_iIndicesCount, RI.m_pVisAttr,
								& pItem->m_si.a, RI.m_pAdvances, RI.m_pGoffsets, & RI.m_ABC);

	if( hRes == E_PENDING)
	{
		_setupFontOnDC(pFont, false);
		bFontSetUpOnDC = true;
		
		hdc = m_printHDC ? m_printHDC : m_hdc;

		// we remember the hdc for which we measured so we can remeasure when hdc changes
		RI.m_hdc = hdc;
		
		hRes = fScriptPlace(hdc, pFont->getScriptCache(), RI.m_pIndices,
							RI.m_iIndicesCount, RI.m_pVisAttr,
							& pItem->m_si.a, RI.m_pAdvances, RI.m_pGoffsets, & RI.m_ABC);

		UT_ASSERT_HARMLESS( !hRes );
	}
	else if(hRes)
	{
		// some other error, at least set the advances and offsets to 0
		memset(RI.m_pAdvances, 0, sizeof(int) * RI.m_iIndicesCount);
		memset(RI.m_pGoffsets, 0, sizeof(GOFFSET) * RI.m_iIndicesCount);
		memset(&RI.m_ABC, 0, sizeof(ABC));
	}
	

	const UT_uint32 iAdvSize = 80;
	int iAdvances[iAdvSize];
	GOFFSET stGoffsets[iAdvSize];
	UT_uint32 iCount = iAdvSize;
	ABC stABC;

	int * pAdvances = &iAdvances[0];
	GOFFSET * pGoffsets = &stGoffsets[0];

	bool bAdjustAdvances = false;
	
#if 1
	if(!m_bPrint && getPrintDC())
	{
		// This is a trick to avoid unseemly gaps between letters on screen due to
		// discrepancies between font metrics of screen and printer. We measure the font
		// also on screen and distribute the difference to the left and right of each
		// character by adjusting each character's m_pGoffsets.du. This improves things,
		// but does not completely remove the problem; whether it is worth the extra
		// is to be seen. Tomas, Apr 9, 2005
		_setupFontOnDC(pFont, true);
		
		if(iCount <= RI.m_iIndicesCount)
		{
			iCount = RI.m_iIndicesCount + 1;
			pAdvances = new int[iCount];
			pGoffsets = new GOFFSET[iCount];
		}
		
		
		SCRIPT_CACHE sc = NULL;
		HRESULT hResFSP = fScriptPlace(m_hdc, &sc, RI.m_pIndices, RI.m_iIndicesCount, RI.m_pVisAttr,
									& pItem->m_si.a, pAdvances, pGoffsets, & stABC);

		UT_ASSERT_HARMLESS( !hResFSP );
		if(!hResFSP)
		{
			bAdjustAdvances = true;
		}
		else
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			LOG_USP_EXCPT_X("fScriptPlace failed", hResFSP)
		}

		if(sc)
			fScriptFreeCache(&sc);			
	}
#endif
	
	pItem->m_si.a.eScript = eScript;

	// remember the zoom at which we calculated this ...
	// RI.m_iZoom = iZoom;

	if(RI.s_pOwnerDraw == & ri)
	{
		// we currently own the static buffers; invalidate
		RI.s_pOwnerDraw = NULL;
	}

	if(RI.s_pOwnerCP == & ri)
	{
		// we currently own the static buffers; invalidate
		RI.s_pOwnerCP = NULL;
	}

	// now convert the whole lot to layout units
	double dWidth = 0;
	double dDeviceWidth = 0;
	double dPrevDeviceWidth = 0;
	double dAdjustment = 0;
	
	if(getPrintDC())
	{
		double fXYRatio = m_fXYRatio * m_fXYRatioPrint;

		for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
		{
			dWidth += (double)RI.m_pAdvances[i];
			dDeviceWidth = dWidth*(double)getResolution() / ((double)m_nPrintLogPixelsY*fXYRatio);

			if(bAdjustAdvances)
			{
				dAdjustment = (((double)RI.m_pAdvances[i]*(double)getResolution()
								/((double)m_nPrintLogPixelsY*fXYRatio))
							   - (double)pAdvances[i]*(double)getResolution()*100.
							   /((double)getDeviceResolution()*(double)iZoom * m_fXYRatio))/2.;
			}
			RI.m_pAdvances[i]   = (UT_sint32)(dDeviceWidth - dPrevDeviceWidth + 0.5);
			RI.m_pGoffsets[i].du = (UT_sint32)((double)RI.m_pGoffsets[i].du*(double)getResolution()
											/((double)m_nPrintLogPixelsY*fXYRatio) + 2*dAdjustment + 0.5);
			RI.m_pGoffsets[i].dv = (UT_sint32)((double)RI.m_pGoffsets[i].dv*(double)getResolution()
											/((double)m_nPrintLogPixelsY) + 0.5);

			dPrevDeviceWidth = dDeviceWidth;
		}

		RI.m_ABC.abcA = (UT_sint32)((double) RI.m_ABC.abcA * (double)getResolution()
									/((double)m_nPrintLogPixelsY*fXYRatio) + 0.5);
		RI.m_ABC.abcB = (UT_sint32)((double) RI.m_ABC.abcB * (double)getResolution()
									/((double)m_nPrintLogPixelsY*fXYRatio) + 0.5);
		RI.m_ABC.abcC = (UT_sint32)((double) RI.m_ABC.abcC * (double)getResolution()
									/((double)m_nPrintLogPixelsY*fXYRatio) + 0.5);
	}
	else if (m_bPrint)
	{
		for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
		{
			dWidth += (double)RI.m_pAdvances[i];
			dDeviceWidth = dWidth*(double)getResolution()/
									   ((double)getDeviceResolution()*m_fXYRatio);
			
			RI.m_pAdvances[i] = (UT_sint32)(dDeviceWidth - dPrevDeviceWidth + 0.5);
			dPrevDeviceWidth = dDeviceWidth;

			RI.m_pGoffsets[i].du = (UT_sint32)((double)RI.m_pGoffsets[i].du*(double)getResolution()/
											((double)getDeviceResolution()*m_fXYRatio) + 0.5);
			RI.m_pGoffsets[i].dv = (UT_sint32)((double)RI.m_pGoffsets[i].dv*(double)getResolution()/
											((double)getDeviceResolution()) + 0.5);
		}

		RI.m_ABC.abcA = (UT_sint32)((double) RI.m_ABC.abcA * (double)getResolution() /
									((double)getDeviceResolution()*m_fXYRatio) + 0.5);
		RI.m_ABC.abcB = (UT_sint32)((double) RI.m_ABC.abcB * (double)getResolution() /
									((double)getDeviceResolution()*m_fXYRatio) + 0.5);
		RI.m_ABC.abcC = (UT_sint32)((double) RI.m_ABC.abcC * (double)getResolution() /
									((double)getDeviceResolution()*m_fXYRatio) + 0.5);
	}
	else
	{
		for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
		{
			dWidth += (double)RI.m_pAdvances[i];
			dDeviceWidth = dWidth*(double)getResolution()/
									   ((double)getDeviceResolution()*(double)GR_WIN32_USP_FONT_SCALING*m_fXYRatio);

			RI.m_pAdvances[i]   = (UT_sint32)(dDeviceWidth - dPrevDeviceWidth + 0.5);

			RI.m_pGoffsets[i].du = (UT_sint32)((double)RI.m_pGoffsets[i].du*(double)getResolution()/
											((double)getDeviceResolution()*(double)GR_WIN32_USP_FONT_SCALING*m_fXYRatio)
											   + 0.5);
			
			RI.m_pGoffsets[i].dv = (UT_sint32)((double)RI.m_pGoffsets[i].dv*(double)getResolution()/
											((double)getDeviceResolution()*(double)GR_WIN32_USP_FONT_SCALING) + 0.5);
			dPrevDeviceWidth = dDeviceWidth;

		}

		RI.m_ABC.abcA = (UT_sint32)((double) RI.m_ABC.abcA * (double)getResolution() /
									((double)getDeviceResolution() * (double)GR_WIN32_USP_FONT_SCALING*m_fXYRatio) + 0.5);
		RI.m_ABC.abcB = (UT_sint32)((double) RI.m_ABC.abcB * (double)getResolution() /
									((double)getDeviceResolution() * (double)GR_WIN32_USP_FONT_SCALING*m_fXYRatio) + 0.5);
		RI.m_ABC.abcC = (UT_sint32)((double) RI.m_ABC.abcC * (double)getResolution() /
									((double)getDeviceResolution() * (double)GR_WIN32_USP_FONT_SCALING*m_fXYRatio) + 0.5);
	}
	
	RI.m_bRejustify = true;

	if(iCount != iAdvSize)
	{
		delete[] pAdvances;
		delete[] pGoffsets;
	}
	
	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptPlace failed", hRes)
	}
}

void GR_Win32USPGraphics::appendRenderedCharsToBuff(GR_RenderInfo & /*ri*/, UT_GrowBuf & /*buf*/) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}
/*!
    returns true on success
 */
bool GR_Win32USPGraphics::_scriptBreak(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pText && ri.m_pItem, false);
	
	if(ri.s_pOwnerChar != &ri)
	{
		UT_return_val_if_fail(ri.m_pText->getStatus() == UTIter_OK, false);
		UT_uint32 iPosStart = ri.m_pText->getPosition();
		UT_uint32 iPosEnd   = ri.m_pText->getUpperLimit();
		UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);

		UT_uint32 iLen = UT_MIN(iPosEnd - iPosStart + 1, (UT_uint32)ri.m_iLength); // including iPosEnd

		ri.s_pOwnerChar = &ri;
		
		if(ri.s_iAdvancesSize < iLen)
		{
			UT_return_val_if_fail( ri.allocStaticBuffers(iLen),false);
		}
		
		for(UT_uint32 i = 0; i < iLen; ++i, ++(*(ri.m_pText)))
		{
			ri.s_pChars[i] = (WCHAR)ri.m_pText->getChar();
		}

		// restore the iterrator to the initial position
		*(ri.m_pText) -= iLen;
		
		GR_Win32USPItem &I = (GR_Win32USPItem &)*ri.m_pItem;
		HRESULT hRes = fScriptBreak(ri.s_pChars, iLen, &I.m_si.a, ri.s_pLogAttr);

		UT_return_val_if_fail(!hRes,false);
	}

	return true;
}

/*!
    this function should return true if break can occur AFTER the character at indicated
    position; Uniscribe functions indicate if break can occur BEFORE the character
*/
bool GR_Win32USPGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter)
{
	// for now we will call ScriptBreak from here; should this be too
	// much of a bottle neck, we will store the values (but we are
	// already storing loads of data per char)
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_iOffset < ri.m_iLength, false);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	iNext = -1;

	if(!_scriptBreak(RI))
		return false;

	if(ri.m_iLength > (UT_sint32)RI.s_iAdvancesSize)
	{
		UT_return_val_if_fail( RI.allocStaticBuffers(ri.m_iLength),false );
	}
	
	
	if(_needsSpecialBreaking(RI))
	{
		UT_uint32 iDelta  = 0;
#if 1
		if(bAfter)
		{
			// the caller wants to know if break can occur on the (logically) right edge of the given
			// character
			if(ri.m_iOffset + 1 == ri.m_iLength)
			{
				// we are quering the last char of a run, for which we do not have the info
				// we will return false, which should force the next run to be examined ...
				return false;
			}

			// we will examine the next character, since USP tells us about breaking on the left edge
			iDelta = 1;
		}
#endif
		if(RI.s_pLogAttr[ri.m_iOffset + iDelta].fSoftBreak)
			return true;

		// find the next break
		for(UT_sint32 i = ri.m_iOffset + iDelta + 1; i < RI.m_iLength; ++i)
		{
			if(RI.s_pLogAttr[i].fSoftBreak)
			{
				iNext = i - iDelta;
				break;
			}
		}
	}
	else
	{
		// we look for white-space break points, so we do not adjust the offset for bAfter
		if(RI.s_pLogAttr[ri.m_iOffset].fWhiteSpace)
			return true;

		// this fixes 9462;
		bool bBreak = GR_Graphics::canBreak(ri, iNext, bAfter);

		// if the base class gave us a break or indicated where the break is, return
		if(bBreak || iNext >= 0)
			return bBreak;
		
	}

	if(iNext == -1)
	{
		// we have not found any breaks in this run -- signal this to the caller
		iNext = -2;
	}
	
	return false;
}

bool GR_Win32USPGraphics::_needsSpecialBreaking(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(s_ppScriptProperties && ri.m_pItem, false);
	if(ri.m_bShapingFailed)
		return false;
	
	return (s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsWordBreaking != 0);
}

UT_uint32 GR_Win32USPGraphics::adjustCaretPosition(GR_RenderInfo & ri, bool bForward)
{
	// for now we will call ScriptBreak from here; since most of the time caret changes happen on
	// the same line, this should not be a performance problem
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, ri.m_iOffset);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(!_needsSpecialCaretPositioning(RI))
		return ri.m_iOffset;

	if(!_scriptBreak(RI))
		return ri.m_iOffset;

	UT_sint32 iPos = (UT_sint32)ri.m_iOffset;
	
	if(bForward)
	{
		while(iPos < ri.m_iLength && !RI.s_pLogAttr[iPos].fCharStop)
			iPos++;

		// iPos == m_iLength, we reached the end of the run -- we will assumed the position
		// immediately after the run implicitely valid, so we do no adjustments.
	}
	else
	{
		while(iPos >= 0 && !RI.s_pLogAttr[iPos].fCharStop)
			iPos--;

		if(iPos < 0)
		{
			// it would seem that the runs starts in illegal character; we return 0
			iPos = 0;
		}
		
	}

	return (UT_uint32)iPos;
}

void GR_Win32USPGraphics::adjustDeletePosition(GR_RenderInfo & ri)
{
	// for now we will call ScriptBreak from here; since most of the time caret changes happen on
	// the same line, this should not be a performance problem
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(ri.m_iLength >= (UT_sint32)RI.m_iCharCount)
	{
		// we are deleting the last character of the run, or past it; we will assume that clusters
		// do not cross run boundaries and allow the deletion happen
		return;
	}

	if(!_needsSpecialCaretPositioning(RI))
		return;

	// _scriptBreak expects the length of the segment to process to be set in ri.m_iLength which
	// currently holds the length of the deletion (we always process the whole run, as this
	// simplifies things)
	UT_uint32 iCharCount = ri.m_iLength;
	ri.m_iLength = RI.m_iCharCount;
	
	if(!_scriptBreak(RI))
		return;

	// deletion can start anywhere, but can only end on cluster boundary if the base character is
	// included in the deletion
	
	// get the offset of the character that follows the delete segment
	UT_sint32 iNextOffset = (UT_sint32)ri.m_iOffset + iCharCount;

	if(RI.s_pLogAttr[iNextOffset].fCharStop)
	{
		// the next char is a valid caret position, so we are OK
		// restore the original length
		ri.m_iLength = iCharCount;
		return;
	}

	// If we got this far, we were asked to end the deletion before a character that is not a valid
	// caret position. We need to determine if the segment we are asked to delete contains thi
	// character's base character; if it does, we have to expand the seletion to delete the entire
	// cluster.

	UT_sint32 iOffset = iNextOffset - 1;
	while(iOffset > 0 && iOffset > ri.m_iOffset && !RI.s_pLogAttr[iOffset].fCharStop)
		iOffset--;

	if(RI.s_pLogAttr[iOffset].fCharStop)
	{
		// our delete segment includes the base character, so we have to delete the entire cluster
		iNextOffset = iOffset + 1;
		
		while(iNextOffset < (UT_sint32)RI.m_iCharCount
			  && !RI.s_pLogAttr[iNextOffset].fCharStop)
			iNextOffset++;

		
		ri.m_iLength = iNextOffset - ri.m_iOffset;
		return;
	}
	
	// two posibilities: we are deleting only a cluster appendage or the run does not contain
	// base character. The latter should probably not happen, but in both cases we will let the
	// delete proceed as is

	// restore the original length
	ri.m_iLength = iCharCount;
}

bool GR_Win32USPGraphics::needsSpecialCaretPositioning(GR_RenderInfo & ri)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, false);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	
	return _needsSpecialCaretPositioning(RI);
}
	

bool GR_Win32USPGraphics::_needsSpecialCaretPositioning(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(s_ppScriptProperties && ri.m_pItem, false);
	if(ri.m_bShapingFailed)
		return false;

	return (s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsCaretInfo != 0);
}



UT_sint32 GR_Win32USPGraphics::resetJustification(GR_RenderInfo & ri, bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;
	
	//UT_sint32 iWidth1 = 0;
	UT_sint32 iWidth2 = 0;
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		//iWidth1 += RI.m_pAdvances[i];
		iWidth2 += RI.m_pJustify[i];
	}

	if(bPermanent)
	{
		delete [] RI.m_pJustify;
		RI.m_pJustify = NULL;
	}
	else
	{
		memset(RI.m_pJustify, 0, RI.m_iIndicesSize * sizeof(int));
	}
	
	if(RI.s_pOwnerDraw == & RI)
		RI.s_pOwnerDraw = NULL;

	if(RI.s_pOwnerCP == & RI)
		RI.s_pOwnerCP = NULL;
	
	return -iWidth2;
}

UT_sint32 GR_Win32USPGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pItem, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	UT_return_val_if_fail(RI.m_pVisAttr,0);
	
	UT_sint32 iCountSpace     = 0;
	UT_sint32 iCountSpaceAR   = 0;
	UT_sint32 iCountKashida   = 0;
	UT_sint32 iCountInterChar = 0;
	bool bBlank = true; // will change to false if we find anything
						// that we do not have a counter for

	// we have to work from the end to the beginning, which means decreasing index in LTR and
	// increasing it in RTL
	UT_sint32 i, iInc, iLimit, iStart;

	GR_Win32USPItem * pItem = (GR_Win32USPItem *) ri.m_pItem;

	if(pItem->m_si.a.fRTL)
	{
		iStart = 0;
		iInc = 1;
		iLimit = RI.m_iIndicesCount;
	}
	else
	{
		iStart = RI.m_iIndicesCount - 1;
		iInc = -1;
		iLimit = -1;
	}
	
	bool bFoundNonBlank = !ri.m_bLastOnLine; // treat items that are not last on line as non-blank
	for(i = iStart; i != iLimit; i += iInc)
	{
		if(!bFoundNonBlank)
		{
			if(RI.m_pVisAttr[i].uJustification == SCRIPT_JUSTIFY_BLANK)
			{
				// this is a trailing blank character, ignore it ...
				continue;
			}
			else
			{
				bFoundNonBlank = true;
			}
		}
		
		switch(RI.m_pVisAttr[i].uJustification)
		{
			case SCRIPT_JUSTIFY_ARABIC_BLANK:
				iCountSpaceAR++;
				break;
				
			case SCRIPT_JUSTIFY_CHARACTER:
				iCountInterChar++;
				break;
				
			case SCRIPT_JUSTIFY_BLANK:
				iCountSpace++;
				break;
				
			case SCRIPT_JUSTIFY_ARABIC_NORMAL:
			case SCRIPT_JUSTIFY_ARABIC_KASHIDA:
			case SCRIPT_JUSTIFY_ARABIC_ALEF:
			case SCRIPT_JUSTIFY_ARABIC_HA:
			case SCRIPT_JUSTIFY_ARABIC_RA:
			case SCRIPT_JUSTIFY_ARABIC_BA:
			case SCRIPT_JUSTIFY_ARABIC_BARA:
			case SCRIPT_JUSTIFY_ARABIC_SEEN:
				iCountKashida++;
				break;
				
			case SCRIPT_JUSTIFY_NONE:
			case SCRIPT_JUSTIFY_RESERVED1:
			case SCRIPT_JUSTIFY_RESERVED2:
			case SCRIPT_JUSTIFY_RESERVED3:
			default:
				bBlank = false;
		}
	}

	// now we need to make sense of the stats
	if(iCountKashida)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_ARABIC_KASHIDA;
		
		return iCountSpace + iCountKashida;
	}
	
	if(iCountSpace)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_BLANK;
		if(bBlank && !iCountInterChar)
			return -iCountSpace;
		else
			return iCountSpace;
	}

	if(iCountSpaceAR)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_ARABIC_BLANK;
		if(bBlank && !iCountInterChar)
			return -iCountSpaceAR;
		else
			return iCountSpaceAR;
	}
	
	// only use intercharacter justification if the script requires it
	if(iCountInterChar && s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsCharacterJustify)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_CHARACTER;
		return iCountInterChar;
	}

	RI.m_eJustification = SCRIPT_JUSTIFY_NONE;
	return 0;
}

void GR_Win32USPGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pItem);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount)
		return;
	
	if(!RI.m_pJustify)
		RI.m_pJustify = new int[RI.m_iIndicesSize];

	// mark the static width caches dirty
	if(RI.s_pOwnerDraw == & RI)
		RI.s_pOwnerDraw = NULL;

	if(RI.s_pOwnerCP == & RI)
		RI.s_pOwnerCP = NULL;
	
	UT_return_if_fail(RI.m_pJustify);
	memset(RI.m_pJustify, 0, RI.m_iIndicesSize * sizeof(int));
	
	UT_uint32 iExtraSpace = RI.m_iJustificationAmount;
	UT_uint32 iPoints     = RI.m_iJustificationPoints;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *) ri.m_pItem;
	
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		UT_uint32 k = pItem->m_si.a.fRTL ? RI.m_iIndicesCount - i - 1: i;
		if(RI.m_pVisAttr[k].uJustification == RI.m_eJustification ||
		   (RI.m_eJustification == SCRIPT_JUSTIFY_ARABIC_KASHIDA &&
			RI.m_pVisAttr[k].uJustification >= SCRIPT_JUSTIFY_ARABIC_KASHIDA))
		{
			UT_uint32 iSpace = iExtraSpace/iPoints;
			iExtraSpace -= iSpace;
			iPoints--;

			RI.m_pJustify[k] = iSpace;

			if(!iPoints)
				break;
		}
	}

	UT_ASSERT_HARMLESS( !iExtraSpace );
}

UT_uint32 GR_Win32USPGraphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 /*y*/) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) ri;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_val_if_fail(pItem, 0);

	// x = tdu(x); -- no conversion here, since we keep m_pAdvances in
	// layout units
	
	int iPos;
	int iTrail;
	int * pAdvances = RI.m_pJustify ? RI.s_pJustifiedAdvances : RI.m_pAdvances;
	
	if(RI.m_pJustify && RI.s_pOwnerCP != &RI)
	{
		if(RI.s_iAdvancesSize < RI.m_iIndicesCount)
		{
			UT_return_val_if_fail( RI.allocStaticBuffers (RI.m_iIndicesCount), 0);
		}
		
		
		for(UT_uint32 i  = 0; i < RI.m_iIndicesCount; ++i)
		{
			RI.s_pJustifiedAdvances[i] = RI.m_pAdvances[i] + RI.m_pJustify[i];
		}

		RI.s_pOwnerCP = &RI;
	}
	
	HRESULT hRes = fScriptXtoCP(x, RI.m_iLength, RI.m_iIndicesCount, RI.m_pClust,
								RI.m_pVisAttr, pAdvances,
								& pItem->m_si.a, &iPos, &iTrail);

	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptXtoCP failed", hRes)
	}

	return iPos + iTrail;
}

void GR_Win32USPGraphics::positionToXY(const GR_RenderInfo & ri,
										  UT_sint32& x, UT_sint32& /*y*/,
										  UT_sint32& x2, UT_sint32& /*y2*/,
										  UT_sint32& /*height*/, bool& /*bDirection*/) const
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) ri;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	//UT_uint32 iZoom = getZoomPercentage();
	
	if(!pItem)
		return;

	bool bTrailing = true;

	int * pAdvances = RI.m_pJustify ? RI.s_pJustifiedAdvances : RI.m_pAdvances;
	
	if(RI.m_pJustify && RI.s_pOwnerCP != &RI)
	{
		if( RI.s_iAdvancesSize < RI.m_iIndicesCount)
		{
			UT_return_if_fail( RI.allocStaticBuffers(RI.m_iIndicesCount));
		}
		
		
		for(UT_uint32 i  = 0; i < RI.m_iIndicesCount; ++i)
		{
			RI.s_pJustifiedAdvances[i] = RI.m_pAdvances[i] + RI.m_pJustify[i];
		}

		RI.s_pOwnerCP = &RI;
	}
	
	HRESULT hRes = fScriptCPtoX(RI.m_iOffset,
							   bTrailing, /* fTrailing*/
							   RI.m_iLength, RI.m_iIndicesCount, RI.m_pClust, RI.m_pVisAttr,
							   pAdvances, & pItem->m_si.a, &x);

	

	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptCPtoX failed", hRes)
	}

	x = x;
	x2 = x;
}

void GR_Win32USPGraphics::_drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * /*pCharWidth*/)
{
	if(!pChars || ! iLength)
		return;
	
	if(GetBkMode(m_hdc) != TRANSPARENT)
	{
		SetBkMode(m_hdc, TRANSPARENT); // this is necessary
	}

	static WCHAR buff[100];
	WCHAR * pwChars = buff;
    bool bDelete = false;
	
	if(iLength > 100)
	{
		pwChars = new WCHAR[iLength];
		bDelete = true;
	}

	UT_sint32 i = 0;
	for(i = 0; i < iLength; ++i)
	{
		pwChars[i] = pChars[i+iCharOffset];
	}
	
	SCRIPT_STRING_ANALYSIS SSA;
	UT_uint32 flags = SSA_GLYPHS;

	// need to make sure that the HDC has the correct font set
	// so that ScriptShape measures it correctly for the cache
	GR_Win32USPFont * pFont = static_cast<GR_Win32USPFont *>(m_pFont);
	_setupFontOnDC(pFont, true);
	
	HRESULT hRes = fScriptStringAnalyse(m_hdc, pwChars, iLength, iLength*3/2 + 1,
						-1, flags, 0, NULL, NULL, NULL, NULL, NULL, &SSA);

	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptStringAnalyse failed", hRes)
	}

	hRes = 0; //reset this so we don't log misleading results
	
	if(SSA)
	{
		hRes = fScriptStringOut(SSA, (UT_sint32)((double)_tduX(xoff) * m_fXYRatio), _tduY(yoff),
								0, NULL, 0, 0, FALSE);
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	if(hRes)
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		LOG_USP_EXCPT_X("fScriptStringOut failed", hRes)
	}

	if(SSA)
		fScriptStringFree(&SSA);

	if(bDelete)
		delete [] pwChars;
}

void GR_Win32USPGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * pCharWidth)
{
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	_drawChars(pChars, iCharOffset, iLength, xoff, yoff, pCharWidth);
}


void GR_Win32USPGraphics::drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
								 int iCharOffset,
								 int iLength,
								 UT_sint32 xoff,
								 UT_sint32 yoff,
								 int* pCharWidths)
{
	SetTextAlign(m_hdc, TA_LEFT | TA_BASELINE);
	_drawChars(pChars, iCharOffset, iLength, xoff, yoff, pCharWidths);
}


void GR_Win32USPGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	if(getZoomPercentage() != iZoom)
	{
		
		GR_Graphics::setZoomPercentage(iZoom);

		// now force remeasuring of character widths in any views that are associated with
		// this graphics
		XAP_App * pApp = XAP_App::getApp();
		UT_return_if_fail( pApp );

		UT_uint32 iFrameCount = pApp->getFrameCount();
		for(UT_uint32 i = 0; i < iFrameCount; i++)
		{
			XAP_Frame * pFrame = pApp->getFrame(i);
			if(!pFrame)
				continue;

			AV_View * pView = pFrame->getCurrentView();
			if(pView)
			{
				GR_Graphics * pG = pView->getGraphics();

				if(pG == this)
					pView->remeasureCharsWithoutRebuild();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_WinUSPRenderInfo Implementation
//

bool GR_Win32USPRenderInfo::append(GR_RenderInfo & /*ri*/, bool /*bReverse*/)
{
	//UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
	m_bNeedsReshaping = true;
	
	return false;
}

bool GR_Win32USPRenderInfo::split (GR_RenderInfo *&pri, bool /*bReverse*/)
{
	UT_return_val_if_fail(m_pGraphics && m_pFont && m_pItem, false);

	UT_ASSERT_HARMLESS(!pri);
	if(!pri)
	{
		pri = new GR_Win32USPRenderInfo(m_eScriptType);
		UT_return_val_if_fail(pri,false);
	}

	pri->m_pItem = m_pItem->makeCopy();
	UT_return_val_if_fail(pri->m_pItem, false);

	m_bNeedsReshaping = true;
	return false;
	
	// trying to split the data is just too precarious ...
#if 0

	if(m_bNeedsReshaping)
	{
		// we have not been shaped, so that is all we can do for the caller
		return false;
	}
	
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) *pri;
	RI.m_bShapingFailed = m_bShapingFailed;
	
	UT_return_val_if_fail( (UT_sint32)m_iClustSize > m_iOffset, false );

	GR_Win32USPItem &I = (GR_Win32USPItem &)*m_pItem;
	
	UT_uint32 iGlyphOffset = m_pClust[m_iOffset];

	if(I.isRTL())
	{
		// need to include the glyph at the offset as well
		iGlyphOffset++;
	}
	
	UT_uint32 iGlyphLen1 = iGlyphOffset;
	UT_uint32 iGlyphLen2 = m_iIndicesCount - iGlyphLen1;

	if(I.isRTL())
	{
		UT_uint32 t = iGlyphLen1;
		iGlyphLen1 =  iGlyphLen2;
		iGlyphLen2 = t;
	}
	
	UT_uint32 iCharLen1 = m_iOffset;
	UT_uint32 iCharLen2 = m_iCharCount - iCharLen1;

	// this is an attempt to simply split the data, but I am not sure
	// what to do about the ABC and GOFFSET, hence we recalculate the
	// placements
	if(RI.m_iIndicesSize < iGlyphLen2)
	{
		delete [] RI.m_pIndices;  RI.m_pIndices  = new WORD [iGlyphLen2];
		delete [] RI.m_pAdvances; RI.m_pAdvances = new int  [iGlyphLen2];
		delete [] RI.m_pVisAttr;  RI.m_pVisAttr  = new SCRIPT_VISATTR [iGlyphLen2];
		delete [] RI.m_pGoffsets; RI.m_pGoffsets = new GOFFSET[iGlyphLen2];

		UT_return_val_if_fail(RI.m_pIndices && RI.m_pAdvances && RI.m_pVisAttr, false);
		
		if(m_pJustify)
		{
			delete [] RI.m_pJustify; RI.m_pJustify = new int [iGlyphLen2];
			memset(RI.m_pJustify, 0, iGlyphLen2 * sizeof(int));;
			UT_return_val_if_fail(RI.m_pJustify, false);
		}
		
		RI.m_iIndicesSize = iGlyphLen2;
	}

	if(RI.m_iClustSize < iCharLen2)
	{
		delete [] RI.m_pClust; RI.m_pClust = new WORD [iCharLen2];
		UT_return_val_if_fail(RI.m_pClust, false);
		RI.m_iClustSize = iCharLen2;
	}

	if(I.isRTL())
	{
		UT_uint32 i;
		// RI is the first segment of the visual buffers
		memcpy(RI.m_pIndices, m_pIndices, sizeof(WORD)*iGlyphLen2);
		memcpy(RI.m_pVisAttr,  m_pVisAttr, sizeof(SCRIPT_VISATTR)*iGlyphLen2);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(i = 0; i < iCharLen2; ++i)
		{
			RI.m_pClust[i] = m_pClust[i+m_iOffset];
		}
		
		memmove(m_pIndices, m_pIndices + iGlyphOffset, sizeof(WORD)*iGlyphLen1);
		memmove(m_pVisAttr, m_pVisAttr + iGlyphOffset, sizeof(SCRIPT_VISATTR)*iGlyphLen1);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(i = 0; i < iCharLen1; ++i)
		{
			m_pClust[i] = m_pClust[i] - iCharLen2;
		}
	}
	else
	{
		memcpy(RI.m_pIndices,  m_pIndices  + iGlyphOffset, sizeof(WORD)*iGlyphLen2);
		memcpy(RI.m_pVisAttr,  m_pVisAttr  + iGlyphOffset, sizeof(SCRIPT_VISATTR)*iGlyphLen2);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(UT_uint32 i = 0; i < iCharLen2; ++i)
		{
			RI.m_pClust[i] = m_pClust[i+m_iOffset] - m_iOffset;
		}
	}
	
	m_iIndicesCount = iGlyphLen1; RI.m_iIndicesCount = iGlyphLen2;
	m_iCharCount    = iCharLen1;  RI.m_iCharCount    = iCharLen2;
	RI.m_pGraphics = m_pGraphics;
	RI.m_pFont = m_pFont;

	RI.m_eShapingResult = m_eShapingResult;
	RI.m_eState = m_eState;
	RI.m_hdc = m_hdc;

	GR_Graphics * pG = const_cast<GR_Graphics*>(m_pGraphics);

	pri->m_bLastOnLine = m_bLastOnLine;
	m_bLastOnLine = false;
	
	pG->measureRenderedCharWidths(*this);
	pG->measureRenderedCharWidths(*pri);

	if(m_pJustify && m_iJustificationPoints)
	{
		UT_uint32 iJustAmount = m_iJustificationAmount;
		UT_uint32 iJustPoints = m_iJustificationPoints;
		
		m_iJustificationPoints = pG->countJustificationPoints(*this);
		pri->m_iJustificationPoints = pG->countJustificationPoints(*pri);

		pri->m_iJustificationAmount = iJustAmount * pri->m_iJustificationPoints / iJustPoints;
		m_iJustificationAmount = iJustAmount - pri->m_iJustificationAmount;
	}
	

	if(s_pOwnerChar == &RI)
	{
		s_pOwnerChar = NULL;
	}
	
	return true;
#endif
}

bool GR_Win32USPRenderInfo::cut(UT_uint32 /*offset*/, UT_uint32 /*iLen*/, bool /*bReverse*/)
{
	//UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
	m_bNeedsReshaping = true;
	
	return false;
}

bool GR_Win32USPRenderInfo::isJustified() const
{
	return (m_pJustify != NULL);
}

GR_Win32USPFont *  GR_Win32USPFont::newFont(LOGFONTW &lf, double fPoints, HDC hdc, HDC printHDC)
{
	GR_Win32USPFont * f = new GR_Win32USPFont(lf, fPoints, hdc, printHDC);

	if(!f || !f->getFontHandle())
	{
		delete f;
		f = NULL;
	}

	return f;
}

GR_Win32USPFont::GR_Win32USPFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC)
	: GR_Win32Font(lf, fPoints, hdc, printHDC),
	  m_sc(NULL),
	  m_printHDC(NULL),
	  m_iScreenAscent(0)
{
}

void GR_Win32USPFont::_clearAnyCachedInfo()
{
	if(m_sc != NULL)
	{
		GR_Win32USPGraphics::fScriptFreeCache(&m_sc);
		m_sc = NULL;
	}
}

GR_Win32USPFont::~GR_Win32USPFont()
{
	GR_Win32USPGraphics::fScriptFreeCache(&m_sc);
}


/*********************************/
/* General plugin stuff */
/*********************************/
#ifndef GR_WIN32USP_BUILTIN

#include "xap_Module.h"
#include "xap_App.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_grwin32usp_register
#define abi_plugin_unregister abipgn_grwin32usp_unregister
#define abi_plugin_supports_version abipgn_grwin32usp_supports_version
#endif

static	UT_uint32 s_iPrevDefaultScreen = 0;
static	UT_uint32 s_iPrevDefaultPrinter = 0;

ABI_PLUGIN_DECLARE("gr_Win32USPGraphics")

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	mi->name    = PLUGIN_NAME;
	mi->desc    = "";
	mi->version = "0.1.0.0";
	mi->author  = "Tomas Frydrych <tomasfrydrych@yahoo.co.uk";
	mi->usage   = "";

	UT_VersionInfo v1(0,1,0,0);
	
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, 0);

	GR_GraphicsFactory * pGF = pApp->getGraphicsFactory();
	UT_return_val_if_fail(pGF, 0);
	
	if(!pGF->registerClass(GR_Win32USPGraphics::graphicsAllocator,
						   GR_Win32USPGraphics::graphicsDescriptor,
						   GR_Win32USPGraphics::s_getClassId()))
	{
		// OK, there is a class already registered with our id
		// (probably a built-in version of this class) -- get its
		// version info and replace it if ours is higher
		GR_Win32AllocInfo ai;
		GR_Graphics * pG = pApp->newGraphics(GR_Win32USPGraphics::s_getClassId(), ai);
		UT_return_val_if_fail(pG, 0);

		const UT_VersionInfo & v2 = pG->getVersion();
		if(v1 > v2)
		{
			// first we need to see if this class is registered as the
			// default graphics class; if so we need to changed that
			// to the basic win32 class before we can unregister it
			// we also need to remember the previous values so we can
			// restore them when we are unloaded
			s_iPrevDefaultScreen  = pGF->getDefaultClass(true);
			s_iPrevDefaultPrinter = pGF->getDefaultClass(false);
			
			if(s_iPrevDefaultScreen == GR_Win32USPGraphics::s_getClassId())
			{
				s_iPrevDefaultScreen = GRID_WIN32;
				pGF->registerAsDefault(GRID_WIN32, true);
			}
			
			if(s_iPrevDefaultPrinter == GR_Win32USPGraphics::s_getClassId())
			{
				s_iPrevDefaultPrinter = GRID_WIN32;
				pGF->registerAsDefault(GRID_WIN32, false);
			}

			if(!pGF->unregisterClass(GR_Win32USPGraphics::s_getClassId()
			|| !pGF->registerClass(GR_Win32USPGraphics::graphicsAllocator,
								   GR_Win32USPGraphics::graphicsDescriptor,
								   GR_Win32USPGraphics::s_getClassId())))
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				delete pG;
				return 0;
			}
		}
		else
		{
			delete pG;
			return 0;
		}

		delete pG;
	}

	pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), true);
	pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), false);
	
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, 0);

	GR_GraphicsFactory * pGF = pApp->getGraphicsFactory();
	UT_return_val_if_fail(pGF, 0);
	
	if(pGF->getDefaultClass(true) == GR_Win32USPGraphics::s_getClassId())
	{
		if(pGF->isRegistered(s_iPrevDefaultScreen))
		{
			pGF->registerAsDefault(s_iPrevDefaultScreen, true);
		}
		else
		{
			pGF->registerAsDefault(GRID_WIN32, true);
		}
	}
	
	if(pGF->getDefaultClass(false) == GR_Win32USPGraphics::s_getClassId())
	{
		if(pGF->isRegistered(s_iPrevDefaultPrinter))
		{
			pGF->registerAsDefault(s_iPrevDefaultPrinter, false);
		}
		else
		{
			pGF->registerAsDefault(GRID_WIN32, false);
		}
	}
	
	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
				 UT_uint32 release)
{

	return 1;
}
#endif
