/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1999 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au> 
 * Copyright (C) 2001, 2004 Hubert Figuiere <hfiguiere@teaser.fr>
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


/*
  This part contains all the code to handle pictures and objects inside 
  the RTF importer.
 */

#include "ut_locale.h"

#include "ie_imp_RTF.h"
#include "ie_imp_RTFParse.h"
#include "ie_types.h"
#include "ie_impGraphic.h"

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"


// move this one away.
#define MAX_KEYWORD_LEN 256

static IEGraphicFileType
iegftForRTF(IE_Imp_RTF::PictFormat format)
{
	switch (format) 
		{
		case IE_Imp_RTF::picPNG:  return IEGFT_PNG;
		case IE_Imp_RTF::picJPEG: return IEGFT_JPEG;
		case IE_Imp_RTF::picBMP:  return IEGFT_BMP;
		case IE_Imp_RTF::picWMF:  return IEGFT_WMF;
		default:
			break;
		}

	return IEGFT_Unknown;
}

/*!
  Load the picture data
  \param format the Picture Format.
  \param image_name the name of the image. Must be unique.
  \param imgProps the RTF properties for the image.
  \return true if success, otherwise false.
  \desc Load the picture data from the flow. Will move the file position
  and assume proper RTF file structure. It will take care of inserting
  the picture into the document.
  \todo TODO: We assume the data comes in hex. Check this assumption
  as we might have to handle binary data as well
  \see IE_Imp_RTF::HandlePicture
*/
bool IE_Imp_RTF::LoadPictData(PictFormat format, const char * image_name,
							  struct RTFProps_ImageProps & imgProps, 
							  bool isBinary, long binaryLen)
{
	// first, we load the actual data into a buffer
	bool ok;

	const UT_uint16 chars_per_byte = 2;
	const UT_uint16 BITS_PER_BYTE = 8;
	const UT_uint16 bits_per_char = BITS_PER_BYTE / chars_per_byte;

	UT_ByteBuf * pictData = new UT_ByteBuf();
	UT_uint16 chLeft = chars_per_byte;
	UT_Byte pic_byte = 0;

	unsigned char ch;

	if (!ReadCharFromFile(&ch))
		return false;

	if (!isBinary) {
		while (ch != '}')
		{
			int digit;
			
			if (!hexVal(ch, digit))
				return false;
			
			pic_byte = (pic_byte << bits_per_char) + digit;
			
			// if we have a complete byte, we put it in the buffer
			if (--chLeft == 0)
			{
				pictData->append(&pic_byte, 1);
				chLeft = chars_per_byte;
				pic_byte = 0;
			}
			
			if (!ReadCharFromFile(&ch))
				return false;
		}
	} else {
#if 1
		UT_ASSERT_HARMLESS(UT_TODO);
		UT_DEBUGMSG(("BIN SHPPICT control not currently handled correctly\n"));
#else
		UT_ASSERT_HARMLESS(binaryLen);
		for (long i = 0; i < binaryLen; i++) {
			pictData->append(&ch, 1);
			if (!ReadCharFromFile(&ch))
				return false;
		}
#endif
	}

	// We let the caller handle this
	SkipBackChar(ch);

	// TODO: investigate whether pictData is leaking memory or not
	IE_ImpGraphic * pGraphicImporter = NULL;

	UT_Error error = IE_ImpGraphic::constructImporter(pictData, iegftForRTF(format), &pGraphicImporter);

	if ((error == UT_OK) && pGraphicImporter)
	{
		FG_Graphic* pFG = 0;

		// TODO: according with IE_ImpGraphic header, we shouldn't free
		// TODO: the buffer. Confirm that.
		error = pGraphicImporter->importGraphic(pictData, &pFG);
		DELETEP(pGraphicImporter);

		if (error != UT_OK || !pFG)
		{
			UT_DEBUGMSG(("Error parsing embedded PNG\n"));
			// Memory for pictData was destroyed if not properly loaded.
			return false;
		}

		UT_ByteBuf * buf = 0;
		buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
		imgProps.width = static_cast<UT_uint32>(pFG->getWidth ());
		imgProps.height = static_cast<UT_uint32>(pFG->getHeight ());
		// Not sure whether this is the right way, but first, we should
		// insert any pending chars
		if (!FlushStoredChars(true))
		{
			UT_DEBUGMSG(("Error flushing stored chars just before inserting a picture\n"));
			delete pictData;
			return false;
		}

		ok = InsertImage (buf, image_name, imgProps);
		if (!ok)
		{
			delete pictData;
			return false;
		}
	}
	else
	{
		// if we're not inserting a graphic, we should destroy the buffer
		UT_DEBUGMSG (("no translator found: %d\n", error));
		delete pictData;
	}

	return true;
}


