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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gstdio.h>
#include <io.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_path.h"
#include "ut_debugmsg.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_PrintPreview.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "gr_Win32Graphics.h"
#include "ut_rand.h"
#include "ut_Win32OS.h"

#include "fp_PageSize.h"
#include "fv_View.h"

#define INITGUID
#include <ole2.h>

#define GDIPCONST const
#define WINGDIPAPI __stdcall

typedef int GpStatus;
typedef int PixelFormat;

typedef void GpImage;
typedef void GpBitmap;
typedef void GpMetafile;
typedef void GpGraphics;

struct _GdiplusStartupInput
{
    guint32 GdiplusVersion;
    gpointer DebugEventCallback;
    gboolean SuppressBackgroundThread;
    gboolean SuppressExternalCodecs;
};
typedef struct _GdiplusStartupInput GdiplusStartupInput;

struct EncoderParameter
{
  GUID    Guid;
  ULONG   NumberOfValues;
  ULONG   Type;
  VOID*   Value;
};

struct EncoderParameters
{
  UINT Count;                      /* Number of parameters in this structure */
  EncoderParameter Parameter[1];   /* Parameter values */
};

typedef enum {
  EncoderParameterValueTypeByte = 1,
  EncoderParameterValueTypeASCII = 2,
  EncoderParameterValueTypeShort = 3,
  EncoderParameterValueTypeLong = 4,
  EncoderParameterValueTypeRational = 5,
  EncoderParameterValueTypeLongRange = 6,
  EncoderParameterValueTypeUndefined = 7,
  EncoderParameterValueTypeRationalRange = 8,
  EncoderParameterValueTypePointer = 9
} EncoderParameterValueType;

enum EncoderValue
  {
    EncoderValueColorTypeCMYK,
    EncoderValueColorTypeYCCK,
    EncoderValueCompressionLZW,
    EncoderValueCompressionCCITT3,
    EncoderValueCompressionCCITT4,
    EncoderValueCompressionRle,
    EncoderValueCompressionNone,
    EncoderValueScanMethodInterlaced,
    EncoderValueScanMethodNonInterlaced,
    EncoderValueVersionGif87,
    EncoderValueVersionGif89,
    EncoderValueRenderProgressive,
    EncoderValueRenderNonProgressive,
    EncoderValueTransformRotate90,
    EncoderValueTransformRotate180,
    EncoderValueTransformRotate270,
    EncoderValueTransformFlipHorizontal,
    EncoderValueTransformFlipVertical,
    EncoderValueMultiFrame,
    EncoderValueLastFrame,
    EncoderValueFlush,
    EncoderValueFrameDimensionTime,
    EncoderValueFrameDimensionResolution,
    EncoderValueFrameDimensionPage
  };

#define    PixelFormatIndexed      0x00010000 // Indexes into a palette
#define    PixelFormatGDI          0x00020000 // Is a GDI-supported format
#define    PixelFormatAlpha        0x00040000 // Has an alpha component
#define    PixelFormatPAlpha       0x00080000 // Pre-multiplied alpha
#define    PixelFormatExtended     0x00100000 // Extended color 16 bits/channel
#define    PixelFormatCanonical    0x00200000 

#define    PixelFormatUndefined       0
#define    PixelFormatDontCare        0

#define    PixelFormat1bppIndexed     (1 | ( 1 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat4bppIndexed     (2 | ( 4 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat8bppIndexed     (3 | ( 8 << 8) | PixelFormatIndexed | PixelFormatGDI)
#define    PixelFormat16bppGrayScale  (4 | (16 << 8) | PixelFormatExtended)
#define    PixelFormat16bppRGB555     (5 | (16 << 8) | PixelFormatGDI)
#define    PixelFormat16bppRGB565     (6 | (16 << 8) | PixelFormatGDI)
#define    PixelFormat16bppARGB1555   (7 | (16 << 8) | PixelFormatAlpha | PixelFormatGDI)
#define    PixelFormat24bppRGB        (8 | (24 << 8) | PixelFormatGDI)
#define    PixelFormat32bppRGB        (9 | (32 << 8) | PixelFormatGDI)
#define    PixelFormat32bppARGB       (10 | (32 << 8) | PixelFormatAlpha | PixelFormatGDI | PixelFormatCanonical)
#define    PixelFormat32bppPARGB      (11 | (32 << 8) | PixelFormatAlpha | PixelFormatPAlpha | PixelFormatGDI)
#define    PixelFormat48bppRGB        (12 | (48 << 8) | PixelFormatExtended)
#define    PixelFormat64bppARGB       (13 | (64 << 8) | PixelFormatAlpha  | PixelFormatCanonical | PixelFormatExtended)
#define    PixelFormat64bppPARGB      (14 | (64 << 8) | PixelFormatAlpha  | PixelFormatPAlpha | PixelFormatExtended)

