#ifndef MSWORDVIEW_HEADER
#define MSWORDVIEW_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************/

/*
  Pass in one of these as the flushout callback.  The implementation
  of this callback should return 0 (false) if the decoding should
  continue, or a non-zero value (true) if the decoding should stop.
*/
typedef int (* FlushCallback) (char * buffer,
							   int len,
							   void * data);

/*
  The library interface to the whole routine.  Look at the comments
  above the implementation for more information.
*/
int decodeWordFile(int argc, char ** argv, char * buf, int len, void * data,
				   FlushCallback func);
	
/***********************************************************************/
	
#ifndef PATH_MAX
#define PATH_MAX 255 /*seems a reasonable figure*/
#endif
 
#define U32 unsigned int
#define S32 signed int
#define U16 unsigned short
#define S16 signed short
#define U8 unsigned char

#define DEFAULTINDENT 1800
#define TWIRPS_PER_BQ 1440
#define TWIRPS_PER_H_PIXEL 20
#define TWIRPS_PER_V_PIXEL 15

#define SPACEPIXELS 6

struct tatrd
	{
	U16 xstUsrInitl[10];
	U16 ibst;
	U16 ak;			/*unused*/
	U16 grfbmc;		/*unused*/
	U32 lTagBkmk;
	};

typedef struct tatrd ATRD;

struct tstringgroup
	{
	U16 *author;
	struct tstringgroup *next;
	int noofstrings;
	};

typedef struct tstringgroup stringgroup;

struct tTLP
	{
	U16 itl;
	U8 fShading;
	U8 fColor;
	U8 fHdrRows;
	U8 fLastRow;
	U8 fHdrCols;
	U8 fLastCol;
	};

typedef struct tTLP TLP;

struct ttablelook
	{
	/* TOP BAR
	color for the top left entry
	color for the otherwise odd top entries
	color for the even top entries
	*/

	/* EVEN ROWS
	color for the left entry
	color for the otherwise odd row entries
	color for the even entries
	*/

	/* ODD ROWS
	color for the left entry
	color for the otherwise odd row entries
	color for the even entries
	*/
	
	char *color[9];
	};

typedef struct ttablelook tablelook;

struct tSTTBF
	{
	U16 exflag;
	U16 no_of_strings;
	U16 extra_byte_flag;
	U16 **chars;
	U8 **extra_bytes;
	};

typedef struct tSTTBF STTBF;

struct tBKF
	{
	U16 ibkl;
	U16 flags;
	};

typedef struct tBKF BKF;

struct tobj_by_spid
	{
	U16 spid;
	char *filename;
	struct tobj_by_spid *next;
	};

typedef struct tobj_by_spid obj_by_spid;

struct node
	{
	char streamname[255];
	char filename[PATH_MAX];
	struct node *next;
	int level;
	};

typedef struct node olestream;

struct tsep
	{
	U8 bkc;
	U8 fTitlePage;
	U8 nfcPgn;
	U8 fEndNote;
	U8 lnc;
	U8 nLnnMod;
	U16 ccolM1;
	U16 pgnStart;
	U8 restart;
	S16 leftmargin;
	};

typedef struct tsep sep;

struct tchp
    {
	unsigned short istd;
    U8 fBold;
    U8 fItalic;
	U8 fCaps;
	U8 fSmallCaps;
	U8 animation;
	U16 ascii_font;
	U16 eastfont;
	U16 noneastfont;
	U16 fontsize; /*half points*/
	U8 supersubscript;
	U16 fontcode;
	U16 fontspec;
	char color[8];
	U16 underline;
	U8 fSpec;
	U8 fObj;
	U8 fOle2;
	U8 idctHint;
	U32 fcPic;
	U8 fData;
	U8 fStrike;
	U8 fDStrike;
    };

typedef struct tchp chp;

struct tlist_def
	{
	U16 *list_string;	
	int len;
	S16 begin_no;
	int no_type;
	int fPrev;
	U32 id;
	chp achp;
	struct tlist_def *sub_def_list;
	};

typedef struct tlist_def list_def;

struct tanld
	{
	U8 nfc;
	U8 cxchTextBefore;
	U8 cxchTextAfter;
	U8 jc:2;
	U8 fPrev:1;
	U8 fHang:1;
	U8 fSetBold:1;
	U8 fSetItalic:1;
	U8 fSetSmallCaps:1;
	U8 fSetCaps:1;
	/*
	U8 flags1;
	*/
	U8 flags2;
	U8 flags3;
	U16 ftc;
	U16 hps;
	U16 startat;
	U16 rgxch[32];
	};

