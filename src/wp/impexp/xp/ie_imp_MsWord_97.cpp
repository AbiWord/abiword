/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ie_imp_MsWord_97.h"
#include "pd_Document.h"

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Imp_MsWord_97::importFile(const char * szFilename)
{
	int i = 0;
	unsigned long header[2];
	FIB fib;
	int nBytes;
	UT_GrowBuf gbBlock(1024);
	int offset = 0;

	UT_Bool bResult;
	//const char *attr[] = {"type", "Box", "left", "0pt", "top", "0pt", "width", "*", "height", "*", NULL};
	
	FILE *fp = NULL;

	fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_iestatus = IES_FileNotFound;
		goto Cleanup;
	}
	
	// read the File Information Block

	fseek(fp, 0x200, SEEK_SET);
	readFIB(fp, &fib);
	fseek(fp, 0, SEEK_SET);

	// check the file header

	if (readULong(fp, header, 2) < 0 ||
		(header[0] != 0xe011cfd0 || header[1] != 0xe11ab1a1))
    {
		UT_DEBUGMSG(("Problem reading header\n"));
		goto Cleanup;
	}

	/*******************************************************************/
	/* TODO - figure out what exactly the bytes between 0x0 - 0x1c are */
	/* 0x1c is the beginning of our property header                    */
	/*******************************************************************/

	fseek(fp, 0x1c, SEEK_SET);

	fseek(fp, 0x200 + fib.fcMin, SEEK_SET);

	nBytes = fib.fcMac - fib.fcMin;
	
	m_pDocument->appendStrux(PTX_Section, NULL);
	m_pDocument->appendStrux(PTX_Block, NULL);

	while (i < nBytes)
	{
		unsigned char c;

		readUByte(fp, &c, 1);

		if (c == 13)
		{
			m_pDocument->appendStrux(PTX_Block, NULL);

			if (gbBlock.getLength() > 0)
			{
				bResult = m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength());
				gbBlock.truncate(0);
			}
			offset = 0;
		}
		else
		{
			// HACK: this cast is bogus
			UT_UCSChar * uc = (UT_UCSChar *) &c;

			gbBlock.ins(offset, 1);
			gbBlock.overwrite(offset, uc, 1);
			offset++;
		}

		++i;
	}

	m_iestatus = IES_OK;

Cleanup:
	if (fp)
		fclose(fp);
	return m_iestatus;
}

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_iestatus = IES_OK;
	m_parseState = _PS_Init;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_MsWord_97::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".doc") == 0);
}

IEStatus IE_Imp_MsWord_97::StaticConstructor(const char * szSuffix,
										PD_Document * pDocument,
										IE_Imp ** ppie)
{
	UT_ASSERT(RecognizeSuffix(szSuffix));
	
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Imp_MsWord_97::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList)
{
	*pszDesc = "Word 97 (.doc)";
	*pszSuffixList = "*.doc";
	return UT_TRUE;
}


#if 0	// save this for later?
/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0
#define TT_DOCUMENT		1
#define TT_SECTION		2
#define TT_BLOCK		3
#define TT_INLINE		4
struct _TokenTable
{
	const char *	m_name;
	int				m_type;
};

static struct _TokenTable s_Tokens[] =
{	{	"awml",			TT_DOCUMENT		},
	{	"section",		TT_SECTION		},
	{	"p",			TT_BLOCK		},
	{	"c",			TT_INLINE		},
	{	"*",			TT_OTHER		}};	// must be last

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

static UT_uint32 s_mapNameToToken(const XML_Char * name)
{
	for (unsigned int k=0; k<TokenTableSize; k++)
		if (s_Tokens[k].m_name[0] == '*')
			return k;
		else if (UT_stricmp(s_Tokens[k].m_name,name)==0)
			return k;
	UT_ASSERT(0);
	return 0;
}

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))			\
									  {  m_iestatus = IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))								\
									  {  m_iestatus = IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))								\
									  {  m_iestatus = IES_Error;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_iestatus != IES_OK) return; } while (0)

/*****************************************************************/
/*****************************************************************/
#endif

int IE_Imp_MsWord_97::readUByte(FILE *fp, unsigned char *c, int num)
{
	int ret, i;

	for (i = 0; i < num; i++)
	{
		ret = fread(&c[i], 1, 1, fp);

		if (ret <= 0)
			return -1;
	}

	return 0;
}

