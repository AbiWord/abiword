/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1998-2000 Hubert Figuiere
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
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_impexp_MSWrite.h"
#include "ie_imp_MSWrite.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*****************************************************************/
/* Generic IO                                                    */
/*****************************************************************/

// FIXME : should be guessed from the build target
#define __LITTLE_ENDIAN 1

#if defined (__LITTLE_ENDIAN) && (__LITTLE_ENDIAN == 1)
#define ReadWord16(file, value)		{ fread(&(value), sizeof(UT_uint16), 1, (file)); }
#else
#define ReadWord16(file, value)		{ fread(&(value), sizeof(UT_uint16), 1, (file)); \
											value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8); }
#endif


#if defined (__LITTLE_ENDIAN) && (__LITTLE_ENDIAN == 1)
#define ReadWord32(file, value)		{ fread(&(value), sizeof(UT_uint32), 1, (file)); }
#else
#define ReadWord32(file, value)		\
   { UT_uint16 w; UT_uint32 tmp; ReadWord16(file, w); value = w; \
		   ReadWord16(file, w); tmp = w; value |= tmp << 16; }
#endif


/*****************************************************************/
/*****************************************************************/

bool IE_Imp_MSWrite_Sniffer::recognizeContents(const char * szBuf, 
											   UT_uint32 iNumbytes)
{
    if ( iNumbytes > 8 )
    {
        if ( szBuf[0] == (char)0x31 && szBuf[1] == (char)0xbe &&
             szBuf[2] == (char)0 && szBuf[3] == (char)0 )
        {
            return(true);
        }
    }
    return(false);
}

bool IE_Imp_MSWrite_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (UT_stricmp(szSuffix,".wri") == 0);
}


UT_Error IE_Imp_MSWrite_Sniffer::constructImporter(PD_Document * pDocument,
												   IE_Imp ** ppie)
{
    IE_Imp_MSWrite * p = new IE_Imp_MSWrite(pDocument);
    *ppie = p;
    return UT_OK;
}

bool	IE_Imp_MSWrite_Sniffer::getDlgLabels(const char ** pszDesc,
											 const char ** pszSuffixList,
											 IEFileType * ft)
{
    *pszDesc = "MS-Write (.wri)";
    *pszSuffixList = "*.wri";
    *ft = getFileType();
    return true;
}

/*****************************************************************/
/*****************************************************************/

/*
  Import MS Write file.
*/

static const char PROPS_XML_ATTR_NAME[] = "props";

// char attributes
static const char FONT_FAMILY[] = "font-family:%s;";

static const char FONT_STYLE[] = "font-style:%s;";
static const char FONT_STYLE_ITALIC[] = "italic";

static const char FONT_WEIGHT[] = "font-weight:%s;";
static const char FONT_WEIGHT_BOLD[] = "bold";

static const char FONT_SIZE[] = "font-size:%spt;";



/*****************************************************************/
/*****************************************************************/

#define X_CleanupIfError(ies,exp)	do { if (((ies)=(exp)) != UT_OK) goto Cleanup; } while (0)

UT_Error IE_Imp_MSWrite::importFile(const char * szFilename)
{
    FILE *fp = fopen(szFilename, "r");
    if (!fp)
    {
        UT_DEBUGMSG(("Could not open file %s\n",szFilename));
        return UT_IE_FILENOTFOUND;
    }
    
    UT_Error iestatus;
    
    X_CleanupIfError(iestatus,_writeHeader(fp));
    X_CleanupIfError(iestatus,_parseFile(fp));
    
    iestatus = UT_OK;
    
 Cleanup:
    fclose(fp);
    return iestatus;
}

#undef X_CleanupIfError

/*****************************************************************/
/*****************************************************************/

IE_Imp_MSWrite::~IE_Imp_MSWrite()
{
}

IE_Imp_MSWrite::IE_Imp_MSWrite(PD_Document * pDocument)
    : IE_Imp(pDocument)
{
}

/*****************************************************************/
/*****************************************************************/

