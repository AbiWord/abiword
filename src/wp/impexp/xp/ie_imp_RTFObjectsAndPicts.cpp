/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1999 AbiSource, Inc.
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
 * Copyright (C) 2003 Martin Sevior <msevior@physics.unimelb.edu.au> 
 * Copyright (C) 2001, 2004, 2009 Hubert Figuiere <hub@figuiere.net>
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


/*
  This part contains all the code to handle pictures and objects inside 
  the RTF importer.
*/

#include <utility>
#include <string>

#include "ut_locale.h"
#include "ut_std_string.h"

#include "ie_imp_RTF.h"
#include "ie_imp_RTFParse.h"
#include "ie_types.h"
#include "ie_impGraphic.h"
#include "fl_FrameLayout.h"
#include "ut_rand.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

// move this one away.
#define MAX_KEYWORD_LEN 256

static IEGraphicFileType
iegftForRTF(IE_Imp_RTF::PictFormat format)
{
	switch (format) 
	{
	case IE_Imp_RTF::picPNG:  return IEGFT_PNG;
	case IE_Imp_RTF::picJPEG: return IEGFT_JPEG;
	case IE_Imp_RTF::picSVG:  return IEGFT_SVG;
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
	bool retval = true;

	const UT_uint16 chars_per_byte = 2;
	const UT_uint16 BITS_PER_BYTE = 8;
	const UT_uint16 bits_per_char = BITS_PER_BYTE / chars_per_byte;

	UT_ByteBufPtr pictData(new UT_ByteBuf);
	UT_uint16 chLeft = chars_per_byte;
	UT_Byte pic_byte = 0;
	FG_ConstGraphicPtr pFG;
	UT_Error error = UT_OK;
	unsigned char ch;

	if (!isBinary) {
		if (!ReadCharFromFile(&ch)) {
			retval = false;
			goto cleanup;
		}

		while (ch != '}')
		{
			int digit;
			
			if (!hexVal(ch, digit)) {
				retval = false;
				goto cleanup;
			}
			
			pic_byte = (pic_byte << bits_per_char) + digit;
			
			// if we have a complete byte, we put it in the buffer
			if (--chLeft == 0)
			{
				pictData->append(&pic_byte, 1);
				chLeft = chars_per_byte;
				pic_byte = 0;
			}
			
			if (!ReadCharFromFile(&ch)) {
				retval = false;
				goto cleanup;
			}
		}
	} else {
		UT_ASSERT_HARMLESS(binaryLen);
		UT_DEBUGMSG(("Loading binary data image of %ld bytes\n", binaryLen));
		for (long i = 0; i < binaryLen; i++) {
			if (!ReadCharFromFileWithCRLF(&ch)) {
				retval = false;
				goto cleanup;
			}
			pictData->append(&ch, 1);
		}
	}

	// We let the caller handle this
	SkipBackChar(ch);

	error = IE_ImpGraphic::loadGraphic(pictData, iegftForRTF(format), pFG);

	if ((error == UT_OK) && pFG)
	{
		imgProps.width = static_cast<UT_uint32>(pFG->getWidth ());
		imgProps.height = static_cast<UT_uint32>(pFG->getHeight ());
		// Not sure whether this is the right way, but first, we should
		// insert any pending chars
		if (!FlushStoredChars(true))
		{
			UT_DEBUGMSG(("Error flushing stored chars just before inserting a picture\n"));
			return false;
		}

		ok = InsertImage (pFG, image_name, imgProps);
		if (!ok)
		{
			return false;
		}
	}
	else
	{
		// if we're not inserting a graphic, we should destroy the buffer
		UT_DEBUGMSG (("no translator found: %d\n", error));
	}

 cleanup:
	return retval;
}


/*!
  \param buf the buffer the image content is in.
  \param image_name the image name inside the XML file or the piecetable.
  \param imgProps the RTF image properties.
  \return true is successful.
  Insert and image at the current position.
  Check whether we are pasting or importing
*/
bool IE_Imp_RTF::InsertImage (const FG_ConstGraphicPtr& pFG, const char * image_name,
							  const struct RTFProps_ImageProps & imgProps)
{
	std::string propBuffer;
	double wInch = 0.0f;
	double hInch = 0.0f;
    double cropt = 0.0f;
	double cropb = 0.0f;
	double cropl = 0.0f;
	double cropr = 0.0f;
	bool resize1 = false;
	if (!bUseInsertNotAppend())
	{
		// non-null file, we're importing a doc
		// Now, we should insert the picture into the document

		switch (imgProps.sizeType)
		{
		case RTFProps_ImageProps::ipstGoal:
			UT_DEBUGMSG (("Goal\n"));
			resize1 = true;
			wInch = static_cast<double>(imgProps.wGoal) / 1440.0f;
			hInch = static_cast<double>(imgProps.hGoal) / 1440.0f;
			break;
		case RTFProps_ImageProps::ipstScale:
			UT_DEBUGMSG (("Scale: x=%d, y=%d, w=%d, h=%d\n", imgProps.scaleX, imgProps.scaleY, imgProps.width, imgProps.height));
			resize1 = true;
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
			resize1 = false;
			break;
		}

		if ( imgProps.bCrop )
		{
			cropt = imgProps.cropt / 1440.0f;
			cropb = imgProps.cropb / 1440.0f;
			cropl = imgProps.cropl / 1440.0f;
			cropr = imgProps.cropr / 1440.0f;
			resize1 = true;
		}
	  
		if (resize1) 
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			UT_DEBUGMSG (("resizing...\n"));
			propBuffer = UT_std_string_sprintf("width:%fin; height:%fin; cropt:%fin; cropb:%fin; cropl:%fin; cropr:%fin",
							  wInch, hInch, cropt, cropb, cropl, cropr);
			UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		PP_PropertyVector propsArray(resize1 ? 4 : 2);
		propsArray[0] = "dataid";
		propsArray[1] = image_name;
		if (resize1) {
			propsArray[2] = "props";
			propsArray[3] = propBuffer;
		}
		if(!isStruxImage())
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Appending Object 2 m_bCellBlank %d m_bEndTableOpen %d \n",m_bCellBlank,m_bEndTableOpen));
			if(m_bCellBlank || m_bEndTableOpen)
			{
				xxx_UT_DEBUGMSG(("Append block 13 \n"));

				getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
				m_bCellBlank = false;
				m_bEndTableOpen = false;
			}

			if (!getDoc()->appendObject(PTO_Image, propsArray))
			{
				return false;
			}
		}
		if (!getDoc()->createDataItem(image_name, false,
									  pFG->getBuffer(), pFG->getMimeType(), 
									  NULL))
		{
			// taken care of by createDataItem
			//FREEP(mimetype);
			return false;
		}
		if(isStruxImage())
		{
			m_sImageName = image_name;
		}
		else
		{
			clearImageName();
		}
	}
	else
	{
		// null file, we're pasting an image. this really makes
		// quite a difference to the piece table

		// Get a unique name for image.
		UT_ASSERT_HARMLESS(image_name);
		std::string szName;
#if 0
		if( !image_name)
		{
			image_name = "image_z";
		}
		UT_uint32 ndx = 0;
		for (;;)
		{
			szName = UT_std_string_sprintf("%s_%d", image_name, ndx);
			if (!getDoc()->getDataItemDataByName(szName.c_str(), NULL, NULL, NULL))
			{
				break;
			}
			ndx++;
		}
#else
		UT_uint32 iid = getDoc()->getUID(UT_UniqueId::Image);
		szName = UT_std_string_sprintf("%d", iid);
#endif
//
// Code from fg_GraphicsRaster.cpp
//
		/*
		  Create the data item
		*/
		bool bOK = false;
		bOK = getDoc()->createDataItem(szName.c_str(), false, 
									   pFG->getBuffer(), 
									   pFG->getMimeType(), NULL);
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
			if ((imgProps.wGoal != 0) && (imgProps.hGoal != 0)) {
				// want image scaled against the w&h specified, not the image's natural size
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * imgProps.wGoal/ 1440.0f);
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * imgProps.hGoal/ 1440.0f);
			} else {
				wInch = ((static_cast<double>(imgProps.scaleX) / 100.0f) * (imgProps.width));
				hInch = ((static_cast<double>(imgProps.scaleY) / 100.0f) * (imgProps.height));
			}
			break;
		default:
			resize = false;
			break;
		}

		if (resize)
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			UT_DEBUGMSG (("resizing...\n"));
			propBuffer = UT_std_string_sprintf("width:%fin; height:%fin",
							  wInch, hInch);
			UT_DEBUGMSG (("props are %s\n", propBuffer.c_str()));
		}

		PP_PropertyVector propsArray(resize ? 4 : 2);
		propsArray[0] = "dataid";
		propsArray[1] = szName;
		if (resize)	{
			propsArray[2] = "props";
			propsArray[3] = propBuffer;
		}
		m_sImageName = szName.c_str();
		if(!isStruxImage())
		{
			getDoc()->insertObject(m_dposPaste, PTO_Image, propsArray, PP_NOPROPS);
			m_dposPaste++;
		}
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

	unsigned char ch1;
	bool bPictProcessed = false;
	PictFormat format = picNone;

	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint32 parameter = 0;
	bool parameterUsed = false;
	RTFProps_ImageProps imageProps;

	bool isBinary = false;
	long binaryLen = 0;
	RTF_KEYWORD_ID keywordID;

	do {
		if (!ReadCharFromFile(&ch1))
			return false;

		switch (ch1)
		{
		case '\\':
			UT_return_val_if_fail(!bPictProcessed, false);
			// Process keyword

			if (!ReadKeyword(keyword, &parameter, &parameterUsed, MAX_KEYWORD_LEN))
			{
				UT_DEBUGMSG(("Unexpected EOF during RTF import?\n"));
			}
			UT_DEBUGMSG(("Doing standard picture stuff with keyword %s \n",keyword));
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			UT_ASSERT(RTF_KW_cell  != keywordID);
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
			case RTF_KW_svgblip:
				format = picSVG;
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
				if ((parameterUsed) && (parameter != 100))       // scale the image if one of these two keywords appear
				{
					imageProps.sizeType = RTFProps_ImageProps::ipstScale;
					imageProps.scaleX = static_cast<unsigned short>(parameter);
				}
				break;
			case RTF_KW_picscaley:
				if ((parameterUsed) && (parameter != 100))
				{
					imageProps.sizeType = RTFProps_ImageProps::ipstScale;
					imageProps.scaleY = static_cast<unsigned short>(parameter);
				}
				break;
			case RTF_KW_piccropt:
				imageProps.cropt = parameter;
				imageProps.bCrop = true;
				break;
			case RTF_KW_piccropb:
				imageProps.cropb = parameter;
				imageProps.bCrop = true;
				break;
			case RTF_KW_piccropl:
				imageProps.cropl = parameter;
				imageProps.bCrop = true;
				break;
			case RTF_KW_piccropr:
				imageProps.cropr = parameter;
				imageProps.bCrop = true;
				break;
			case RTF_KW_bin:
				/* this code is completely broken. 
				   if we encounter the \bin keyboard, then we must
				   immediately read the binary stream....
				   skip the first space after the keyword.
				 */
				UT_ASSERT_HARMLESS(parameterUsed);
				if (parameterUsed) {
					isBinary = true;
					binaryLen = parameter;
					UT_UTF8String image_name;
					UT_UTF8String_sprintf(image_name,"%d",getDoc()->getUID(UT_UniqueId::Image));

					unsigned char ch;
					if(ReadCharFromFileWithCRLF(&ch)) {
						if(ch != ' ') {
							SkipBackChar(ch);
						}
						else {
							UT_DEBUGMSG(("Skipped space after \\bin keyword \n"));
						}
					}
					
					if (!LoadPictData(format, image_name.utf8_str(), imageProps, isBinary, binaryLen)) {
						return false;
					}
					bPictProcessed = true;
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
			if (!bPictProcessed) {
				// It this a conforming rtf, this should be the pict data
				// if we know how to handle this format, we insert the picture
				// But we'll skip this if this is a binary data because we found
				// a \binN keyword a processed the picture.
				UT_DEBUGMSG(("Inserting Image data now \n"));
				UT_UTF8String image_name;
				UT_UTF8String_sprintf(image_name,"%d",getDoc()->getUID(UT_UniqueId::Image));

				// the first char belongs to the picture too
				SkipBackChar(ch1);

				if (!LoadPictData(format, image_name.utf8_str(), imageProps, isBinary, binaryLen))
					if (!SkipCurrentGroup(false))
						return false;

				bPictProcessed = true;
			}
		}
	} while (ch1 != '}');

	// The last brace is put back on the stream, so that the states stack
	// doesn't get corrupted
	SkipBackChar(ch1);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

// Frame properties
class ABI_EXPORT RTFProps_FrameProps
{
public:
	typedef std::pair<std::string, std::string> PropertyPair;

	RTFProps_FrameProps();
	virtual ~RTFProps_FrameProps()
		{};
	void _setProperty(const PropertyPair *pair);

	void             clear();
	UT_sint32        m_iLeftPos;
	UT_sint32        m_iRightPos;
	UT_sint32        m_iTopPos;
	UT_sint32        m_iBotPos;
	UT_sint32        m_iLeftPad;
	UT_sint32        m_iRightPad;
	UT_sint32        m_iTopPad;
	UT_sint32        m_iBotPad;
	UT_sint32        m_iFrameType;
	UT_sint32        m_iFramePositionTo;
	bool             m_bCleared;
	UT_sint32        m_iFrameWrapMode;
    UT_sint32        m_iBackgroundColor;
	UT_sint32	     m_iFillType;
	std::string      m_abiProps;
};


RTFProps_FrameProps::RTFProps_FrameProps(void):
	m_iLeftPos(0),
	m_iRightPos(0),
	m_iTopPos(0),
	m_iBotPos(0),
	m_iLeftPad(0),
	m_iRightPad(0),
	m_iTopPad(0),
	m_iBotPad(0),
	m_iFrameType(-1),
	m_iFramePositionTo(-1),
	m_bCleared(true),
	m_iFrameWrapMode(3),
	m_iBackgroundColor(0),
	m_iFillType(0),
	m_abiProps("")
{
}

void RTFProps_FrameProps::clear(void)
{
	m_iLeftPos = 0;
	m_iRightPos = 0;
	m_iTopPos = 0;
	m_iBotPos = 0;
	m_iLeftPad = 0;
	m_iRightPad = 0;
	m_iTopPad = 0;
	m_iBotPad = 0;
	m_iFrameType = -1;
	m_iFramePositionTo =1;
	m_bCleared= true;
	m_iFrameWrapMode=3;
	m_iBackgroundColor= 0;
	m_iFillType = 0;
	m_abiProps = "";
}


/*!
  Set the property from a property pair

  \param pair the property pair
 */
void RTFProps_FrameProps::_setProperty(const PropertyPair *pair)
{
	UT_return_if_fail(pair);

	const std::string &propName = pair->first;
	const std::string &propValue = pair->second;

	if(propName.empty())
		return;

	UT_sint32 ival = 0;
	if(propName == "dxTextLeft")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iLeftPad = ival;
	}
	else if(propName == "dxTextRight")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iRightPad = ival;
	}
	else if(propName == "dxTextTop")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iTopPad = ival;
	}
	else if(propName == "dxTextBottom")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iBotPad = ival;
	}
	else if(propName == "fillColor")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iBackgroundColor = ival;
	}
	else if(propName == "fillType")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iFillType = ival;
	}
	else if(propName == "shapeType")
	{
		if(!propValue.empty())
			ival = atoi(propValue.c_str());
		m_iFrameType = 0; // no others implemented
		if(ival == 202)
		{
			m_iFrameType = 0;
		}
		else if(ival == 75)
		{
			m_iFrameType =1 ; // Image??
		}
	}
	else if(propName == "pib")
	{
//
// We have a positioned image. This has been processed elsewhere.
//
		UT_DEBUGMSG(("Found positioned Image \n"));
	}
	else if(!propValue.empty())
	{
		UT_DEBUGMSG(("unknown property %s with value %s\n", propName.c_str(),
					 propValue.c_str() ));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////


/*!
  Handle the \\sp keyword inside shapes
*/

class ABI_EXPORT IE_Imp_ShpPropParser
	: public IE_Imp_RTFGroupParser
{
public:


	IE_Imp_ShpPropParser()
		: m_propPair(NULL), 
		  m_last_grp(0),
		  m_last_kwID(RTF_UNKNOWN_KEYWORD),
		  m_name(NULL),
		  m_value(NULL), 
		  m_lastData(NULL),
		  m_found_image(false)
		{}
	~IE_Imp_ShpPropParser()
		{ 

			DELETEP(m_propPair); 
			DELETEP(m_name); 
			DELETEP(m_value); 
			DELETEP(m_lastData);
		}
	virtual bool tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID, 
							  UT_sint32 param, bool paramUsed);
	virtual bool tokenOpenBrace(IE_Imp_RTF * ie);
	virtual bool tokenCloseBrace(IE_Imp_RTF * ie);
	virtual bool tokenData(IE_Imp_RTF * ie, UT_UTF8String & data);
	
	virtual bool finalizeParse(void);

	/*!
	  Fetch the property key/value pair
	*/
	RTFProps_FrameProps::PropertyPair *getProp(void)
		{ return m_propPair; }

private:

	RTFProps_FrameProps::PropertyPair *m_propPair;
	
	int m_last_grp;
	RTF_KEYWORD_ID m_last_kwID;
	
	std::string *m_name, *m_value;
	std::string *m_lastData;
	bool m_found_image;
};