/*!
  \param buf the buffer the image content is in.
  \param image_name the image name inside the XML file or the piecetable.
  \param imgProps the RTF image properties.
  \return true is successful.
  Insert and image at the current position.
  Check whether we are pasting or importing
 */
bool IE_Imp_RTF::InsertImage (const UT_ByteBuf * buf, const char * image_name,
							  const struct RTFProps_ImageProps & imgProps)
{
	UT_String propBuffer;
	double wInch = 0.0f;
	double hInch = 0.0f;
	bool resize = false;
	if (!bUseInsertNotAppend())
	{
		// non-null file, we're importing a doc
		// Now, we should insert the picture into the document

		const char * mimetype = NULL;
		mimetype = UT_strdup("image/png");

		switch (imgProps.sizeType)
		{
		case RTFProps_ImageProps::ipstGoal:
			UT_DEBUGMSG (("Goal\n"));
			resize = true;
			wInch = static_cast<double>(imgProps.wGoal) / 1440.0f;
			hInch = static_cast<double>(imgProps.hGoal) / 1440.0f;
			break;
		case RTFProps_ImageProps::ipstScale:
			UT_DEBUGMSG (("Scale: x=%d, y=%d, w=%d, h=%d\n", imgProps.scaleX, imgProps.scaleY, imgProps.width, imgProps.height));
			resize = true;
			if ((imgProps.wGoal != 0) && (imgProps.hGoal != 0)) {
				// want image scaled against the w&h specified, not the image's natural size
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * (imgProps.wGoal/ 1440.0f));
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * (imgProps.hGoal/ 1440.0f));
			} else {
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * (imgProps.width));
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * (imgProps.height));
			}
			break;
		default:
			resize = false;
			break;
		}

		if (resize) {
			UT_LocaleTransactor(LC_NUMERIC, "C");
			UT_DEBUGMSG (("resizing...\n"));
			UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
							  wInch, hInch);
			xxx_UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		const XML_Char* propsArray[5];
		propsArray[0] = static_cast<const XML_Char *>("dataid");
		propsArray[1] = static_cast<const XML_Char *>(image_name);
		if (resize)
		{
			propsArray[2] = static_cast<const XML_Char *>("props");
			propsArray[3] = propBuffer.c_str();
			propsArray[4] = NULL;
		}
		else
		{
			propsArray[2] = NULL;
		}
		xxx_UT_DEBUGMSG(("SEVIOR: Appending Object 2 m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
		if(m_bCellBlank || m_bEndTableOpen)
		{
			xxx_UT_DEBUGMSG(("Append block 13 \n"));

			getDoc()->appendStrux(PTX_Block,NULL);
			m_bCellBlank = false;
			m_bEndTableOpen = false;
		}

		if (!getDoc()->appendObject(PTO_Image, propsArray))
		{
			FREEP(mimetype);
			return false;
		}

		if (!getDoc()->createDataItem(image_name, false,
									  buf, static_cast<const void*>(mimetype), NULL))
		{
			// taken care of by createDataItem
			//FREEP(mimetype);
			return false;
		}
	}
	else
	{
		// null file, we're pasting an image. this really makes
		// quite a difference to the piece table

		// Get a unique name for image.
		UT_ASSERT_HARMLESS(image_name);
		UT_String szName;
#if 0
		if( !image_name)
		{
			image_name = "image_z";
		}
		UT_uint32 ndx = 0;
		for (;;)
		{
			UT_String_sprintf(szName, "%s_%d", image_name, ndx);
			if (!getDoc()->getDataItemDataByName(szName.c_str(), NULL, NULL, NULL))
			{
				break;
			}
			ndx++;
		}
#else
		UT_uint32 iid = getDoc()->getUID(UT_UniqueId::Image);
		UT_String_sprintf(szName, "%d", iid);
#endif
//
// Code from fg_GraphicsRaster.cpp
//
		/*
		  Create the data item
		*/
		const char * mimetype = NULL;
		mimetype = UT_strdup("image/png");
		bool bOK = false;
		bOK = getDoc()->createDataItem(szName.c_str(), false, buf, static_cast<const void *>(mimetype), NULL);
		UT_return_val_if_fail(bOK, false);
		/*
		  Insert the object into the document.
		*/
		bool resize = false;

		switch (imgProps.sizeType)
		{
		case RTFProps_ImageProps::ipstGoal:
			UT_DEBUGMSG (("Goal\n"));
			resize = true;
			wInch = static_cast<double>(imgProps.wGoal) / 1440.0f;
			hInch = static_cast<double>(imgProps.hGoal) / 1440.0f;
			break;
		case RTFProps_ImageProps::ipstScale:
			UT_DEBUGMSG (("Scale: x=%d, y=%d, w=%d, h=%d\n", imgProps.scaleX, imgProps.scaleY, imgProps.width, imgProps.height));
			resize = true;
			wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * imgProps.width);
			hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * imgProps.height);
			break;
		default:
			resize = false;
			break;
		}

		if (resize)
		{
			UT_LocaleTransactor(LC_NUMERIC, "C");
			UT_DEBUGMSG (("resizing...\n"));
			UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
							  wInch, hInch);
			UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		const XML_Char* propsArray[5];
		propsArray[0] = static_cast<const XML_Char *>("dataid");
		propsArray[1] = static_cast<const XML_Char *>(szName.c_str());
		if (resize)
		{
			propsArray[2] = static_cast<const XML_Char *>("props");
			propsArray[3] = propBuffer.c_str();
			propsArray[4] = NULL;
		}
		else
		{
			propsArray[2] = NULL;
		}
		getDoc()->insertObject(m_dposPaste, PTO_Image, propsArray, NULL);
		m_dposPaste++;
	}
	return true;
}

