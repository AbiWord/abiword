/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "ut_bytebuf.h"
#include "ut_units.h"
#include "pd_Document.h"

#include "ie_types.h"
#include "ie_imp_MsWord_97.h"

#define X_ReturnIfFail(exp,error)     do { UT_Bool b = (exp); if (!b) return (error); } while (0)
#define X_ReturnNoMemIfError(exp)   X_ReturnIfFail(exp,UT_IE_NOMEMORY)

#define X_CheckError(v)         do {  if (!(v))                             \
                                      {  m_error = UT_ERROR;            \
                                         return; } } while (0)
#define X_CheckError0(v)        do {  if (!(v))                             \
                                      {  m_error = UT_ERROR;            \
                                         return 0; } } while (0)


int CharProc(wvParseStruct *ps,U16 eachchar,U8 chartype, U16 lid);
int SpecCharProc(wvParseStruct *ps,U16 eachchar, CHP* achp);
int ElementProc(wvParseStruct *ps,wvTag tag, void *props, int dirty);
int DocProc(wvParseStruct *ps,wvTag tag);

// a little look-up table for mapping Word text colors 
// (the comments) to Abiword's superior RGB color encoding
static int word_colors[][3] = {
   {0x00, 0x00, 0x00}, /* black */
   {0x00, 0x00, 0xff}, /* blue */
   {0x00, 0xff, 0xff}, /* cyan */
   {0x00, 0xff, 0x00}, /* green */
   {0xff, 0x00, 0xff}, /* magenta */
   {0xff, 0x00, 0x00}, /* red */
   {0xff, 0xff, 0x00}, /* yellow */
   {0xff, 0xff, 0xff}, /* white */
   {0x00, 0x00, 0x80}, /* dark blue */
   {0x00, 0x80, 0x80}, /* dark cyan */
   {0x00, 0x80, 0x00}, /* dark green */
   {0x80, 0x00, 0x80}, /* dark magenta */
   {0x80, 0x00, 0x00}, /* dark red */
   {0x80, 0x80, 0x00}, /* dark yellow */
   {0x80, 0x80, 0x80}, /* dark gray */
   {0xc0, 0xc0, 0xc0}, /* light gray */
};


/*****************************************************************/

UT_Error IE_Imp_MsWord_97::importFile(const char * szFilename)
{
	FILE *fp = NULL;
	UT_DEBUGMSG(("got to import file\n"));

	fp = fopen(szFilename, "rb");
	if (!fp)
	{
	   UT_DEBUGMSG(("Could not open file %s\n",szFilename));
	   m_error = UT_IE_FILENOTFOUND;
	   return m_error;
	}
	UT_DEBUGMSG(("wv importer\n"));
	fclose(fp);

	wvParseStruct ps;
	if (wvInitParser(&ps,(char *)szFilename))
		{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		wvOLEFree();
		m_error = UT_IE_BOGUSDOCUMENT;
		return m_error;
		}
	
	UT_DEBUGMSG(("just here\n"));
	ps.userData = this;
	wvSetElementHandler(ElementProc);
	wvSetCharHandler(CharProc);
        wvSetSpecialCharHandler(SpecCharProc);
        wvSetDocumentHandler(DocProc);

	wvText(&ps);

	wvOLEFree();

   	m_error = UT_OK;
	return m_error;
}

