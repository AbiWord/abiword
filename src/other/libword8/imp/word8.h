/* this has all of the structs I've used so far.
 * also, pap, and stylesheets from mswordview are here, but haven't
 * been modified yet (since I haven't figured out what the function
 * is doing quite yet)
 */

typedef U32 unsigned int
typedef S32 signed int
typedef U16 unsigned short
typedef S16 signed short
typedef U8 unsigned char
typedef S8 signed char

struct ATRD {
   U16 xstUsrInitl[10];
   U16 ibst;
   U16 ak;	 /*unused*/
   U16 grfbmc;	 /*unused*/
   U32 lTagBkmk;
} ATRD;

struct ffn {
   char name[65];
   U8 chs;
   struct ffn *next;
} ffn;

typedef struct PAP {
   U16 istd;
   U8 fInTable;
   U8 fTtp;
   U8 tableflag;
   S16 justify;
   S16 ilvl; /*list level, 0 to 8*/
   S32 ilfo; /*list index*/
   /*link to list information*/
   list_def *list_data;
   ANLD anld;
   tap ourtap;
   S16 leftmargin;
   S16 rightmargin;
   S16 firstline;
   U32 brcBottom;
   U32 brcLeft;
   U32 brcRight;
   U32 brcBetween;
   U16 dxaWidth;
   U32 dyaBefore;
   U32 dyaAfter;
} PAP;

typedef struct style {
   PAP *pap;
   CHP *chp;
} style;

typedef struct field_info {
   U32 *cps;
   U8 *flds;
   U16 no;
} field_info;

typedef struct fib {
   U16 wIdent; /* 0 */
   U16 nFib;
   U16 nProduct;
   U16 lid;
   S16 pnNext;
   struct { /* a series of bit-fields, 2-bytes total */
      unsigned int fDot : 1;
      unsigned int fGlsy : 1;
      unsigned int fComplex : 1;
      unsigned int fHasPic : 1;
      unsigned int cQuickSaves : 4;
      unsigned int fEncrypted : 1;
      unsigned int fWhichTblStm	: 1;
      unsigned int fReadOnlyRecommended : 1;
      unsigned int fWriteReservation : 1;
      unsigned int fExtChar : 1;
      unsigned int fLoadOverride : 1;
      unsigned int fFarEast : 1;
      unsigned int fCrypto : 1;
   } fFields;
   U16 nFibBack;
   U32 lKey;
   U8 envr;
   U8 fMac;
   U16 chs;
   U16 chsTables;
   S32 fcMin;
   S32 fcMac;
   
   S32 cbMac; /* 64 */
   U32 lProductCreated;
   U32 lProductRevised;
   
   S32 ccpText; /* 76 */
   S32 ccpFtn;
   S32 ccpHdd;
   S32 ccpMcr;
   S32 ccpAtn;
   S32 ccpEdn;
   S32 ccpTxbx;
   S32 ccpHdrTxbx;
	
   S32 pnChpFirst; /* 112 */
   S32 cpnBteChp;
	
   S32 pnPapFirst; /*124 */
   S32 cpnBtePap;
	
   S32 pnLvcFirst; /* 136 */
   S32 cpnBteLvc;
	
   S32 fcStshfOrig; /* 154 */
   U32 lcbStshfOrig;
   S32 fcStshf;
   U32 lcbStshf;
   S32 fcPlcffndRef; /* 170 */
   U32 lcbPlcffndRef;
   S32 fcPlcffndTxt;
   U32 lcbPlcffndTxt;
   /* annotations */
   S32 fcPlcfandRef; /* 180 */
   U32 lcbPlcfandRef;
   S32 fcPlcfandTxt;
   S32 lcbPlcfandTxt;
   /* section table */
   S32 fcPlcfsed; /* 202 */
   U32 lcbPlcfsed;	
   
   S32 fcPlcfhdd; /* 242 */
   U32 lcbPlcfhdd;
   S32 fcPlcfbteChpx; /* 250 */
   U32 lcbPlcfbteChpx;
   S32 fcPlcfbtePapx;
   U32 lcbPlcfbtePapx;
   
   S32 fcSttbfffn; /* 274 */
   U32 lcbSttbfffn;
   S32 fcPlcffldMom; /* 282 */
   U32 lcbPlcffldMom;
   S32 fcPlcffldHdr;
   U32 lcbPlcffldHdr;
   S32 fcPlcffldFtn;
   U32 lcbPlcffldFtn;
   S32 fcPlcffldAtn;
   U32 lcbPlcffldAtn;
   
   S32 fcClx; /* 418 */
   U32 lcbClx;
   
   S32 fcGrpXstAtnOwners; /* 442 */
   U32 lcbGrpXstAtnOwners;
   
   S32 fcPlcspaMom; /* 474 */
   U32 lcbPlcspaMom;
   
   /* endnotes */
   S32 fcPlcfendRef; /* 522 */
   U32 lcbPlcfendRef; 
   S32 fcPlcfendTxt;
   U32 lcbPlcfendTxt;
   S32 fcPlcffldEdn;
   U32 lcbPlcffldEdn;
   
   /* we use these to get spids, and then suck the pictures out 
    * of the the resulting tables found though fcDggInfo ?? 
    */
   S32 fcDggInfo; /* 554 */
   U32 lcbDggInfo;
   
   S32 fcSttbFnm; /* 730 */
   U32 lcbSttbFnm;
   S32 fcPlcfLst;
   U32 lcbPlcfLst;
   S32 fcPlfLfo;
   U32 lcbPlfLfo;
} fib_info;

typedef struct doc {

   /* ? */
   struct {
      char *main;
      char *data;
      char *table;
      char *table0;
      char *table1;
   } streams;
	
   fib_info fib;

   field_info main_fields;
   field_info header_fields;
   field_info footnote_fields;
   field_info annotation_fields;
   field_info endnote_fields;

   U32 footnote_ref_no;	
   U32 *footnote_ref;
   S16 *footnote_FRD;
   S16 *footnote_TrueFRD;
   U32 footnote_txt_no;
   U32 *footnote_txt;
   S16 footnote_list[256];
   S16 footnote_list_no;
   S16 footnote_auto;
   S16 footnote_last;
	
   U32 endnote_ref_no;	
   U32 *endnote_ref;
   S16 *endnote_FRD;
   S16 *endnote_TrueFRD;
   U32 footnote_txt_no;
   U32 *endnote_txt;
   S16 endnote_list[256];
   S16 endnote_list_no;
   S16 endnote_auto;
   S16 endnote_last;
	
   U32 annotation_ref_no;	
   U32 *annotation_ref;
   S16 *annotation_atrd;
   U32 annotation_txt_no;
   U32 *annotation_txt;
   S16 annotation_list[256];
   S16 annotation_list_no;
   
   U32 *sections_cps;
   U32 *sections_fcs;
   S16 sections_nos;
   
   ffn *fontnames;

} doc;

		