/*!
  Handle a picture in the current group
  \return false if failed
  \desc Once the \\pict has been read, hande the picture contained in
  the current group. Calls LoadPictData
  \see IE_Imp_RTF::LoadPictData
  \fixme TODO handle image size and other options in the future
 */
bool IE_Imp_RTF::HandlePicture()
{
	// this method loads a picture from the file
	// and insert it in the document.

	unsigned char ch;
	bool bPictProcessed = false;
	PictFormat format = picNone;

	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint16 parameter = 0;
	bool parameterUsed = false;
	RTFProps_ImageProps imageProps;

	bool isBinary = false;
	long binaryLen = 0;
	RTF_KEYWORD_ID keywordID;

	do {
		if (!ReadCharFromFile(&ch))
			return false;

		switch (ch)
		{
		case '\\':
			UT_return_val_if_fail(!bPictProcessed, false);
			// Process keyword

			if (!ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
			{
				UT_DEBUGMSG(("Unexpected EOF during RTF import?\n"));
			}
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			switch (keywordID)
			{
			case RTF_KW_pngblip:
				format = picPNG;
				break;
			case RTF_KW_jpegblip:
				format = picJPEG;
				break;
			case RTF_KW_wmetafile:
				format = picWMF;
				break;
			case RTF_KW_picwgoal:
				if (parameterUsed)
				{
					if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)) {
						imageProps.sizeType = RTFProps_ImageProps::ipstGoal;
					}
					imageProps.wGoal = parameter;
				}
				break;
			case RTF_KW_pichgoal:
				if (parameterUsed)
				{
					if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)) {
						imageProps.sizeType = RTFProps_ImageProps::ipstGoal;
					}
					imageProps.hGoal = parameter;
				}
				break;
			case RTF_KW_picscalex:
				if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)
					|| (imageProps.sizeType == RTFProps_ImageProps::ipstScale))
				{
					if ((parameterUsed) && (parameter != 100))
					{
						imageProps.sizeType = RTFProps_ImageProps::ipstScale;
						imageProps.scaleX = static_cast<unsigned short>(parameter);
					}
				}
				break;
			case RTF_KW_picscaley:
				if ((imageProps.sizeType == RTFProps_ImageProps::ipstNone)
					|| (imageProps.sizeType == RTFProps_ImageProps::ipstScale))
				{
					if ((parameterUsed) && (parameter != 100))
					{
						imageProps.sizeType = RTFProps_ImageProps::ipstScale;
						imageProps.scaleY = static_cast<unsigned short>(parameter);
					}
				}
				break;
			case RTF_KW_bin:
				UT_ASSERT_HARMLESS(parameterUsed);
				if (parameterUsed) {
					isBinary = true;
					binaryLen = parameter;
				}
				break;
			default:
				UT_DEBUGMSG(("Unhandled keyword in \\pict group: %s\n", keyword));
			}
			break;
		case '{':
			UT_return_val_if_fail(!bPictProcessed, false);

			// We won't handle nested groups, at least in this version,
			// we just skip them
			SkipCurrentGroup(true);
			break;
		case '}':
			// check if a pict was found ( and maybe inserted )
			// as this would be the last iteration
			if (!bPictProcessed)
			{
				UT_DEBUGMSG(("Bogus RTF: \\pict group without a picture\n"));
				return false;
			}
			break;
		default:
			UT_return_val_if_fail(!bPictProcessed, false);
			// It this a conforming rtf, this should be the pict data
			// if we know how to handle this format, we insert the picture

			UT_String image_name;
			UT_String_sprintf(image_name,"%d",getDoc()->getUID(UT_UniqueId::Image));

			// the first char belongs to the picture too
			SkipBackChar(ch);

			if (!LoadPictData(format, image_name.c_str(), imageProps, isBinary, binaryLen))
				if (!SkipCurrentGroup())
					return false;

			bPictProcessed = true;
		}
	} while (ch != '}');

	// The last brace is put back on the stream, so that the states stack
	// doesn't get corrupted
	SkipBackChar(ch);

	return true;
}

