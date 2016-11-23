/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
 *
 * Portions from Nisus Software and Apple documentation
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

#import <Cocoa/Cocoa.h>

#include "ie_impGraphic_Cocoa.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

#include "fg_GraphicRaster.h"

const IE_SuffixConfidence *IE_ImpGraphicCocoa_Sniffer::getSuffixConfidence()
{
    static IE_SuffixConfidence *suffixConfidence = NULL;
    int idx = 0;
    NSArray *fileTypes = [NSImage imageFileTypes];

    suffixConfidence = new IE_SuffixConfidence[[fileTypes count] + 1];

	NSEnumerator* e = [fileTypes objectEnumerator];
    while(NSString *aType = [e nextObject])
    {
        suffixConfidence[idx].suffix = [aType UTF8String];
        suffixConfidence[idx].confidence = UT_CONFIDENCE_PERFECT;
        idx++;
    }

    // NULL-terminator
    suffixConfidence[idx].confidence = UT_CONFIDENCE_ZILCH;

    return suffixConfidence;
}

const IE_MimeConfidence * IE_ImpGraphicCocoa_Sniffer::getMimeConfidence()
{
	static IE_MimeConfidence mimeConfidence[] = {
		{ IE_MIME_MATCH_FULL, "image/png", UT_CONFIDENCE_PERFECT },
		{ IE_MIME_MATCH_FULL, "image/jpeg", UT_CONFIDENCE_PERFECT },
		{ IE_MIME_MATCH_FULL, "image/tiff", UT_CONFIDENCE_PERFECT },
		{ IE_MIME_MATCH_BOGUS,"",           UT_CONFIDENCE_ZILCH }
	};
	
	return mimeConfidence;
}

UT_Confidence_t IE_ImpGraphicCocoa_Sniffer::recognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	bool conf = UT_CONFIDENCE_ZILCH;
	NSData* data = [[NSData alloc] initWithBytes:szBuf length:iNumbytes];
	id repClass = [NSImageRep imageRepClassForData:data];
	if (nil != repClass) {
		conf = UT_CONFIDENCE_PERFECT;
	}
	else  {
		conf = UT_CONFIDENCE_POOR;
	}
	[data release];
	return conf;
}

bool IE_ImpGraphicCocoa_Sniffer::getDlgLabels(const char ** pszDesc, const char ** pszSuffixList, IEGraphicFileType * ft)
{
//	NSArray* a = [NSImage imageFileTypes] ;
	*pszDesc = "Cocoa-Readable Image";
	*pszSuffixList = "*.tiff; *.tif; *.pict; *.jpg; *.jpeg";
	//[[a componentsJoinedByString: @"; *."] UTF8String];
	*ft = getType ();
	return true;
}

UT_Error IE_ImpGraphicCocoa_Sniffer::constructImporter(IE_ImpGraphic **ppieg)
{
	//fprintf(stderr, "importer constructed\n");
	*ppieg = new IE_ImpGraphic_Cocoa();
	if (*ppieg == NULL)
	  return UT_IE_NOMEMORY;

	return UT_OK;
}

//  This actually creates our FG_Graphic object for a PNG
UT_Error IE_ImpGraphic_Cocoa::importGraphic(UT_ByteBuf* pBB, 
											FG_ConstGraphicPtr & pfg)
{
	UT_Error err = _convertGraphic(pBB); 
   	if (err != UT_OK) 
		return err;

	FG_GraphicRasterPtr pFGR(new FG_GraphicRaster);
	if(pFGR == NULL)
		return UT_IE_NOMEMORY;

	if(!pFGR->setRaster_PNG(m_pPngBB)) {
		return UT_IE_FAKETYPE;
	}

	pfg = std::move(pFGR);
	return UT_OK;
}

UT_Error IE_ImpGraphic_Cocoa::convertGraphic(UT_ByteBuf* pBB,
					   UT_ByteBuf** ppBB)
{
   	if (!ppBB) return UT_ERROR;

   	UT_Error err = _convertGraphic(pBB);
   	if (err != UT_OK) return err;
   
	*ppBB = m_pPngBB;
   	return UT_OK;
}

static NSData* convertImageToPNG(NSImage* image)
{
	NSDictionary* props = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], 
							NSImageInterlaced, nil];
	NSBitmapImageRep* rep = [NSBitmapImageRep imageRepWithData:[image TIFFRepresentation]];
	return [rep representationUsingType:NSPNGFileType properties:props];
}

static NSData* convertImageDataToPNG(NSData* data) 
{
	NSData *returned;
	NSImage* image = [[NSImage alloc] initWithData: data];
	returned = convertImageToPNG(image);
	[image release];
	return returned;
}

UT_Error IE_ImpGraphic_Cocoa::_convertGraphic (UT_ByteBuf* pBB)
{
	NSData* converted;
	UT_uint32 length;
	NSData* data = [[NSData alloc] initWithBytes:pBB->getPointer(0) length:pBB->getLength()];
	converted = convertImageDataToPNG(data);
	[data release];
	length = [converted length];
	m_pPngBB = new UT_ByteBuf;  /* Byte Buffer for Converted Data */	
	m_pPngBB->append((UT_Byte*)[converted bytes], length);
	return (length != 0 ? UT_OK : UT_ERROR);
}

#ifdef ABI_PLUGIN_BUILTIN

#define abi_plugin_register abipgn_cocoa_register
#define abi_plugin_unregister abipgn_cocoa_unregister
#define abi_plugin_supports_version abipgn_cocoa_supports_version

#include "xap_Module.h"

/*******************************************************************/
/*******************************************************************/

ABI_PLUGIN_DECLARE("COCOA")

// we use a reference-counted sniffer
static IE_ImpGraphicCocoa_Sniffer * m_impSniffer = 0;


ABI_FAR_CALL int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_impSniffer)
	{
	  m_impSniffer = new IE_ImpGraphicCocoa_Sniffer();
	}
	else
	{
		m_impSniffer->ref();
	}

	mi->name = "Cocoa Image Import Plugin";
	mi->desc = "Import Images Using Cocoa";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Nisus Software and Hubert Figuiere";
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
	if (!m_impSniffer->unref())
	{
		m_impSniffer = 0;
	}

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, UT_uint32 release)
{
		return 1;
}

#endif

/*******************************************************************/
/*******************************************************************/
