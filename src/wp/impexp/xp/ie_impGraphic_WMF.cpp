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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "fg_GraphicRaster.h"
#include "ie_impGraphic_WMF.h"

#ifdef HAVE_LIBWMF

#include <stdio.h>

#include <libwmf/api.h>
#include <libwmf/gd.h>

extern "C" int  AbiWord_WMF_read (void * context);
extern "C" int  AbiWord_WMF_seek (void * context,long pos);
extern "C" long AbiWord_WMF_tell (void * context);

extern "C" int  AbiWord_WMF_function (void * context,char * buffer,int length);

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

#endif /* HAVE_LIBWMF */

UT_Confidence_t IE_ImpGraphicWMF_Sniffer::recognizeSuffix(const char * szSuffix)
{
	if (UT_stricmp(szSuffix,".wmf") == 0)
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_ImpGraphicWMF_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
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
#ifdef HAVE_LIBWMF
	*ppieg = new IE_ImpGraphic_WMF();
	if (*ppieg == 0)
		return UT_IE_NOMEMORY;

	return UT_OK;
#else  /* HAVE_LIBWMF */
	*ppieg = 0;

	return UT_ERROR;
#endif /* HAVE_LIBWMF */
}

// This creates our FG_Graphic object for a PNG
UT_Error IE_ImpGraphic_WMF::importGraphic(UT_ByteBuf* pBBwmf, 
					  FG_Graphic ** ppfg)
{
#ifdef HAVE_LIBWMF
	UT_Error err = UT_OK;

	UT_ByteBuf * pBBpng = 0;

	FG_GraphicRaster * pFGR = 0;

	*ppfg = 0;

	UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Begin -\n"));

	err = convertGraphic(pBBwmf,&pBBpng);
   	if (err != UT_OK) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Conversion failed...\n"));
		return err;
	}

	pFGR = new FG_GraphicRaster();
	if(pFGR == 0) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Ins. Mem.\n"));
		err = UT_IE_NOMEMORY;
	}
	else if(!pFGR->setRaster_PNG(pBBpng)) {
		UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic Fake type?\n"));
		DELETEP(pFGR);
		err = UT_IE_FAKETYPE;
	}
	else {
		*ppfg = (FG_Graphic *) pFGR;
	}

	UT_DEBUGMSG(("IE_ImpGraphic_WMF::importGraphic - End\n"));

	return err;
#else  /* HAVE_LIBWMF */
	*ppfg = 0;

	return UT_ERROR;
#endif /* HAVE_LIBWMF */
}

UT_Error IE_ImpGraphic_WMF::convertGraphic(UT_ByteBuf* pBBwmf,
					   UT_ByteBuf** ppBBpng)
{
#ifdef HAVE_LIBWMF
	UT_ByteBuf * pBBpng = 0;

	wmf_error_t err;

	wmf_gd_t * ddata = 0;

	wmfAPI * API = 0;
	wmfAPI_Options api_options;

	wmfD_Rect bbox;

	unsigned long flags;

	unsigned int max_width  = 500;
	unsigned int max_height = 500;

	float width;
	float height;

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

	err = wmf_size (API,&width,&height);
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
#else  /* HAVE_LIBWMF */
	*ppBBpng = 0;

	return UT_ERROR;
#endif /* HAVE_LIBWMF */
}

#ifdef HAVE_LIBWMF

// returns unsigned char cast to int, or EOF
extern "C" int AbiWord_WMF_read (void * context)
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
extern "C" int AbiWord_WMF_seek (void * context,long pos)
{
	bbuf_read_info * info = (bbuf_read_info *) context;

	info->pos = (UT_uint32) pos;

	return 0;
}

// returns (-1) on error, else pos
extern "C" long AbiWord_WMF_tell (void * context)
{
	bbuf_read_info * info = (bbuf_read_info *) context;

	return (long) info->pos;
}

extern "C" int AbiWord_WMF_function (void * context,char * buffer,int length)
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

#endif /* HAVE_LIBWMF */