int IE_Imp_MsWord_97::readByte(FILE *fp, char *c, int num)
{
	int ret, i;

	for (i = 0; i < num; i++)
	{
		ret = fread(&c[i], 1, 1, fp);

		if (ret <= 0)
			return -1;
	}

	return 0;
}

int IE_Imp_MsWord_97::readUShort(FILE *fp, unsigned short *s, int num)
{
	int i;
	unsigned char b[2];

	for (i = 0; i < num; i++)
	{
		if (readUByte(fp, b, 2) < 0)
			return -1;

		*s = (b[1] << 8) | b[0];
	}

	return 0;
}

int IE_Imp_MsWord_97::readShort(FILE *fp, short *s, int num)
{
	int i;
	unsigned char b[2];

	for (i = 0; i < num; i++)
	{
		if (readUByte(fp, b, 2) < 0)
			return -1;

		*s = (b[1] << 8) | b[0];
	}

	return 0;
}

int IE_Imp_MsWord_97::readULong(FILE *fp, unsigned long *l, int num)
{
	unsigned char b[4];
	int i;

	for (i = 0; i < num; i++)
	{
		if (readUByte(fp, b, 4) < 0)
			return -1;

		l[i] = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	}

	return 0;
}

int IE_Imp_MsWord_97::readLong(FILE *fp, long *l, int num)
{
	unsigned char b[4];
	int i;

	for (i = 0; i < num; i++)
	{
		if (readUByte(fp, b, 4) < 0)
			return -1;

		l[i] = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	}

	return 0;
}

