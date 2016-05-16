/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych, <tomas@frydrych.uklinux.net>
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

#ifndef GR_GRAPHICS_H
#define GR_GRAPHICS_H

#include "xap_Features.h"

#include "ut_types.h"
#include "ut_units.h"
#include "ut_growbuf.h"
#include "ut_misc.h"
#include "gr_Image.h"
#include "gr_Caret.h"
#include "gr_Transform.h"
#include "gr_CharWidthsCache.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ut_TextIterator.h"

#ifdef ABI_GRAPHICS_PLUGIN
#define VIRTUAL_SFX = 0
#else
#define VIRTUAL_SFX
#endif

class UT_RGBColor;
class XAP_PrefsScheme;
class XAP_Frame;
class UT_String;
class GR_RenderInfo;
class GR_Itemization;
class GR_ShapingInfo;


/*!
  GR_Font is a reference to a font.  As it happens, everything about fonts
  is platform-specific, so the class contains nothing.  All of its behavior
  and functionality is contained within its subclasses, each of which provides
  the implementation for some specific font technology.

  May 16 2003
  The assertion made above is mostly wrong. Font works almost the same
  on every platform. We have metrics, glyphs, measurement, etc. Sure implementation is utterly
  platform dependent, but we can provide a mostly XP interface to that.
  That would certainly reduce platform code a increase XP code.
   -- Hub
*/

class GR_Graphics;
class GR_Painter;
class GR_Caret;

enum GrFontType {
	GR_FONT_UNSET=0,
	GR_FONT_UNIX,
	GR_FONT_UNIX_PANGO,
	GR_FONT_WIN32,
	GR_FONT_WIN32_USP
};

class ABI_EXPORT GR_Font
{
	friend class GR_Graphics;
	friend class std::map<std::string, GR_Font*>;

 public:
	// want the destructor public so that the derrived graphics classes can delete font
	// objects without having to be declared here as friends
 	virtual ~GR_Font();


	enum FontFamilyEnum { FF_Unknown = 0, FF_Roman, FF_Swiss, FF_Modern,
				   FF_Script, FF_Decorative, FF_Technical, FF_BiDi, FF_Last };
	enum FontPitchEnum { FP_Unknown = 0, FP_Fixed, FP_Variable };

	// The following is actually implemented in platform code.
	// It is primarily used to characterize fonts for RTF export.
	static void s_getGenericFontProperties(const char * szFontName,
										   FontFamilyEnum * pff,
										   FontPitchEnum * pfp,
										   bool * pbTrueType);

	virtual const char* getFamily() const { return NULL; }
	UT_uint32           getAllocNumber() const {return m_iAllocNo;}
	/*!
		Measure the unremapped char to be put into the cache.
		That means measuring it for a font size of 120
	 */
	virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const = 0;
	virtual const std::string & hashKey(void) const;
	UT_sint32 getCharWidthFromCache (UT_UCSChar c) const;

	/*reimplement if you want to instanciate something else */
	virtual GR_CharWidths* newFontWidths(void) const;
	/*
	   implemented using character widths; platforms might want to
	   provide different implementation
	   NB: it is essential that this function is fast
	*/
	virtual bool doesGlyphExist(UT_UCS4Char g) const;
//
// UT_Rect of glyph in Logical units.
// rec.left = bearing Left (distance from origin to start)
// rec.width = width of the glyph
// rec.top = distance from the origin to the top of the glyph
// rec.height = total height of the glyph

	virtual bool glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG) = 0;
	static  bool s_doesGlyphExist(UT_UCS4Char g, void *instance)
	{
		UT_return_val_if_fail(instance, false);
		GR_Font * pThis = static_cast<GR_Font*>(instance);
		return pThis->doesGlyphExist(g);
	}

	GrFontType getType()const {return m_eType;}

  protected:

	GR_Font();

	GR_CharWidths * _getCharWidths() const {return m_pCharWidths;}
	/*!
	  hash key for font cache. Must be initialized in ctor
	 otherwise override hashKey() method
	*/
	mutable std::string		m_hashKey;

	GrFontType               m_eType;

private:

	static UT_uint32 s_iAllocCount;
	UT_uint32        m_iAllocNo;
	mutable GR_CharWidths*	m_pCharWidths;
};


