/* haven't touched */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;

extern U16 colorlookupr[17];
extern U16 colorlookupg[17];
extern U16 colorlookupb[17];

void decode_sprm(FILE* in,U16 clist,pap *retpap,chp *retchp,sep *retsep,U16 *l,U8 **list,style *sheet,U16 istd)
	{
	int i;
	int sprmlen;
	U16 sprm2;
	U8 len;
	U8 flags;
	U16 j;
	int orig;
	chp emchp;
	S16 temp;
	U16 temp2;
	U16 temp3;
	U16 temp4;
	chp modifiedchp;
	chp blank_chp;
	

	error(erroroutput,"SPRM is %x\n",(int)clist);

	switch(clist)
		{
		/*sep*/
		case 0x3009:
			retsep->bkc=dgetc(in,list);
			(*l)++;
			break;
		case 0x300A:
			retsep->fTitlePage=dgetc(in,list);
			(*l)++;
			break;
		case 0x300E:
			retsep->nfcPgn=dgetc(in,list);
			error(erroroutput,"nfcPgn is %d\n",retsep->nfcPgn);
			(*l)++;
			break;
		case 0x3011:
			retsep->restart=dgetc(in,list);
			(*l)++;
			break;
		case 0x3012:
			retsep->fEndNote=dgetc(in,list);
			(*l)++;
			break;
		case 0x3013:
			retsep->lnc=dgetc(in,list);
			(*l)++;
			break;
		case 0x500B:
			retsep->ccolM1=dread_16ubit(in,list);
			(*l)+=2;
			break;
		case 0x5015:
			retsep->nLnnMod=dgetc(in,list);
			(*l)++;
			break;
		case 0x501C:
			retsep->pgnStart=dread_16ubit(in,list);
			(*l)+=2;
			break;
		/*pap*/
		case 0xC63E:
			error(erroroutput,"anld here\n");
			len = dgetc(in,list);
			(*l)++;
			retpap->anld.nfc = dgetc(in,list);
			retpap->anld.cxchTextBefore = dgetc(in,list);
			retpap->anld.cxchTextAfter= dgetc(in,list);
			flags=dgetc(in,list);
			retpap->anld.jc = flags&0x03;
			retpap->anld.fPrev = flags&0x04;
			retpap->anld.fHang = flags&0x08;
			retpap->anld.fSetBold = flags&0x10;
			retpap->anld.fSetItalic = flags&0x20;
			retpap->anld.fSetSmallCaps = flags&0x40;
			retpap->anld.fSetCaps = flags&0x80;
			retpap->anld.flags2=dgetc(in,list);
			retpap->anld.flags3=dgetc(in,list);
			retpap->anld.ftc=dread_16ubit(in,list);
			retpap->anld.hps=dread_16ubit(in,list);
			retpap->anld.startat=dread_16ubit(in,list);
			dread_16ubit(in,list);
			dread_16ubit(in,list);
			dread_16ubit(in,list);
			dread_16ubit(in,list);
			for (i=0;i<32;i++)
				retpap->anld.rgxch[i] = dread_16ubit(in,list);
			(*l)+=84;
			break;
		case 0x2602:
		case 0x2640:
			error(erroroutput,"aha lvl!!!\n");
			dgetc(in,list);
			(*l)++;
			break;
		case 0xC615:
		case 0xC60D:
			error(erroroutput,"TABTABTAB\n");
			len = dgetc(in,list);
			error(erroroutput,"len is %d\n",len);
			(*l)++;
			for (i=0;i<len;i++)
				{
				dgetc(in,list);
				(*l)++;
				}
			break;
		case 0x2416:
			retpap->fInTable = dgetc(in,list);
			error(erroroutput,"FIntable set to %d\n",retpap->fInTable);
			(*l)++;
			break;
		case 0x2417:
			retpap->fInTable = 1;
			retpap->fTtp = dgetc(in,list);
			error(erroroutput,"FIntable set to %d indirectly\n",retpap->fInTable);
			(*l)++;
			break;
		case 0x260A:
			retpap->ilvl = dgetc(in,list);
			error(erroroutput,"ilvl set to %d\n",retpap->ilvl);
			if (retpap->ilvl > 8)
				{
				error(erroroutput,"hmm, strange ilvl here, setting to -1\n");
				retpap->ilvl =-1;
				}
			(*l)++;
			break;
		case 0x460B:
			retpap->ilfo = dread_16ubit(in,list);
			/*
			like Collin Park (collin@hpycla.kobe.hp.com) says
			i probably have to back through loads of code and 
			check for sign extensions and friends, oh oh
			*/
			error(erroroutput,"ilfo is %d\n",retpap->ilfo);
			if (retpap->ilfo == 2047)
				error(erroroutput,"warning, special list depth\n");
			else
				{
				if (retpap->ilfo == 0)
					error(erroroutput,"not numbered (bulleted ?)\n");
				retpap->ilfo--;
				}
			error(erroroutput,"ilfo is now %d\n",retpap->ilfo);
			(*l)+=2;
			break;
		case 0x4600:
			retpap->istd = dread_16ubit(in,list);
			error(erroroutput,"MAGIC: istd is %d\n",retpap->istd);
			(*l)+=2;
			break;

		/*chp*/
		case 0x802:
			orig = retchp->fData;
			retchp->fData= dgetc(in,list);
			(*l)++;
			switch (retchp->fData)
				{
				case 1:
					retchp->fData= 1;
					break;
				case 0:
					retchp->fData= 0;
					break;
				case 128:
					/*in this case bold value is the based upon val*/
					retchp->fData= orig;
					break;
				case 129:
					error(erroroutput,"swapping around\n");
					retchp->fData= abs(orig-1);
					break;
				}
			error(erroroutput,"fData is now %d\n",retchp->fData);
			(*l)++;
			break;
		case 0x2A32:
			retchp->fBold=0;
			retchp->fItalic=0;
			retchp->fCaps=0;
			retchp->fSmallCaps=0;
			retchp->underline=0;
			retchp->fStrike=0;
			retchp->color[0] = '\0';
			dgetc(in,list);
			(*l)++;
			break;
		case 0x2A33:
			temp = retchp->fSpec;
			init_chp_from_istd(retchp->istd,sheet,retchp); /*i think this is right, not tested*/
			retchp->fSpec = temp;
			break;
		case 0x2859:
			retchp->animation = dgetc(in,list);
			(*l)++;
			break;
		case 0x286F:
			retchp->idctHint = dgetc(in,list);
			error(erroroutput,"hint is %d\n",retchp->idctHint);
			(*l)++;
			break;
		case 0xCA47:
		case 0xCA4A:
			/*
			take the embedded chp, compare to external chp
			the equal fields are set to the styles ones
			*/
			sprmlen=dgetc(in,list);
			(*l)++;
			error(erroroutput," the len is %d\n",sprmlen);
			j=0;
			init_chp(&emchp);
			while(j<sprmlen)
				{
				sprm2 = dread_16ubit(in,list);
				j+=2;
				error(erroroutput,"embedded sprm is %X\n",sprm2);
				/*
				decode_sprm(in,sprm2,retpap,&emchp,&j,list,sheet,retpap->istd);
				*/
				decode_sprm(in,sprm2,retpap,&emchp,retsep,&j,list,sheet,istd);
				error(erroroutput,"em j got set to %d\n",j);
				}
			error(erroroutput,"retpap->istd is %d\n",retpap->istd);
			retpap->istd = 10; /*surely blatent rubbish ?, this does do the right thing, ive misread
			something somewhere, and im not seeing the obvious*/

			*l = *l + j;
#if 0
			if (emchp.fBold == retchp->fBold)
				retchp->fBold = sheet[retpap->istd].thechp.fBold;
			if (emchp.fItalic== retchp->fItalic)
				retchp->fItalic= sheet[retpap->istd].thechp.fItalic;
			if (emchp.fontsize== retchp->fontsize)
				retchp->fontsize= sheet[retpap->istd].thechp.fontsize;
			if (emchp.underline== retchp->underline)
				retchp->underline= sheet[retpap->istd].thechp.underline;
#endif
			if (emchp.fBold == retchp->fBold)
				retchp->fBold = sheet[istd].thechp.fBold;
			if (emchp.fItalic== retchp->fItalic)
				retchp->fItalic= sheet[istd].thechp.fItalic;
			if (emchp.fontsize== retchp->fontsize)
				retchp->fontsize= sheet[istd].thechp.fontsize;
			if (emchp.underline== retchp->underline)
				retchp->underline= sheet[istd].thechp.underline;
			if (emchp.fStrike== retchp->fStrike)
				retchp->fStrike= sheet[istd].thechp.fStrike;
			/*others not implemented yet*/
			break;
		case 0x486D:
		case 0x486E:
			error(erroroutput,"lid data is %X",dread_16ubit(in,list));
			(*l)+=2;
			break;
		case 0x0835:
			orig = retchp->fBold;
			retchp->fBold= dgetc(in,list);
			(*l)++;
			switch (retchp->fBold)
				{
				case 1:
					retchp->fBold= 1;
					break;
				case 0:
					retchp->fBold= 0;
					break;
				case 128:
					/*in this case bold value is the based upon val*/
					retchp->fBold = orig;
					break;
				case 129:
					error(erroroutput,"swapping around\n");
					retchp->fBold = abs(orig-1);
					break;
				}
			error(erroroutput,"BOLD is %d\n",retchp->fBold);
			/*not sure what each 0,127 etc mean yet*/
			error(erroroutput,"j here is %d\n",*l);
			break;
		case 0x083A:
			orig = retchp->fSmallCaps;
			retchp->fSmallCaps= dgetc(in,list);
			(*l)++;
			error(erroroutput,"SMALLCAPS is %d\n",retchp->fSmallCaps);
			switch(retchp->fSmallCaps)
				{
				case 1:
					retchp->fSmallCaps= 1;
					break;
				case 0:
					retchp->fSmallCaps= 0;
					break;
				case 128:
					/*set to original*/
					retchp->fSmallCaps= orig;
					break;
				case 129:
					retchp->fSmallCaps= abs(orig-1);
					break;
				}
			break;
		case 0x083B:
			orig = retchp->fCaps;
			retchp->fCaps= dgetc(in,list);
			(*l)++;
			error(erroroutput,"CAPS is %d\n",retchp->fCaps);
			switch(retchp->fCaps)
				{
				case 1:
					retchp->fCaps= 1;
					break;
				case 0:
					retchp->fCaps= 0;
					break;
				case 128:
					/*set to original*/
					retchp->fCaps= orig;
					break;
				case 129:
					retchp->fCaps= abs(orig-1);
					break;
				}
			break;
		case 0x0836:
			orig = retchp->fItalic;
			retchp->fItalic = dgetc(in,list);
			(*l)++;
			error(erroroutput,"ITALIC is %d\n",retchp->fItalic);
			switch(retchp->fItalic)
				{
				case 1:
					retchp->fItalic = 1;
					break;
				case 0:
					retchp->fItalic = 0;
					break;
				case 128:
					/*set to original*/
					retchp->fItalic = orig;
					break;
				case 129:
					retchp->fItalic = abs(orig-1);
					break;
				}
			break;
		case 0x2A53:
			orig = retchp->fDStrike;
			retchp->fDStrike= dgetc(in,list);
			(*l)++;
			error(erroroutput,"STRIKE is %d\n",retchp->fDStrike);
			switch(retchp->fDStrike)
				{
				case 1:
					retchp->fDStrike= 1;
					break;
				case 0:
					retchp->fDStrike= 0;
					break;
				case 128:
					/*set to original*/
					retchp->fDStrike= orig;
					break;
				case 129:
					retchp->fDStrike= abs(orig-1);
					break;
				}
			break;
		case 0x0837:
			orig = retchp->fStrike;
			retchp->fStrike= dgetc(in,list);
			(*l)++;
			error(erroroutput,"STRIKE is %d\n",retchp->fStrike);
			switch(retchp->fStrike)
				{
				case 1:
					retchp->fStrike= 1;
					break;
				case 0:
					retchp->fStrike= 0;
					break;
				case 128:
					/*set to original*/
					retchp->fStrike= orig;
					break;
				case 129:
					retchp->fStrike= abs(orig-1);
					break;
				}
			break;
		case 0x841A:
			retpap->dxaWidth=dread_16ubit(in,list);
			(*l)+=2;
			break;
		case 0x6426:
			/*
			dread_16ubit(in,list);
			dgetc(in,list);
			len = dgetc(in,list);
			len = len&0x1f;
			*/
			retpap->brcBottom=dread_32ubit(in,list);
			error(erroroutput,"border depth is %d\n",(retpap->brcBottom>>24)&0x1f);
			(*l)+=4;
			break;
		case 0x6425:
			retpap->brcLeft=dread_32ubit(in,list);
			error(erroroutput,"border depth is %d\n",(retpap->brcLeft>>24)&0x1f);
			(*l)+=4;
			break;
		case 0x6427:
			retpap->brcRight=dread_32ubit(in,list);
			error(erroroutput,"border depth is %d\n",(retpap->brcRight>>24)&0x1f);
			(*l)+=4;
			break;
		case 0x6428:
			retpap->brcBetween=dread_32ubit(in,list);
			error(erroroutput,"border depth is %d\n",(retpap->brcBetween>>24)&0x1f);
			(*l)+=4;
			break;
		case 0x6424:
		case 0x6429:
			dread_16ubit(in,list);
			dread_16ubit(in,list);
			(*l)+=4;
			break;
		case 0x0800:
		case 0x0838:
		case 0x0839:
		case 0x083C:
		case 0x2407:
		case 0x2431:
			dgetc(in,list);
			(*l)++;
			break;
		case 0x4845:
			/*how much to sub and super*/
			dread_16ubit(in,list);
			(*l)+=2;
			break;
		case 0x4A43:
			retchp->fontsize = dread_16ubit(in,list);
			(*l)+=2;
			error(erroroutput,"font is %d\n",retchp->fontsize);
			break;
		case 0x4A30:
			retchp->istd = dread_16ubit(in,list);
			error(erroroutput,"aha wants to set char istd to %ld ?\n",retchp->istd);
			/*does this mean that i set everything back based on that istd ?, im guessing so*/
			/*
			*retchp = sheet[sprm3].thechp;
			*/
			modifiedchp = *retchp; /*bitwisecopy will work fine*/
			init_chp(&blank_chp);
			init_chp_from_istd(retchp->istd,sheet,retchp);
			
			merge_chps(&blank_chp,&modifiedchp,retchp);
			
			(*l)+=2;
			break;
		case 0x261B:
			dgetc(in,list);
			(*l)++;
			break;
		case 0x4A4F:
			retchp->ascii_font=dread_16ubit(in,list);
			error(erroroutput,"font stuff for ascii text is %ld\n",retchp->ascii_font);
			(*l)+=2;
			break;
		case 0x4A50:
			retchp->eastfont=dread_16ubit(in,list);
			error(erroroutput,"font stuff for east text is %ld\n",retchp->eastfont);
			(*l)+=2;
			break;
		case 0x4A51:
			retchp->noneastfont=dread_16ubit(in,list);
			error(erroroutput,"font stuff for noneast text is %ld\n",retchp->noneastfont);
			(*l)+=2;
			break;
		case 0xA413:
			retpap->dyaBefore=dread_16ubit(in,list);
			error(erroroutput,"before is %d\n",retpap->dyaBefore);
			(*l)+=2;
			break;
		case 0xA414:
			retpap->dyaAfter=dread_16ubit(in,list);
			error(erroroutput,"after is %d\n",retpap->dyaAfter);
			(*l)+=2;
			break;
		case 0x486B:
		case 0x0418:
		case 0x484B:
		case 0x484E:
			error(erroroutput,"eat\n");
			dread_16ubit(in,list);
			(*l)+=2;
			break;
		case 0xB021:
			retsep->leftmargin = (S16)dread_16ubit(in,list);
			error(erroroutput,"this sections indent is %d\n",retsep->leftmargin);
			(*l)+=2;
			break;
		case 0x840F:
			retpap->leftmargin=(S16)dread_16ubit(in,list);
			error(erroroutput,"left indent is %d\n",retpap->leftmargin);
			(*l)+=2;
			break;
		case 0x840E:
			retpap->rightmargin=(S16)dread_16ubit(in,list);
			error(erroroutput,"right indent is %d\n",retpap->rightmargin);
			(*l)+=2;
			break;
		case 0x4610:
			temp = (S16)dread_16ubit(in,list);
			error(erroroutput,"nest left indent is %d\n",temp);
			if (temp < 0)
				retpap->leftmargin=0;
			else
				retpap->leftmargin+=temp;
			(*l)+=2;
			break;
		case 0x8411:
			retpap->firstline = (S16)dread_16ubit(in,list);
			error(erroroutput,"first line left1 indent is %d\n",retpap->firstline);
			(*l)+=2;
			break;
		case 0x9407:
			retpap->ourtap.rowheight = (S16)dread_16ubit(in,list);
			error(erroroutput,"the height of the table is %d\n",retpap->ourtap.rowheight);
			(*l)+=2;
			break;
		case 0x9602:
			error(erroroutput,"9602: gives %d\n",dread_16ubit(in,list));
			(*l)+=2;
			break;
		case 0x2A42:
			temp2 = dgetc(in,list);
			error(erroroutput,"the color is set to %d\n",temp2);	
			(*l)++;
			if (temp2 == 0)
				retchp->color[0] = '\0';
			else
				sprintf(retchp->color,"#%.2x%.2x%.2x",colorlookupr[temp2],colorlookupg[temp2],colorlookupb[temp2]);
			error(erroroutput,"the color is set to %s\n",retchp->color);	
			break;
		case 0x2403:
			retpap->justify = dgetc(in,list);
			error(erroroutput,"para just is %d\n",retpap->justify);
			(*l)++;
			break;
		case 0x0868:
		case 0x2406:
			dgetc(in,list);
			(*l)++;
			break;
		case 0x2A3E:
			retchp->underline = dgetc(in,list);
			error(erroroutput,"underline set to %d\n",retchp->underline);
			(*l)++;
			break;
		case 0x2A48:
			retchp->supersubscript = dgetc(in,list);
			error(erroroutput,"sub super is %d\n",retchp->supersubscript);
			(*l)++;
			break;
		case 0x740A:
			retpap->ourtap.tlp.itl = dread_16ubit(in,list);
			temp = dread_16ubit(in,list);
			if (temp & 0x0002)
				retpap->ourtap.tlp.fShading=1;
			if (temp & 0x0008)
				retpap->ourtap.tlp.fColor=1;
			if (temp & 0x0020)
				retpap->ourtap.tlp.fHdrRows=1;
			if (temp & 0x0040)
				retpap->ourtap.tlp.fLastRow=1;
			if (temp & 0x0080)
				retpap->ourtap.tlp.fHdrCols=1;
			if (temp & 0x0100)
				retpap->ourtap.tlp.fLastCol=1;
			(*l)+=4;
			break;
		case 0xD605:
			/*
			retpap->fInTable = 1;
			*/
			len = dgetc(in,list);
			(*l)++;
			error(erroroutput,"FIntable set to %d indirectly\n",retpap->fInTable);
			for(i=0;i<len;i++)
				{
				dgetc(in,list);
				(*l)++;
				}
			break;
		case 0xD609:
		/*the specs look wrong here, im winging it again :-)*/
			len = dgetc(in,list);
			retpap->ourtap.shade_no = len;
			(*l)++;
			error(erroroutput,"no of shaded things is %d\n",retpap->ourtap.shade_no);
			for (i=0;i<retpap->ourtap.shade_no/2;i++)
				{
				temp2 = dread_16ubit(in,list);
				error(erroroutput,"temp2 is %x\n",temp2);
				retpap->ourtap.cell_fronts[i] = temp2 & 0x001F;
				/*this is disabled until table looks are supported
				*/
				retpap->ourtap.cell_backs[i] = (temp2 & 0x03E0)>>5;
				
				retpap->ourtap.cell_pattern[i] = (temp2 & 0xFC00)>>10;
				error(erroroutput,"front color is %d\n",(temp2 & 0x001F));
				error(erroroutput,"back color is %d\n",((temp2 & 0x03E0))>>5);
				error(erroroutput,"shade pattern is %d\n",((temp2 & 0xFC00))>>10);
				(*l)+=2;
				}
			break;
		case 0x7627:
			/*not tested, might be wrong, specs are a little ambigious*/
			temp4 = dgetc(in,list);
			temp3 = dgetc(in,list);
			temp2 = dread_16ubit(in,list);
			(*l)+=4;
			for (i=temp4;i<temp3;i++)
				{
				retpap->ourtap.cell_fronts[i] = temp2 & 0x001F;
				retpap->ourtap.cell_backs[i] = (temp2 & 0x03E0)>>5;
				retpap->ourtap.cell_pattern[i] = (temp2 & 0xFC00)>>10;
				}
			error(erroroutput,"1: UNTESTED SHADE SUPPORT !!!!\n");
			break;
		case 0x7628:
			/*not tested, might be wrong, specs are a little ambigious*/
			temp4 = dgetc(in,list);
			temp3 = dgetc(in,list);
			temp2 = dread_16ubit(in,list);
			(*l)+=4;
			for (i=temp4;i<temp3;i++)
				{
				if (isodd(i))
					{
					retpap->ourtap.cell_fronts[i] = temp2 & 0x001F;
					retpap->ourtap.cell_backs[i] = (temp2 & 0x03E0)>>5;
					retpap->ourtap.cell_pattern[i] = (temp2 & 0xFC00)>>10;
					}
				}
			error(erroroutput,"2: UNTESTED SHADE SUPPORT !!!!\n");
			break;
		case 0xD608:
			/*
			retpap->fInTable = 1;
			*/
			len = dread_16ubit(in,list);
			(*l)+=2;
			error(erroroutput,"FIntable set to %d indirectly, val %x\n",retpap->fInTable,len);
			retpap->ourtap.cell_no = dgetc(in,list);
			(*l)++;
			error(erroroutput,"there are %d cells in this row\n",retpap->ourtap.cell_no);
			retpap->ourtap.tablewidth=0;
			for(i=0;i<retpap->ourtap.cell_no+1;i++)
				{
				retpap->ourtap.cellwidth[i] = (S16) dread_16ubit(in,list);
				error(erroroutput,"a dx is %d\n",retpap->ourtap.cellwidth[i]);
				(*l)+=2;
				}
			retpap->ourtap.tablewidth=retpap->ourtap.cellwidth[retpap->ourtap.cell_no] - retpap->ourtap.cellwidth[0];
			for(i=0;i<len-2-((retpap->ourtap.cell_no+1)*2);i++)
				{
				dgetc(in,list);
				(*l)++;
				}
			break;
		case 0x6A09:
			retchp->fontcode=dread_16ubit(in,list);
			error(erroroutput,"font code is %d\n", retchp->fontcode);
			retchp->fontspec=dread_16ubit(in,list);
			error(erroroutput,"char spec is %d\n",retchp->fontspec);
			retchp->fSpec=1;
			(*l)+=4;
			break;
		case 0x080A:
			retchp->fOle2=dgetc(in,list);
			error(erroroutput,"is an ole2 %d\n",retchp->fOle2);
			(*l)++;
			break;
			break;
		case 0x0856:
			retchp->fObj=dgetc(in,list);
			error(erroroutput,"is an object is %d\n",retchp->fObj);
			(*l)++;
			break;
		case 0x0854:
			dgetc(in,list);
			(*l)++;
			break;
		case 0x0855:
			retchp->fSpec=dgetc(in,list);
			error(erroroutput,"fSpec, special set to %d\n",retchp->fSpec);
			(*l)++;
			break;
		case 0x8840:
			/*i think these all are lumped together and take two bytes*/
			dread_16ubit(in,list);
			(*l)+=2;
			break;
		case 0x6A03:
			retchp->fSpec=1;
			retchp->fcPic = dread_32ubit(in,list);
			error(erroroutput,"fcpic is %x, fSpec\n", retchp->fcPic);
			/*
			get_blips(U32 fcDggInfo,U32 lcbDggInfo,FILE *tablefd,FILE *mainfd,int *noofblips,int dontdopre);
			*/
			(*l)+=4;
			break;
		case 0x6412:
			dread_16ubit(in,list);
			dread_16ubit(in,list);
			(*l)+=4;
			break;
		default:
			{
			error(erroroutput,"unsupported %x l is %d",clist,*l);
			clist = (clist & 0xe000);
			error(erroroutput,"val %d",clist);
			clist = clist >> 13;
			error(erroroutput,"val %d",clist);
			len=0;
			switch (clist)
				{
				case 0:
				case 1:
					len=1;
					break;
				case 2:
				case 4:
				case 5: 
					len=2;
					break;
				case 7:
					len=3;
					break;
				case 3:
					len=4;
					break;
				case 6:
					len=dgetc(in,list);
					(*l)++;
					break;
				default:
					error(erroroutput,"shouldnt see this len value!!\n");
					break;
				}
			
			for(i=0;i<len;i++)
				{
				dgetc(in,list);
				(*l)++;
				}
			error(erroroutput,"l is now %d\n",*l);
			}
		
		}
	}