int CharProc(wvParseStruct *ps,U16 eachchar,U8 chartype,U16 lid)
	{
	   IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	   
	   // convert incoming character to unicode
	   if (chartype)
	     eachchar = wvHandleCodePage(eachchar, lid);

	   UT_DEBUGMSG(("word 97 char is %c (%d), type is %d\n",eachchar,(int)eachchar,chartype));

	   // take care of any oddities in Microsoft's character "encoding"
	   // TODO: does the above code page handler take care of these?
	   if (chartype == 1 && eachchar == 146) eachchar = 39; // apostrophe

	   // marks, breaks, etc.
	   switch (eachchar)
	     {
	      case 13: // paragraph end
		return 0;
	      case 11: // hard line break
		UT_DEBUGMSG(("a line break\n"));
		eachchar = UCS_LF;
		break;
	      case 12: // page breaks, section marks
		// flush current text buffer
		pDocReader->_charData(pDocReader->m_pTextRun, pDocReader->m_iTextRunLength);
		pDocReader->m_iTextRunLength = 0;
		// we'll go ahead and always add this as a page break.
		// when we hit an end-of-section, we'll remove it if it's
		// the last character in the text buffer. since we just 
		// flushed it above, there's no chance of auto-flushing
		// before the end-of-section hits.
		UT_DEBUGMSG(("a page break/section mark\n"));
		eachchar = UCS_FF;
		break;
	      case 14: // column break
		UT_DEBUGMSG(("a column break\n"));
		eachchar = UCS_VTAB;
		break;
	      case 19: // field begin
		// flush current text buffer
		pDocReader->_charData(pDocReader->m_pTextRun, pDocReader->m_iTextRunLength);
		pDocReader->m_iTextRunLength = 0;
		UT_DEBUGMSG(("a field is beginning\n"));
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		return 0;
	      case 20: // field separator
		UT_DEBUGMSG(("a field separator\n"));
		ps->fieldmiddle = 1;
		return 0;
	      case 21: // field end
		UT_DEBUGMSG(("a field has ended\n"));
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		return 0;
	     }

	   // HACK: it seems the text which is displayed by a field is contained
	   // HACK: after the field separator. since I haven't written real field
	   // HACK: import support, yet, this will fake it somewhat...
	   if (ps->fieldstate && !ps->fieldmiddle) return 0;
	   
	   // add character to our current text run
	   pDocReader->m_pTextRun[pDocReader->m_iTextRunLength++] = 
	     (UT_UCSChar) eachchar;
	   
	   if (pDocReader->m_iTextRunLength == 
	       pDocReader->m_iTextRunMaxLength) 
	     {
		// we can't hold any more characters in this run,
		// so send what we currently have
		int iRes = pDocReader->_charData(pDocReader->m_pTextRun, pDocReader->m_iTextRunLength);
		pDocReader->m_iTextRunLength = 0;
		return iRes;
	     }
	   else
	     {
		return 0;
	     }
	}

int SpecCharProc(wvParseStruct *ps, U16 eachchar, CHP* achp)
{
   IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;

   Blip blip;
   wvStream *fil;	
   long pos;
   FSPA * fspa;
   PICF picf;
   FDOA * fdoa;
   
   // TODO: handle special characters (images, objects, fields(?))

   switch (eachchar)
     {
      case 19: // field begin
	// flush current text buffer
	pDocReader->_charData(pDocReader->m_pTextRun, pDocReader->m_iTextRunLength);
	pDocReader->m_iTextRunLength = 0;
	UT_DEBUGMSG(("a field is beginning\n"));
	ps->fieldstate++;
	ps->fieldmiddle = 0;
	return 0;
      case 20: // field separator
	UT_DEBUGMSG(("a field separator\n"));
	ps->fieldmiddle = 1;
	return 0;
      case 21: // field end
	UT_DEBUGMSG(("a field has ended\n"));
	ps->fieldstate--;
	ps->fieldmiddle = 0;
	return 0;
     }
   
   /* it seems some fields characters slip through here which tricks
    * the import into thinking it has an image with it really does
    * not. this catches special characters in a field
    */
   if (ps->fieldstate) {
      /* HACK: we should do something with these field characters... */
      return 0;
   }
   
   switch (eachchar) 
     {
      case 0x01:
	
	if (achp->fOle2) {
	   // TODO: support embedded OLE2 components...
	   UT_DEBUGMSG(("embedded OLE2 component. currently unsupported"));
	   return 0;
	}
	
	pos = wvStream_tell(ps->data);
	
	wvStream_goto(ps->data, achp->fcPic_fcObj_lTagObj);
	
	wvGetPICF(wvQuerySupported(&ps->fib, NULL), &picf, ps->data);
	
	fil = picf.rgb;
	
	if (wv0x01(&blip, fil, picf.lcb - picf.cbHeader))
	  {
	     pDocReader->_handleImage(&blip, picf.dxaGoal, picf.dyaGoal);
	  }
	
	wvStream_goto(ps->data, pos);

	return 0;
	break;
	
      case 0x08:
	
	if (wvQuerySupported(&ps->fib, NULL) == WORD8)
	  {
	  if(ps->nooffspa>0)
    	     {
 
	     	fspa = wvGetFSPAFromCP(ps->currentcp, ps->fspa,
		    			    ps->fspapos, ps->nooffspa);
	     
	     	if (wv0x08(&blip, fspa->spid, ps))
	        {
		    pDocReader->_handleImage(&blip, fspa->xaRight-fspa->xaLeft,
					   fspa->yaBottom-fspa->yaTop);
	      	}
	    }
	  else
	    {
	      UT_DEBUGMSG(("nooffspa was <=0 -- ignoring"));
	    } 
	  }
	else
	  {
	     UT_DEBUGMSG(("pre Word8 0x08 graphic -- unsupported at the moment"));
	     fdoa = wvGetFDOAFromCP(ps->currentcp, NULL, ps->fdoapos, ps->nooffdoa);
	     
	     // TODO: do something with the data in this fdoa someday...
	     
	  }
	
	return 0;
	break;
	
     }
   
   return 0;
}