#define X_ReturnIfFail(exp,ies)		do { bool b = (exp); if (!b) return (ies); } while (0)
#define X_ReturnNoMemIfError(exp)	X_ReturnIfFail(exp,UT_IE_NOMEMORY)

UT_Error IE_Imp_MSWrite::_writeHeader(FILE * /* fp */)
{
    X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));
    
    return UT_OK;
}

UT_Error IE_Imp_MSWrite::_parseFile(FILE * fp)
{
    UT_sint32 err;
    
    int textPosPointer, fodPosPointer;
    WRI_format_page_t aFormatPage, aFormatPage2;
    WRI_format_page_t *pFmt1 = &aFormatPage;
    WRI_format_page_t *pFmt2 = &aFormatPage2;
    
    WRI_write_file_desc_t fileDesc;
    
    err = ReadFileDesc (fp, fileDesc);
    if (err == NO_ERROR) {
        
        UT_uint32 sizeToRead = fileDesc.fcMac;
        UT_uint32 rangeBeginning = 0,
                     rangeEnding = 0;
        
        fodPosPointer = fileDesc.fcMac;
        // move to the beginning of the text data
        textPosPointer = PAGE_SIZE * 1;
        fseek(fp, textPosPointer, SEEK_SET);
        
        ReadFormatPage (fp, fodPosPointer, pFmt1);
        do 
        {
            ReadFormatPage (fp, fodPosPointer, pFmt2);
            
            // where does begin the current formatting page
            rangeBeginning = pFmt1->fcFirst;
            rangeEnding = pFmt2->fcFirst;
            
            
            if (ReadTextRangeWithFormat (fp, sizeToRead, *pFmt1) == UT_OK) 
            {
            }
        }
        while (rangeEnding < fodPosPointer - 1);
        
        
        // FIXIT : maybe we should remove this case since the file is supposed
        // to bo consistent
        /*
          if (gbBlock.getLength() > 0)
          {
          // if we have text left over (without final CR/LF),
          // create a paragraph and emit the text now.
          X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
          X_ReturnNoMemIfError(m_pDocument->appendSpan(gbBlock.getPointer(0), 
          gbBlock.getLength()));
          }
        */
    }
    else 
    {
        return UT_ERROR;
    }
    
    return UT_OK;
}


UT_Error
IE_Imp_MSWrite::ReadTextRangeWithFormat (FILE * fp, const UT_uint32 end, 
                                         const WRI_format_page_t & fmtData)
{
    bool bEatLF = false;
    UT_uint32 count;
    UT_Byte c;
    UT_GrowBuf gbBlock(1024);
    UT_UCSChar uc   ;
    UT_uint32 nextPropPos;      //next property position
    int currentFod = 0;
    
    WRI_Format_Page formatting (&fmtData);
    
    
    // extract a FOD
    
    count = ftell (fp);
    
    //   end = formatting.getFod(currentFod)->fcLim;
    
    while ((count < end) && 
           (fread(&c, 1, sizeof(c), fp) > 0))
    {
        do 
        {
            count++;
            
            switch (c)
            {
            case '\r':
            case '\n':
                if ((c == '\n') && bEatLF)
                {
                    bEatLF = false;
                    break;
                }
                
                if (c == '\r')
                {
                    bEatLF = true;
                }
                
                // a paragraph is delimited by a CRLF as specified 
                // by the file format specification
                
                X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, NULL));
                if (gbBlock.getLength() > 0)
                {
                    X_ReturnNoMemIfError(
                        m_pDocument->appendSpan(gbBlock.getPointer(0), 
                                                gbBlock.getLength()));
                    gbBlock.truncate(0);
                }
                break;
            case 0x09:
                // handle tabs
                uc = UCS_VTAB;
                X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
                break;
            case 0x0C:
                // handle form feeds
                uc = UCS_FF;
                X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
                break;
            default:
                bEatLF = false;
                
                // deal with plain character.
                // this cast is OK.  we have US-ASCII (actually Latin-1) character
                // data, so we can do this.
                
                // TODO consider scanning for CP1252 
                // because actually it is CP1252 and NOT Latin-1
                
                UT_UCSChar uc = (UT_UCSChar) c;
                X_ReturnNoMemIfError(gbBlock.ins(gbBlock.getLength(),&uc,1));
                break;
            }
        } while (count <= nextPropPos);
        // we read the corresponding text.
        
        // now add the atttributes.
        XML_Char * propsBuffer = (XML_Char *)malloc (1024 * sizeof (XML_Char));
        
        const XML_Char* propsArray[3];
        propsArray[0] = PROPS_XML_ATTR_NAME;
        propsArray[1] = propsBuffer;
        propsArray[2] = NULL;
        
        m_pDocument->appendFmt (propsArray);
        free (propsBuffer);
    } 
    
    return UT_OK;
}