/*
    GR_GraphicsId defines IDs returned by GR_Graphics::getClassId()
    used to identify the class in communication with GR_GraphicsFactory.

    There are three types of IDs: default, built-in and plugin.

    Default IDs are not assigned permanently to any class and are used
	by the factory to allocate default graphics. These IDs are
	reallocable -- if the factory receives a request to register a
	class under one of these IDs, it will automatically unregister any
	class currently registered under that ID.

	Built-in IDs are permanently assinged to specific derrived
	classes. The factory will refuse to register a class under one of
	these IDs if another class is already registered with it.

	Plugin IDs are dynamically generated for a graphics class by the
	factory. The main draw-back of plugin IDs is that they cannot be
	stored in preference profiles. A plugin desinger might prefer to
	request a fixed ID from the built-in range (we could use UUIDs to
	identify graphics, but that seems an overkill).
 */
enum GR_GraphicsId
{
	/* default id's */
	/* these id's can be reregistered at will */
	GRID_DEFAULT         =  0x0,
	GRID_DEFAULT_PRINT   =  0x1,

	GRID_LAST_DEFAULT    =  0xff,

	/* IDs for built-in classes: DO NOT CHANGE THE ASSIGNED VALUES !!!*/
	/* (these classes cannot be unregistered) */
	GRID_COCOA           =  0x102,
	GRID_WIN32           =  0x104,
	GRID_UNIX            =  0x105,
	GRID_UNIX_PS         =  0x106,
	GRID_CAIRO_NULL      =  0x107,

	/*add new built-in ids here*/

	GRID_LAST_BUILT_IN = 0x200,

	/* IDs for extension classes (can be both built-in and plugins) */
	/* (these classes can be unregistered by explicit call to
	   unregisterClass()) */
	GRID_UNIX_PANGO      =  0x201,
	GRID_WIN32_UNISCRIBE =  0x202,
	GRID_UNIX_PANGO_PRINT = 0x203,
	GRID_UNIX_PANGO_PIXMAP = 0x204,
        GRID_COCOA_PANGO     = 0x205,
	GRID_QT				 = 0x206,

	GRID_LAST_EXTENSION = 0x0000ffff,

	/* id's for plugins will be auto-generatoed from between here */

	GRID_UNKNOWN = 0xffffffff
};

// or-able graphics type
enum GR_Capability
{
	GRCAP_UNKNOWN            = 0,
	GRCAP_SCREEN_ONLY        = 1,
	GRCAP_PRINTER_ONLY       = 2,
	GRCAP_SCREEN_AND_PRINTER = 3
};

/*!
    The following class serves as an argument to GR_GraphicsFactory::newGraphics()

    There should be only one derived class for each platform (not each
    graphics), capable of holding information for all different
    graphics classes on the platform.

    For example, on Unix we have three different classes with the
    following constructors:

   	    GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App *app)

		PS_Graphics(const char * szFilename,
		            const char * szTitle,
					const char * szSoftwareNameAndVersion,
					XAP_UnixFontManager * fontManager,
					bool		 bIsFile,
					XAP_App *pApp);

		UnixNull_Graphics(XAP_UnixFontManager * fontManager,XAP_App *pApp);

	GR_UnixAllocInfo will need to be able to hold parameters for all
	the constructors, something like

	    class GR_UnixAllocInfo
	    {
	        GdkWindow *           win;
	        XAP_UnixFontManager * fontManager;
	        XAP_App *             app;
			const char *          szFilename;
		    const char *          szTitle;
			const char *          szSoftwareNameAndVersion;
			bool		          bIsFile;
	    };

	This does impose some limitations on classes implemented as
	plugins: if the plugin class needs something that is not in the
	platform class and cannot obtain it by other means (for example by
	quering XAP_App), the AllocInfo class will need to be extended.

	Platform implementation needs to override getType() so that
	graphicsAllocator() can do type-checking.
*/
class ABI_EXPORT GR_AllocInfo
{
  public:
	virtual ~GR_AllocInfo() {}

	virtual GR_GraphicsId getType() const {UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return GRID_UNKNOWN;}
	virtual bool isPrinterGraphics()const {UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return false;}
};