int DocProc(wvParseStruct *ps,wvTag tag)
	{
	IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	return(pDocReader->_docProc(ps, tag));
	}

int ElementProc(wvParseStruct *ps,wvTag tag,void *props, int dirty)
	{
	IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	UT_DEBUGMSG(("element tag = %d\n", tag));
	return(pDocReader->_eleProc(ps, tag, props, dirty));
	}

int IE_Imp_MsWord_97::_charData(UT_UCSChar * charstr, int len)
	{
	   if (len)
	     X_CheckError0(m_pDocument->appendSpan(charstr, len));
	   return(0);
	}

int IE_Imp_MsWord_97::_docProc(wvParseStruct * ps, wvTag tag)
	{
	   if (m_iTextRunLength)
	     {
		// flush any text in the current run
		int iRes = _charData(m_pTextRun, m_iTextRunLength);
		m_iTextRunLength = 0;
		UT_ASSERT(iRes == 0);
	     }
		
	   
	   switch(tag)
		{
		case DOCBEGIN:
		   /* a section will be started in the eleProc handler */
		   break;
		case DOCEND:	
		   /*abiword doesn't need this*/
		default:
		   break;
		}
	return(0);
	}

int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps, wvTag tag, void *props, int dirty)
	{
	XML_Char propBuffer[1024];
	XML_Char* pProps = "PROPS";
	const XML_Char* propsArray[3];
	   
	propBuffer[0] = 0;
	UT_DEBUGMSG(("element started\n"));
	PAP *apap;
	CHP *achp;
	SEP *asep;
	int iRes;
	   
	switch(tag)
	     {
	     	  case SECTIONBEGIN:
		
		  // flush character run
		  iRes = _charData(m_pTextRun, m_iTextRunLength);
		  m_iTextRunLength = 0;
		  UT_ASSERT(iRes == 0);

		   UT_DEBUGMSG(("section properties...\n"));
		   asep = (SEP*)props;

		   // page margins
		   // -left
		   sprintf(propBuffer + strlen(propBuffer),
			   "page-margin-left:%s;", 
			   UT_convertToDimensionString(DIM_IN, (((float)asep->dxaLeft) / 1440), "1.4"));
		   // -right
		   sprintf(propBuffer + strlen(propBuffer),
			   "page-margin-right:%s;", 
			   UT_convertToDimensionString(DIM_IN, (((float)asep->dxaRight) / 1440), "1.4"));
		   // -top
		   sprintf(propBuffer + strlen(propBuffer),
			   "page-margin-top:%s;", 
			   UT_convertToDimensionString(DIM_IN, (((float)asep->dyaTop) / 1440), "1.4"));
		   // -left
		   sprintf(propBuffer + strlen(propBuffer),
			   "page-margin-bottom:%s;", 
			   UT_convertToDimensionString(DIM_IN, (((float)asep->dyaBottom) / 1440), "1.4"));

		   // columns
		   if (asep->ccolM1) {
		      // number of columns
		      sprintf(propBuffer + strlen(propBuffer),
			      "columns:%d;", (asep->ccolM1+1));
		      // gap between columns
		      sprintf(propBuffer + strlen(propBuffer),
			      "column-gap:%s;", 
			      UT_convertToDimensionString(DIM_IN, (((float)asep->dxaColumns) / 1440), "1.4"));
		   }
		   
		   // space after section 
		   // TODO: this is the gutter, right?
		   sprintf(propBuffer + strlen(propBuffer),
			   "section-space-after:%s;",
			   UT_convertToDimensionString(DIM_IN, (((float)asep->dzaGutter) / 1440), "1.4"));

		   // remove trailing semi-colon
		   propBuffer[strlen(propBuffer)-1] = 0;

		   propsArray[0] = pProps;
		   propsArray[1] = propBuffer;
		   propsArray[2] = NULL;
		   UT_DEBUGMSG(("the section propBuffer is %s\n",propBuffer));
		   X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, propsArray));
		   break;

		 case PARABEGIN:

		   // flush character run
		   iRes = _charData(m_pTextRun, m_iTextRunLength);
		   m_iTextRunLength = 0;
		   UT_ASSERT(iRes == 0);

		   UT_DEBUGMSG(("paragraph properties...\n"));
		   apap = (PAP*)props;

		   // break before paragraph?
		   if (apap->fPageBreakBefore)
		     {
		        // TODO: this should really set a property in
		        // TODO: in the paragraph, instead; but this
		        // TODO: gives a similar effect for now.
			UT_UCSChar ucs = UCS_FF;
                        m_pDocument->appendSpan(&ucs,1);
		     }

		   // paragraph alignment
		   strcat(propBuffer, "text-align:");
		   switch(apap->jc)
		     {
		      case 0:
			strcat(propBuffer, "left");
			break;
		      case 1:
			strcat(propBuffer, "center");
			break;
		      case 2:
			strcat(propBuffer, "right");
			break;
		      case 3:
			strcat(propBuffer, "justify");
			break;
		      case 4:			
			/* this type of justification is of unknown purpose and is 
			 * undocumented , but it shows up in asian documents so someone
			 * should be able to tell me what it is someday C. */
			strcat(propBuffer, "justify");
			break;
		     }
		   strcat(propBuffer, ";");

		   // line spacing (single-spaced, double-spaced, etc.)
		   if (apap->lspd.fMultLinespace) {
		      strcat(propBuffer, "line-height:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%s;", UT_convertToDimensionlessString( (((float)apap->lspd.dyaLine) / 240), "1.1"));
		   } else { 
		      // I'm not sure Abiword currently handles the other method
		      // which requires setting the height of the lines exactly
		   }
		   
		   // margins
		   // -right
		   if (apap->dxaRight) {
		      strcat(propBuffer, "margin-right:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%s;", UT_convertToDimensionString(DIM_IN, (((float)apap->dxaRight) / 1440), "1.4"));
		   }
		   // -left
		   if (apap->dxaLeft) {
		      strcat(propBuffer, "margin-left:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%s;", UT_convertToDimensionString(DIM_IN, (((float)apap->dxaLeft) / 1440), "1.4"));
		   }
		   // -left first line (indent)
		   if (apap->dxaLeft1) {
		      strcat(propBuffer, "text-indent:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%s;", UT_convertToDimensionString(DIM_IN, (((float)apap->dxaLeft1) / 1440), "1.4"));
		   }
		   // -top
		   if (apap->dyaBefore) {
		      strcat(propBuffer, "margin-top:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%dpt;", (apap->dyaBefore / 20));
		   }		   
		   // -bottom
		   if (apap->dyaAfter) {
		      strcat(propBuffer, "margin-bottom:");
		      sprintf(propBuffer + strlen(propBuffer),
			      "%dpt;", (apap->dyaAfter / 20));
		   }

		   // keep paragraph together?
		   if (apap->fKeep) {
		      strcat(propBuffer, "keep-together:yes;");
		   }
		   // keep with next paragraph?
		   if (apap->fKeepFollow) {
		      strcat(propBuffer, "keep-with-next:yes;");
		   }
		   
		   // widowed lines
		   if (!apap->fWidowControl) {
		      // I believe Word only allows control of
		      // widows/orphans with a single flag. I believe 
		      // these AbiWord properties give the same effect
		      // (with orphan/widow control off)
		      strcat(propBuffer, "orphans:0;widows:0;");
		   }
		   
		   // tabs
		   if (apap->itbdMac) {
		      strcat(propBuffer, "tabstops:");
		      for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			 sprintf(propBuffer + strlen(propBuffer),
				 "%s/",
				 UT_convertToDimensionString(DIM_IN, (((float)apap->rgdxaTab[iTab]) / 1440), "1.4"));
			 switch (apap->rgtbd[iTab].jc) {
			  case 1:
			    strcat(propBuffer, "C,");
			    break;
			  case 2:
			    strcat(propBuffer, "R,");
			    break;
			  case 3:
			    strcat(propBuffer, "D,");
			    break;	
			  case 4:
			    strcat(propBuffer, "B,");
			    break;
			  case 0:
			  default:
			    strcat(propBuffer, "L,");
			    break;
			 }
		      }
		      // replace final comma with semi-colon
		      propBuffer[strlen(propBuffer)-1] = ';';
		   }
			 
		   // remove trailing semi-colon
		   propBuffer[strlen(propBuffer)-1] = 0;

		   propsArray[0] = pProps;
		   propsArray[1] = propBuffer;
		   propsArray[2] = NULL;
		   UT_DEBUGMSG(("the paragraph propBuffer is %s\n",propBuffer));
		   X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, propsArray));
		   break;

	         case CHARPROPBEGIN:

		   // flush character buffer
		   iRes = _charData(m_pTextRun, m_iTextRunLength);
		   m_iTextRunLength = 0;
		   UT_ASSERT(iRes == 0);

		   UT_DEBUGMSG(("character properties...\n"));
		   achp = (CHP*)props;

		   // bold text
		   if (achp->fBold) { 
		      strcat(propBuffer, "font-weight:bold;");
		   }
		   // italic text
		   if (achp->fItalic) {
		      strcat(propBuffer, "font-style:italic;");
		   }

		   // underline and strike-through
		   if (achp->fStrike || achp->kul) {
			 strcat(propBuffer, "text-decoration:");
		      if (achp->fStrike && achp->kul) {
			 strcat(propBuffer, "underline line-through;");
		      } else if (achp->kul) {
			 strcat(propBuffer, "underline;");
		      } else {
			 strcat(propBuffer, "line-through;");
		      }
		   }

		   // text color
		   if (achp->ico) {
		      sprintf((propBuffer + strlen(propBuffer)), 
			      "color:%02x%02x%02x;", 
			      word_colors[achp->ico-1][0], 
			      word_colors[achp->ico-1][1], 
			      word_colors[achp->ico-1][2]);
		   }

		   // font family
		   char *fname;
		   // if FarEast flag is set, use the FarEast font,
		   // otherwise, we'll use the ASCII font.
		   if (!ps->fib.fFarEast) {
		     fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcAscii);
		      UT_DEBUGMSG(("ASCII font id = %d\n", achp->ftcAscii));
		   } else {
		     fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcFE);
		      UT_DEBUGMSG(("FE font id = %d\n", achp->ftcFE));
		   }
		   // there are times when we should use the third, Other font, 
		   // and the logic to know when somehow depends on the
		   // character sets or encoding types? it's in the docs.
		   
		   UT_ASSERT(fname != NULL);
		   UT_DEBUGMSG(("font-family = %s\n", fname));

		    strcat(propBuffer, "font-family:");
		    strcat(propBuffer, fname);
		    strcat(propBuffer, ";");

			FREEP(fname);
		   
		   // font size (hps is half-points)
		   sprintf(propBuffer + strlen(propBuffer), 
			   "font-size:%dpt;", (achp->hps/2));

		   // sub/superscript
		   if (achp->iss == 1) {
		      strcat(propBuffer, "text-position: superscript;");
		   } else if (achp->iss == 2) {
		      strcat(propBuffer, "text-position: subscript;");
		   }

		   // done processing character properties

		   // remove trailing ;
		   propBuffer[strlen(propBuffer)-1] = 0;

		   propsArray[0] = pProps;
		   propsArray[1] = propBuffer;
		   propsArray[2] = NULL;
		   UT_DEBUGMSG(("the character propBuffer is %s\n",propBuffer));
		   X_ReturnNoMemIfError(m_pDocument->appendFmt(propsArray));
		   break;

	        case SECTIONEND:

		   // if we're at the end of a section, we need to check for a section mark
		   // at the end of our character stream and remove it (to prevent page breaks
		   // between sections)
		   if (m_iTextRunLength && 
		       m_pTextRun[m_iTextRunLength-1] == UCS_FF)
		     {
			m_iTextRunLength--;
			UT_DEBUGMSG(("section mark removed\n"));
		     }
		   UT_DEBUGMSG(("section end\n"));
		   break;

	        case CHARPROPEND: /* not needed */
		case PARAEND:	/* not needed */
		default:
		   break;
	     }
	UT_DEBUGMSG(("element ended\n"));
	return(0);
	}


