/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
#define abi_plugin_register abipgn_wmf_register
#define abi_plugin_unregister abipgn_wmf_unregister
#define abi_plugin_supports_version abipgn_wmf_supports_version
#endif

#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "ie_impGraphic_WMF.h"

#include <stdio.h>
#include <math.h>

#include <libwmf/api.h>
#include <libwmf/gd.h>
#include <libwmf/svg.h>

static int  AbiWord_WMF_read (void * context);
static int  AbiWord_WMF_seek (void * context,long pos);
static long AbiWord_WMF_tell (void * context);
static int  AbiWord_WMF_function (void * context,char * buffer,int length);

typedef struct _bbuf_read_info  bbuf_read_info;
typedef struct _bbuf_write_info bbuf_write_info;

struct _bbuf_read_info
{
	UT_ByteBuf* pByteBuf;

	UT_uint32 len;
	UT_uint32 pos;
};

struct _bbuf_write_info
{
	UT_ByteBuf* pByteBuf;
};

#define WMF2SVG_MAXPECT (1 << 0)
#define WMF2SVG_MAXSIZE (1 << 1)

// supported suffixes
static IE_SuffixConfidence IE_ImpGraphicWMF_Sniffer__SuffixConfidence[] = {
	{ "wmf", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_ImpGraphicWMF_Sniffer::getSuffixConfidence ()
{
	return IE_ImpGraphicWMF_Sniffer__SuffixConfidence;
}

UT_Confidence_t IE_ImpGraphicWMF_Sniffer::recognizeContents(const char * /*szBuf*/, UT_uint32 /*iNumbytes*/)
{
	return ( UT_CONFIDENCE_POOR ); // Don't know how to recognize metafiles, so say yes
}

bool IE_ImpGraphicWMF_Sniffer::getDlgLabels(const char ** pszDesc,
					const char ** pszSuffixList,
					IEGraphicFileType * ft)
{
	*pszDesc = "Windows Metafile (.wmf)";
	*pszSuffixList = "*.wmf";
	*ft = getType ();
	return true;
}

UT_Error IE_ImpGraphicWMF_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
{
	*ppieg = new IE_ImpGraphic_WMF();
	if (*ppieg == 0)
		return UT_IE_NOMEMORY;

	return UT_OK;
}

// This creates our FG_Graphic object for a PNG
UT_Error IE_ImpGraphic_WMF::importGraphic(UT_ByteBuf* pBBwmf,
                                          FG_ConstGraphicPtr &pfg)
{
	UT_Error err = UT_OK;

	pfg.reset();
	UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Begin -\n"));

	bool importAsPNG = true;

#ifdef TOOLKIT_GTK_ALL
	importAsPNG = false;
#endif

	if (importAsPNG) {

		UT_ByteBuf * pBBpng = 0;

		err = convertGraphic(pBBwmf,&pBBpng);
		if (err != UT_OK) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Conversion failed...\n"));
			return err;
		}

		FG_GraphicRasterPtr pFGR(new FG_GraphicRaster);
		if(!pFGR) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Ins. Mem.\n"));
			err = UT_IE_NOMEMORY;
		}
		else if(!pFGR->setRaster_PNG(pBBpng)) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Fake type?\n"));
			err = UT_IE_FAKETYPE;
		}
		else {
			pfg = std::move(pFGR);
		}
	} else {
		UT_ByteBuf *svg = 0;
		err = convertGraphicToSVG(pBBwmf, &svg);
		if (err != UT_OK) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Conversion failed...\n"));
			return err;
		}

		FG_GraphicVectorPtr pFGR(new FG_GraphicVector);
		if(!pFGR) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Ins. Mem.\n"));
			err = UT_IE_NOMEMORY;
		}
		else if(!pFGR->setVector_SVG(svg)) {
			UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Fake type?\n"));
			err = UT_IE_FAKETYPE;
		}
		else {
			pfg = std::move(pFGR);
		}
	}

	UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic - End\n"));

	return err;
}

static int explicit_wmf_error (const char* str, wmf_error_t err)
{
	UT_UNUSED(str);
	switch (err)
	{
	case wmf_E_None:
		return 0;
	default:
		return 1;
	}
}