/*!
 * a "sp" keyword has been found the text following is the property.
 */
void IE_Imp_RTF::HandleShapeProp(void)
{
	UT_ASSERT_HARMLESS(m_sPendingShapeProp.size() == 0);
	UT_Byte ch = 0;
	xxx_UT_DEBUGMSG(("Handle sp \n"));
	while (ch != '}')
	{
		if (!ReadCharFromFile(&ch)) 
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		if (ch != '}' && (ch != ' ')) 
		{
			m_sPendingShapeProp += ch;
		}
	}
	SkipBackChar (ch);
}

/*!
 * a "sv" keyword has been found. The text following is the value of the property
 */
void IE_Imp_RTF::HandleShapeVal(void)
{
	UT_String sVal;
	sVal.clear();
	UT_Byte ch = 0;
	UT_ASSERT_HARMLESS(m_sPendingShapeProp.size() > 0);
	xxx_UT_DEBUGMSG(("Handle sp \n"));
	while (ch != '}')
	{
		if (!ReadCharFromFile(&ch)) 
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		if (ch != '}' && (ch != ' ')) 
		{
			sVal += ch;
		}
	}
	SkipBackChar (ch);
//
// OK got the  prop,val pair. Do something with them!
//	
	UT_sint32 ival = 0;
	if(strcmp(m_sPendingShapeProp.c_str(),"dxTextLeft")== 0)
	{
		ival = atoi(sVal.c_str());
		m_currentFrame.m_iLeftPad = ival;
	}
	else if(strcmp(m_sPendingShapeProp.c_str(),"dxTextRight")== 0)
	{
		ival = atoi(sVal.c_str());
		m_currentFrame.m_iRightPad = ival;
	}
	else if(strcmp(m_sPendingShapeProp.c_str(),"dxTextTop")== 0)
	{
		ival = atoi(sVal.c_str());
		m_currentFrame.m_iTopPad = ival;
	}
	else if(strcmp(m_sPendingShapeProp.c_str(),"dxTextBottom")== 0)
	{
		ival = atoi(sVal.c_str());
		m_currentFrame.m_iBotPad = ival;
	}
	else if(strcmp(m_sPendingShapeProp.c_str(),"shapeType")== 0)
	{
		ival = atoi(sVal.c_str());
		if(ival == 202)
		{
			m_currentFrame.m_iFrameType = 0;
		}
		m_currentFrame.m_iFrameType = 0; // no others implemented
	}
	m_sPendingShapeProp.clear();
}