typedef DWORD ARGB;

typedef GpStatus (WINGDIPAPI* GdiplusStartupFunc) (gpointer, const gpointer, gpointer);
typedef GpStatus (WINGDIPAPI* GdipDisposeImageFunc) (GpImage*);
typedef GpStatus (WINGDIPAPI* GdipSaveImageToFileFunc) (GpImage *image, GDIPCONST WCHAR* filename, 
							GDIPCONST CLSID* clsidEncoder, 
							GDIPCONST EncoderParameters* encoderParams);
typedef GpStatus (WINGDIPAPI* GdipLoadImageFromFileFunc) (GDIPCONST WCHAR* filename, GpImage **image);
typedef GpStatus (WINGDIPAPI* GdipSaveAddFunc) (GpImage *image, GDIPCONST EncoderParameters* encoderParams);
typedef GpStatus (WINGDIPAPI* GdipSaveAddImageFunc) (GpImage *orig, GpImage *image, 
						     GDIPCONST EncoderParameters* encoderParams);
typedef GpStatus (WINGDIPAPI* GdipCreateMetafileFromEmfFunc) (HENHMETAFILE hEmf, BOOL deleteEmf, GpMetafile **image);
typedef GpStatus (WINGDIPAPI* GdipCreateMetafileFromFileFunc) (GDIPCONST WCHAR* file, GpMetafile **metafile);
typedef GpStatus (WINGDIPAPI* GdipCreateBitmapFromFileFunc) (GDIPCONST WCHAR* filename, GpBitmap **bitmap);
typedef GpStatus (WINGDIPAPI* GdipDeleteGraphicsFunc) (GpGraphics *graphics);
typedef GpStatus (WINGDIPAPI* GdipCreateBitmapFromScan0Func) (INT width, INT height, INT stride, PixelFormat format, BYTE* scan0,
                                                              GpBitmap** bitmap);
typedef GpStatus (WINGDIPAPI* GdipGetImageWidthFunc) (GpImage*, guint*);
typedef GpStatus (WINGDIPAPI* GdipGetImageHeightFunc) (GpImage*, guint*);
typedef GpStatus (WINGDIPAPI* GdipDrawImageIFunc) (GpGraphics *graphics, GpImage *image, INT x, INT y);
typedef GpStatus (WINGDIPAPI* GdipGetImageGraphicsContextFunc) (GpImage *image, GpGraphics **graphics);
typedef GpStatus (WINGDIPAPI* GdipFlushFunc) (GpGraphics *graphics, INT intention);
typedef GpStatus (WINGDIPAPI* GdipGraphicsClearFunc) (GpGraphics *graphics, ARGB color);
typedef GpStatus (WINGDIPAPI* GdipBitmapSetResolutionFunc) (GpBitmap* bitmap, float xdpi, float ydpi);
typedef GpStatus (WINGDIPAPI* GdipGetImageHorizontalResolutionFunc) (GpImage *image, float *resolution);
typedef GpStatus (WINGDIPAPI* GdipGetImageVerticalResolutionFunc) (GpImage *image, float *resolution);

static GdiplusStartupFunc GdiplusStartup = 0;
static GdipDisposeImageFunc GdipDisposeImage = 0;
static GdipSaveImageToFileFunc GdipSaveImageToFile = 0;
static GdipLoadImageFromFileFunc GdipLoadImageFromFile = 0;
static GdipSaveAddImageFunc GdipSaveAddImage = 0;
static GdipSaveAddFunc GdipSaveAdd = 0;
static GdipCreateMetafileFromEmfFunc GdipCreateMetafileFromEmf = 0;
static GdipCreateMetafileFromFileFunc GdipCreateMetafileFromFile = 0;
static GdipCreateBitmapFromFileFunc GdipCreateBitmapFromFile = 0;
static GdipDeleteGraphicsFunc GdipDeleteGraphics = 0;
static GdipCreateBitmapFromScan0Func GdipCreateBitmapFromScan0 = 0;
static GdipGetImageWidthFunc GdipGetImageWidth = 0;
static GdipGetImageHeightFunc GdipGetImageHeight = 0;
static GdipDrawImageIFunc GdipDrawImageI = 0;
static GdipGetImageGraphicsContextFunc GdipGetImageGraphicsContext = 0;
static GdipFlushFunc GdipFlush = 0;
static GdipGraphicsClearFunc GdipGraphicsClear = 0;
static GdipBitmapSetResolutionFunc GdipBitmapSetResolution = 0;
static GdipGetImageHorizontalResolutionFunc GdipGetImageHorizontalResolution = 0;
static GdipGetImageVerticalResolutionFunc GdipGetImageVerticalResolution = 0;