typedef GR_Graphics * (*GR_Allocator)(GR_AllocInfo&);
typedef const char *  (*GR_Descriptor)(void);
/*
   The purpose of GR_GraphicsFactory is to allow us to have parallel
   graphics implementations. For example, on win32 we could have a
   graphics class using Uniscribe, graphics class without shaping
   support and graphics class using SIL Graphite. The user then could
   specify through preferences which shaping engine s/he wishes to
   use. The factory provides access to all graphics classes known to
   the application.
*/
class ABI_EXPORT GR_GraphicsFactory
{

  public:
	GR_GraphicsFactory(){};
	virtual ~GR_GraphicsFactory(){};

	UT_uint32     getClassCount() const {return m_vClassIds.getItemCount();}

	bool          registerClass(GR_Allocator, GR_Descriptor, UT_uint32 iClassId);
	UT_uint32     registerPluginClass(GR_Allocator, GR_Descriptor);

	void          registerAsDefault(UT_uint32 iClassId, bool bScreen)
	                     {
							 if(bScreen)
								 m_iDefaultScreen = iClassId;
							 else
								 m_iDefaultPrinter = iClassId;
						 }

	UT_uint32     getDefaultClass(bool bScreen) const {if(bScreen) return m_iDefaultScreen; else return m_iDefaultPrinter;}
	bool          unregisterClass(UT_uint32 iClassId);
	bool          isRegistered(UT_uint32 iClassId) const;

	GR_Graphics * newGraphics(UT_uint32 iClassId, GR_AllocInfo &param) const;
	const char *  getClassDescription(UT_uint32 iClassId) const;


  private:
	UT_GenericVector<GR_Allocator>       m_vAllocators;
	UT_GenericVector<GR_Descriptor>       m_vDescriptors;
	UT_NumberVector m_vClassIds;

	UT_uint32       m_iDefaultScreen;
	UT_uint32       m_iDefaultPrinter;
};


enum GRShapingResult
{
	GRSR_BufferClean = 0x00,                  // clear all bits; see notes above !!!
	GRSR_None = 0x01,                         // bit 0 set
	GRSR_ContextSensitive = 0x02,             // bit 1 set
	GRSR_Ligatures = 0x04,                    // bit 2 set
	GRSR_ContextSensitiveAndLigatures = 0x06, // bit 1, 2 set
	GRSR_Unknown = 0xef,                      // bits 0-6 set, initial value for text in our runs
	GRSR_Error = 0xff                         // bits 0-7 set
};

/*
  GR_Graphics is a portable interface to a simple 2-d graphics layer.  It is not
  an attempt at a general purpose portability layer.  Rather, it contains only
  functions which are needed.
*/

#define GR_OC_LEFT_FLUSHED 0x40000000 // flip bit 31
#define GR_OC_MAX_WIDTH    0x3fffffff

class ABI_EXPORT AllCarets
{
	friend class GR_Graphics;
 public:
	AllCarets(GR_Graphics * pG,
			  GR_Caret ** pCaret,
			  UT_GenericVector<GR_Caret *>* vecCarets  );
	virtual ~AllCarets(){}
	GR_Caret *  getBaseCaret(void);
	void	    enable(void);
	void		disable(bool bNoMulti = false);
	void		setBlink(bool bBlink);
	void        JustErase(UT_sint32 xPoint,UT_sint32 yPoint);
	void        setWindowSize(UT_uint32 width, UT_uint32 height);
	void		setCoords(UT_sint32 x, UT_sint32 y, UT_uint32 h,
						  UT_sint32 x2 = 0, UT_sint32 y2 = 0, UT_uint32 h2 = 0,
						  bool bPointDirection = false,
						  const UT_RGBColor * pClr = NULL);
	void		setInsertMode (bool mode);
	void		forceDraw(void);

 private:
	GR_Graphics * m_pG;
	GR_Caret **    m_pLocalCaret;
	UT_GenericVector<GR_Caret *>* m_vecCarets;
};


class ABI_EXPORT GR_Graphics
{
	friend class GR_Painter;
	friend class GR_Caret;
	friend class AllCarets;
 public:
	virtual ~GR_Graphics();

