/* haven't touched */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern int NORMAL;
extern int lastrowlen;
extern S16 lastcellwidth[65];
extern int nostyles;
extern FILE *erroroutput;

void copy_tap(tap *rettap,tap *intap)
	{
	int i;
	rettap->tlp.itl=intap->tlp.itl;
	rettap->tlp.fShading=intap->tlp.fShading;
	rettap->tlp.fColor=intap->tlp.fColor;
	rettap->tlp.fHdrRows=intap->tlp.fHdrRows;
	rettap->tlp.fLastRow=intap->tlp.fLastRow;
	rettap->tlp.fHdrCols=intap->tlp.fHdrCols;
	rettap->tlp.fLastCol=intap->tlp.fLastCol;
	rettap->tablewidth = intap->tablewidth;
	rettap->cell_no = intap->cell_no;
	rettap->rowheight=intap->rowheight;
	rettap->shade_no=intap->shade_no;

	for (i=0;i<65;i++)
		{
		rettap->cell_backs[i] = intap->cell_backs[i];
		rettap->cell_fronts[i] = intap->cell_fronts[i];
		rettap->cell_pattern[i] = intap->cell_pattern[i];
		rettap->cellwidth[i] = intap->cellwidth[i];
		}
	}


void init_pap(pap * retpap)
	{
	int i;
	retpap->fInTable = 0;
    retpap->fTtp= 0;
	retpap->justify = 0;
    retpap->ilvl = -1;
    retpap->ilfo = -1; /*index into hpllfo*/
    retpap->list_data=NULL;
    retpap->tableflag=0;
    retpap->leftmargin=0;
    retpap->rightmargin=0;
    retpap->firstline=0;
	retpap->istd=0;
	retpap->ourtap.tlp.itl=0;
	retpap->ourtap.tlp.fShading=0;
	retpap->ourtap.tlp.fColor=0;
	retpap->ourtap.tlp.fHdrRows=0;
	retpap->ourtap.tlp.fLastRow=0;
	retpap->ourtap.tlp.fHdrCols=0;
	retpap->ourtap.tlp.fLastCol=0;
    retpap->ourtap.tablewidth = 0;
	retpap->ourtap.cell_no = lastrowlen;
    retpap->ourtap.rowheight=0;
    retpap->ourtap.shade_no=0;
    for (i=0;i<65;i++)
        {
        retpap->ourtap.cell_backs[i] = 0;
        retpap->ourtap.cell_fronts[i] = 0;
        retpap->ourtap.cell_pattern[i] = 0;
        }
    if (lastrowlen > 63)	
	lastrowlen=63;
    for (i=0;i<lastrowlen+1;i++)
        {
        retpap->ourtap.cellwidth[i] = lastcellwidth[i];
        retpap->ourtap.tablewidth+=lastcellwidth[i];
        }

	retpap->brcBottom=0;
	retpap->brcLeft=0;
	retpap->brcRight=0;
	retpap->brcBetween=0;
	retpap->dxaWidth=0;
	retpap->dyaBefore=0;
	retpap->dyaAfter=0;
	}


void init_pap_from_istd(U16 istd,style *sheet,pap *retpap)
	{
	retpap->fInTable  = sheet[retpap->istd].thepap.fInTable;
    retpap->fTtp = sheet[retpap->istd].thepap.fTtp;
    retpap->ilvl = sheet[retpap->istd].thepap.ilvl;
    retpap->ilfo = sheet[retpap->istd].thepap.ilfo;
    retpap->justify= sheet[retpap->istd].thepap.justify;
    retpap->list_data = sheet[retpap->istd].thepap.list_data;
    retpap->anld = sheet[retpap->istd].thepap.anld;
    retpap->leftmargin = sheet[retpap->istd].thepap.leftmargin;
    retpap->rightmargin = sheet[retpap->istd].thepap.rightmargin;
    retpap->firstline = sheet[retpap->istd].thepap.firstline;
    retpap->tableflag= sheet[retpap->istd].thepap.tableflag;
    retpap->brcBottom= sheet[retpap->istd].thepap.brcBottom;
    retpap->brcLeft= sheet[retpap->istd].thepap.brcLeft;
    retpap->brcRight= sheet[retpap->istd].thepap.brcRight;
    retpap->brcBetween= sheet[retpap->istd].thepap.brcBetween;
	retpap->dxaWidth=sheet[retpap->istd].thepap.dxaWidth;
	retpap->dyaAfter=sheet[retpap->istd].thepap.dyaAfter;
	retpap->dyaBefore=sheet[retpap->istd].thepap.dyaBefore;
	copy_tap(&(retpap->ourtap),&(sheet[retpap->istd].thepap.ourtap));
	}