UT_Error IE_ImpGraphic_WMF::convertGraphicToSVG(UT_ByteBuf* pBBwmf, UT_ByteBuf** ppBB)
{
	int status = 0;

	unsigned int disp_width  = 0;
	unsigned int disp_height = 0;

	float wmf_width;
	float wmf_height;
	float ratio_wmf;
	float ratio_bounds;

	unsigned long flags;

	unsigned int max_width  = 768;
	unsigned int max_height = 512;
	unsigned long max_flags = 0;

	static const char* Default_Description = "wmf2svg";

	wmf_error_t err;

	wmf_svg_t* ddata = 0;

	wmfAPI* API = 0;
	wmfD_Rect bbox;

	wmfAPI_Options api_options;

	bbuf_read_info  read_info;

	char *stream = NULL;
	unsigned long stream_len = 0;

	*ppBB = 0;

	flags = 0;

	flags = WMF_OPT_IGNORE_NONFATAL | WMF_OPT_FUNCTION;
	api_options.function = wmf_svg_function;

	err = wmf_api_create (&API,flags,&api_options);
	status = explicit_wmf_error ("wmf_api_create",err);

	if (status)
	{	
		if (API) 
			wmf_api_destroy (API);
		return (UT_ERROR);
	}

	read_info.pByteBuf = pBBwmf;

	read_info.len = pBBwmf->getLength();
	read_info.pos = 0;

	err = wmf_bbuf_input (API,AbiWord_WMF_read,AbiWord_WMF_seek,AbiWord_WMF_tell,(void *) &read_info);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Bad input set\n"));
		goto ErrorHandler;
	}

	err = wmf_scan (API,0,&(bbox));
	status = explicit_wmf_error ("wmf_scan",err);

	if (status)
	{	
		goto ErrorHandler;
	}

/* Okay, got this far, everything seems cool.
 */
	ddata = WMF_SVG_GetData (API);

	ddata->out = wmf_stream_create(API, NULL);

	ddata->Description = (char *)Default_Description;

	ddata->bbox = bbox;

	wmf_display_size (API,&disp_width,&disp_height,72,72);

	wmf_width  = (float) disp_width;
	wmf_height = (float) disp_height;

	if ((wmf_width <= 0) || (wmf_height <= 0))
	{	fputs ("Bad image size - but this error shouldn't occur...\n",stderr);
		status = 1;
		wmf_api_destroy (API);
		return UT_ERROR;
	}

	if ((wmf_width  > (float) max_width )
	 || (wmf_height > (float) max_height))
	{	if (max_flags == 0) max_flags = WMF2SVG_MAXPECT;
	}

	if (max_flags == WMF2SVG_MAXPECT) /* scale the image */
	{	ratio_wmf = wmf_height / wmf_width;
		ratio_bounds = (float) max_height / (float) max_width;

		if (ratio_wmf > ratio_bounds)
		{	ddata->height = max_height;
			ddata->width  = (unsigned int) ((float) ddata->height / ratio_wmf);
		}
		else
		{	ddata->width  = max_width;
			ddata->height = (unsigned int) ((float) ddata->width  * ratio_wmf);
		}
	}
	else if (max_flags == WMF2SVG_MAXSIZE) /* bizarre option, really */
	{	ddata->width  = max_width;
		ddata->height = max_height;
	}
	else
	{	ddata->width  = (unsigned int) ceil ((double) wmf_width );
		ddata->height = (unsigned int) ceil ((double) wmf_height);
	}

	ddata->flags |= WMF_SVG_INLINE_IMAGES;

	ddata->flags |= WMF_GD_OUTPUT_MEMORY | WMF_GD_OWN_BUFFER;

	if (status == 0)
	{	err = wmf_play (API,0,&(bbox));
		status = explicit_wmf_error ("wmf_play",err);
	}

	wmf_stream_destroy(API, ddata->out, &stream, &stream_len);

	if (status == 0) 
	{
		UT_ByteBuf* pBB = new UT_ByteBuf;
		pBB->append((const UT_Byte*)stream, (UT_uint32)stream_len);
		*ppBB = pBB;
		DELETEP(pBBwmf);
		wmf_free(API, stream);
		wmf_api_destroy (API);
		return UT_OK;
	}

ErrorHandler:
	DELETEP(pBBwmf);
	if(API) 
	{
		if(stream) 
		{
			wmf_free(API, stream);
		}
		wmf_api_destroy (API);
	}
	return UT_ERROR;
}