/*****************************************************************/

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
   FREEP(m_pTextRun);
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	UT_DEBUGMSG(("constructed wv\n"));
	m_error = UT_OK;
   
	// to increase the speed and efficiency of the important,
	// we'll queue characters into runs of text, and only
	// append this to the document when something changes
	// (or we run out of space in the buffer)
   	m_iTextRunLength = 0;
   	m_iTextRunMaxLength = 256;
   	m_pTextRun = (UT_UCSChar*) calloc(m_iTextRunMaxLength, sizeof(UT_UCSChar));
   	UT_ASSERT(m_pTextRun != NULL);

	m_iImageCount = 0;
 }

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_MsWord_97::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	// TODO: This is rather crude, because we don't parse OLE files.
	// TODO: For the time being, we assume that any OLE file is an
	// TODO: msword document.
	// TODO: Caolan is gonna kill me for this.  :)
	// Most of the magic numbers here were taken from the public domain
	// /etc/magic file distributed with the file(1) command written
	// by Ian F. Darwin, with contributions and magic entries from
	// Rob McMahon, Guy Harris, Christos Zoulas <christos@astron.com>,
	// Mark Moraes <moraes@deshaw.com>, and Pawel Wiecek.
	char *magic ;
	int magicoffset ;
	magic = "Microsoft Word 6.0 Document" ;
	magicoffset = 2080 ;
	if ( iNumbytes > magicoffset+strlen(magic) )
	{
		if ( strncmp(szBuf+magicoffset, magic, strlen(magic)) == 0 )
		{
			return(UT_TRUE);
		}
	}
	magic = "Documento Microsoft Word 6" ;
	magicoffset = 2080 ;
	if ( iNumbytes > magicoffset+strlen(magic) )
	{
		if ( strncmp(szBuf+magicoffset, magic, strlen(magic)) == 0 )
		{
			return(UT_TRUE);
		}
	}
	magic = "MSWordDoc" ;
	magicoffset = 2112 ;
	if ( iNumbytes > magicoffset+strlen(magic) )
	{
		if ( strncmp(szBuf+magicoffset, magic, strlen(magic)) == 0 )
		{
			return(UT_TRUE);
		}
	}
	if ( iNumbytes > 8 )
	{
		if ( szBuf[0] == (char)0x31 && szBuf[1] == (char)0xbe &&
			 szBuf[2] == (char)0 && szBuf[3] == (char)0 )
		{
			return(UT_TRUE);
		}
		if ( szBuf[0] == 'P' && szBuf[1] == 'O' &&
			 szBuf[2] == '^' && szBuf[3] == 'Q' && szBuf[4] == '`' )
		{
			return(UT_TRUE);
		}
		if ( szBuf[0] == (char)0xfe && szBuf[1] == (char)0x37 &&
			 szBuf[2] == (char)0 && szBuf[3] == (char)0x23 )
		{
			return(UT_TRUE);
		}
		// OLE magic:
		// TODO: Dig through the OLE file
		if ( szBuf[0] == (char)0xd0 && szBuf[1] == (char)0xcf &&
			 szBuf[2] == (char)0x11 && szBuf[3] == (char)0xe0 &&
			 szBuf[4] == (char)0xa1 && szBuf[5] == (char)0xb1 &&
			 szBuf[6] == (char)0x1a && szBuf[7] == (char)0xe1 )
		{
			return(UT_TRUE);
		}
		if ( szBuf[0] == (char)0xdb && szBuf[1] == (char)0xa5 &&
			 szBuf[2] == (char)0x2d && szBuf[3] == (char)0 &&
			 szBuf[4] == (char)0 && szBuf[5] == (char)0 )
		{
			return(UT_TRUE);
		}
	}
	return(UT_FALSE);
}