void init_chp(chp * achp)
    {
    achp->istd=10;
    achp->fBold=0;
    achp->fItalic=0;
    achp->fCaps=0;
    achp->fSmallCaps=0;
    achp->animation=0;
    achp->ascii_font=0;
    achp->eastfont=0;
    achp->noneastfont=0;
    achp->fontsize=NORMAL;
    achp->supersubscript=0;
    achp->fontcode=0;
    achp->fontspec=0;
    achp->fSpec=0;
    achp->fObj=0;
    achp->fOle2=0;
    achp->color[0]='\0';
    achp->underline=0;
    achp->idctHint=0;
	achp->fcPic=-1;
	achp->fData=0;
	achp->fStrike=0;
	achp->fDStrike=0;
    }


void init_chp_from_istd(U16 istd,style *sheet,chp *retchp)
    {
	if (istd >= nostyles)
		{
		error(erroroutput,"istd greater than no of styles !!\n");
		return;
		}
	retchp->istd = istd;
    retchp->fBold=sheet[istd].thechp.fBold;
    retchp->fItalic=sheet[istd].thechp.fItalic;
    retchp->fCaps =sheet[istd].thechp.fCaps;
    retchp->fSmallCaps =sheet[istd].thechp.fSmallCaps;
    retchp->animation =sheet[istd].thechp.animation;
    retchp->ascii_font=sheet[istd].thechp.ascii_font;
    retchp->eastfont=sheet[istd].thechp.eastfont;
    retchp->noneastfont=sheet[istd].thechp.noneastfont;
    retchp->fontsize=sheet[istd].thechp.fontsize;
    retchp->supersubscript=sheet[istd].thechp.supersubscript;
    retchp->fontcode=sheet[istd].thechp.fontcode;
    retchp->fontspec=sheet[istd].thechp.fontspec;
    strcpy(retchp->color,sheet[istd].thechp.color);
    retchp->underline=sheet[istd].thechp.underline; 
    retchp->fSpec=sheet[istd].thechp.fSpec;
    retchp->fObj=sheet[istd].thechp.fObj;
    retchp->fOle2=sheet[istd].thechp.fOle2;
    retchp->idctHint=sheet[istd].thechp.idctHint;
    retchp->fcPic =sheet[istd].thechp.fcPic;
    retchp->fData=sheet[istd].thechp.fData;
    retchp->fStrike=sheet[istd].thechp.fStrike;
    retchp->fDStrike=sheet[istd].thechp.fDStrike;
    }

void merge_chps(chp *blank,chp *modified,chp *result)
	{
	if (blank->fBold != modified->fBold) result->fBold = modified->fBold;
	if (blank->fItalic != modified->fItalic) result->fItalic= modified->fItalic;
	if (blank->fCaps != modified->fCaps) result->fCaps= modified->fCaps;
	if (blank->fSmallCaps != modified->fSmallCaps) result->fSmallCaps= modified->fSmallCaps;
	if (blank->animation!= modified->animation) result->animation= modified->animation;
	if (blank->ascii_font!= modified->ascii_font) result->ascii_font= modified->ascii_font;
	if (blank->eastfont!= modified->eastfont) result->eastfont= modified->eastfont;
	if (blank->noneastfont!= modified->noneastfont) result->noneastfont= modified->noneastfont;
	if (blank->fontsize!= modified->fontsize) result->fontsize= modified->fontsize;
	if (blank->supersubscript!= modified->supersubscript) result->supersubscript= modified->supersubscript;
	if (blank->fontcode!= modified->fontcode) result->fontcode= modified->fontcode;
	if (blank->fontspec!= modified->fontspec) result->fontspec= modified->fontspec;
	if (blank->underline!= modified->underline) result->underline= modified->underline;
	if (blank->fSpec!= modified->fSpec) result->fSpec= modified->fSpec;
	if (blank->fObj!= modified->fObj) result->fObj= modified->fObj;
	if (blank->fOle2!= modified->fOle2) result->fOle2= modified->fOle2;
	if (blank->idctHint!= modified->idctHint) result->idctHint= modified->idctHint;
	if (blank->fcPic!= modified->fcPic) result->fcPic= modified->fcPic;
	if (blank->fData!= modified->fData) result->fData= modified->fData;
	if (strcmp(blank->color,modified->color)) strcpy(result->color,modified->color);
	if (blank->fStrike!= modified->fStrike) result->fStrike= modified->fStrike;
	if (blank->fDStrike!= modified->fDStrike) result->fDStrike= modified->fDStrike;
	}