	// the static method allows us to retrive the the class id for
	// purposes of registration; we also need the virtual to identify
	// the class from a generic GR_Graphics pointer
//	static UT_uint32 s_getClassId() {UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return GRID_UNKNOWN;}
	virtual UT_uint32 getClassId() = 0;

	virtual GR_Capability getCapability() {UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return GRCAP_UNKNOWN;}
#if 0
	// the following two static functions have to be implemented by all
	// derrived classes and registered with GR_GraphicsFactory
	static const char *    graphicsDescriptor(void){UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return "???";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&){UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED); return NULL;}
#endif

	AllCarets *	allCarets();
	void		disableAllCarets();
	void		enableAllCarets();

	UT_sint32	tdu(UT_sint32 layoutUnits) const;
	UT_sint32	tlu(UT_sint32 deviceUnits) const;
	double	    tduD(double layoutUnits) const;
	double  	tluD(double deviceUnits) const;
	void        antiAliasAlways(bool bAntiAlias)
	{ m_bAntiAliasAlways = bAntiAlias;}
	bool        getAntiAliasAlways(void)
	{ return m_bAntiAliasAlways;}

	/*!
		Font units to layout units. Returns the dimension in layout units since font
		are not Zoomed
	 */
	UT_sint32	ftlu(UT_sint32 fontUnits) const;
	double		ftluD(double fontUnits) const;

	virtual void      setFont(const GR_Font* pFont) = 0;
    virtual void      clearFont(void) = 0;
	virtual UT_uint32 getFontAscent() = 0;
	virtual UT_uint32 getFontDescent() = 0;
	virtual UT_uint32 getFontHeight() = 0;
	void              invalidateCache(void);
	virtual bool      canQuickPrint(void) const { return false;}
	virtual UT_uint32 measureString(const UT_UCSChar*s,
									int iOffset,
									int num,
									UT_GrowBufElement* pWidths, UT_uint32 *height = 0);

	virtual UT_sint32 measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * height = 0) = 0;
	virtual void getCoverage(UT_NumberVector& coverage) = 0;

	/* GR_Font versions of the above -- TODO: should I add drawChar* methods too? */
	virtual UT_uint32 getFontAscent(const GR_Font *)  = 0;
	virtual UT_uint32 getFontDescent(const GR_Font *) = 0;
	virtual UT_uint32 getFontHeight(const GR_Font *)  = 0;
	virtual double    getResolutionRatio(void) const {return 1.0;}

	void         getMaxCharacterDimension (const UT_UCSChar*s, UT_uint32 Length, UT_uint32 &width, UT_uint32 &height);

	virtual void      setColor(const UT_RGBColor& clr) = 0;
	virtual void      getColor(UT_RGBColor& clr) = 0;
	virtual GR_Font*  getGUIFont() = 0;

	GR_Font*  findFont(const char* pszFontFamily,
					   const char* pszFontStyle,
					   const char* pszFontVariant,
					   const char* pszFontWeight,
					   const char* pszFontStretch,
					   const char* pszFontSize,
					   const char* pszLang);

	/* Static 'virtual' -- if you are providing an implementation for this
	 * function in a derrived graphics class, please define
	 * XAP_HAVE_GR_findNearestFont in platform xap_*Features.h file
	 */
#ifdef XAP_HAVE_GR_findNearestFont
	static const char* findNearestFont(const char* pszFontFamily,
									   const char* pszFontStyle,
									   const char* pszFontVariant,
									   const char* pszFontWeight,
									   const char* pszFontStretch,
									   const char* pszFontSize,
									   const char* pszLang);
#else
	static const char* findNearestFont(const char* pszFontFamily,
									   const char* /*pszFontStyle*/,
									   const char* /*pszFontVariant*/,
									   const char* /*pszFontWeight*/,
									   const char* /*pszFontStretch*/,
									   const char* /*pszFontSize*/,
									   const char* /*pszLang*/)
		{return pszFontFamily;}