void IE_Imp_RTF::HandleShapeText(void)
{

// Ok this is the business end of the frame where we actually insert the
// Strux.

// Flush any stored chars now.
	FlushStoredChars(true);
	UT_DEBUGMSG(("Doing Handle shptxt \n"));
// OK Assemble the attributes/properties for the Frame
	const XML_Char * attribs[3] = {"props",NULL,NULL};
	UT_UTF8String sPropString;
	UT_UTF8String sP;
	UT_UTF8String sV;
	sP = "frame-type";
	sV = "textbox";
	UT_UTF8String_setProperty(sPropString,sP,sV); // fixme make other types

	sP = "position-to";
	sV = "block-above-text";
	UT_UTF8String_setProperty(sPropString,sP,sV); // fixme make other types

	{
		UT_LocaleTransactor(LC_NUMERIC, "C");

		double dV = static_cast<double>(m_currentFrame.m_iLeftPos)/1440.0;
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "xpos";
		UT_UTF8String_setProperty(sPropString,sP,sV);
		
		dV = static_cast<double>(m_currentFrame.m_iTopPos)/1440.0;
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "ypos";
		UT_UTF8String_setProperty(sPropString,sP,sV);
		
		dV = static_cast<double>(m_currentFrame.m_iRightPos - m_currentFrame.m_iLeftPos)/1440.0;
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "frame-width";
		UT_UTF8String_setProperty(sPropString,sP,sV); 
		
		dV = static_cast<double>(m_currentFrame.m_iBotPos - m_currentFrame.m_iTopPos)/1440.0;
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "frame-height";
		UT_UTF8String_setProperty(sPropString,sP,sV); 
		
		dV = static_cast<double>(m_currentFrame.m_iRightPad + m_currentFrame.m_iLeftPad)/9114400.0; // EMU
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "xpad";
		UT_UTF8String_setProperty(sPropString,sP,sV); 
		
		dV = static_cast<double>(m_currentFrame.m_iBotPad + m_currentFrame.m_iTopPad)/9114400.0; //EMU
		sV= UT_UTF8String_sprintf("%fin",dV);
		sP= "ypad";
		UT_UTF8String_setProperty(sPropString,sP,sV); 
	}
	attribs[1] = sPropString.utf8_str();


	if(!bUseInsertNotAppend())
	{
		getDoc()->appendStrux(PTX_SectionFrame,attribs);
		UT_DEBUGMSG(("Append block in Frame \n"));
		getDoc()->appendStrux(PTX_Block,NULL);
	}
	else
	{
		getDoc()->insertStrux(m_dposPaste,PTX_SectionFrame,attribs,NULL);
		m_dposPaste++;
		UT_DEBUGMSG((" Insert Block at Frame \n"));
		markPasteBlock();
		getDoc()->insertStrux(m_dposPaste,PTX_Block);
		m_dposPaste++;
	}

}