UT_Bool IE_Imp_MsWord_97::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".doc") == 0);
}

UT_Error IE_Imp_MsWord_97::StaticConstructor(PD_Document * pDocument,
					     IE_Imp ** ppie)
{
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return UT_OK;
}

UT_Bool	IE_Imp_MsWord_97::GetDlgLabels(const char ** pszDesc,
				       const char ** pszSuffixList,
				       IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc)";
	*pszSuffixList = "*.doc";
	*ft = IEFT_MsWord_97;
	return UT_TRUE;
}

UT_Bool IE_Imp_MsWord_97::SupportsFileType(IEFileType ft)
{
	return (IEFT_MsWord_97 == ft);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void IE_Imp_MsWord_97::pasteFromBuffer(PD_DocumentRange * pDocRange,
				       unsigned char * pData, UT_uint32 lenData)
{
	UT_DEBUGMSG(("TODO IE_Imp_MsWord_97::pasteFromBuffer\n"));
}

UT_Error IE_Imp_MsWord_97::_handleImage(Blip * b, long width, long height)
{
   int data;
   const char * mimetype = NULL;
   
   UT_ByteBuf * buf = new UT_ByteBuf();
   
   switch(b->type)
     {
      case msoblipPNG:
	// conveniently, PNG is the internal format, so we do nothing here
	// but copy the bitstream
	while (EOF != (data = getc((FILE*)(b->blip.bitmap.m_pvBits))))
	  buf->append((UT_Byte*)&data, 1);
	mimetype = strdup("image/png");
	break;
      case msoblipWMF:
      case msoblipEMF:
      case msoblipPICT:
      case msoblipJPEG:
      case msoblipDIB:
      default:
	// TODO: support other image types
	DELETEP(buf);
	return UT_OK;
	break;
     }

   XML_Char propBuffer[128];
   propBuffer[0] = 0;
   sprintf(propBuffer, "width:%fin; height:%fin", 
	   (double)width / (double)1440, 
	   (double)height / (double)1440);
   XML_Char propsName[32];
   propsName[0] = 0;
   sprintf(propsName, "image%d", m_iImageCount++);

   const XML_Char* propsArray[5];
   propsArray[0] = "PROPS";
   propsArray[1] = propBuffer;
   propsArray[2] = "DATAID";
   propsArray[3] = propsName;
   propsArray[4] = NULL;

   X_ReturnNoMemIfError(m_pDocument->appendObject(PTO_Image, propsArray));
   X_CheckError0(m_pDocument->createDataItem(propsName, UT_FALSE,
					     buf, (void*)mimetype, NULL));

   DELETEP(buf);

   return UT_OK;
}