#undef X_ReturnNoMemIfError
#undef X_ReturnIfFail


/*****************************************************************/
/*****************************************************************/

//
// What is this for ? I don't know...
// FIXME
//
void IE_Imp_MSWrite::pasteFromBuffer(PD_DocumentRange * pDocRange,
                                     unsigned char * pData, UT_uint32 lenData)
{
    UT_ASSERT(m_pDocument == pDocRange->m_pDoc);
    UT_ASSERT(pDocRange->m_pos1 == pDocRange->m_pos2);
    
    UT_GrowBuf gbBlock(1024);
    bool bEatLF = false;
    bool bSuppressLeadingParagraph = true;
    bool bInColumn1 = true;
    unsigned char * pc;
    
    PT_DocPosition dpos = pDocRange->m_pos1;
    
    for (pc=pData; (pc<pData+lenData); pc++)
    {
        unsigned char c = *pc;
        
        switch (c)
        {
        case '\r':
        case '\n':
            if ((c == '\n') && bEatLF)
            {
                bEatLF = false;
                break;
            }
            
            if (c == '\r')
            {
                bEatLF = true;
            }
            
            // we interprete either CRLF, CR, or LF as a paragraph break.
            
            if (gbBlock.getLength() > 0)
            {
                // flush out what we have
                m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
                dpos += gbBlock.getLength();
                gbBlock.truncate(0);
            }
            bInColumn1 = true;
            break;
            
        default:
            bEatLF = false;
            if (bInColumn1 && !bSuppressLeadingParagraph)
            {
                m_pDocument->insertStrux(dpos,PTX_Block);
                dpos++;
            }
            
            // deal with plain character.
            // this cast is OK.  we have US-ASCII (actually Latin-1) character
            // data, so we can do this.
            
            UT_UCSChar uc = (UT_UCSChar) c;
            gbBlock.ins(gbBlock.getLength(),&uc,1);
            
            bInColumn1 = false;
            bSuppressLeadingParagraph = false;
            break;
        }
    } 
    
    if (gbBlock.getLength() > 0)
    {
        // if we have text left over (without final CR/LF),
        m_pDocument->insertSpan(dpos, gbBlock.getPointer(0), gbBlock.getLength());
        dpos += gbBlock.getLength();
    }
    
    return;
}

/*****************************************************************/
/*****************************************************************/


///////////////////////////////////////////////////////////////////
UT_uint32 IE_Imp_MSWrite::ReadFileDesc(FILE * file, 
                                       WRI_write_file_desc_t & desc)
{
    int i;
    UT_uint16 current;
    
    fseek(file, 0, SEEK_SET);
    //0
    ReadWord16(file, desc.wIdent);
    if ((desc.wIdent != IDENT) && (desc.wIdent != IDENT_OLE)) {
        return BAD_FORMAT;
    }
    //1
    ReadWord16(file, current);
    if (current != 0) {
        return BAD_FORMAT;
    }
    //2
    ReadWord16(file, current);
    if (current != TOOL_WORD) {
        return BAD_FORMAT;
    }
    for (i = 3; i <= 6; i++) {
        ReadWord16(file, current);
        if (current != 0) {
            return BAD_FORMAT;
        }
    }
    
    ReadWord32(file, desc.fcMac);
    ReadWord16(file, desc.pnPara);
    ReadWord16(file, desc.pnFntb);
    ReadWord16(file, desc.pnSep);
    ReadWord16(file, desc.pnSetb);
    ReadWord16(file, desc.pnPgtb);
    ReadWord16(file, desc.pnFntb);
    
    //reserved for Word
    fseek(file, 48, SEEK_SET);
    
    ReadWord16(file, desc.pnMac);
    
    desc.pnChar = (desc.fcMac + 127) / 128;
    
    return NO_ERROR;
}