void IE_Imp_MsWord_97::readFIB(FILE* f, FIB* fib)
{
	readUShort(f, &fib->wIdent, 1);
	readUShort(f, &fib->nFib, 1);
	readUShort(f, &fib->nProduct, 1);
	readUShort(f, &fib->lid, 1);
	readShort(f, &fib->pnNext, 1);
	readUShort(f, &fib->fDot, 1);
	readUShort(f, &fib->nFibBack, 1);
	readULong(f, &fib->lKey, 1);
	readUByte(f, &fib->envr, 1);
	readUByte(f, &fib->fMac, 1);
	readUShort(f, &fib->chs, 1);
	readUShort(f, &fib->chsTables, 1);
	readLong(f, &fib->fcMin, 1);
	readLong(f, &fib->fcMac, 1);
	readUShort(f, &fib->csw, 1);
	readUShort(f, &fib->wMagicCreated, 1);
	readUShort(f, &fib->wMagicRevised, 1);
	readUShort(f, &fib->wMagicCreatedPrivate, 1);
	readUShort(f, &fib->wMagicRevisedPrivate, 1);
	readShort(f, &fib->pnFbpChpFIrst_W6, 1);      /* not used */
	readShort(f, &fib->pnChpFirst_W6, 1);         /* not used */
	readShort(f, &fib->cpnBteChp_W6, 1);          /* not used */
	readShort(f, &fib->pnFbpPapFirst_W6, 1);      /* not used */
	readShort(f, &fib->pnPapFirst_W6, 1);         /* not used */
	readShort(f, &fib->cpnBtePap_W6, 1);          /* not used */
	readShort(f, &fib->pnFbpLvcFirst_W6, 1);      /* not used */
	readShort(f, &fib->pnLvcFirst_W6, 1);         /* not used */
	readShort(f, &fib->cpmBteLvc_W6, 1);          /* not used */
	readShort(f, &fib->lidFE, 1);
	readUShort(f, &fib->clw, 1);
	readLong(f, &fib->cbMac, 1);
	readULong(f, &fib->lProductCreated, 1);
	readULong(f, &fib->lProductRevised, 1);
	readLong(f, &fib->ccpText, 1);
	readLong(f, &fib->ccpFtn, 1);
	readLong(f, &fib->ccpHdd, 1);
	readLong(f, &fib->ccpMcr, 1);
	readLong(f, &fib->ccpAtn, 1);
	readLong(f, &fib->ccpEdn, 1);
	readLong(f, &fib->ccpTxbx, 1);
	readLong(f, &fib->ccpHdrTxbx, 1);
	readLong(f, &fib->pnFbpChpFirst, 1);
	readLong(f, &fib->pnChpFirst, 1);
	readLong(f, &fib->cpnBteChp, 1);
	readLong(f, &fib->pnFbpPapFirst, 1);
	readLong(f, &fib->pnPapFirst, 1);
	readLong(f, &fib->cpnBtePap, 1);
	readLong(f, &fib->pnFbpLvcFirst, 1);
	readLong(f, &fib->pnLvcFirst, 1);
	readLong(f, &fib->cpnBteLvc, 1);
	readLong(f, &fib->fcIslandFirst, 1);
	readLong(f, &fib->fcIslandLim, 1);
	readUShort(f, &fib->cfclcb, 1);
	readLong(f, &fib->fcStshfOrig, 1);
	readULong(f, &fib->lcbStshOrig, 1);
	readLong(f, &fib->fcStshf, 1);
	readULong(f, &fib->lcbStshf, 1);
	readLong(f, &fib->fcPlcffndRef, 1);
	readULong(f, &fib->lcbPlcffndRef, 1);
	readLong(f, &fib->fcPlcffndTxt, 1);
	readULong(f, &fib->lcbPlcffndTxt, 1);
	readLong(f, &fib->fcPlcfandRef, 1);
	readULong(f, &fib->lcbPlcfandRef, 1);
	readLong(f, &fib->fcPlcfandTxt, 1);
	readULong(f, &fib->lcbPlcfandTxt, 1);
	readLong(f, &fib->fcPlcfsed, 1);
	readULong(f, &fib->lcbPlcfsed, 1);
	readLong(f, &fib->fcPlcpad, 1);
	readULong(f, &fib->lcbPlcpad, 1);
	readLong(f, &fib->fcPlcfphe, 1);
	readULong(f, &fib->lcbPlcfphe, 1);
	readLong(f, &fib->fcSttbfglsy, 1);
	readULong(f, &fib->lcbSttbfglsy, 1);
	readLong(f, &fib->fcPlcfglsy, 1);
	readULong(f, &fib->lcbPlcfglsy, 1);
	readLong(f, &fib->fcPlcfhdd, 1);
	readULong(f, &fib->lcbPlcfhdd, 1);
	readLong(f, &fib->fcPlcfbteChpx, 1);
	readULong(f, &fib->lcbPlcfbteChpx, 1);
	readLong(f, &fib->fcPlcfbtePapx, 1);
	readULong(f, &fib->lcbPlcfbtePapx, 1);
	readLong(f, &fib->fcPlcfsea, 1);
	readULong(f, &fib->lcbPlcfsea, 1);
	readLong(f, &fib->fcSttbfffn, 1);
	readULong(f, &fib->lcbSttbfffn, 1);
	readLong(f, &fib->fcPlcffldMom, 1);
	readULong(f, &fib->lcbPlcffldMom, 1);
	readLong(f, &fib->fcPlcffldHdr, 1);
	readULong(f, &fib->lcbPlcffldHdr, 1);
	readLong(f, &fib->fcPlcffldFtn, 1);
	readULong(f, &fib->lcbPlcffldFtn, 1);
	readLong(f, &fib->fcPlcffldAtn, 1);
	readULong(f, &fib->lcbPlcffldAtn, 1);
	readLong(f, &fib->fcPlcffldMcr, 1);
	readULong(f, &fib->lcbPlcffldMcr, 1);
	readLong(f, &fib->fcSttbfbkmk, 1);
	readULong(f, &fib->lcbSttbfbkmk, 1);
	readLong(f, &fib->fcPlcfbfk, 1);
	readULong(f, &fib->lcbPlcfbfk, 1);
	readLong(f, &fib->fcPlcfbkl, 1);
	readULong(f, &fib->lcbPlcfbkl, 1);
	readLong(f, &fib->fcCmds, 1);
	readULong(f, &fib->lcbCmds, 1);
	readLong(f, &fib->fcPlcmcr, 1);
	readULong(f, &fib->lcbPlcmcr, 1);
	readLong(f, &fib->fcSttbfmcr, 1);
	readULong(f, &fib->lcdSttbfmcr, 1);
	readLong(f, &fib->fcPrDrvr, 1);
	readULong(f, &fib->lcbPrDrvr, 1);
	readLong(f, &fib->fcPrEnvPort, 1);
	readULong(f, &fib->lcbPrEnvPort, 1);
	readLong(f, &fib->fcPrEnvLand, 1);
	readULong(f, &fib->lcbPrEnvLand, 1);
	readLong(f, &fib->fcWss, 1);
	readULong(f, &fib->lcbWss, 1);
	readLong(f, &fib->fcDop, 1);
	readULong(f, &fib->lcbDop, 1);
	readLong(f, &fib->fcSttbfAssoc, 1);
	readULong(f, &fib->lcbSttbfAssoc, 1);
	readLong(f, &fib->fcClx, 1);
	readULong(f, &fib->lcbClx, 1);
	readLong(f, &fib->fcPlcfpgdFtn, 1);
	readULong(f, &fib->lcbPlcfpgdFtn, 1);
	readLong(f, &fib->fcAutosaveSource, 1);
	readULong(f, &fib->lcbAutosaveSource, 1);
	readLong(f, &fib->fcGrpXstAtnOwners, 1);
	readULong(f, &fib->lcbGrpXstAtnOwners, 1);
	readLong(f, &fib->fcSttbfAtnbkmk, 1);
	readULong(f, &fib->lcbSttbAtnbkmk, 1);
	readLong(f, &fib->fcPlcdoaMom, 1);
	readULong(f, &fib->lcbPlcdoaMom, 1);
	readLong(f, &fib->fcPlcdoaHdr, 1);
	readULong(f, &fib->lcbPlcdoaHdr, 1);
	readLong(f, &fib->fcPlcspaMom, 1);
	readULong(f, &fib->lcbPlcspaMom, 1);
	readLong(f, &fib->fcPlcspaHdr, 1);
	readULong(f, &fib->lcbPlcspaHdr, 1);
	readLong(f, &fib->fcPlcfAtnbkf, 1);
	readULong(f, &fib->lcbPlcfAtnbkf, 1);
	readLong(f, &fib->fcPlcfAtnbkl, 1);
	readULong(f, &fib->lcbPlcfAtnbkl, 1);
	readLong(f, &fib->fcPms, 1);
	readULong(f, &fib->lcbPms, 1);
	readLong(f, &fib->fcFormFldSttbs, 1);
	readULong(f, &fib->lcbFormFldSttbs, 1);
	readLong(f, &fib->fcPlcfendRef, 1);
	readULong(f, &fib->lcbPlcfendRef, 1);
	readLong(f, &fib->fcPlcfendTxt, 1);
	readULong(f, &fib->lcbPlcfendTxt, 1);
	readLong(f, &fib->fcPlcffldEdn, 1);
	readULong(f, &fib->lcbPlcffldEdn, 1);
	readLong(f, &fib->fcPlcfpgdEdn, 1);
	readULong(f, &fib->lcbPlcfpgdEdn, 1);
	readLong(f, &fib->fcDggInfo, 1);
	readULong(f, &fib->lcbDggInfo, 1);
	readLong(f, &fib->fcSttbfRMark, 1);
	readULong(f, &fib->lcbSttbfRMark, 1);
	readLong(f, &fib->fcSttbCaption, 1);
	readULong(f, &fib->lcbSttbCaption, 1);
	readLong(f, &fib->fcSttbAutoCaption, 1);
	readULong(f, &fib->lcbSttbAutoCaption, 1);
	readLong(f, &fib->fcPlcfwkb, 1);
	readULong(f, &fib->lcbPlcfwkb, 1);
	readLong(f, &fib->fcPlcfspl, 1);
	readULong(f, &fib->lcbPlcfspl, 1);
	readLong(f, &fib->fcPlcftxbxTxt, 1);
	readULong(f, &fib->lcbPlcftxbxTxt, 1);
	readLong(f, &fib->fcPlcffldTxbx, 1);
	readULong(f, &fib->lcbPlcffldTxbx, 1);
	readLong(f, &fib->fcPlcfhdrtxbxTxt, 1);
	readULong(f, &fib->lcbPlcfhdrtxbxTxt, 1);
	readLong(f, &fib->fcPlcffldHdrTxbx, 1);
	readULong(f, &fib->lcbPlcffldHdrTxbx, 1);
	readLong(f, &fib->fcStwUser, 1);
	readULong(f, &fib->lcbStwUser, 1);
	readLong(f, &fib->fcSttbttmdb, 1);
	readULong(f, &fib->lcbSttbttmbd, 1);
	readLong(f, &fib->fcUnused, 1);
	readULong(f, &fib->lcbUnused, 1);
	readLong(f, &fib->rgpgdbkd, 1);   /* change me */
	readLong(f, &fib->fcPgdMother, 1);
	readULong(f, &fib->lcbPgbMother, 1);
	readLong(f, &fib->fcBkdMother, 1);
	readULong(f, &fib->lcbBkdMother, 1);
	readLong(f, &fib->fcPgdFtn, 1);
	readULong(f, &fib->lcbPgdFtn, 1);
	readLong(f, &fib->fcBkdFtn, 1);
	readULong(f, &fib->lcdBkdftn, 1);
	readLong(f, &fib->fcPgdEdn, 1);
	readULong(f, &fib->lcbPgdEdn, 1);
	readLong(f, &fib->fcBkdEdn, 1);
	readULong(f, &fib->lcbBkdEdn, 1);
	readLong(f, &fib->fcSttbfIntlFld, 1);
	readULong(f, &fib->lcbSttbfIntlFld, 1);
	readLong(f, &fib->fcRouteSlip, 1);
	readULong(f, &fib->lcbRouteSlip, 1);
	readLong(f, &fib->fcSttbSavedBy, 1);
	readULong(f, &fib->lcbSttbSavedBy, 1);
	readLong(f, &fib->fcSttbFnm, 1);
	readULong(f, &fib->lcbSttbFnm, 1);
	readLong(f, &fib->fcPlcfLst, 1);
	readULong(f, &fib->lcbPlcfLst, 1);
	readLong(f, &fib->fcPlfLfo, 1);
	readULong(f, &fib->lcbPlfLfo, 1);
	readLong(f, &fib->fcPlcftxbxBkd, 1);
	readULong(f, &fib->lcbPlcftxbxBkd, 1);
	readLong(f, &fib->fcPlcftxbxHdrBkd, 1);
	readULong(f, &fib->lcbPlcftxbxHdrBkd, 1);
	readLong(f, &fib->fcDocUndo, 1);
	readULong(f, &fib->lcbDocUndo, 1);
	readLong(f, &fib->fcRgbuse, 1);
	readULong(f, &fib->lcbRgbuse, 1);
	readLong(f, &fib->fcUsp, 1);
	readULong(f, &fib->lcbUsp, 1);
	readLong(f, &fib->fcUskf, 1);
	readULong(f, &fib->lcbUskf, 1);
	readLong(f, &fib->fcPlcupcRgbuse, 1);
	readULong(f, &fib->lcbPlcupcRgbuse, 1);
	readLong(f, &fib->fcPlcupcUsp, 1);
	readULong(f, &fib->lcbPlcupcUsp, 1);
	readLong(f, &fib->fcSttbGlsyStyle, 1);
	readULong(f, &fib->lcbSttbGlsyStyle, 1);
	readLong(f, &fib->fcPlgosl, 1);
	readULong(f, &fib->lcbPlgosl, 1);
	readLong(f, &fib->fcPlcocx, 1);
	readULong(f, &fib->lcbPlcocx, 1);
	readLong(f, &fib->fcPlcfbteLvc, 1);
	readULong(f, &fib->lcbPlcfbteLvc, 1);
	readLong(f, &fib->ftModified, 1);       /* change me */
	readULong(f, &fib->dwLowDateTime, 1);
	readULong(f, &fib->dwHighDateTime, 1);
	readLong(f, &fib->fcPlcflvc, 1);
	readULong(f, &fib->lcbPlcflvc, 1);
	readLong(f, &fib->fcPlcasumy, 1);
	readULong(f, &fib->lcbPlcasumy, 1);
	readLong(f, &fib->fcPlcfgram, 1);
	readULong(f, &fib->lcbPlcfgram, 1);
	readLong(f, &fib->fcSttbListNames, 1);
	readULong(f, &fib->lcbSttbListNames, 1);
	readLong(f, &fib->fcSttbfUssr, 1);
	readULong(f, &fib->lcbSttbfUssr, 1);
}


