#ifndef IE_IMP_MSWORD_DUMP
#error This file must only be included in ie_imp_MsWord97.cpp !!!
#else

#ifndef IE_IMP_MSWORD_DUMP_H
#define IE_IMP_MSWORD_DUMP_H

#if 0 // changed when used again
static void s_dump_chp(CHP *achp)
{
	FILE * dumpf = fopen("chp_fmt_dump.txt","a");
	UT_return_if_fail(dumpf);

	fprintf(dumpf,"IE_Imp_MsWord_97::_beginChar: achp dump:\n"
	"	fBold %d\n"
	"	fItalic %d\n"
	"	fRMarkDel %d\n"
	"	fOutline %d\n"
	"	fFldVanish %d\n"
	"	fSmallCaps %d\n"
	"	fCaps %d\n"
	"	fVanish %d\n"
	"	fRMark %d\n"
	"	fSpec %d\n"
	"	fStrike %d\n"
	"	fObj %d\n"
	"	fShadow %d\n"
	"	fLowerCase %d\n"
	"	fData %d\n"
	"	fOle2 %d\n"
	"	fEmboss %d\n"
	"	fImprint %d\n"
	"	fDStrike %d\n"
	"	fUsePgsuSettings %d\n"
	"	reserved1 %d\n"
	"	reserved2 %d\n"
	"	reserved11 %d\n"
	"	ftc %d\n"
	"	ftcAscii %d\n"
	"	ftcFE %d\n"
	"	ftcOther %d\n"
	"	hps %d\n"
	"	dxaSpace %d\n"
	"	iss %d\n"
	"	kul %d\n"
	"	fSpecSymbol %d\n"
	"	ico %d\n"
	"	reserved3 %d\n"
	"	fSysVanish %d\n"
	"	hpsPos %d\n"
	"	super_sub %d\n"
	"	lid %d\n"
	"	lidDefault %d\n"
	"	lidFE %d\n"
	"	idct %d\n"
	"	idctHint %d\n"
	"	wCharScale %d\n"
	"	fcPic_fcObj_lTagObj %d\n"
	"	ibstRMark %d\n"
	"	ibstRMarkDel %d\n"
	"	dttmRMark %d\n"
	"	dttmRMarkDel %d\n"
	"	reserved4 %d\n"
	"	istd %d\n"
	"	ftcSym %d\n"
	"	xchSym %d\n"
	"	idslRMReason %d\n"
	"	idslReasonDel %d\n"
	"	ysr %d\n"
	"	chYsr %d\n"
	"	cpg %d\n"
	"	hpsKern %d\n"
	"	icoHighlight %d\n"
	"	fHighlight %d\n"
	"	kcd %d\n"
	"	fNavHighlight %d\n"
	"	fChsDiff %d\n"
	"	fMacChs %d\n"
	"	fFtcAsciSym %d\n"
	"	reserved5 %d\n"
	"	fPropRMark %d\n"
	"	ibstPropRMark %d\n"
	"	dttmPropRMark %d\n"
	"	sfxtText %d\n"
	"	reserved6 %d\n"
	"	reserved7 %d\n"
	"	reserved8 %d\n"
	"	reserved9 %d\n"
	"	reserved10 %d\n"
	"	fDispFldRMark %d\n"
	"	ibstDispFldRMark %d\n"
	"	dttmDispFldRMark %d\n"
	"	xstDispFldRMark %p\n"
	"	shd %d\n"
	"	brc %d\n"
	"	fBidi %d\n"
	"	fBoldBidi %d\n"
	"	fItalicBidi %d\n"
	"	ftcBidi %d\n"
	"	hpsBidi %d\n"
	"	icoBidi %d\n"
	"	lidBidi %d\n",

		achp->fBold,
		achp->fItalic,
		achp->fRMarkDel,
		achp->fOutline,
		achp->fFldVanish,
		achp->fSmallCaps,
		achp->fCaps,
		achp->fVanish,
		achp->fRMark,
		achp->fSpec,
		achp->fStrike,
		achp->fObj,
		achp->fShadow,
		achp->fLowerCase,
		achp->fData,
		achp->fOle2,
		achp->fEmboss,
		achp->fImprint,
		achp->fDStrike,
		achp->fUsePgsuSettings,
		achp->reserved1,
		achp->reserved2,
		achp->reserved11,
		achp->ftc,
		achp->ftcAscii,
		achp->ftcFE,
		achp->ftcOther,
		achp->hps,
		achp->dxaSpace,
		achp->iss,
		achp->kul,
		achp->fSpecSymbol,
		achp->ico,
		achp->reserved3,
		achp->fSysVanish,
		achp->hpsPos,
		achp->super_sub,
		achp->lid,
		achp->lidDefault,
		achp->lidFE,
		achp->idct,
		achp->idctHint,
		achp->wCharScale,
		achp->fcPic_fcObj_lTagObj,
		achp->ibstRMark,
		achp->ibstRMarkDel,
		*(static_cast<int*>(&achp->dttmRMark)),
		*(static_cast<int*>(&achp->dttmRMarkDel)),
		achp->reserved4,
		achp->istd,
		achp->ftcSym,
		achp->xchSym,
		achp->idslRMReason,
		achp->idslReasonDel,
		achp->ysr,
		achp->chYsr,
		achp->cpg,
		achp->hpsKern,
		achp->icoHighlight,
		achp->fHighlight,
		achp->kcd,
		achp->fNavHighlight,
		achp->fChsDiff,
		achp->fMacChs,
		achp->fFtcAsciSym,
		achp->reserved5,
		achp->fPropRMark,
		achp->ibstPropRMark,
		*(static_cast<int*>(&achp->dttmPropRMark)),
		achp->sfxtText,
		achp->reserved6,
		achp->reserved7,
		achp->reserved8,
		achp->reserved9,
		*(static_cast<int*>(&achp->reserved10)),
		achp->fDispFldRMark,
		achp->ibstDispFldRMark,
		*(static_cast<int*>(&achp->dttmDispFldRMark)),
		achp->xstDispFldRMark,
		*(static_cast<int*>(&achp->shd)),
		*(static_cast<int*>(&achp->brc)),
		achp->fBidi,
		achp->fBoldBidi,
		achp->fItalicBidi,
		achp->ftcBidi,
		achp->hpsBidi,
		achp->icoBidi,
		achp->lidBidi
	);
	if(dumpf)
		fclose(dumpf);
}

#endif

#endif
#endif