#endif

	const char *      invertDimension(UT_Dimension, double) const;

	bool              scaleDimensions(const char * szLeftIn,
									  const char * szWidthIn,
									  UT_uint32 iWidthAvail,
									  UT_sint32 * piLeft,
									  UT_uint32 * piWidth) const;

   	virtual GR_Image* createNewImage(const char* pszName,
									 const UT_ByteBuf* pBB,
									 const std::string& mimetype,
									 UT_sint32 iWidth,
									 UT_sint32 iHeight,
									 GR_Image::GRType iType = GR_Image::GRT_Raster);

	virtual void      setLineWidth(UT_sint32) = 0;

	virtual void      setClipRect(const UT_Rect* pRect) = 0;
	const UT_Rect *   getClipRect(void) const { return m_pRect;}
	virtual void      scroll(UT_sint32, UT_sint32) = 0;
	virtual void      scroll(UT_sint32 x_dest,
							 UT_sint32 y_dest,
							 UT_sint32 x_src,
							 UT_sint32 y_src,
							 UT_sint32 width,
							 UT_sint32 height) = 0;

	enum Properties { DGP_SCREEN, DGP_PAPER, DGP_OPAQUEOVERLAY };

	enum JoinStyle
	{
		JOIN_MITER,
		JOIN_ROUND,
		JOIN_BEVEL
	};

	enum CapStyle
	{
		CAP_BUTT,
		CAP_ROUND,
		CAP_PROJECTING
	};

	enum LineStyle
	{
		LINE_SOLID,
		LINE_ON_OFF_DASH,
		LINE_DOUBLE_DASH,
		LINE_DOTTED
	};

	virtual void setLineProperties ( double inWidthPixels,
					 JoinStyle inJoinStyle = JOIN_MITER,
					 CapStyle inCapStyle   = CAP_BUTT,
					 LineStyle inLineStyle = LINE_SOLID ) ;

	virtual bool      queryProperties(GR_Graphics::Properties gp) const = 0;
	/* the following 3 are only used for printing */

	virtual bool      startPrint(void) = 0;

	virtual bool      startPage(const char * szPageLabel,
								UT_uint32 pageNumber,
								bool bPortrait,
								UT_uint32 iWidth,
								UT_uint32 iHeight) = 0;

	virtual bool      endPrint(void) = 0;

	virtual void      flush(void);

	/* specific color space support */

	enum ColorSpace {
		GR_COLORSPACE_COLOR,
		GR_COLORSPACE_GRAYSCALE,
		GR_COLORSPACE_BW
	};

	virtual void      setColorSpace(GR_Graphics::ColorSpace c) = 0;
	virtual GR_Graphics::ColorSpace getColorSpace(void) const = 0;

	/* multiple cursor support */

	enum Cursor {
		GR_CURSOR_INVALID = 0,
		GR_CURSOR_DEFAULT,
		GR_CURSOR_IBEAM,
		GR_CURSOR_RIGHTARROW,
		GR_CURSOR_IMAGE,
		GR_CURSOR_IMAGESIZE_NW,
		GR_CURSOR_IMAGESIZE_N,
		GR_CURSOR_IMAGESIZE_NE,
		GR_CURSOR_IMAGESIZE_E,
		GR_CURSOR_IMAGESIZE_SE,
		GR_CURSOR_IMAGESIZE_S,
		GR_CURSOR_IMAGESIZE_SW,
		GR_CURSOR_IMAGESIZE_W,
		GR_CURSOR_LEFTRIGHT,
		GR_CURSOR_UPDOWN,
		GR_CURSOR_EXCHANGE,
		GR_CURSOR_GRAB,
		GR_CURSOR_LINK,
		GR_CURSOR_WAIT,
		GR_CURSOR_LEFTARROW,
		GR_CURSOR_VLINE_DRAG,
		GR_CURSOR_HLINE_DRAG,
		GR_CURSOR_CROSSHAIR,
		GR_CURSOR_DOWNARROW,
		GR_CURSOR_DRAGTEXT,
		GR_CURSOR_COPYTEXT
	};

	virtual void      setCursor(GR_Graphics::Cursor c) = 0;
	virtual GR_Graphics::Cursor getCursor(void) const = 0;

	virtual void      setZoomPercentage(UT_uint32 iZoom);
	inline UT_uint32  getZoomPercentage(void) const {return m_iZoomPercentage; }
	static UT_uint32  getResolution(void) { return UT_LAYOUT_RESOLUTION; }
	inline void       setPortrait (bool b) {m_bIsPortrait = b;}
	inline bool       isPortrait (void) const {return m_bIsPortrait;}

	enum GR_Color3D {
		CLR3D_Foreground = 0,				/* color of text/foreground on a 3d object */
		CLR3D_Background = 1,				/* color of face/background on a 3d object */
		CLR3D_BevelUp = 2,					/* color of bevel-up  */
		CLR3D_BevelDown = 3,				/* color of bevel-down */
		CLR3D_Highlight = 4				/* color half-way between up and down */
	};