typedef struct tanld ANLD;


struct ttap
	{
	TLP tlp;
	int tablewidth;
	S16 cellwidth[65];
	int cell_no;
	int shade_no;
	int cell_backs[65];
	int cell_fronts[65];
	int cell_pattern[65];
	int rowheight;
	};

typedef struct ttap tap;

struct tpap
    {
    unsigned short istd;
    U8 fInTable;
    U8 fTtp;
	U8 tableflag;
	int justify;
	int ilvl; /*list level, 0 to 8*/
	long ilfo; /*list index*/
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
    };

typedef struct tpap pap;


struct field_pro
	{
	U32 *cps;
	U8 *flds;
	int no;
	};

typedef struct field_pro field_info;

struct tlist_info
	{
	/*
	now this is very hairy, i not sure how this is supposed to work
	so lists are a bit tentitive, basically theres no many things you 
	*can* do with lists, but hopefully this will sort out whether they
	are bulleted or enumerated, and ignore all sorts of shite like
	what kind of bullet were talking about, and whether some list
	items are numbered etc etc
	*/
	U8 *array;
	int count;
	int nooflsts;
	U32 *o_lst_ids;
	int **current_index_nos;
	list_def *o_list_def;
	U8 *level;

	U8 *lstarray;
	int lstcount;
	int nooflfos;
	U32 *lst_ids;
	list_def *a_list_def;
	int *overridecount;
	};

typedef struct tlist_info list_info;

struct tsprm
	{
	U8 *list;
	struct tsprm *next;
	int len;
	};

typedef struct tsprm Sprm;



struct tstyle
	{
	pap thepap;
	chp thechp;
	};

typedef struct tstyle style;

struct tbookmark_limits
	{
	U32 bookmark_b_no;
	U32 *bookmark_b_cps;
	BKF *bookmark_b_bkfs;
	U32 bookmark_e_no;
	U32 *bookmark_e_cps;
	};

typedef struct tbookmark_limits bookmark_limits;


struct ttextportions
	{
	U32 fcMin;
	U32 fcMac;
	U32 ccpText;
	U32 ccpFtn;
	U32 ccpHdd;
	U32 ccpAtn;
	U32 ccpEdn;
	U32 fcPlcfhdd;
	U32 lcbPlcfhdd;
	U32 *headercplist;
	U8 headercpno;

	U32 fndref_no;
	U32 fndtxt_no;
	U32 *fndRef;
	S16 *fndFRD;
	S16 *fndTrueFRD;
	U32 *fndTxt;
	int list_footnotes[256];
	int list_foot_no;
	int auto_foot;
	int last_foot;

	U32 endref_no;
	U32 endtxt_no;
	U32 *endRef;
	S16 *endFRD;
	S16 *endTrueFRD;
	U32 *endTxt;
	int list_endnotes[256];
	int list_end_no;
	int auto_end;

	U32 andref_no;
	U32 *andRef;
	U32 andtxt_no;
	U32 *andTxt;
	int list_annotations[256];
	int list_anno_no;
	stringgroup *authors;
	STTBF annotations;
	bookmark_limits a_bookmarks;
	ATRD *the_atrd;
	int last_anno;
	
	bookmark_limits l_bookmarks;
	STTBF bookmarks;

	U32 *section_cps;
	U32 *section_fcs;
	int section_nos;

	int noofficedraw;
	U32 *officedrawcps;
	U32 *spids; /*im ignoring the rest of the FSPA for now*/

	int noofblipdata;
	obj_by_spid *ablipdata;
	};

typedef struct ttextportions textportions;

struct tffn
	{
	char name[65];
	U8 chs;
	struct tffn *next;
	};

typedef struct tffn ffn;

#define IGNORENUM 0
#define DONTIGNORENUM 1
#define IGNOREALL 2


U16 read_16ubit(FILE *);
U32 read_32ubit(FILE *);

U32 sread_32ubit(U8 *in);
U16 sread_16ubit(U8 *in);
U8 sgetc(U8 *in);

U32 dread_32ubit(FILE *in,U8 **list);
U16 dread_16ubit(FILE *in,U8 **list);
U8 dgetc(FILE *in,U8 **list);

