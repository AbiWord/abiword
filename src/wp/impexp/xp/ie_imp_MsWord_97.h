 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef IE_IMP_MSWORD_97_H
#define IE_IMP_MSWORD_97_H

#include <stdio.h>
#include "xmlparse.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ie_imp.h"

class PD_Document;

#define WORD     unsigned short
#define DWORD    unsigned long
#define LONG     unsigned long
#define BYTE     unsigned char
#define FCPGD    long              /* CHANGE ME */
#define FILETIME long              /* CHANGE ME */

typedef struct _FIB
{
	WORD wIdent;
	WORD nFib;
	WORD nProduct;
	WORD lid;
	short pnNext;
	WORD fDot;
	WORD nFibBack;
	LONG lKey;
	BYTE envr;
	BYTE fMac;
	WORD chs;
	WORD chsTables;
	long fcMin;
	long fcMac;
	WORD csw;
	WORD wMagicCreated;
	WORD wMagicRevised;
	WORD wMagicCreatedPrivate;
	WORD wMagicRevisedPrivate;
	short pnFbpChpFIrst_W6;      /* not used */
	short pnChpFirst_W6;         /* not used */
	short cpnBteChp_W6;          /* not used */
	short pnFbpPapFirst_W6;      /* not used */
	short pnPapFirst_W6;         /* not used */
	short cpnBtePap_W6;          /* not used */
	short pnFbpLvcFirst_W6;      /* not used */
	short pnLvcFirst_W6;         /* not used */
	short cpmBteLvc_W6;          /* not used */
	short lidFE;
	WORD clw;
	long cbMac;
	LONG lProductCreated;
	LONG lProductRevised;
	long ccpText;
	long ccpFtn;
	long ccpHdd;
	long ccpMcr;
	long ccpAtn;
	long ccpEdn;
	long ccpTxbx;
	long ccpHdrTxbx;
	long pnFbpChpFirst;
	long pnChpFirst;
	long cpnBteChp;
	long pnFbpPapFirst;
	long pnPapFirst;
	long cpnBtePap;
	long pnFbpLvcFirst;
	long pnLvcFirst;
	long cpnBteLvc;
	long fcIslandFirst;
	long fcIslandLim;
	WORD cfclcb;
	long fcStshfOrig;
	LONG lcbStshOrig;
	long fcStshf;
	LONG lcbStshf;
	long fcPlcffndRef;
	LONG lcbPlcffndRef;
	long fcPlcffndTxt;
	LONG lcbPlcffndTxt;
	long fcPlcfandRef;
	LONG lcbPlcfandRef;
	long fcPlcfandTxt;
	LONG lcbPlcfandTxt;
	long fcPlcfsed;
	LONG lcbPlcfsed;
	long fcPlcpad;
	LONG lcbPlcpad;
	long fcPlcfphe;
	LONG lcbPlcfphe;
	long fcSttbfglsy;
	LONG lcbSttbfglsy;
	long fcPlcfglsy;
	LONG lcbPlcfglsy;
	long fcPlcfhdd;
	LONG lcbPlcfhdd;
	long fcPlcfbteChpx;
	LONG lcbPlcfbteChpx;
	long fcPlcfbtePapx;
	LONG lcbPlcfbtePapx;
	long fcPlcfsea;
	LONG lcbPlcfsea;
	long fcSttbfffn;
	LONG lcbSttbfffn;
	long fcPlcffldMom;
	LONG lcbPlcffldMom;
	long fcPlcffldHdr;
	LONG lcbPlcffldHdr;
	long fcPlcffldFtn;
	LONG lcbPlcffldFtn;
	long fcPlcffldAtn;
	LONG lcbPlcffldAtn;
	long fcPlcffldMcr;
	LONG lcbPlcffldMcr;
	long fcSttbfbkmk;
	LONG lcbSttbfbkmk;
	long fcPlcfbfk;
	LONG lcbPlcfbfk;
	long fcPlcfbkl;
	LONG lcbPlcfbkl;
	long fcCmds;
	LONG lcbCmds;
	long fcPlcmcr;
	LONG lcbPlcmcr;
	long fcSttbfmcr;
	LONG lcdSttbfmcr;
	long fcPrDrvr;
	LONG lcbPrDrvr;
	long fcPrEnvPort;
	LONG lcbPrEnvPort;
	long fcPrEnvLand;
	LONG lcbPrEnvLand;
	long fcWss;
	LONG lcbWss;
	long fcDop;
	LONG lcbDop;
	long fcSttbfAssoc;
	LONG lcbSttbfAssoc;
	long fcClx;
	LONG lcbClx;
	long fcPlcfpgdFtn;
	LONG lcbPlcfpgdFtn;
	long fcAutosaveSource;
	LONG lcbAutosaveSource;
	long fcGrpXstAtnOwners;
	LONG lcbGrpXstAtnOwners;
	long fcSttbfAtnbkmk;
	LONG lcbSttbAtnbkmk;
	long fcPlcdoaMom;
	LONG lcbPlcdoaMom;
	long fcPlcdoaHdr;
	LONG lcbPlcdoaHdr;
	long fcPlcspaMom;
	LONG lcbPlcspaMom;
	long fcPlcspaHdr;
	LONG lcbPlcspaHdr;
	long fcPlcfAtnbkf;
	LONG lcbPlcfAtnbkf;
	long fcPlcfAtnbkl;
	LONG lcbPlcfAtnbkl;
	long fcPms;
	LONG lcbPms;
	long fcFormFldSttbs;
	LONG lcbFormFldSttbs;
	long fcPlcfendRef;
	LONG lcbPlcfendRef;
	long fcPlcfendTxt;
	LONG lcbPlcfendTxt;
	long fcPlcffldEdn;
	LONG lcbPlcffldEdn;
	long fcPlcfpgdEdn;
	LONG lcbPlcfpgdEdn;
	long fcDggInfo;
	LONG lcbDggInfo;
	long fcSttbfRMark;
	LONG lcbSttbfRMark;
	long fcSttbCaption;
	LONG lcbSttbCaption;
	long fcSttbAutoCaption;
	LONG lcbSttbAutoCaption;
	long fcPlcfwkb;
	LONG lcbPlcfwkb;
	long fcPlcfspl;
	LONG lcbPlcfspl;
	long fcPlcftxbxTxt;
	LONG lcbPlcftxbxTxt;
	long fcPlcffldTxbx;
	LONG lcbPlcffldTxbx;
	long fcPlcfhdrtxbxTxt;
	LONG lcbPlcfhdrtxbxTxt;
	long fcPlcffldHdrTxbx;
	LONG lcbPlcffldHdrTxbx;
	long fcStwUser;
	LONG lcbStwUser;
	long fcSttbttmdb;
	LONG lcbSttbttmbd;
	long fcUnused;
	LONG lcbUnused;
	FCPGD rgpgdbkd;
	long fcPgdMother;
	LONG lcbPgbMother;
	long fcBkdMother;
	LONG lcbBkdMother;
	long fcPgdFtn;
	LONG lcbPgdFtn;
	long fcBkdFtn;
	LONG lcdBkdftn;
	long fcPgdEdn;
	LONG lcbPgdEdn;
	long fcBkdEdn;
	LONG lcbBkdEdn;
	long fcSttbfIntlFld;
	LONG lcbSttbfIntlFld;
	long fcRouteSlip;
	LONG lcbRouteSlip;
	long fcSttbSavedBy;
	LONG lcbSttbSavedBy;
	long fcSttbFnm;
	LONG lcbSttbFnm;
	long fcPlcfLst;
	LONG lcbPlcfLst;
	long fcPlfLfo;
	LONG lcbPlfLfo;
	long fcPlcftxbxBkd;
	LONG lcbPlcftxbxBkd;
	long fcPlcftxbxHdrBkd;
	LONG lcbPlcftxbxHdrBkd;
	long fcDocUndo;
	LONG lcbDocUndo;
	long fcRgbuse;
	LONG lcbRgbuse;
	long fcUsp;
	LONG lcbUsp;
	long fcUskf;
	LONG lcbUskf;
	long fcPlcupcRgbuse;
	LONG lcbPlcupcRgbuse;
	long fcPlcupcUsp;
	LONG lcbPlcupcUsp;
	long fcSttbGlsyStyle;
	LONG lcbSttbGlsyStyle;
	long fcPlgosl;
	LONG lcbPlgosl;
	long fcPlcocx;
	LONG lcbPlcocx;
	long fcPlcfbteLvc;
	LONG lcbPlcfbteLvc;
	FILETIME ftModified;
	LONG dwLowDateTime;
	LONG dwHighDateTime;
	long fcPlcflvc;
	LONG lcbPlcflvc;
	long fcPlcasumy;
	LONG lcbPlcasumy;
	long fcPlcfgram;
	LONG lcbPlcfgram;
	long fcSttbListNames;
	LONG lcbSttbListNames;
	long fcSttbfUssr;
	LONG lcbSttbfUssr;
} FIB;

// The importer/reader for Microsoft Word 97 file format.

class IE_Imp_MsWord_97 : public IE_Imp
{
public:
	IE_Imp_MsWord_97(PD_Document * pDocument);
	~IE_Imp_MsWord_97();

	IEStatus			importFile(const char * szFilename);

	// the following are public only so that the
	// XML parser callback routines can access them.
	
protected:
	int readByte(FILE *fp, char *c, int num);
	int readUByte(FILE *fp, unsigned char *c, int num);
	int readLong(FILE *fp, long *l, int num);
	int readULong(FILE *fp, unsigned long *l, int num);
	int readShort(FILE *fp, short *s, int num);
	int readUShort(FILE *fp, unsigned short *s, int num);

	void readFIB(FILE* f, FIB* fib);

protected:
	typedef enum _parseState { _PS_Init,
							   _PS_Doc,
							   _PS_Sec,
							   _PS_ColSet,
							   _PS_Col,
							   _PS_Block } ParseState;

	IEStatus		m_iestatus;
	ParseState		m_parseState;

	UT_Vector		m_vecInlineFmt;
	UT_Stack		m_stackFmtStartIndex;
};

#endif /* IE_IMP_MSWORD_97_H */