#define COUNT_3D_COLORS 5

	virtual void      setColor3D(GR_Color3D c) = 0;
	virtual bool      getColor3D(GR_Color3D /*name*/, UT_RGBColor & /*color*/) = 0;

	const GR_Transform & getTransform() const {return m_Transform;}

	/* returns true on success, false on failure */
	bool              setTransform(const GR_Transform & tr)
	                     {
							 bool ret = _setTransform(tr);
							 if(!ret)
								 return false;
							 m_Transform = tr;
							 return true;
						 }

	void              createCaret()
		{
			UT_ASSERT_HARMLESS(!m_pCaret);
			m_pCaret = new GR_Caret(this);
		}

	GR_Caret *        createCaret(const std::string& sID);
	GR_Caret *        getCaret(const std::string& sID) const;
	GR_Caret *        getNthCaret(UT_sint32 i) const;
	void              removeCaret(const std::string& sID);

	virtual void	  saveRectangle(UT_Rect & r, UT_uint32 iIndx) = 0;
	virtual void	  restoreRectangle(UT_uint32 iIndx) = 0;
	virtual UT_uint32 getDeviceResolution(void) const = 0;
//
// Use these methods to fix off by 1 errors while scrolling. Add the
// the logical difference to these first, then calculate how much
// the screen needs to scroll in device units
//
	UT_sint32         getPrevYOffset(void) const { return m_iPrevYOffset;}
	UT_sint32         getPrevXOffset(void) const { return m_iPrevXOffset;}
	void              setPrevYOffset(UT_sint32 y) { m_iPrevYOffset = y;}
	void              setPrevXOffset(UT_sint32 x) { m_iPrevXOffset = x;}

	UT_sint32         _tduX(UT_sint32 layoutUnits) const;


	///////////////////////////////////////////////////////////////////
	// complex script processing; see default implementations of these
	// functions for documentation
	//
	virtual bool itemize(UT_TextIterator & text, GR_Itemization & I) VIRTUAL_SFX;

	// translates GR_ShapingInfo into GR_RenderInfo which then can be
	// passed to renderChars()
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri) VIRTUAL_SFX;

	// like drawChars, except uses generic (platform specific) input
	// the default implementation simply maps to drawChars and needs
	// to be replaced by platform code
	virtual void prepareToRenderChars(GR_RenderInfo & ri) VIRTUAL_SFX;
	virtual void renderChars(GR_RenderInfo & ri) VIRTUAL_SFX;

	virtual void appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const VIRTUAL_SFX;
	virtual void measureRenderedCharWidths(GR_RenderInfo & ri) VIRTUAL_SFX;

	// expects ri.m_iOffset set to the run offset condsidered for break
	//         ri.m_pText set positioned at start of the run
	//         represented by ri, its uper limit set appropriately
	//         ri.m_iLength is set to the run length
	// iNext -- if break is not possible at the given offset, the
	//          class might return the next possible break offset in
	//          iNext, relative to start of the text represented by ri
	//          (not relative to m_iOffset); if the class does not
	//          know where the next break point lies, it should set
	//          iNext to -1; if it knows that there is no break in this run, it should set
	//          iNext to -2
	// bAfter indicates whether we are quering for a break after the character at given offset

	virtual bool canBreak(GR_RenderInfo & ri, UT_sint32 &iNext, bool bAfter) VIRTUAL_SFX;

	// indicates if special caret positioning has to be done for the run of text; this allows us
	// to speed things up when this is not needed
	virtual bool needsSpecialCaretPositioning(GR_RenderInfo & /*ri*/) VIRTUAL_SFX {return false;}

	// adjusts caret position if given script restricts where caret can be placed
	// the caller has to set initial position within the run in ri.m_iOffset, overall length of
	// the run in ri.m_iLength and provide a text iterator over the text of the run in ri.m_pText
	//
	// return value is the adjusted offset
	// the default implementation simply returns the passed value
	virtual UT_uint32 adjustCaretPosition(GR_RenderInfo & ri, bool bForward) VIRTUAL_SFX;

	// Adjusts position for delete if given script restricts deletion to character clusters.
	// The caller has to set initial position within the run in ri.m_iOffset, overall length to be
	// deleted in ri.m_iLength and provide a text iterator over the text of the run in ri.m_pText
	// on return ri.m_iOffset contains the adjusted (run-relative) position and ri.m_iLength the count
	// the adjusted length of the delete
	//
	// the default implementation simply returns the passed value
	virtual void adjustDeletePosition(GR_RenderInfo & ri) VIRTUAL_SFX;

	// the AbiWord line breaking was designed looking for breaks at the right edge of a character,
	// i.e., the character that can break is included with the left part of the split run.
	// the Uniscribe library, however, holds breaking info for left edge, and sometimes it is useful
	// to know what system we are dealing with.
	virtual bool nativeBreakInfoForRightEdge() VIRTUAL_SFX {return true;}

	virtual UT_sint32 resetJustification(GR_RenderInfo & ri, bool bPermanent) VIRTUAL_SFX;
	virtual UT_sint32 countJustificationPoints(const GR_RenderInfo & ri) const VIRTUAL_SFX;
	virtual void justify(GR_RenderInfo & ri) VIRTUAL_SFX;

    virtual UT_uint32 XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const VIRTUAL_SFX;
    virtual void      positionToXY(const GR_RenderInfo & ri,
								   UT_sint32& x, UT_sint32& y,
								   UT_sint32& x2, UT_sint32& y2,
								   UT_sint32& height, bool& bDirection) const VIRTUAL_SFX;

	// FIXME: this method should return a larger integer (or a floating point number)
	// because it might overflow in some cases, see bug 13709
	virtual UT_sint32 getTextWidth(GR_RenderInfo & ri) VIRTUAL_SFX;

	// should be overriden by any classes implemented as plugins
	// NB: you must not use s_Version to store the version of derrived
	// classes, but have your own static variable for the derrived
	// class !!!
	virtual const UT_VersionInfo & getVersion() const {UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED ); return s_Version;}
	UT_uint32         getPaintCount(void) const
		{ return  m_paintCount;}

	/* all drawing should happen between calls to these two functions. this
	 * arranges for cairo contexts to be created/destroyed etc.  if you don't
	 * call these functions, bad things can and will happen */
	void beginPaint ();
	void endPaint ();

	static GR_Graphics* newNullGraphics();

 protected:

	GR_Graphics();
	GR_Caret *        getCaret() { return m_pCaret; }

	// todo: make these pure virtual
	virtual void _beginPaint () {}
	virtual void _endPaint () {}

	UT_sint32         _tduY(UT_sint32 layoutUnits) const;
	UT_sint32         _tduR(UT_sint32 layoutUnits) const;

	void _destroyFonts ();

	virtual GR_Font* _findFont(const char* pszFontFamily,
							   const char* pszFontStyle,
							   const char* pszFontVariant,
							   const char* pszFontWeight,
							   const char* pszFontStretch,
							   const char* pszFontSize,
							   const char* pszLang) = 0;

	// only called by GR_Painter
	virtual void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2) = 0;