RETSIGTYPE reaper(int);
RETSIGTYPE timeingout(int );
void signal_handle (int sig, SigHandler * handler);

#if 0
void cleanupstreams(char *analyze,char *slashtmp);
olestream * divide_streams(char *filename,char **analyze,char **slashtmp, char *argv0);
#endif

int decode_word8(FILE *mainfd, FILE *tablefd0,FILE *tablefd1,FILE *data,int core);
void get_table_info(FILE *tablefd,list_info *a_list_info,U32 fcSttbFnm,U32 lcbSttbFnm,U32 fcPlcfLst,U32 lcbPlcfLst,U32 fcPlfLfo,U32 lcbPlfLfo,style *sheet);

pap *get_pap(U32 pageindex,FILE *in,U32 charindex, U32 *nextfc,style *sheet,list_info *a_list_info);
chp *get_chp(U32 pageindex,FILE *in, FILE *data, U32 charindex, U32 *nextfc,style *sheet,U16 istd);
sep *get_sep(U32 offset,FILE *in);

void decode_clx(U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headfooterflag);
void decode_clx_header(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag);
void decode_clx_footer(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag);
int decode_clx_endnote(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag);

void decode_simple(FILE *mainfd,FILE *tablefd,FILE *data,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag);
int decode_simple_footer(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag);
int decode_simple_endnote(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag);
void decode_simple_header(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag);

int decode_letter(int letter,int flag,pap *apap, chp * achp,field_info *magic_fields,FILE *main,FILE *data,ffn *fontnamelist,list_info *a_list_info,textportions *portions,int *issection);
void decode_f_reference(textportions *portions);
void decode_e_reference(textportions *portions);
void get_next_f_ref(textportions *portions,signed long *nextfootnote);
void get_next_e_ref(textportions *portions,signed long *nextendnote);
void decode_annotation(textportions *portions, FILE *main);

void decode_s_specials(pap *apap,chp *achp,list_info *a_list_info);
void decode_s_table(pap *apap,chp *achp,list_info *a_list_info);
void decode_e_specials(pap *apap,chp *achp,list_info *a_list_info);
int decode_e_table(pap *apap,chp *achp,list_info *a_list_info);

void decode_s_chp(chp *achp, ffn *fontnamelist);
void decode_e_chp(chp *achp);

void chpsoff(void);
void chpson(void);

void decode_list_nfc(int *value,int no_type);
void decode_list_level(pap *apap,int inalist,int num);

int flushbreaks(int);

void decode_s_anld(pap *apap,chp *achp,list_info *a_list_info,ffn *fontnamelist);
void decode_s_list(pap *apap,chp *achp,list_info *a_list_info,ffn *fontnamelist,int num);
void decode_e_list(pap *apap,chp *achp,list_info *a_list_info);

void decode_field(FILE *main,field_info *magic_fields,long *cp,U8 *fieldwas,long *swallowcp1,long *swallowcp2);

int find_FKPno_papx(U32 fc,U32 *plcfbtePapx,U32 intervals);
int find_FKPno_chpx(U32 fc,U32 *plcfbteChpx,U32 intervals);
U32 find_FC_sepx(U32 cp,U32 *sepcp,textportions *portions);
U32 find_next_smallest_fc(U32 charindex,U32 pageindex, FILE *in, S16 *location,long *pos);
U32 find_next_biggest_fc(U32 charindex,U32 pageindex, FILE *in, U16 *location,long *pos);
U32 find_next_biggest_orequal_fc(U32 charindex,U32 pageindex, FILE *in, U16 *location,long *pos);

pap * get_complex_pap(U32 fc,U32 *plcfbtePapx,U16 i,U16 nopieces,U32 intervals,U32 *rgfc,FILE *main,U32 *avalrgfc,U32 *thenextone,U32 *paraendfc,int *paraendpiece,style *sheet,list_info *a_list_info);
chp * get_complex_chp(U32 fc,U32 *plcfbteChpx,U16 i,U16 nopieces,U32 chpintervals,U32 *rgfc,FILE *main,U32 *avalrgfc,U32 *thenextone,style *sheet,U16 istd);

void decode_gpprls(pap *apap,chp *achp,sep *asep,U16* gpprlindex521,int ndx,Sprm *sprmlists, style *sheet);

style *decode_stylesheet(FILE *tablefd,U32 stsh,U32 stshlen);
void fill_pap(style *stylelist,int m,int b);