void IE_Imp_RTF::HandleEndShape(void)
{
	UT_DEBUGMSG(("Doing HandleEndShape \n"));
	if(!bUseInsertNotAppend())
	{
		getDoc()->appendStrux(PTX_EndFrame,NULL);
		m_bEndFrameOpen = true;
	}
	else
	{
		getDoc()->insertStrux(m_dposPaste,PTX_EndFrame);
		m_dposPaste++;
		m_bEndFrameOpen = true;
	}
	m_bFrameOpen = false;
	m_iStackDepthAtFrame = 0;
}


/*!
  Handle the shppict RTF keyword.
 */
void IE_Imp_RTF::HandleShape()
{
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint16 parameter = 0;
	bool paramUsed = false;	
	int nested = 1;           // nesting level	
	RTF_KEYWORD_ID keywordID;

	do
	{
		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN,false);
		switch (tokenType)
		{
		case RTF_TOKEN_ERROR:
			UT_ASSERT_NOT_REACHED();
			return ;
			break;
		case RTF_TOKEN_KEYWORD:
		{
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			switch (keywordID)
			{
			case RTF_KW_pict:
				HandlePicture();
				break;
			default:
				break;
			}
			break;
		}
		case RTF_TOKEN_OPEN_BRACE:
			nested++;
			PushRTFState();
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			nested--;
			PopRTFState();			
			break;
		case RTF_TOKEN_DATA:	//Ignore data
			break;
		default:
			break;
		}
	} while (!((tokenType == RTF_TOKEN_CLOSE_BRACE) && (nested == 0)));
}


/*!
  Handle a object in the current group
  \return false if failed
  \desc Once the \\object has been read, handle the object contained in
  the current group.  
 */
bool IE_Imp_RTF::HandleObject()
{	
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint16 parameter = 0;
	bool paramUsed = false;	
	int nested = 1;           // nesting level	
	RTF_KEYWORD_ID keywordID;
	int beginResult = 0;     // carries the nesting level where the result is found
	enum { 
		OBJ_TYPE_NONE,
		OBJ_TYPE_RTF,
		OBJ_TYPE_PICT,
		OBJ_TYPE_BMP,
		OBJ_TYPE_TXT,
		OBJ_TYPE_HTML
	} objectType = OBJ_TYPE_NONE;
		
	do
	{
		tokenType = NextToken (keyword, &parameter, &paramUsed, MAX_KEYWORD_LEN,false);
		switch (tokenType)
		{
		case RTF_TOKEN_ERROR:
			UT_ASSERT_NOT_REACHED();
			return false;
			break;
		case RTF_TOKEN_KEYWORD:
		{
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			switch (keywordID)
			{
			case RTF_KW_rsltrtf:
				objectType = OBJ_TYPE_RTF;
				break;
			case RTF_KW_rsltpict:
				objectType = OBJ_TYPE_PICT;
				break;
			case RTF_KW_rsltbmp:
				objectType = OBJ_TYPE_BMP;
				break;
			case RTF_KW_rslttxt: 
				objectType = OBJ_TYPE_TXT;
				break;
			case RTF_KW_rslthtml:
				objectType = OBJ_TYPE_HTML;
				break;
			case RTF_KW_result:
				// handle a paragraph.
				beginResult = nested;
				break;
			case RTF_KW_shppict:
				if (beginResult <= nested) {
					HandleShape();
				}
				break;
			case RTF_KW_nonshppict:
				UT_DEBUGMSG(("Hub: skip nonshppict in \\object\n"));
				SkipCurrentGroup();
				break;
			default:
				break;
			}
			break;
		}
		case RTF_TOKEN_OPEN_BRACE:
			nested++;
			PushRTFState();
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			if (beginResult == nested) {
				// we are closing the group where the result started
				beginResult = 0;
			}
			nested--;
			PopRTFState();			
			break;
		case RTF_TOKEN_DATA:	//Ignore data
			break;
		default:
			break;
		}
	} while (!((tokenType == RTF_TOKEN_CLOSE_BRACE) && (nested == 0)));

	return true;
}