#if XAP_DONTUSE_XOR
#else
	virtual void xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2) = 0;
#endif
	virtual void invertRect(const UT_Rect* pRect) = 0;
#if XAP_DONTUSE_XOR
#else
	void xorRect(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	void xorRect(const UT_Rect& r);
#endif

	virtual void fillRect(GR_Image *pImg, const UT_Rect &src, const UT_Rect & dest);
	virtual void fillRect(const UT_RGBColor& c, const UT_Rect &r);
	virtual void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
						  UT_sint32 w, UT_sint32 h) = 0;

	virtual void clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h) = 0;
	virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	virtual void fillRect(GR_Color3D c, UT_Rect &r) = 0;
	virtual void fillRect(GR_Color3D c,
						  UT_sint32 x, UT_sint32 y,
						  UT_sint32 w, UT_sint32 h) = 0;
	virtual void polygon(const UT_RGBColor& c, const UT_Point *pts, UT_uint32 nPoints);
	virtual void polyLine(const UT_Point * pts, UT_uint32 nPoints) = 0;
	virtual void drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff) = 0;
	virtual void drawChars(const UT_UCSChar* pChars,
						   int iCharOffset,
						   int iLength,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   int* pCharWidths = NULL) = 0;

	virtual void drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
											 int iCharOffset,
											 int iLength,
											 UT_sint32 xoff,
											 UT_sint32 yoff,
											 int* pCharWidths = NULL);

	virtual GR_Image *	  genImageFromRectangle(const UT_Rect & r) = 0;

 private:
	virtual bool _setTransform(const GR_Transform & /*tr*/)
		{
			UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
			return false;
		}

 public:
	// TODO -- this should not be public, create access methods !!!
	//
	// Postscript context positions graphics wrt top of current PAGE, NOT
	// wrt top of document. The screen graphics engine, though positions
	// graphics wrt the top of the document, therefore if we are printing
	// page 5 we need to adjust the vertical position of the graphic in the
	// postscript image printing routine by (current_page_number-1) * page_height
	// I'm going to call this variable m_iRasterPosition, for want of a better name,
	// it's not acutally a rasterposition --- any better names would be a good idea,
	// I jusy can't think of one right now.
	UT_uint32         m_iRasterPosition;

 protected:
	UT_uint32	      m_iZoomPercentage;
	UT_uint32         m_iFontAllocNo;

	static XAP_PrefsScheme *m_pPrefsScheme;
	static UT_uint32 m_uTick;

	const UT_Rect *  m_pRect;

	bool m_bHave3DColors;

	UT_uint32 m_paintCount;

	// Double buffering infrastructure.

	// The default implementation here leads to no double buffering,
	// as they perform no action at all. Should be overriden in derived
	// classes
	virtual void _DeviceContext_SwitchToBuffer() { };
	virtual void _DeviceContext_SwitchToScreen() { };

	// returns the token for the current call
	bool beginDoubleBuffering();

	// does the actual buffer-to-screen switch only when it gets
	// the correct token
	void endDoubleBuffering(bool token);

	// SUSPEND / RESUME drawings infrastructure
	// Drawing code (through gr_Graphics) will have no effect between SUSPEND - RESUME.
	// The default implementation does not suspend anything
	// (ie. changes are still taking effect)

	virtual void _DeviceContext_SuspendDrawing() { };
	virtual void _DeviceContext_ResumeDrawing() { };

	bool suspendDrawing();
	void resumeDrawing(bool token);

	// Device context switch management
	bool m_bDoubleBufferingActive;
	bool m_bDrawingSuspended;

	enum DeviceContextSwitchType {
		SWITCHED_TO_BUFFER = 0,
		DRAWING_SUSPENDED
	};

	UT_NumberStack m_DCSwitchManagementStack;

 private:
	GR_Caret *		 m_pCaret;
    bool             _PtInPolygon(const UT_Point * pts, UT_uint32 nPoints, UT_sint32 x,UT_sint32 y);
    bool             m_bIsPortrait;
	bool             m_bSpawnedRedraw;
	UT_Rect          m_PendingExposeArea;
	UT_Rect          m_RecentExposeArea;
	bool             m_bExposePending;
	bool             m_bIsExposedAreaAccessed;
	bool             m_bDontRedraw;
	bool             m_bDoMerge;
//
// These hold the previous x and Y offset calculated from the scrolling code
// in Logical units. We need them to avoid off by 1 errors in scrolling.
//
	UT_sint32        m_iPrevYOffset;
	UT_sint32        m_iPrevXOffset;
	GR_Transform     m_Transform;

	typedef std::map<std::string, GR_Font*> FontCache;
	FontCache  m_hashFontCache;

	static UT_VersionInfo   s_Version;
	static UT_uint32        s_iInstanceCount;
	static UT_UCS4Char      s_cDefaultGlyph;
	UT_GenericVector<GR_Caret *>  m_vecCarets;
	AllCarets               m_AllCarets;
	bool                    m_bAntiAliasAlways;
};

#endif /* GR_GRAPHICS_H */
