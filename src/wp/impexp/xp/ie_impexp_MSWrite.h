/* AbiWord
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

// $Id$


#ifndef __IE_IMPEXP_MSWRITE_H__
#define __IE_IMPEXP_MSWRITE_H__

#include "ut_types.h"

/*****************************************************************/
/* Write file format definitions                                 */
/*****************************************************************/

// format is little endian...

// word 0
enum {
   IDENT =	0xBE31,
   IDENT_OLE =	0xBE32
};

// word 1
// always 0

// word 2
#define TOOL_WORD	0xAB00

// word 3-6
// reserved : always 0

// word 7-8
// fcMac

// word 9
// pnPara

// word 10
// pnFntb

// word 11
// pnSep

// word 12
// pnSetb

// word 13
// pnPgtb

// word 14
// pnFfntb

// word 15-47
// always 0. Reserved for Word

// word 48
// pnMac


enum {
   PAGE_SIZE = 128
};

/*
  File header descriptor.
  This struct contain the basic description of the file....
   */
typedef struct {
   UT_uint16 wIdent;
   UT_uint32 fcMac;
   UT_uint16 pnPara;
   UT_uint16 pnFntb;
   UT_uint16 pnSep;
   UT_uint16 pnSetb;
   UT_uint16 pnPgtb;
   UT_uint16 pnFfntb;
   UT_uint16 pnMac;
   UT_uint16 pnChar;		//must be computed = (fcMac + 127) / 128
} WRI_write_file_desc_t;


/*
	format descriptor
*/

/* page of formatting information... sizeof = 128 */
typedef struct {
   UT_uint32 fcFirst;
   UT_Byte data[123];      // this field is in the file endian
   UT_Byte cFod;
} WRI_format_page_t;

typedef struct {
   UT_uint32 fcLim;
   UT_uint16 bfProp;
} WRI_fod_t;


typedef struct {
   UT_Byte cch;		//num of bytes
   char rgchProp[1] ;   //size is unknown
} WRI_fprop_t;

/*
  Character property...
*/
typedef struct {
   UT_Byte ignored;
   UT_Byte styleFont;
   UT_Byte hps;			//half-point size
   UT_Byte styleOther;
   UT_Byte styleFtcExtra;
   UT_Byte hpsPos;
} WRI_chp_t;

/*
  picture header for pictures...
  need to define METAFILEPICT from Windows headers...
*/


#if !defined(_WIN32)
/*
  Some windoze code declaration...
*/

typedef void* HMETAFILE;

typedef struct tagMETAFILEPICT {
   UT_sint32        mm;
   UT_sint32        xExt;
   UT_sint32        yExt;
   HMETAFILE   hMF;
} METAFILEPICT;

#endif


typedef struct {
   METAFILEPICT mfp;		//verifier que les tailles correspondent bien
   UT_uint16 dxaOffset;
   UT_uint16 dxaSize;
   UT_uint16 dyaSize;
   UT_uint16 cbOldSize;
   UT_Byte bm[14];
   UT_uint16 cbHeader;
   UT_uint32 cbSize;
   UT_uint16 mx;
   UT_uint16 my;
} picture_header;


enum {
   PICT_MM_BITMAP = 0xE3
};

/*
  OLE header for OLE embedded objects.
*/
typedef struct {
   UT_Byte mm;
   UT_uint32 unused1;
   UT_uint16 objectType;
   UT_uint16 dxaSize;
   UT_uint16 dyaSize;
   UT_uint16 unused2;
   UT_uint32 dwDataSize;
   UT_uint32 unused3;
   UT_uint32 dwObjNul;
   UT_uint16 unused4;
   UT_uint16 cbHeader;
   UT_uint32 unused5;
   UT_uint16 mx;
   UT_uint16 my;
   UT_Byte content;	//up to dwDataSize - 1 bytes following.
} ole_header;


enum {
   OLE_MM = 0xE4,
   
   OLE_OT_STATIC = 1,
   OLE_OT_EMBEDDED = 2,
   OLE_OT_LINK = 3
};

#if !defined (_WIN32)
/*
  Font families as defined in WINGDI.H
*/

#define FF_DONTCARE         (0<<4)  /* Don't care or don't know. */
#define FF_ROMAN            (1<<4)  /* Variable stroke width, serifed. */
                                    /* Times Roman, Century Schoolbook, etc. */
#define FF_SWISS            (2<<4)  /* Variable stroke width, sans-serifed. */
                                    /* Helvetica, Swiss, etc. */
#define FF_MODERN           (3<<4)  /* Constant stroke width, serifed or sans-serifed. */
                                    /* Pica, Elite, Courier, etc. */
#define FF_SCRIPT           (4<<4)  /* Cursive, etc. */
#define FF_DECORATIVE       (5<<4)  /* Old English, etc. */


#endif


enum {
   NO_ERROR = 0,
   OS_ERROR = 1,
   BAD_FORMAT = 2,
   BUFFER_TOO_SMALL = 3,
   NO_MORE_DATA = 4
};




/*****************************************************************/
/* Wrapper classes                                               */
/*****************************************************************/

class WRI_Format_Page
{
 public:
   WRI_Format_Page (const WRI_format_page_t * data);
   virtual ~WRI_Format_Page ();
   const WRI_fod_t *getFod (int i) const;
 private:
   WRI_format_page_t *mData;
   WRI_fod_t         *mFods;    /* all the fods extracted */
   void LoadFods ();
};



#endif