UT_Error IE_ImpGraphic_WMF::convertGraphic(UT_ByteBuf* pBBwmf,
					   UT_ByteBuf** ppBBpng)
{
	UT_ByteBuf * pBBpng = 0;

	wmf_error_t err;

	wmf_gd_t * ddata = 0;

	wmfAPI * API = 0;
	wmfAPI_Options api_options;

	wmfD_Rect bbox;

	unsigned long flags;

#if 0 // the code that uses these two variables is in an if 0 block below
	unsigned int max_width  = 500;
	unsigned int max_height = 500;
#endif

	unsigned int width, height;

	bbuf_read_info  read_info;
	bbuf_write_info write_info;

   	if (!pBBwmf) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Bad Arg (1)\n"));
		return UT_ERROR;
	}
   	if (!ppBBpng) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Bad Arg (2)\n"));
		return UT_ERROR;
	}

	*ppBBpng = 0;

	flags = WMF_OPT_IGNORE_NONFATAL | WMF_OPT_FUNCTION;

	api_options.function = wmf_gd_function;

	err = wmf_api_create(&API,flags,&api_options);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic No API\n"));
		return UT_ERROR;
	}

	ddata = WMF_GD_GetData(API);
	if ((ddata->flags & WMF_GD_SUPPORTS_PNG) == 0) { // Impossible, but...
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic No PNG\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

	read_info.pByteBuf = pBBwmf;

	read_info.len = pBBwmf->getLength();
	read_info.pos = 0;

	err = wmf_bbuf_input (API,AbiWord_WMF_read,AbiWord_WMF_seek,AbiWord_WMF_tell,(void *) &read_info);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Bad input set\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

	err = wmf_scan (API,0,&bbox);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Scan failed\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

	/* TODO: be smarter about getting the resolution from screen 
	 */
	double resolution_x, resolution_y;
	resolution_x = resolution_y = 72.0;

	err = wmf_display_size (API, &width, &height, resolution_x, resolution_y);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Get size failed\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

	ddata->width  = (unsigned int) width;
	ddata->height = (unsigned int) height;

	if ((ddata->width == 0) || (ddata->height == 0)) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Size error (1)\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

#if 0
	// not sure if this branch is needed any more after the recent changes
	// done by FJF and myself inside of libWMF for better size detection - DAL

	if ((ddata->width >= max_width) || (ddata->height >= max_height)) {
		float ratio_wmf = height / width;
		float ratio_bounds = (float) max_height / (float) max_width;

		if (ratio_wmf > ratio_bounds) {
			ddata->height = max_height;
			ddata->width  = (unsigned int) ((float) ddata->height / ratio_wmf);
		}
		else {
			ddata->width  = max_width;
			ddata->height = (unsigned int) ((float) ddata->width  * ratio_wmf);
		}
	}
#endif

	if ((ddata->width == 0) || (ddata->height == 0)) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Size error (1)\n"));
		wmf_api_destroy(API);
		return UT_ERROR;
	}

	ddata->bbox = bbox;

	ddata->type = wmf_gd_png;

	pBBpng = new UT_ByteBuf;
	if (pBBpng == 0) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Ins. Mem.\n"));
		wmf_api_destroy(API);
		return UT_IE_NOMEMORY;
	}

	write_info.pByteBuf = pBBpng;

	ddata->flags |= WMF_GD_OUTPUT_MEMORY | WMF_GD_OWN_BUFFER;

	ddata->sink.context = (void *) &write_info;
	ddata->sink.function = AbiWord_WMF_function;

	err = wmf_play(API,0,&bbox);
	if (err != wmf_E_None) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Play failed\n"));
	}

	err = wmf_api_destroy(API);

	if (err == wmf_E_None) {
		*ppBBpng = pBBpng;
		return UT_OK;
	}

	UT_DEBUGMSG(("IE_ImpGraphic_WMF::convertGraphic Err. on destroy\n"));

	DELETEP(pBBpng);

	return UT_ERROR;
}

// returns unsigned char cast to int, or EOF
static int AbiWord_WMF_read (void * context)
{
	bbuf_read_info * info = (bbuf_read_info *) context;

	const UT_Byte* pByte = 0;

	if (info->pos == info->len)
		return EOF;

	pByte = info->pByteBuf->getPointer(info->pos);

	info->pos++;

	return (int) ((unsigned char) *pByte);
}

// returns (-1) on error, else 0
static int AbiWord_WMF_seek (void * context,long pos)
{
	bbuf_read_info * info = (bbuf_read_info *) context;

	info->pos = (UT_uint32) pos;

	return 0;
}

// returns (-1) on error, else pos
static long AbiWord_WMF_tell (void * context)
{
	bbuf_read_info * info = (bbuf_read_info *) context;

	return (long) info->pos;
}

static int AbiWord_WMF_function (void * context,char * buffer,int length)
{
	bbuf_write_info * info = (bbuf_write_info *) context;

	UT_Byte a_byte;

	int i = 0;

	while (i < length) {
		a_byte = (UT_Byte) ((unsigned char) buffer[i]); // why char I know not...
		if (!info->pByteBuf->append(&a_byte,1))
			break;
		i++;
	}

	return i;
}

/*******************************************************************/
/*******************************************************************/

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("WMF")

// we use a reference-counted sniffer
static IE_ImpGraphicWMF_Sniffer * m_impSniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_impSniffer)
	{
	  m_impSniffer = new IE_ImpGraphicWMF_Sniffer();
	}

	mi->name = "WMF Import Plugin";
	mi->desc = "Import Windows Metafiles";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_ImpGraphic::registerImporter (m_impSniffer);
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

	UT_ASSERT (m_impSniffer);

	IE_ImpGraphic::unregisterImporter (m_impSniffer);
	delete m_impSniffer;
	m_impSniffer = 0;

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
				 UT_uint32 /*release*/)
{
  return 1;
}

/*******************************************************************/
/*******************************************************************/