bool IE_Imp_ShpPropParser::tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID, 
										UT_sint32 /*param*/, bool /*paramUsed*/)
{
	switch(kwID) {
	case RTF_KW_sn:
		m_found_image = false;
		// fall through
	case RTF_KW_sv:
		UT_DEBUGMSG(("IE_Imp_ShpPropParser: found keyword %d\n", kwID));
		m_last_grp = nested();
		m_last_kwID = kwID;
		break;
	case RTF_KW_pict:
		m_found_image = true;
		ie->setStruxImage(true);
		ie->clearImageName();
		ie->HandlePicture();
		break;
	default:
		break;
	}
	return true;
}

bool IE_Imp_ShpPropParser::tokenOpenBrace(IE_Imp_RTF * ie)
{
	return IE_Imp_RTFGroupParser::tokenOpenBrace(ie);
}


bool IE_Imp_ShpPropParser::tokenCloseBrace(IE_Imp_RTF * ie)
{
	UT_DEBUGMSG(("Prop::tokenCloseBrace() last group = %d, nested = %d\n", m_last_grp, nested()));
	if(m_last_grp && (nested() == m_last_grp))
	{
		switch(m_last_kwID) {
		case RTF_KW_sn:
			UT_ASSERT(m_lastData);
			DELETEP(m_name);
			m_name = m_lastData;
			m_lastData = NULL;
			break;
		case RTF_KW_sv:
			UT_ASSERT(m_lastData || m_found_image);
			DELETEP(m_value);
			m_value = m_lastData;
			m_lastData = NULL;
			break;
		default:
			break;
		}
		m_last_grp = 0;
	}

	return IE_Imp_RTFGroupParser::tokenCloseBrace(ie);
}