static GDIPCONST CLSID tiff_clsid = { 0x557cf405, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } };
static GDIPCONST GUID EncoderSaveFlag = { 0x292266FC, 0xac40, 0x47bf, { 0x8c, 0xfc, 0xa8, 0x5b, 0x89, 0xa6,0x55, 0xde } };

#if 0
#define d(x) G_STMT_START { (x); } G_STMT_END
#else
#define d(x) G_STMT_START { } G_STMT_END
#endif

static GpStatus
gdip_init (void)
{
  GdiplusStartupInput input;
  ULONG_PTR gdiplusToken = 0;
  static HINSTANCE gdipluslib = NULL;

  if (!gdipluslib)
    gdipluslib = LoadLibraryW (L"gdiplus.dll");
  else
    return 0; /* gdip_init() is idempotent */

  if (!gdipluslib)
    return 18; // GdiplusNotInitialized 

#define LOOKUP(func) \
  G_STMT_START { \
    func = (func##Func) GetProcAddress (gdipluslib, #func); \
    if (!func) {\
      g_warning ("Couldn't load function: %s\n", #func); \
      return 18; \
    } \
  } G_STMT_END

  LOOKUP (GdiplusStartup);
  LOOKUP (GdipDisposeImage);
  LOOKUP (GdipSaveImageToFile);
  LOOKUP (GdipLoadImageFromFile);
  LOOKUP (GdipSaveAddImage);
  LOOKUP (GdipSaveAdd);
  LOOKUP (GdipCreateMetafileFromEmf);
  LOOKUP (GdipCreateMetafileFromFile);
  LOOKUP (GdipCreateBitmapFromFile);
  LOOKUP (GdipCreateBitmapFromScan0);
  LOOKUP (GdipDeleteGraphics);
  LOOKUP (GdipGetImageWidth);
  LOOKUP (GdipGetImageHeight);
  LOOKUP (GdipDrawImageI);
  LOOKUP (GdipGetImageGraphicsContext);
  LOOKUP (GdipFlush);
  LOOKUP (GdipGraphicsClear);
  LOOKUP (GdipBitmapSetResolution);
  LOOKUP (GdipGetImageHorizontalResolution);
  LOOKUP (GdipGetImageVerticalResolution);

#undef LOOKUP

  input.GdiplusVersion = 1;
  input.DebugEventCallback = NULL;
  input.SuppressBackgroundThread = input.SuppressExternalCodecs = FALSE;
  
  return GdiplusStartup (&gdiplusToken, &input, NULL);
}

#define ASSERT_GDIP_SUCCESS(status, func) \
  G_STMT_START { \
    GpStatus _gdip_status = (status); \
    if (_gdip_status != 0) { \
      g_warning ("%s failed with reason %d\n", #func, _gdip_status); \
    } \
  } G_STMT_END

class ABI_EXPORT GR_Win32PrintPreviewGraphics : public GR_Win32Graphics
{
public:

  GR_Win32PrintPreviewGraphics(const RECT &rect)
    : GR_Win32Graphics(createbestmetafilehdc(), getDocInfo()),
      m_pGraphics(0), 
      page_rect(rect),
      metafile_dc(0), 
      m_multiPageTiff(0),
      m_tiffFilename(0)
  {
    GpStatus status = gdip_init ();
    ASSERT_GDIP_SUCCESS (status, gdip_init);
    if (status != 0)
      throw "GDI+ initialization failed";

    d (g_print ("gdip_init() was successful. preview graphics created\n"));

    metafile_xres = GetDeviceCaps(getPrimaryDC(), LOGPIXELSX);
    metafile_yres = GetDeviceCaps(getPrimaryDC(), LOGPIXELSY);

    d (g_print ("xres: %d | yres: %d\n", metafile_xres, metafile_yres));
  }

  virtual ~GR_Win32PrintPreviewGraphics()
  {
    d (g_print ("~GR_Win32PrintPreviewGraphics()\n"));
	if (m_multiPageTiff)
      GdipDisposeImage (m_multiPageTiff);
    g_free(m_tiffFilename);

    // todo: delete DC and docInfo
  }

  virtual UT_uint32 getClassId() {return s_getClassId();}
  
  virtual GR_Capability getCapability() {return GRCAP_PRINTER_ONLY;}
  
  static const char * graphicsDescriptor(){return "Win32 Print Preview";}

  virtual void drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("drawGlyph()\n"));
    m_pGraphics->drawGlyph(glyph_idx, xoff, yoff);
  }

  virtual void drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("drawChar()\n"));
    m_pGraphics->drawChar(Char, xoff, yoff);
  }

  virtual void drawChars(const UT_UCSChar* pChars,
			 int iCharOffset, int iLength,
			 UT_sint32 xoff, UT_sint32 yoff,
			 int * pCharWidth)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("drawChars()\n"));
    m_pGraphics->drawChars(pChars, iCharOffset, iLength, xoff, yoff, pCharWidth);
  }

  virtual void setFont(const GR_Font* pFont)
  {
    if (m_pGraphics) {
      m_pGraphics->setFont(pFont);
      d (g_print ("setFont()\n"));
    }
    else
      GR_Win32Graphics::setFont(pFont);
  }

  virtual void clearFont(void)
  {
    if (m_pGraphics) {
      m_pGraphics->clearFont();
      d (g_print ("clearFont()\n"));
    }
    else
      GR_Win32Graphics::clearFont();
  }

  virtual void setColor(const UT_RGBColor& clr)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("setColor()\n"));
    m_pGraphics->setColor(clr);
  }

  virtual void getColor(UT_RGBColor& clr)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("getColor()\n"));
    m_pGraphics->getColor(clr);
  }
  
  virtual void drawLine(UT_sint32 a, UT_sint32 b, UT_sint32 c, UT_sint32 d)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("drawLine()\n"));
    m_pGraphics->drawLine(a, b, c, d);
  }

  virtual void xorLine(UT_sint32 a, UT_sint32 b, UT_sint32 c, UT_sint32 d)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("xorLine()\n"));
    m_pGraphics->xorLine(a, b, c, d);
  }

  virtual void setLineWidth(UT_sint32 w)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("setLineWidth()\n"));
    m_pGraphics->setLineWidth(w);
  }
  
  virtual void setLineProperties ( double inWidthPixels,
				   JoinStyle inJoinStyle = JOIN_MITER,
				   CapStyle inCapStyle   = CAP_BUTT,
				   LineStyle inLineStyle = LINE_SOLID )
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("setLineProperties()\n"));
    m_pGraphics->setLineProperties(inWidthPixels, inJoinStyle, inCapStyle, inLineStyle);
  }

  virtual void polyLine(const UT_Point * pts, UT_uint32 nPoints)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("polyLine()\n"));
    m_pGraphics->polyLine(pts, nPoints);
  }

  virtual void fillRect(const UT_RGBColor& c,
			UT_sint32 x, UT_sint32 y,
			UT_sint32 w, UT_sint32 h)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("fillRect()\n"));
    m_pGraphics->fillRect(c, x, y, w, h);
  }

  virtual void invertRect(const UT_Rect* pRect)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("invertRect()\n"));
    m_pGraphics->invertRect(pRect);
  }

  virtual void setClipRect(const UT_Rect* pRect)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("setClipRect()\n"));
    m_pGraphics->setClipRect(pRect);
  }

  virtual void clearArea(UT_sint32 a, UT_sint32 b, UT_sint32 c, UT_sint32 d)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("clearArea()\n"));
    m_pGraphics->clearArea(a, b, c, d);
  }

  virtual GR_Image* createNewImage(const char* pszName, const UT_ByteBuf* pBB, const std::string& mimetype,
				   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
				   GR_Image::GRType iType)
  {    
    d (g_print ("createNewImage()\n"));
    if (m_pGraphics)
      return m_pGraphics->createNewImage(pszName, pBB, mimetype, iDisplayWidth, iDisplayHeight, iType);
    else
      return GR_Win32Graphics::createNewImage(pszName, pBB, mimetype, iDisplayWidth, iDisplayHeight, iType);
  }
  
  virtual void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("drawImage()\n"));
    m_pGraphics->drawImage(pImg, xDest, yDest);
  }
  
  virtual bool queryProperties(GR_Graphics::Properties gp) const
  {
    d (g_print ("queryProperties()\n"));
    return (gp == DGP_PAPER);
  }
  
  virtual void polygon(const UT_RGBColor& c, const UT_Point *pts, UT_uint32 nPoints)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("polygon()\n"));
    m_pGraphics->polygon(c, pts, nPoints);
  }

  virtual GR_Image * genImageFromRectangle(const UT_Rect & r)
  {
    UT_return_val_if_fail(m_pGraphics != 0, 0);
    d (g_print ("genImageFromRectangle()\n"));
    return m_pGraphics->genImageFromRectangle(r);
  }

  virtual void saveRectangle(UT_Rect & r, UT_uint32 iIndx)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("saveRectangle()\n"));
    m_pGraphics->saveRectangle(r, iIndx);
  }

  virtual void restoreRectangle(UT_uint32 iIndx)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("restoreRectangle()\n"));
    m_pGraphics->restoreRectangle(iIndx);
  }

  virtual void flush(void)
  {
    UT_return_if_fail(m_pGraphics != 0);
    d (g_print ("flush()\n"));
    m_pGraphics->flush();
  }

  virtual bool startPrint(void)
  {
    std::string sName = UT_createTmpFile("pr", ".tif");

    d (g_print ("saving to %s\n", sName.c_str()));
    m_tiffFilename = g_utf8_to_utf16 (sName.c_str(), -1, NULL, NULL, NULL);

    return true;
  }

  virtual bool startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
			 bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
  {
    endPage(); // prints any previous pages and clears the font cache

    d (g_print ("startPage(%d)\n", pageNumber));

    metafile_dc = CreateEnhMetaFileW (getPrimaryDC(), NULL, &page_rect, L"AbiWord\0Print Preview\0\0");
    
    GR_Win32AllocInfo ai(metafile_dc, m_pDocInfo, NULL);
    m_pGraphics = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);

    return true;
  }

  virtual bool endPrint(void)
  {
    endPage();

    EncoderParameters encoder_params;
    ULONG parameterValue = EncoderValueFlush;

    encoder_params.Count = 1;
    encoder_params.Parameter[0].Guid = EncoderSaveFlag;
    encoder_params.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoder_params.Parameter[0].NumberOfValues = 1;
    encoder_params.Parameter[0].Value = &parameterValue;

    GpStatus status = GdipSaveAdd (m_multiPageTiff, &encoder_params);
    ASSERT_GDIP_SUCCESS (status, GdipSaveAdd);

    status = GdipDisposeImage (m_multiPageTiff);
    m_multiPageTiff = 0;
    ASSERT_GDIP_SUCCESS (status, GdipDisposeImage);

    d (g_print("endPrint()\n"));

    ShellExecuteW (NULL, L"open", (WCHAR *)m_tiffFilename, NULL, NULL, SW_SHOW);

    return true;
  }

  void endPage()
  {
    invalidateCache(); // clear the font cache

    d (g_print ("endPage()\n"));
    if (metafile_dc)
      {
	GpStatus status;
	HENHMETAFILE metafile;
	
	metafile = CloseEnhMetaFile (metafile_dc);
	d (g_print ("metafile = %p\n", metafile));

	// must be a Bitmap for the multi-page stuff to work
	GpImage *meta;

	// jump through hoops to convert a metafile to a bitmap
	// saveaddimage() won't work if the base image or the
	// new image is a metafile.
	GpImage *m;
	status = GdipCreateMetafileFromEmf(metafile, TRUE, &m);
	ASSERT_GDIP_SUCCESS (status, GdipCreateMetafileFromEmf);
	
	guint width, height;
	GdipGetImageWidth(m, &width);
	GdipGetImageHeight(m, &height);

	d (g_print ("w: %d | h: %d\n", width, height));

	status = GdipCreateBitmapFromScan0 (width, height, 0, PixelFormat32bppARGB, NULL, &meta);
	ASSERT_GDIP_SUCCESS (status, GdipCreateBitmapFromScan0);

	status = GdipBitmapSetResolution (meta, metafile_xres, metafile_yres);
	ASSERT_GDIP_SUCCESS (status, GdipBitmapSetResolution);

	GpGraphics *g;
	status = GdipGetImageGraphicsContext (meta, &g);
	ASSERT_GDIP_SUCCESS (status, GdipGetImageGraphicsContext);
	
	// gotta clear the bitmap
	status = GdipGraphicsClear(g, 0xffffffff);
	ASSERT_GDIP_SUCCESS (status, GdipGraphicsClear);
	
	status = GdipDrawImageI(g, m, 0, 0);
	ASSERT_GDIP_SUCCESS (status, GdipDrawImage);
	
	status = GdipFlush(g, 1);
	ASSERT_GDIP_SUCCESS (status, GdipFlush);
	
	GdipDeleteGraphics(g);
	GdipDisposeImage(m);

	if (!m_multiPageTiff)
	  {
	    EncoderParameters encoder_params;
	    ULONG parameterValue = EncoderValueMultiFrame;

	    encoder_params.Count = 1;
	    encoder_params.Parameter[0].Guid = EncoderSaveFlag;
	    encoder_params.Parameter[0].Type = EncoderParameterValueTypeLong;
	    encoder_params.Parameter[0].NumberOfValues = 1;
	    encoder_params.Parameter[0].Value = &parameterValue;

	    status = GdipSaveImageToFile (meta, (WCHAR *)m_tiffFilename, &tiff_clsid, &encoder_params);
	    ASSERT_GDIP_SUCCESS (status, GdipSaveImageToFile);

	    d (g_print ("Saved EMF to file\n"));

	    m_multiPageTiff = meta;
	  }
	else
	  {
	    EncoderParameters encoder_params;
	    ULONG parameterValue = EncoderValueFrameDimensionPage;

	    encoder_params.Count = 1;
	    encoder_params.Parameter[0].Guid = EncoderSaveFlag;
	    encoder_params.Parameter[0].Type = EncoderParameterValueTypeLong;
	    encoder_params.Parameter[0].NumberOfValues = 1;
	    encoder_params.Parameter[0].Value = &parameterValue;

	    status = GdipSaveAddImage (m_multiPageTiff, meta, &encoder_params);
	    ASSERT_GDIP_SUCCESS (status, GdipSaveAddImage);

	    d (g_print ("SaveAddImage\n"));

	    GdipDisposeImage (meta);
	  }

	metafile_dc = 0;
	DELETEP(m_pGraphics);
      }
  }