///////////////////////////////////////////////////////////////////
UT_uint32 
IE_Imp_MSWrite::ReadText(FILE * file, const WRI_write_file_desc_t & desc, 
                                   UT_uint16 pageNumber,/* start at 1 */
                                   UT_Byte * buf, size_t & bufLen)
   /*
     Reads the text at page pageNumber into buf which has bufLen bytes
     len. Then bufLen contains the num of byte reads.
     pageNumber 0 is invalid !! It the header...
     
     Currently the pictures are read as is and might generate some unwanted
     chars in the text stream.
     Separating picture will come when decoding paragraph informations.
   */
{
    size_t sizeToWrite;
    size_t numRemaining = desc.fcMac - (pageNumber * 128);
    
    sizeToWrite = (numRemaining > bufLen ? bufLen : numRemaining);
    
    fseek(file, PAGE_SIZE * pageNumber, SEEK_SET);
    bufLen = fread(buf, 1, sizeToWrite, file);
    
    return NO_ERROR;
}


/////////////////////////////////////////////////////////////////
UT_uint32 
IE_Imp_MSWrite::ReadFormatPage (FILE * file, int & pos, 
                                          WRI_format_page_t * aPage)
{
    // FIXME : check error code
    
    int filePos = ftell (file);
    
    fseek (file, pos, SEEK_SET);
    
    ReadWord32 (file, aPage->fcFirst);
    fread (&aPage->data, 1, sizeof (aPage->data), file);
    fread (&aPage->cFod, 1, sizeof (aPage->cFod), file);
    
    // restore file position to return in the previous position, 
    // probably the text stream
    pos = ftell (file);
    fseek (file, filePos, SEEK_SET);
    
    return NO_ERROR;
}




/////////////////////////////////////////////////////////////////
XML_Char * 
IE_Imp_MSWrite::MakeProperties (XML_Char * buf, const size_t bufSize,
                                XML_Char * attr, XML_Char * value)
   //  build a properties XML pair and add it to the buffer.
   //  return buf.
   //
   //  TODO: make buf dynamically reallocated if needed.
{
    XML_Char * temp = (XML_Char *)malloc ((strlen (attr) + strlen (value) + 1) 
                                          * sizeof (XML_Char));
    // here we know that 'temp' has the right size. No overflow possible
    sprintf (temp, attr, value);
    strncat (buf, temp, bufSize);
    free (temp);
    
    return buf;
}



//  UT_uint32 IE_Imp_MSWrite::ReadStyle(FILE * file, const write_file_desc & desc, 
//                                      UT_uint16 pageNumber,/* start at 1 */
//                                      text_run_array * & buf, size_t textLen)
//     /*
//       Return the text run array for the text starting a pageNumber.
     
//       Typically this function should be called right after a ReadText with
//       bufLen == textLen so that you retrieve both the text and formatting. 
     
//       buf will be allocated and must be freed by the caller. input value
//       is IGNORED.
     
//       All offsets are relative to the beginning of the document and not of the 
//       current data seek.
//     */
//  {
//     size_t remainingLen = textLen;
//     UT_uint16 currentPage = pageNumber + desc.pnPara - 1;
//     fseek(file, PAGE_SIZE * currentPage);
   
   
//     while (remainingLen > 0) {
//        ReadPageStyle(buf, currentPage, remainingLen);
//        currentPage++;
//     }
   
//     return NO_ERROR;
//  }

//  UT_uint32 IE_Imp_MSWrite::ReadPageStyle(text_run_array * & buf, 
//                                          UT_uint16 currentPage, 
//                                          size_t & remainingLen)
//  {
//     char page[PAGE_SIZE];
//     UT_Byte numOfFod;
   
//     fread(file, 1, PAGE_SIZE, page);
   
//     numOfFod = page[127];
//  }