bool IE_Imp_ShpPropParser::tokenData(IE_Imp_RTF * /*ie*/, UT_UTF8String & data)
{
	DELETEP(m_lastData);
	m_lastData = new std::string(data.utf8_str());
	return true;
}


bool IE_Imp_ShpPropParser::finalizeParse(void)
{
	UT_ASSERT(m_name);
	UT_ASSERT(m_value || m_found_image);
	if(m_name) 
	{
		m_propPair = new RTFProps_FrameProps::PropertyPair(*m_name, m_value?*m_value:"");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////

// FIXME: move away from this file. Should belong to the main RTF import
// or to ie_imp_RTFText.cpp

class ABI_EXPORT IE_Imp_TextParaPropParser
	: public IE_Imp_RTFGroupParser
{
public:
	virtual bool tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID, 
							  UT_sint32 param, bool paramUsed);

 	virtual bool tokenOpenBrace(IE_Imp_RTF * ie);
	virtual bool tokenCloseBrace(IE_Imp_RTF * ie);
	virtual bool tokenData(IE_Imp_RTF * ie, UT_UTF8String & data);
	
	virtual bool finalizeParse(void);
};


bool IE_Imp_TextParaPropParser::tokenKeyword(IE_Imp_RTF * ie, 
										   RTF_KEYWORD_ID kwID, 
										   UT_sint32 param, bool paramUsed)
{
	xxx_UT_DEBUGMSG(("IE_Imp_TextParPropParser::tokenKeyword()\n"));
	return ie->TranslateKeywordID(kwID, param, paramUsed);
}

bool IE_Imp_TextParaPropParser::tokenOpenBrace(IE_Imp_RTF * ie)
{
	UT_DEBUGMSG(("IE_Imp_TextParPropParser::tokenOpenBrace()\n"));
	ie->PushRTFState();
	return IE_Imp_RTFGroupParser::tokenOpenBrace(ie);
}


bool IE_Imp_TextParaPropParser::tokenCloseBrace(IE_Imp_RTF * ie)
{
	UT_DEBUGMSG(("IE_Imp_TextParPropParser::tokenCloseBrace()\n"));
	ie->PopRTFState();
	return IE_Imp_RTFGroupParser::tokenCloseBrace(ie);
}

bool IE_Imp_TextParaPropParser::tokenData(IE_Imp_RTF * ie, UT_UTF8String & data)
{
	UT_DEBUGMSG(("IE_Imp_TextParPropParser::tokenData()\n"));	
	const char * str = data.utf8_str();
	bool ok = true;

	while(*str && ok) {
		// This code is actually unsage FIXME
        // because str supposedly points to UTF-8
        // but ParseChar() is completely flawed in that regard.
		ok = ie->ParseChar(*str);
		str++;
	}

	return ok;
}


bool IE_Imp_TextParaPropParser::finalizeParse(void)
{
//	ie->FlushStoredChars(true);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

class ABI_EXPORT IE_Imp_ShpGroupParser
	: public IE_Imp_RTFGroupParser
{
public:
	IE_Imp_ShpGroupParser(IE_Imp_RTF * ie);
	virtual ~IE_Imp_ShpGroupParser();
	virtual bool tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID, 
							  UT_sint32 param, bool paramUsed);
	virtual bool tokenCloseBrace(IE_Imp_RTF *ie);
	virtual bool tokenData(IE_Imp_RTF * ie, UT_UTF8String & data);
	RTFProps_FrameProps & frame(void)
		{ return m_currentFrame; }
private:
	RTF_KEYWORD_ID m_last_kwID;
	IE_Imp_RTF * m_ieRTF;
	RTFProps_FrameProps m_currentFrame;	
	UT_sint32    m_iOrigTableDepth;
	std::string *m_lastData;
};


IE_Imp_ShpGroupParser::IE_Imp_ShpGroupParser(IE_Imp_RTF * ie) 
							: m_ieRTF(ie),
							  m_lastData(NULL)
{
	m_currentFrame.clear();
	m_iOrigTableDepth = m_ieRTF->getPasteDepth();
}


IE_Imp_ShpGroupParser::~IE_Imp_ShpGroupParser()
{
	if(m_ieRTF->getTable() != NULL)
	{
		m_ieRTF->CloseTable(true);
	}
	//
	// Close off any open tables
	//
	if(	(m_ieRTF->getPasteDepth() > 0) && (m_iOrigTableDepth < m_ieRTF->getPasteDepth()) )
	{
		m_ieRTF->closePastedTableIfNeeded();
		if(m_ieRTF->bUseInsertNotAppend())
		{
			m_ieRTF->insertStrux(PTX_Block);
		}
		else
		{
			m_ieRTF->getDoc()->appendStrux(PTX_Block, PP_NOPROPS);
		}
	}
	if(!m_ieRTF->isFrameIn())
	{
		m_ieRTF->addFrame(m_currentFrame);
	}
	m_ieRTF->setStruxImage(false);
	m_ieRTF->clearImageName();
	DELETEP(m_lastData);
}



bool
IE_Imp_ShpGroupParser::tokenKeyword(IE_Imp_RTF * ie, RTF_KEYWORD_ID kwID, 
									UT_sint32 param, bool /*paramUsed*/)
{
	UT_DEBUGMSG(("IE_Imp_ShpGroupParser::tokenKeyword %d\n", kwID));
	m_last_kwID = kwID;
	switch(kwID) {
	case RTF_KW_shprslt:
		UT_DEBUGMSG(("Found \\shprslt\n"));
		ie->SkipCurrentGroup(false);
		break;
	case RTF_KW_sp:
	{
		IE_Imp_ShpPropParser *parser = new IE_Imp_ShpPropParser();
		ie->StandardKeywordParser(parser);
		RTFProps_FrameProps::PropertyPair *prop = parser->getProp();
		m_currentFrame._setProperty(prop);
		delete parser;
	}
	break;
	case RTF_KW_shpleft:
		m_currentFrame.m_iLeftPos = param;
		break;
	case RTF_KW_shpright:
		m_currentFrame.m_iRightPos = param;
		break;
	case RTF_KW_shptop:
		m_currentFrame.m_iTopPos = param;
		break;
	case RTF_KW_shpbypara:
		m_currentFrame.m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		break;
	case RTF_KW_shpbymargin:
		m_currentFrame.m_iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
		break;
	case RTF_KW_shpbypage:
		m_currentFrame.m_iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
		break;
	case RTF_KW_shpwr:
		if(param == 3)
		{
			m_currentFrame.m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
		}
		else
		{
			m_currentFrame.m_iFrameWrapMode = FL_FRAME_WRAPPED_BOTH_SIDES;
		}
		break;
	case RTF_KW_shpbottom:
		m_currentFrame.m_iBotPos = param;
		break;
	case RTF_KW_shptxt:
	{
		ie->HandleShapeText(m_currentFrame);
		IE_Imp_TextParaPropParser *parser = new IE_Imp_TextParaPropParser();
		ie->StandardKeywordParser(parser);	
		delete parser;
		break;
	}
	case RTF_KW_abiframeprops:
	{
		break;
	}
	default:
		break;
	};
	return true;
}


bool IE_Imp_ShpGroupParser::tokenCloseBrace(IE_Imp_RTF * ie)
{
	switch(m_last_kwID) 
	{
	case RTF_KW_abiframeprops:
		UT_ASSERT(m_lastData);
		m_currentFrame.m_abiProps = *m_lastData;
		m_lastData = NULL;
		m_last_kwID = RTF_UNKNOWN_KEYWORD;
		break;
	default:
		break;
	}

	return IE_Imp_RTFGroupParser::tokenCloseBrace(ie);
}


bool IE_Imp_ShpGroupParser::tokenData(IE_Imp_RTF * /*ie*/, UT_UTF8String & data)
{
	DELETEP(m_lastData);
	m_lastData = new std::string(data.utf8_str());
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////

/*!
  Handle the \shp keyword
*/
void IE_Imp_RTF::HandleShape(void)
{
	// save state
	RTFStateStore * pState = m_currentRTFState.clone();
	m_stateStack.push(pState);
	m_currentRTFState.m_bInKeywordStar = false;
	
	IE_Imp_ShpGroupParser *parser = new IE_Imp_ShpGroupParser(this);
	m_bFrameStruxIn = false;
	StandardKeywordParser(parser);
	delete parser;
	
	// restore state
	pState = NULL;
	m_stateStack.pop((void**)(&pState));
	m_currentRTFState = *pState;
	delete pState;
	
	// Formely in HandleEndFrame()
	UT_DEBUGMSG((">>>>End frame\n"));
	if(!bUseInsertNotAppend()) 
	{
		if(m_bFrameTextBox)
		{
			//
			// Look if we have a bare FrameStrux. If so delete it.
			//
			pf_Frag * pf = getDoc()->getLastFrag();
			if(pf && (pf->getType() == pf_Frag::PFT_Strux))
			{
				    pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
					if(pfs->getStruxType() == PTX_SectionFrame)
					{
						getDoc()->deleteFragNoUpdate(pf);
						m_bFrameTextBox = false;
						return;
					}
			}
				
		}
		getDoc()->appendStrux(PTX_EndFrame, PP_NOPROPS);
		m_newParaFlagged = false;
	}
	else 
	{
		insertStrux(PTX_EndFrame);
		m_newParaFlagged = false;
	}
}

/*!
 * Construct all the frame properties and add the frame to the PT
 */
void IE_Imp_RTF::addFrame(RTFProps_FrameProps & frame)
{

// Flush any stored chars now.
	FlushStoredChars(true);

// OK Assemble the attributes/properties for the Frame

	PP_PropertyVector attribs = {
		"props", ""
	};
	if(isStruxImage())
	{
		attribs.push_back(PT_STRUX_IMAGE_DATAID);
		attribs.push_back(m_sImageName.utf8_str());
	}

	std::string sPropString;
	if(frame.m_abiProps.empty())
	{
		std::string sP;
		std::string sV;
		sP = "frame-type";
		m_bFrameTextBox = false;
		if(	frame.m_iFrameType == 1)
		{
			sV = "image";
			UT_std_string_setProperty(sPropString,sP,sV); 
			sP = "top-style";
			sV = "none";
			UT_std_string_setProperty(sPropString,sP,sV); 
			sP = "right-style";
			UT_std_string_setProperty(sPropString,sP,sV); 
			sP = "left-style";
			UT_std_string_setProperty(sPropString,sP,sV); 
			sP = "bot-style";
			UT_std_string_setProperty(sPropString,sP,sV); 
		}
		else
		{
			sV = "textbox";
			UT_std_string_setProperty(sPropString,sP,sV); // fixme make other types
			m_bFrameTextBox = true;
		}
		
		sP = "position-to";
		if(frame.m_iFramePositionTo == FL_FRAME_POSITIONED_TO_COLUMN)
		{
			sV = "column-above-text";
		}
		else if(frame.m_iFramePositionTo == FL_FRAME_POSITIONED_TO_PAGE)
		{
			sV = "page-above-text";
		}
		else
		{
			sV = "block-above-text";
		}
		UT_std_string_setProperty(sPropString,sP,sV); // fixme make other types
		
		sP = "wrap-mode";
		if(frame.m_iFrameWrapMode == FL_FRAME_ABOVE_TEXT)
		{
			sV = "above-text";
		}
		else
		{
			sV = "wrapped-both";
		}
		UT_std_string_setProperty(sPropString,sP,sV); // fixme make other types
		if(frame.m_iBackgroundColor > 0)
		{
			sP = "bg-style";
			if(frame.m_iFillType == 0)
			{
				sV = "solid";
			}
			else
			{
				sV = "none";
			}
			UT_std_string_setProperty(sPropString,sP,sV);
			sP="bgcolor";
			// RTF uses BGR encoding for colors while Abiword uses RGB 
			UT_sint32 iRGBColor = (((frame.m_iBackgroundColor & 0xff0000) >> 16) |
								   ((frame.m_iBackgroundColor & 0xff00)) |
								   ((frame.m_iBackgroundColor & 0xff) << 16));
			sV = UT_std_string_sprintf("%06x",iRGBColor);
			UT_std_string_setProperty(sPropString,sP,sV);
			sP="background-color";
			UT_std_string_setProperty(sPropString,sP,sV);
		}
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			
			//
			// Shift positions a little for pasted frames so they don't
			// appear right on top of other frames
			//
			double dOff = 0.0;
			if(bUseInsertNotAppend())
			{
				dOff = 0.05; // 0.1 inches
				dOff += 0.2*static_cast<double>(UT_rand())/static_cast<double>(UT_RAND_MAX);
			}
			double dV = dOff + static_cast<double>(frame.m_iLeftPos)/1440.0;
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "xpos";
			UT_std_string_setProperty(sPropString,sP,sV);
			sP= "frame-col-xpos";
			UT_std_string_setProperty(sPropString,sP,sV);
			sP= "frame-page-xpos";
			UT_std_string_setProperty(sPropString,sP,sV);
			
			dV = dOff + static_cast<double>(frame.m_iTopPos)/1440.0;
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "ypos";
			UT_std_string_setProperty(sPropString,sP,sV);
			sP= "frame-col-ypos";
			UT_std_string_setProperty(sPropString,sP,sV);
			sP= "frame-page-ypos";
			UT_std_string_setProperty(sPropString,sP,sV);
			
			dV = static_cast<double>(frame.m_iRightPos - frame.m_iLeftPos)/1440.0;
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "frame-width";
			UT_std_string_setProperty(sPropString,sP,sV); 
			
			dV = static_cast<double>(frame.m_iBotPos - frame.m_iTopPos)/1440.0;
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "frame-height";
			UT_std_string_setProperty(sPropString,sP,sV); 
			
			dV = static_cast<double>(frame.m_iRightPad + frame.m_iLeftPad)/9114400.0; // EMU
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "xpad";
			UT_std_string_setProperty(sPropString,sP,sV); 
			
			dV = static_cast<double>(frame.m_iBotPad + frame.m_iTopPad)/9114400.0; //EMU
			sV= UT_std_string_sprintf("%fin",dV);
			sP= "ypad";
			UT_std_string_setProperty(sPropString,sP,sV); 
		}
	}
	else
	{
		sPropString = frame.m_abiProps;
	}
	attribs[1] = sPropString;

	UT_DEBUGMSG(("Start Frame\n"));
	if(!bUseInsertNotAppend())
	{
		getDoc()->appendStrux(PTX_SectionFrame, attribs);
	}
	else
	{
		insertStrux(PTX_SectionFrame, attribs, PP_NOPROPS);
	}
	m_bFrameStruxIn = true;
}

/*!
 * Handle the text inside the shape.
 */
void IE_Imp_RTF::HandleShapeText(RTFProps_FrameProps & frame)
{

// Ok this is the business end of the frame where we actually insert the
// Strux.

	UT_DEBUGMSG(("Doing Handle shptxt \n"));
	if(!m_bFrameStruxIn)
	{
		addFrame(frame);
	}
	setStruxImage(false);
}


/*!
  Handle the shppict RTF keyword.
*/
void IE_Imp_RTF::HandleShapePict()
{
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint32 parameter = 0;
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
			UT_ASSERT(RTF_KW_cell  != keywordID);
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
	} while (!((tokenType == RTF_TOKEN_CLOSE_BRACE) && (nested <= 1)));
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
	UT_sint32 parameter = 0;
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
			UT_ASSERT(RTF_KW_cell  != keywordID);
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
			case RTF_KW_pict:
				if (beginResult <= nested) {
					HandlePicture();
				}
				break;
			case RTF_KW_shppict:
				if (beginResult <= nested) {
					HandleShapePict();
				}
				break;
			case RTF_KW_nonshppict:
				UT_DEBUGMSG(("Hub: skip nonshppict in \\object\n"));
				SkipCurrentGroup(false);
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
	} while (!((tokenType == RTF_TOKEN_CLOSE_BRACE) && (nested <= 1)));

	return true;
}