void decode_sprm(FILE* in,U16 clist,pap *retpap,chp *retchp,sep *retsep,U16 *pos,U8 **list, style *sheet,U16 istd);

void error(FILE *stream,char *fmt, ...);
void oprintf(int silentflag,char *fmt, ...);

int decode_symbol(U16 fontspec);
char *symbolfontdir(void);

int decode_wingding(U16 fontspec);
char *wingdingfontdir(void);

char *patterndir(void);

void decode_header(U32 *begin,U32 *len,textportions *portions,sep *asep);
void decode_header2(U32 *begin,U32 *len,textportions *portions);
void decode_footer(U32 *begin,U32 *len,textportions *portions,sep *asep);
void decode_footnote(U32 *begin,U32 *len,textportions *portions,int i);
void decode_endnote(U32 *begin,U32 *len,textportions *portions,int i);
void decode_footanno(U32 *begin,U32 *len,textportions *portions,int i);

int get_piecetable(FILE *in,U32 **rgfc,U32 **avalrgfc,U16 **sprm,U32 *clxcount);

int find_piece_cp(U32 sepcp,U32  *rgfc,int nopieces);

obj_by_spid * get_blips(U32 fcDggInfo,U32 lcbDggInfo,FILE *tablefd,FILE *mainfd,int *noofblips,int streamtype,obj_by_spid **realhead);
void output_draw(U32 cp,textportions *portions);

void do_indent(pap *apap);
ffn *get_fontnamefromcode(ffn *fontnamelist,int fontcode, int *ndx);

U32 get_fc_from_cp(U32 acp,U32 *rgfc,U32 *avalrgfc,int nopieces);

void end_para(pap *apap, pap *newpap);

int isodd(int i);

/*
returns slot to use in index array which keeps track of how far each list
has got
*/
int decode_ilfo(pap *retpap,list_info *a_list_info);

void init_chp(chp * achp);
void init_pap(pap * apap);

/*result += modified - blank*/
void merge_chps(chp *blank,chp *modified,chp *result);

void init_chp_from_istd(U16 istd,style *sheet,chp *retchp);
void init_pap_from_istd(U16 istd,style *sheet,pap *retpap);

void get_para_bounds(int currentpiece,U32 fc,U32 *rgfc,U32 *avalrgfc, int nopieces, U32 *plcfbtePapx,U32 intervals, FILE *main);

char *ms_strlower(char *in);

int myOLEdecode(char *filename, FILE **mainfd, FILE **tablefd0, FILE **tablefd1,FILE **data);

long get_picture_header(U32 fcPic,FILE *data,U32 *len,U16 *datatype);

void cleanupglobals(void);
char *ms_basename(char *filename);
void outputimgsrc(char *filename);


U32 decode_b_bookmark(bookmark_limits *l_bookmarks, STTBF *bookmarks);
U32 decode_e_bookmark(bookmark_limits *l_bookmarks);

U16 *decode_hyperlink(int letter, long int *swallowcp1, long int *swallowcp2, U16 **deleteme);
void decode_bookmarks(FILE *mainfd,FILE *tablefd,textportions *portions);
U16 *decode_crosslink(int letter,long int *swallowcp1, long int *swallowcp2);

void decode_annotations(FILE *mainfd,FILE *tablefd,textportions *portions);

int decompress(FILE *inputfile,char *outputfile,U32 inlen,U32 outlen);
void myfreeOLEtree(void);

void output_tablebg(pap *apap);
int do_tablelooks(pap *apap);

int setdecom(void);

void pagebreak(void);
void columnbreak(void);
void sectionbreak(sep *asep);
void copy_tap(tap *rettap,tap *intap);
void check_auto_color(chp *achp);

stringgroup *extract_authors(FILE *tablefd,U32 fcGrpXstAtnOwners,U32 lcbGrpXstAtnOwners);
void extract_sttbf(STTBF *bookmarks,FILE *tablefd,U32 fcSttbf,U32 lcbSttbf);
void extract_bookm_limits(bookmark_limits *l_bookmarks,FILE *tablefd,U32 fcPlcfbkf,U32 lcbPlcfbkf, U32 fcPlcfbkl,U32 lcbPlcfbkl);

int use_fontfacequery(chp *achp);

#define NOOFIDS 8

#ifdef __cplusplus
}
#endif

#endif