protected:

  virtual GR_Font* _findFont(const char* pszFontFamily,
			     const char* pszFontStyle,
			     const char* pszFontVariant,
			     const char* pszFontWeight,
			     const char* pszFontStretch,
			     const char* pszFontSize,
			     const char* pszLang)
  {
    if (!m_pGraphics)
      return GR_Win32Graphics::_findFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize, pszLang);
    else
      return m_pGraphics->findFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight, pszFontStretch, pszFontSize, pszLang);
  }

  GR_Win32Graphics *m_pGraphics;
  RECT page_rect;
  HDC metafile_dc;
  GpBitmap *m_multiPageTiff;
  gunichar2 * m_tiffFilename;
  int metafile_xres;
  int metafile_yres;
};

XAP_Win32Dialog_PrintPreview::XAP_Win32Dialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_PrintPreview(pDlgFactory,id), m_pPrintGraphics(0), m_emfFilename(0)
{
}

XAP_Win32Dialog_PrintPreview::~XAP_Win32Dialog_PrintPreview(void)
{
  DELETEP(m_pPrintGraphics);
  g_free(m_emfFilename);
}

XAP_Dialog * XAP_Win32Dialog_PrintPreview::static_constructor(XAP_DialogFactory * pFactory,
							      XAP_Dialog_Id id)
{
  return new XAP_Win32Dialog_PrintPreview (pFactory,id);
}

GR_Graphics * XAP_Win32Dialog_PrintPreview::getPrinterGraphicsContext(void)
{
  return m_pPrintGraphics;
}

void XAP_Win32Dialog_PrintPreview::releasePrinterGraphicsContext(GR_Graphics * /*pGraphics*/)
{
  DELETEP(m_pPrintGraphics);
}

void XAP_Win32Dialog_PrintPreview::runModal(XAP_Frame * pFrame)
{
  RECT rect;

  FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());

  rect.left = 0;
  rect.right = (LONG)(pView->getPageSize().Width (DIM_MM) * 100);
  rect.top = 0;
  rect.bottom = (LONG)(pView->getPageSize().Height (DIM_MM) * 100);

  try
    {
      m_pPrintGraphics = new GR_Win32PrintPreviewGraphics(rect);
    }
  catch (...)
    {
      m_pPrintGraphics = 0;
    }
}
