/*
  Hacked up to spew out AbiWord XML by Shaw Terwilliger,
  sterwill@abisource.com.
*/


/*
Released under GPL, written by Caolan.McNamara@ul.ie.

Copyright (C) 1998 
	Caolan McNamara

Real Life: Caolan McNamara           *  Doing: MSc in HCI
Work: Caolan.McNamara@ul.ie          *  Phone: +353-61-202699
URL: http://skynet.csn.ul.ie/~caolan *  Sig: an oblique strategy
How would you have done it?
*/

/*
this software no longer requires laola
*/

char *version="mswordview 0.5.2";

/*

this code is often all over the shop, being more of an organic entity
that a carefully planed piece of code, so no laughing there at the back!

and send me patches by all means, but think carefully before sending me
a patch that doesnt fix a bug or add a feature but instead just changes
the style of coding, i.e no more thousand line patches that fix my 
indentation. (i like it this way)

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "config.h"
#include "mswordview.h"
#include "roman.h"
#include "utf.h"

extern int nostyles;
extern ATRD *key_atrd;

/*ok i got impatient*/
obj_by_spid *picblips=NULL;

int notablewidth=0;
int madeinmac=0;

int list_author_key=0;

int nofontfaces=0;
int riskbadole=0;
int NORMAL=12;	/* sterwill: was 20 */
int inunderline = 0;
int inbold = 0;
int incenter= 0;
int inrightjust= 0;
int inblink= 0;
int inah1= 0;
int inaheaderfooter=0;
int initalic = 0;
int inafont= 0;
int pagenumber=1;
int sectionpagenumber=1;
int sectionno=1;
int insuper = 0;
int insub = 0;
int inatable=0;
float tabsize=-1;
int cellempty=1;
int inarow=0;
int inacell=0;
int inalist = 0;
int doannos=1;

int noheaders=0;

int deferrednewpage=0;

S16 lastcellwidth[65];
S16 lastrowlen=0;

int colcount;
int rowcount;

U32 mainend;
U32 lastlistid =0;
int listvalue[9];
int currentfontsize;
int currentfontcode=-1;

sep *currentsep=NULL;

int newline=1;
int inacolor;
char incolor[8]="";
char backgroundcolor[8]="";

U8 chps=0;

int padding=0;
int verpadding=0;
int ignoreheadings=0;
long int cp=0;
long int realcp=0;
static U8 header;
int breakcount;
int footnotehack=0;
int instrike=0;

char *symbolurl=NULL;
char *imagesurl=NULL;
char *imagesdir=NULL;
char *wingdingurl=NULL;
char *patternurl=NULL;
char *outputfilename=NULL;
char *errorfilename=NULL;
char *filename=NULL;
FILE *outputfile=NULL;
FILE *erroroutput=NULL;

int no_default_props_hack = 0;

/*  global replacement for outputfile (not used in XML library interface) */
char * outputbuffer = NULL;
int outputbufferlen = 0;
FlushCallback outputbufferfunc = NULL;
void * outputbufferdata = NULL;

void usage( void )
    {
    fprintf(erroroutput,"Usage: mswordview [-v] [--version] [-n] [--nocredits] [-c] [--corehtmlonly]\n"
	                    "[-t seconds] [--timeout seconds] [-f points] [--defaultfontsize points] [-p url]\n"
						"[--patternurl url] [-s url] [--symbolurl url] [-d url] [--wingdingurl url] [-h]\n"
						"[--ignoreheadings] [-m] [--mainonly] [-b] [--riskbadole] [-e] [--nofontfaces]\n"
						"[-o] [--outputfile] [-g erroroutputfile] [--errorfile erroroutputfile] \n"
						"[-y tabvalue] [--tabsize tabvalue] [-i] [--imagesdir] [-j] [--imagesurl] \n"
						"[-k] [--notablewidth] filename.doc\n");
    exit(-1);
    }

/*
  This is now the only way to output XML data... 
*/
int spewString(const char * template, ...)
{
	va_list args;

	if(!outputbuffer && !outputbufferfunc)
		return 0;

	/* BIG PROBLEM: bad things will happen if the buffer size is too short. */
	va_start(args, template);
#if 0
	vsnprintf(outputbuffer, outputbufferlen, template, args);
#else
	vsprintf(outputbuffer, template, args);
#endif
	va_end(args);

	return (* outputbufferfunc) (outputbuffer, strlen(outputbuffer), outputbufferdata);
}

void cutOffFormats(void)
{
	if (inunderline)
	{
		spewString("</c>");
		inunderline=0;
	}
	if (initalic)
	{
		spewString("</c>");
		initalic=0;
	}
	if (inbold)
	{
		spewString("</c>");
		inbold=0;
	}
	if (inblink)
	{
		/*spewString("</BLINK>");*/
		inblink=0;
	}
	if (inafont)
	{
		inacolor=0;
		incolor[0] = '\0';
		spewString("</c>");
		currentfontsize=NORMAL;
		inafont=0;
	}
	if (inacolor)
	{
		spewString("</c>");
		inacolor=0;
	}
	if (instrike)
	{
		spewString("</c>");
		instrike=0;
	}
	if (inah1)
	{
		spewString("</c>");
		inah1=0;
	}
}

/*
  The first two arguments are for lazy options passing.  If (when?) this
  becomes a "real" interface to Caolan's code, we should use something
  a bit more programmatic.

  buf is a character array of length len, and func is a callback
  to let the caller flush out the contents of the buffer (the most
  recently written line of XML).

  Oh, and this whole library is not safely re-entrant or thread-safe.
  Don't even try.

  You'll probably want to call this like:

  int result = decodeWordFile(4,
                              { "mswordview",
							    "-o",
								"-",
								"filename.doc",
								NULL },
								mybuffer,
								MYBUFFER_LEN,
								mydata,
								callback_func);

  ... where one should note the use of "-" (stdout) as the output
  file.  I took a hands-off approach to modifying just the output parts
  of mswordview, so I didn't chnage how the arguments were parsed.
  Using "-" means it won't try to open a file (stdout is always available),
  which shouldn't be done when called from a library.  stdout, however,
  is not written to, it just prevents a cluttered disk.
*/

int decodeWordFile(int argc, char ** argv, char * buf, int len, void * cbdata,
				   FlushCallback func)
	{
	
	
	int ret=0;
	int timeout=-1;
	char *endptr;
	char *buffer;
	char fileinbuf[1024];
	
	FILE *filein;

	FILE *mainfd=NULL;
	FILE *tablefd0=NULL;
	FILE *tablefd1=NULL;
	FILE *data=NULL;

	int c;
	int tail=1;
	int core=1;
	int index=0;

	chp achp;
	pap apap;

	static struct option long_options[] = 
		{
		{ "version",0 , 0, 'v' },
		{ "corehtmlonly",0 , 0, 'c' },
		{ "nocredits",0 , 0, 'n' },
		{ "noannotations",0 , 0, 'a' },
		{ "timeout",1 , 0, 't' },
		{ "horizontalwhite",1,0,'w'},
		{ "verticalwhite",1,0,'u'},
		{ "symbolurl", 1,0,'s'},
		{ "wingdingurl", 1,0,'d'},
		{ "imagesdir", 1,0,'i'},
		{ "imagesurl", 1,0,'j'},
		{ "ignoreheadings", 0,0,'h'},
		{ "defaultfontsize", 1,0,'f'},
		{ "mainonly", 0,0,'m' },
		{ "riskbadole", 0,0,'b'},
		{ "nofontfaces", 0,0,'e'},
		{ "outputfile", 1,0,'c'},
		{ "errorfile", 1,0,'g'},
		{ "patternurl",1,0,'p'},
		{ "tabsize",1,0,'y'},
		{ "notablewidth",1,0,'k'},
		{ 0,      0, 0, '0' },
		};


	erroroutput=stderr;


	while (1)
		{
		c = getopt_long (argc, argv, "abcd:ef:g:hi:j:kmno:p:s:t:u:vw:y:", long_options, &index);
		if (c == -1)
			break;
		switch(c)
			{
			case 'f':
				if (optarg)
					{
                   	NORMAL = strtol(optarg, &endptr, 10);
				   	if ((*optarg == '\0') || (*endptr  != '\0'))
						{
						fprintf(erroroutput,"f option must be followed with a number\n");
						NORMAL=20;
						}
					else
						NORMAL*=2;
				 	}
				else 
					{
					fprintf(erroroutput,"no val given for defaultfontsize (-f) \n");
					NORMAL=20;
					}
				break;
			case 'v':
				printf("%s, by Caolan.McNamara@ul.ie\n\nWeb pages for updates exist at \nhttp://www.gnu.org/~caolan/docs/MSWordView.html\nhttp://www.csn.ul.ie/~caolan/docs/MSWordView.html\n\n", version);
				exit (0);
				break;
			case 'k':
				notablewidth=1;
				break;
			case 'm':
				noheaders=1;
				break;
			case 'e':
				nofontfaces=1;
				break;
			case 'b':
				riskbadole=1;
				break;
			case 'h':
				ignoreheadings=1;
				break;
			case 'n':
				tail=0;
				break;	
			case 'a':
				doannos=0;
				break;	
			case 'u':
				if (optarg)
					{
                   	verpadding = strtol(optarg, &endptr, 10);
				   	if ((*optarg == '\0') || (*endptr  != '\0'))
						{
						fprintf(erroroutput,"u option must be followed with a number\n");
						verpadding=0;
						}
					if ((verpadding < 0) || (verpadding > 2))
						{
						fprintf(erroroutput,"u option accepts on 0,1,2 as an argument, not %d\n",verpadding);
						verpadding=0;
						}
				 	}
				else 
					{
					fprintf(erroroutput,"no val given for verticalpadding (-u) \n");
					verpadding=0;
					}
				
				
				break;	
			case 't':
				if (optarg)
					{
                   	timeout = strtol(optarg, &endptr, 10);
				   	if ((*optarg == '\0') || (*endptr  != '\0'))
						fprintf(erroroutput,"t option must be followed with a number\n");
				 	}
				else 
					fprintf(erroroutput,"no val given for timeout option (-t)\n");
				break;
			case 'o':
				if (optarg)
					{
					outputfilename = malloc(strlen(optarg)+1);
					if (outputfilename == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(outputfilename,optarg);
					}
                else
					{
                    fprintf(erroroutput,"no val given, assuming standard\n");
					}
                break;
			case 'g':
				if (optarg)
					{
					errorfilename = malloc(strlen(optarg)+1);
					if (errorfilename == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(errorfilename,optarg);
					}
                else
					{
                    fprintf(erroroutput,"no val given, assuming standard\n");
					}
                break;
			case 'j':
				if (optarg)
					{
					imagesurl = malloc(strlen(optarg)+1);
					if (imagesurl == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(imagesurl,optarg);
					}
                else
                    fprintf(erroroutput,"no val given for imagesurl\n");
                break;
			case 'i':
				if (optarg)
					{
					imagesdir = malloc(strlen(optarg)+1);
					if (imagesdir == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(imagesdir,optarg);
					}
                else
                    fprintf(erroroutput,"no val given for imagesdir\n");
                break;
			case 's':
				if (optarg)
					{
					symbolurl = malloc(strlen(optarg)+1);
					if (symbolurl == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(symbolurl,optarg);
					}
                else
                    fprintf(erroroutput,"no val given for symbolurl\n");
                break;
			case 'p':
				if (optarg)
					{
					patternurl = malloc(strlen(optarg)+1);
					if (patternurl == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(patternurl,optarg);
					}
                else
                    fprintf(erroroutput,"no val given for patternurl\n");
                break;
			case 'd':
				if (optarg)
					{
					wingdingurl = malloc(strlen(optarg)+1);
					if (wingdingurl == NULL)
						{
						fprintf(erroroutput,"no mem\n");
						exit(-1);
						}
					strcpy(wingdingurl,optarg);
					}
                else
                    fprintf(erroroutput,"no val given for wingdingurl\n");
                break;
			case 'w':
				if (optarg)
					{
                   	padding = strtol(optarg, &endptr, 10);
				   	if ((*optarg == '\0') || (*endptr  != '\0'))
						{
						fprintf(erroroutput,"w option must be followed with a number\n");
						padding=0;
						}
					if ((padding < 0) || (padding > 5))
						{
						fprintf(erroroutput,"w option must be followed with a number from 0..5\n");
						padding=0;
						}
				 	}
				else 
					{
					fprintf(erroroutput,"no val given for horizontalwhite option (-w)\n");
					padding=0;
					}
				break;
			case 'y':
				if (optarg)
					{
                   	tabsize = strtod(optarg, &endptr);
				   	if ((*optarg == '\0') || (*endptr  != '\0'))
						{
						fprintf(erroroutput,"tabsize (y) option must be followed with a number\n");
						tabsize=-1;
						}
				 	}
				else 
					{
					fprintf(erroroutput,"no val given for defaultfontsize (-f) \n");
					tabsize = -1;
					}
				break;
			case 'c':
				core=0;
				break;
			default:
				usage();
				break;
			}
		}

	if (argc <= optind) 
		{
		usage();
		}

	if (errorfilename != NULL)
		{
		erroroutput = fopen(errorfilename,"w");
		if (erroroutput== NULL)
			{
			fprintf(erroroutput,"couldnt open %s for writing\n",errorfilename);
			return(-1);
			}
		}
	else
		erroroutput = erroroutput;

	/* special output path */
	{
		outputbuffer = buf;
		outputbufferlen = len;
		outputbufferfunc = func;
		outputbufferdata = cbdata;
	}

	if (optind < argc)
		filename = strdup(argv[optind]);
	else 
		filename =NULL;

	if (filename == NULL)
		{
		fprintf(erroroutput,"The file %s doesn't exist\n",filename);
		return(ret);
		}

#if 0
	/*set SIGCHLD handled*/
	signal_handle (SIGCHLD, reaper);
#endif

	currentfontsize = NORMAL;

#if 0	
	if (timeout != -1)
		{
		signal_handle (SIGALRM, timeingout);
		/*well abort after this number of seconds*/
		alarm(timeout);
		}
#endif

	if (tabsize == -1)
		{
		if ( (padding == 0) || (padding == 3))
			tabsize=SPACEPIXELS;
		else
			tabsize=8;
		}
	else
		if ( (padding == 0) || (padding == 3))
			tabsize=tabsize/8;


	ret = myOLEdecode(filename,&mainfd,&tablefd0,&tablefd1,&data);
	if (ret)
		{
#if 0
		fprintf(erroroutput,"Sorry main document stream couldnt be found in doc \n%s\n",filename);
		fprintf(erroroutput,"if this *is* a word 8 file, it appears to be corrupt\n");
		fprintf(erroroutput,"remember, mswordview cannot handle rtf,word 6 or word 7 etc\n");

		buffer = (char *) malloc(strlen(filename) +3 + strlen("file "));
		sprintf(buffer,"file \"%s\"",filename);
		filein = popen(buffer,"r");
		if (filein != NULL)
			{
			fprintf(erroroutput,"for your information, the utility \n\"file %s\" reports ...\n\n",filename);
			while (fgets(fileinbuf,1024,filein) != NULL)
				fprintf(erroroutput,"%s",fileinbuf);
			}
		free(buffer);
#endif
		ret=10;
		if (riskbadole) 
			ret = decode_word8(mainfd,tablefd0,tablefd1,data,core);
		}
	else
		{
		ret = decode_word8(mainfd,tablefd0,tablefd1,data,core);
		myfreeOLEtree();
		}

	if ((ret != -1) && (ret != 10))
		{
		if (inacell)
			{
			init_chp(&achp);
			init_pap(&apap);
			decode_e_chp(&achp);
			decode_e_specials(&apap,&achp,NULL);

/*			spewString("\n</TD>\n"); */
			backgroundcolor[0] = '\0';
			}
/*
		if (inarow)
			spewString("\n</tr>\n");
		if (inatable)
			spewString("\n</table>\n");
*/
		do_indent(&apap);

		init_chp(&achp);
		init_pap(&apap);
		decode_e_chp(&achp);

/* sterwill: Do we really want to throw (possibly) mismatching tags at the end? */
/*
		if (inafont)
			{
				 spewString("</c>");
			inafont=0;
			}
		if (inacolor)
		{
			spewString("</c>");
			inacolor=0;
		}
*/		
		decode_e_specials(&apap,&achp,NULL);

/*		spewString("\n<br><img src=\"%s/documentend.gif\"><br>\n",patterndir()); */

/*
		if (tail)
			{
			spewString("<hr><p>\nDocument converted from word 8 by \n<a href=\"http://www.csn.ul.ie/~caolan/docs/MSWordView.html\">MSWordView</a> (%s)<br>\n",version);
			spewString("MSWordView written by <a href=\"mailto:Caolan.McNamara@ul.ie\">Caolan McNamara</a>\n</body>\n");
			}
*/
		/* sterwill: handle section with awml, since we do 1 section now */
		if (core)
		{
/*			spewString("</c>\n");*/	/* the first thing mswordview does is create a top-level <c>
									   to cover the default font setup ("Times New Roman").  End it.
									*/
			spewString("</p>\n"); /*we need at least one paragraph  */
			spewString("</section>\n");
			spewString("</awml>");
		}
		
		}
	cleanupglobals();
	
	if (ret == 10) /*known reason as to why conversion did not occur*/
		ret=0;
	return(ret);
	}

void cleanupglobals(void)
	{
	obj_by_spid *freeme=NULL;

	if ((outputfile!=NULL) && (outputfile!=stdout))
		fclose(outputfile);

	if ((erroroutput!=NULL) && (erroroutput!=stderr))
		fclose(erroroutput);

	if (picblips != NULL)
		{
		freeme = picblips;
		picblips = picblips->next;
		if (freeme->filename != NULL)
			free(freeme->filename);
		free(freeme);
		}
	
	if (symbolurl!=NULL)
		free(symbolurl);
	if (imagesurl!=NULL)
		free(imagesurl);
	if (imagesdir!=NULL)
		free(imagesdir);
	if (wingdingurl!=NULL)
		free(wingdingurl);
	if (patternurl!=NULL)
		free(patternurl);
	if (outputfilename!=NULL)
		free(outputfilename);
	if (errorfilename!=NULL)
		free(errorfilename);
	if (filename!=NULL)
		free(filename);
	if (key_atrd != NULL)
		free(key_atrd);
	}

int decode_word8(FILE *mainfd,FILE *tablefd0,FILE *tablefd1,FILE *data,int core)
	{
	U32 i,j,k;
	
	U16 wIdent,nFib,nProduct,lid,pnNext,nFibBack,chse,chseTables;
	U32 lKey,fcMin,fcMac,cbMac,fcSpare,ccpMcr,ccpTxbx,ccpSpare2,fcStshfOrig,lcbStshfOrig;
	textportions portions;
	U8 fields1,fields2,envr,reserved;
	U8 iscomplex=0;
	U32 pnPapFirst,cpnBtePap;
	U32 pnChpFirst,cpnBteChp;
	U32 pnLvcFirst,cpnBteLvc;
	U32 fcPlcfbtePapx,lcbPlcfbtePapx;
	U32 fcPlcfbteChpx,lcbPlcfbteChpx;
	U32 *plcfbtePapx;
	U32 *plcfbteChpx;
	U32 chpintervals;
	U32 intervals;
	U32 fcClx,lcbClx;
	U32 fcPlcffldMom,lcbPlcffldMom;
	U32 fcPlcffldHdr,lcbPlcffldHdr;
	U32 fcPlcffldFtn,lcbPlcffldFtn;
	U32 fcSttbfffn,lcbSttbfffn;
	ffn fontnamelist;
	ffn *tempnames;
	ffn *freenames;
	obj_by_spid *freeme=NULL;
	list_def *free_def,*free_def2;
	U32 *cp_plcfld;
	U8 *fld_plcfld;
	U32 *cp_plcfld2;
	U8 *fld_plcfld2;
	U32 *cp_plcfld3;
	U8 *fld_plcfld3;
	U32 *cp_plcfld4;
	U8 *fld_plcfld4;
	U32 *cp_plcfld5;
	U8 *fld_plcfld5;
	U8 len;
	
	
	
	field_info *all_fields[5];
	field_info main_fields;
	field_info header_fields;
	field_info footnote_fields;
	field_info annotation_fields;
	field_info endnote_fields;
	U32 fcPlcfLst,lcbPlcfLst;
	U32 fcPlfLfo,lcbPlfLfo;
	U32 fcSttbFnm,lcbSttbFnm;
	U32 fcPlcffndRef;
	U32 fcPlcffndTxt;
	U32 lcbPlcffndRef;
	U32 lcbPlcffndTxt;
	/*begin annotations*/
	U32 fcPlcfandRef,lcbPlcfandRef;
	U32 fcPlcfandTxt,lcbPlcfandTxt;
	U32 fcPlcffldAtn,lcbPlcffldAtn;
	U32 fcGrpXstAtnOwners,lcbGrpXstAtnOwners;
	

	
	stringgroup *freegroup;
	/*end annotations*/
	/*begin endnotes*/
	U32 fcPlcfendRef,lcbPlcfendRef;
	U32 fcPlcfendTxt,lcbPlcfendTxt;
	U32 fcPlcffldEdn,lcbPlcffldEdn;
	
	
	/*end endnotes*/
	/*begin section table*/
	U32 fcPlcfsed,lcbPlcfsed;
	/*end section table*/

	
	U32 ccpHdrTxbx;
	
	
	

	U32 fcPlcspaMom,lcbPlcspaMom; 
	/*we use these to get spids, and then suck the pictures out of the the resulting tables found though fcDggInfo ??*/
	U32 fcDggInfo,lcbDggInfo;

	
	

	U32 stsh,stshlen;
	
	list_info a_list_info;
	style *masterstylesheet=NULL;

	FILE *tablefd;

	int notfinished;

	if (mainfd == NULL)
		{
		fprintf(erroroutput,"There was no document stream, this is probably not a word file at all\n");
		return(10);
		}


	if (outputfilename == NULL)
		{
		outputfilename = malloc(strlen(filename) + strlen(".html") + 1);
		if (outputfilename == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			return(-1);
			}
		sprintf(outputfilename,"%s%s",filename,".html");
		}

#if 0	
	if (strcmp(outputfilename,"-"))
		{
		outputfile = fopen(outputfilename,"w");
		if (outputfile == NULL)
			{
			fprintf(erroroutput,"couldnt open %s for writing\n",outputfilename);
			return(-1);
			}
		}
	else
#endif		
		outputfile = stdout;	

	fseek(mainfd,0,SEEK_END);
	mainend = ftell(mainfd);
	error(erroroutput,"the end of the stream is %x\n",mainend);
	fseek(mainfd,0,SEEK_SET);
	
	wIdent=read_16ubit(mainfd);
	nFib=read_16ubit(mainfd);
	nProduct=read_16ubit(mainfd);


	if (nFib >= 101)
		error(erroroutput,"written by word >= 6, exactly %x\n",nProduct);
	else
		error(erroroutput,"written by word < 6, exactly %x\n",nProduct);

	if ((nProduct >> 8) == 0xe0)
		{
		fprintf(erroroutput,"this is an unsupported word 7 doc, sorry\nthis converter is solely for word8 at the moment\ntry laola (http://wwwwbs.cs.tu-berlin.de/~schwartz/pmh/laola.html) it might get some text out of it\n");
		return(10);
		}
	else if ((nProduct >> 8) == 0xc0)
		{
		fprintf(erroroutput,"this is an unsupported (as of yet) word 6 doc, sorry\nthis converter is solely for word8 at the moment\ntry word2x (http://word2x.astra.co.uk/) for that\n");
		return(10);
		}
	else if ((nProduct >> 8) != 0x00)
		fprintf(erroroutput,"this doesnt appear to be a word 8 doc, but going ahead anyway, expect disaster!!\n");

	lid=read_16ubit(mainfd);
	pnNext=read_16ubit(mainfd);
	fields1 = getc(mainfd);
	error(erroroutput,"wIdent is %d,nFib is %d, nProduct is %d, lid is %d, pnNext? is %d\n",wIdent,nFib,nProduct,lid,pnNext);
		
	if (fields1 & 0x01)
		error(erroroutput,"mainfdot\n");
	if (fields1 & 0x02)
		error(erroroutput,"fGlsy\n");
	if (fields1 & 0x04)
		{
		error(erroroutput,"fComplex\n");
		iscomplex=1;
		error(erroroutput,"complex doc, here we go!\n");
		}
	if (fields1 & 0x08)
		error(erroroutput,"fHasPic\n");
	fields1 &= 0xF0;
	fields1 = fields1 >> 4;
	error(erroroutput,"no od saves is %d\n",fields1);
	fields2 = getc(mainfd);
	if (fields2 & 0x01)
		{
		error(erroroutput,"fEncrypted\n");
		fprintf(erroroutput,"This file is encrypted, mswordview currently cannot handle encrypted files\n");
		return(10);
		}

	if (fields2 & 0x02)
		{
		tablefd = tablefd1;
		error(erroroutput,"use 1Table stream\n");
		}
	else
		{
		tablefd = tablefd0;
		error(erroroutput,"use 0Table stream\n");
		}

	if (fields2 & 0x04)
		error(erroroutput,"fReadOnlyRecommedned\n");
	if (fields2 & 0x08)
		error(erroroutput,"fWriteReservation\n");
	if (fields2 & 0x10)
		error(erroroutput,"fEctChar\n");
	nFibBack = read_16ubit(mainfd);
	lKey = read_32ubit(mainfd);
	envr = getc(mainfd);
	error(erroroutput,"nFibback is %d, lKey is %ld\n",nFibBack,lKey);
	if (envr == 0)
		error(erroroutput,"created in winword\n");
	else if (envr == 1)
		error(erroroutput,"created in mac word\n");
	madeinmac = envr;
	reserved = getc(mainfd);
	chse = read_16ubit(mainfd);
	if (chse == 0)
		error(erroroutput,"windows ansi\n");
	else if (chse == 256)
		error(erroroutput,"mac char set\n");
	chseTables = read_16ubit(mainfd);
	if (chseTables == 0)
		error(erroroutput,"windows ansi\n");
	else if (chseTables== 256)
		error(erroroutput,"mac char set\n");

	fcMin = read_32ubit(mainfd);
	fcMac = read_32ubit(mainfd);
	fseek(mainfd,0x0040,SEEK_SET);
	cbMac = read_32ubit(mainfd);
	fcSpare= read_32ubit(mainfd);
	fcSpare= read_32ubit(mainfd);


	error(erroroutput,"first char is at %ld (%x)\n",fcMin,fcMin);
	error(erroroutput,"last char is at %ld (%x)\n",fcMac,fcMac);

	portions.ccpText = read_32ubit(mainfd);
	error(erroroutput,"the main doc text is of size %ld\n",portions.ccpText);
	portions.ccpFtn = read_32ubit(mainfd);
	error(erroroutput,"the footer text is of size %ld\n",portions.ccpFtn);
	portions.ccpHdd = read_32ubit(mainfd);
	error(erroroutput,"the header text is of size %ld\n",portions.ccpHdd);
	portions.fcMin = fcMin;
	portions.fcMac = fcMac;
	ccpMcr = read_32ubit(mainfd);
	portions.ccpAtn = read_32ubit(mainfd);
	error(erroroutput,"the annotation text is size %ld\n",portions.ccpAtn);
	portions.ccpEdn = read_32ubit(mainfd);
	ccpTxbx = read_32ubit(mainfd);
	ccpHdrTxbx = read_32ubit(mainfd);
	ccpSpare2 = read_32ubit(mainfd);
	fcStshfOrig = read_32ubit(mainfd);
	lcbStshfOrig = read_32ubit(mainfd);
	
	/*attempt to list all paragraph bounds*/

	fseek(mainfd,112,SEEK_SET);	
	pnChpFirst = read_32ubit(mainfd);
	cpnBteChp = read_32ubit(mainfd);
	error(erroroutput,"\n page of first different char type %d\nnumber of different character types %d\n",pnChpFirst,cpnBteChp);


	fseek(mainfd,124,SEEK_SET);	
	pnPapFirst = read_32ubit(mainfd);
	cpnBtePap = read_32ubit(mainfd);
	error(erroroutput,"\n page of first different para type %d\nnumber of different para types %d\n",pnPapFirst,cpnBtePap);
	
	fseek(mainfd,136,SEEK_SET);	
	pnLvcFirst = read_32ubit(mainfd);
	cpnBteLvc = read_32ubit(mainfd);
	error(erroroutput,"\n page of first different lvc type %d\nnumber of different lvc types %d\n",pnLvcFirst,cpnBteLvc);
	
	fseek(mainfd,154,SEEK_SET);	
	error(erroroutput,"the orig stsh is (%x), len (%x)\n",read_32ubit(mainfd),read_32ubit(mainfd));
	
	stsh = read_32ubit(mainfd);
	stshlen = read_32ubit(mainfd);
	error(erroroutput,"the new stsh is (%x), len (%x)\n",stsh,stshlen);

	masterstylesheet = decode_stylesheet(tablefd,stsh,stshlen);

	fcPlcffndRef=read_32ubit(mainfd);
	lcbPlcffndRef=read_32ubit(mainfd);
	fcPlcffndTxt=read_32ubit(mainfd);
	lcbPlcffndTxt=read_32ubit(mainfd);

	fcPlcfandRef=read_32ubit(mainfd);
	lcbPlcfandRef=read_32ubit(mainfd);
	fcPlcfandTxt=read_32ubit(mainfd);
	lcbPlcfandTxt=read_32ubit(mainfd);


	error(erroroutput,"footnote: table offset of frd thingies (%x) of len %d\n",fcPlcffndRef,lcbPlcffndRef);
	error(erroroutput,"there are %d footnotes\n",(lcbPlcffndRef-4)/6);

	error(erroroutput,"footnote: table offset for footnote text (%x) of len %d\n",fcPlcffndTxt,lcbPlcffndTxt);


	portions.fndRef=NULL;
	portions.fndFRD=NULL;
	portions.fndTrueFRD=NULL;
	portions.fndTxt=NULL;
	portions.list_foot_no = 0;
	portions.auto_foot=1;
	portions.last_foot=0;

	portions.fndref_no=0;
	portions.fndtxt_no=0;

	if (lcbPlcffndRef != 0)
		{
		portions.fndref_no=(lcbPlcffndRef-4)/6;
		portions.fndRef = (U32 *) malloc( (portions.fndref_no+1) * sizeof(U32));
		if (portions.fndRef == NULL)
			{
			fprintf(erroroutput,"NO MEM 1\n");
			return(-1);
			}

		portions.fndFRD = (S16 *) malloc( portions.fndref_no * sizeof(S16));
		portions.fndTrueFRD = (S16 *) malloc( portions.fndref_no * sizeof(S16));
		if ((portions.fndFRD == NULL) || (portions.fndTrueFRD == NULL))
			{
			fprintf(erroroutput,"NO MEM 2\n");
			return(-1);
			}
		fseek(tablefd,fcPlcffndRef,SEEK_SET);
		for(i=0;i<portions.fndref_no+1;i++)
			{
			portions.fndRef[i]=read_32ubit(tablefd);
			error(erroroutput,"fndRef is %x\n",portions.fndRef[i]);
			}
		for(i=0;i<portions.fndref_no;i++)
			{
			portions.fndFRD[i]=(S16)read_16ubit(tablefd);
			portions.fndTrueFRD[i]=portions.fndFRD[i];
			error(erroroutput,"fndFRD is %d\n",portions.fndFRD[i]);
			}
		}

	if (lcbPlcffndTxt!= 0)
		{
		portions.fndtxt_no=lcbPlcffndTxt/4;
		portions.fndTxt = (U32 *) malloc( portions.fndtxt_no  * sizeof(U32) );
		if (portions.fndTxt == NULL)
			{
			fprintf(erroroutput,"NO MEM 3\n");
			return(-1);
			}
		fseek(tablefd,fcPlcffndTxt,SEEK_SET);
		for (i=0;i<portions.fndtxt_no;i++)
			{
			portions.fndTxt[i]=read_32ubit(tablefd);
			error(erroroutput,"footnote->%x\n",portions.fndTxt[i]);
			}
		}

	portions.andref_no=0;
	portions.andRef=NULL;
	portions.andTxt=NULL;
	portions.the_atrd=NULL;
	portions.list_anno_no = 0;
	portions.last_anno=0;

	error(erroroutput,"annotation offset is %x, len is %d\n",fcPlcfandRef,lcbPlcfandRef);
	
	if (lcbPlcfandRef != 0)
		{
		portions.andref_no=(lcbPlcfandRef-4)/34;
		portions.andRef = (U32 *) malloc( (portions.andref_no+1)  * sizeof(U32) );
		portions.the_atrd = (ATRD*) malloc( portions.andref_no  * sizeof(ATRD) );
		if ((portions.andRef == NULL) || (portions.the_atrd == NULL))
			{
			fprintf(erroroutput,"NO MEM 3\n");
			return(-1);
			}
		fseek(tablefd,fcPlcfandRef,SEEK_SET);
		for (i=0;i<portions.andref_no+1;i++)
			{
			portions.andRef[i]=read_32ubit(tablefd);
			error(erroroutput,"annotation to be found at %x\n",portions.andRef[i]);
			}
		for (i=0;i<portions.andref_no;i++)
			{
			for (j=0;j<10;j++)
				portions.the_atrd[i].xstUsrInitl[j] = read_16ubit(tablefd);
			portions.the_atrd[i].ibst = read_16ubit(tablefd);
			portions.the_atrd[i].ak = read_16ubit(tablefd);
			portions.the_atrd[i].grfbmc = read_16ubit(tablefd);
			portions.the_atrd[i].lTagBkmk = read_32ubit(tablefd);
			}
		/*ive ignored the ANLD structure for now*/
		}

	if (lcbPlcfandTxt!= 0)
		{
		portions.andtxt_no=lcbPlcfandTxt/4;
		portions.andTxt = (U32 *) malloc( portions.andtxt_no  * sizeof(U32) );
		if (portions.andTxt == NULL)
			{
			fprintf(erroroutput,"NO MEM 3\n");
			return(-1);
			}
		fseek(tablefd,fcPlcfandTxt,SEEK_SET);
		for (i=0;i<portions.andtxt_no;i++)
			{
			portions.andTxt[i]=read_32ubit(tablefd);
			error(erroroutput,"andTxt are %x\n",portions.andTxt[i]);
			}
		}
	
	fcPlcfsed=read_32ubit(mainfd);
	lcbPlcfsed=read_32ubit(mainfd);
	error(erroroutput,"section: table offset for section table (%x) of len %d\n",fcPlcfsed,lcbPlcfsed);
	portions.section_cps=NULL;
	portions.section_fcs=NULL;
	if (lcbPlcfsed >0)
		{
		portions.section_nos = (lcbPlcfsed-4)/16;
		error(erroroutput,"there are %d sections",portions.section_nos);
		portions.section_cps = (U32 *) malloc ((portions.section_nos+1) * sizeof(U32));
		portions.section_fcs = (U32 *) malloc ((portions.section_nos) * sizeof(U32));
		if ((portions.section_cps == NULL) || (portions.section_fcs ==  NULL))
			{
			error(erroroutput,"no mem for section_cps\n");
			return(-1);
			}
		fseek(tablefd,fcPlcfsed,SEEK_SET);
		for (i=0;i< portions.section_nos+1;i++)
			{
			portions.section_cps[i]=read_32ubit(tablefd);
			error(erroroutput,"section offsets are %x\n",portions.section_cps[i]);
			}
		for (i=0;i<portions.section_nos;i++)
			{
			read_16ubit(tablefd); /*internal*/
			portions.section_fcs[i] = read_32ubit(tablefd);
			error(erroroutput,"section file offsets are %x\n",portions.section_fcs[i]);
			read_16ubit(tablefd); /*internal*/
			read_32ubit(tablefd);
			}
		}
	
	fseek(mainfd,242,SEEK_SET);	
	portions.fcPlcfhdd = read_32ubit(mainfd);
	portions.lcbPlcfhdd = read_32ubit(mainfd);
	/*these point to the header/footer information thing*/

	error(erroroutput,"header in table offset of (%x), len is %d\n",portions.fcPlcfhdd,portions.lcbPlcfhdd);
	
	fcPlcfbteChpx = read_32ubit(mainfd);
	lcbPlcfbteChpx = read_32ubit(mainfd);
	error(erroroutput,"\nlocation of char description in table stream is %x\nsize is %ld\n",fcPlcfbteChpx,lcbPlcfbteChpx);
	plcfbteChpx = (U32 *) malloc(lcbPlcfbteChpx);
	fseek(tablefd,fcPlcfbteChpx,SEEK_SET);
	for (i=0;i<lcbPlcfbteChpx/4;i++)
		plcfbteChpx[i] = read_32ubit(tablefd);
	chpintervals = ((lcbPlcfbteChpx/4)-1)/2;
	error(erroroutput,"there are %d charrun intervals ? ending at ",chpintervals);
	for (i=1;i<chpintervals+1;i++)
		error(erroroutput,"%d (%d)", plcfbteChpx[i],plcfbteChpx[i+chpintervals]);
	error(erroroutput,"\n");


	fcPlcfbtePapx = read_32ubit(mainfd);
	lcbPlcfbtePapx = read_32ubit(mainfd);
	error(erroroutput,"\nlocation of para description in table stream is %ld\nsize is %ld\n",fcPlcfbtePapx,lcbPlcfbtePapx);
	/*go to location in table stream, */
	/*i believe that this is just an array of longs(4 bytes blocks)
	 */
	plcfbtePapx = (U32 *) malloc(lcbPlcfbtePapx);
	fseek(tablefd,fcPlcfbtePapx,SEEK_SET);
	for (i=0;i<lcbPlcfbtePapx/4;i++)
		{
		plcfbtePapx[i] = read_32ubit(tablefd);
		error(erroroutput,"papx farting gives %x\n",plcfbtePapx[i]);
		}
	intervals = ((lcbPlcfbtePapx/4)-1)/2;
	error(erroroutput,"there are %d pragraph intervals ? ending at ",intervals);
	for (i=1;i<intervals+1;i++)
		error(erroroutput,"%d %x (%d)", plcfbtePapx[i], plcfbtePapx[i],plcfbtePapx[i+intervals]);
	error(erroroutput,"\n");
	fseek(mainfd,274,SEEK_SET);
	fcSttbfffn=read_32ubit(mainfd);
	lcbSttbfffn=read_32ubit(mainfd);

	if (lcbSttbfffn > 0)
		{
		tempnames = &fontnamelist;
		error(erroroutput,"have a table of font names (%x)\n",fcSttbfffn);
		fseek(tablefd,fcSttbfffn,SEEK_SET);

		j=0;
		/*seems to be a count of font names, followed by three blanks ?*/
		read_16ubit(tablefd);
		read_16ubit(tablefd);
		j=4;
		tempnames->next=NULL;
		while(j<lcbSttbfffn)
			{
			k=0;
			tempnames->name[0] = '\0';
			tempnames->next=NULL;
			len = getc(tablefd);
			error(erroroutput,"len is %d\n",len);
			i=1;
			notfinished=1;
			while(i<=len)
				{
				if ((i >= 40) && (notfinished))
					{
					tempnames->name[k] = read_16ubit(tablefd);
					i+=2;
					error(erroroutput,"font name char is %X %c %x i is %d\n",tempnames->name[k],tempnames->name[k],ftell(tablefd),i);
					if (tempnames->name[k] == 0)
						notfinished=0;
					k++;
					}
				else
					{	
					if (i==4)
						{
						tempnames->chs = getc(tablefd);
						error(erroroutput,"chs is (%x)  %d ",tempnames->chs,tempnames->chs);
						}
					else
						getc(tablefd);
					i++;
					}
				}
			if (tempnames->name[0] != '\0')
				{
				tempnames->next = (ffn *) malloc(sizeof(ffn));
				if (tempnames->next == NULL)
					{
					fprintf(erroroutput,"no memory, arrgh\n");
					return(-1);
					}
				tempnames = tempnames->next;
				tempnames->next=NULL;
				tempnames->name[0] = '\0';
				}
			j+=i;	
			}
		}

	tempnames = &fontnamelist;
	while (tempnames != NULL)
		{
		error(erroroutput,"font names are %s\n",tempnames->name);
		tempnames = tempnames->next;
		}

	/*determine field plc*/
	fcPlcffldMom=read_32ubit(mainfd);
	lcbPlcffldMom=read_32ubit(mainfd);
	error(erroroutput,"in table stream field plc is %ld, and len is %ld\n",fcPlcffldMom,lcbPlcffldMom);

	main_fields.cps = NULL;
	main_fields.flds = NULL;
	main_fields.no = -1;

	if (lcbPlcffldMom > 0)
		{
		error(erroroutput,"guessing that no of entries is %d\n",(lcbPlcffldMom-4)/6);
		cp_plcfld = (U32 *) malloc( (((lcbPlcffldMom-4)/6)+1) * sizeof(U32));
		if (cp_plcfld == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fld_plcfld = (U8 *) malloc( (((lcbPlcffldMom-4)/6)*2) * sizeof(U8));
		if ((fld_plcfld == NULL) && (lcbPlcffldMom-4 > 0))
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcffldMom,SEEK_SET);
		for(i=0;i<(((lcbPlcffldMom-4)/6)+1);i++)
			{
			cp_plcfld[i] =  read_32ubit(tablefd);
			error(erroroutput,"field cps are %x\n",cp_plcfld[i]);
			}
		for(i=0;i<(((lcbPlcffldMom-4)/6)*2);i++)
			{
			fld_plcfld[i] =  getc(tablefd);
			error(erroroutput,"field vals are %d\n",fld_plcfld[i]);
			}

		main_fields.cps = cp_plcfld;
		main_fields.flds = fld_plcfld;
		main_fields.no = (lcbPlcffldMom-4)/6;
		}

	fcPlcffldHdr=read_32ubit(mainfd);
	lcbPlcffldHdr=read_32ubit(mainfd);
	error(erroroutput,"in table stream field header plc is (%x), and len is %ld\n",fcPlcffldHdr,lcbPlcffldHdr);

	header_fields.cps = NULL;
	header_fields.flds = NULL;
	header_fields.no = -1;

	if (lcbPlcffldHdr> 0)
		{
		error(erroroutput,"guessing that no of entries is %d\n",(lcbPlcffldHdr-4)/6);
		cp_plcfld2 = (U32 *) malloc( (((lcbPlcffldHdr-4)/6)+1) * sizeof(U32));
		if (cp_plcfld2 == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fld_plcfld2 = (U8 *) malloc( (((lcbPlcffldHdr-4)/6)*2) * sizeof(U8));
		if ((fld_plcfld2 == NULL) && (lcbPlcffldHdr-4 > 0))
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcffldHdr,SEEK_SET);
		for(i=0;i<(((lcbPlcffldHdr-4)/6)+1);i++)
			{
			cp_plcfld2[i] =  read_32ubit(tablefd);
			error(erroroutput,"header field cps are %x\n",cp_plcfld2[i]);
			}
		for(i=0;i<(((lcbPlcffldHdr-4)/6)*2);i++)
			{
			fld_plcfld2[i] =  getc(tablefd);
			error(erroroutput,"field vals are %d\n",fld_plcfld2[i]);
			}

		header_fields.cps = cp_plcfld2;
		header_fields.flds = fld_plcfld2;
		header_fields.no = (lcbPlcffldHdr-4)/6;
		}

	fcPlcffldFtn=read_32ubit(mainfd);
	lcbPlcffldFtn=read_32ubit(mainfd);
	error(erroroutput,"in table stream field footnote plc is (%x), and len is %ld\n",fcPlcffldFtn,lcbPlcffldFtn);
	fcPlcffldAtn=read_32ubit(mainfd);
	lcbPlcffldAtn=read_32ubit(mainfd);
	error(erroroutput,"in table stream field annotation plc is (%x), and len is %ld\n",fcPlcffldAtn,lcbPlcffldAtn);

	footnote_fields.cps = NULL;
	footnote_fields.flds = NULL;
	footnote_fields.no = -1;

	if (lcbPlcffldFtn >0)
		{
		error(erroroutput,"guessing that no of entries is %d\n",(lcbPlcffldFtn-4)/6);
		cp_plcfld3 = (U32 *) malloc( (((lcbPlcffldFtn-4)/6)+1) * sizeof(U32));
		if (cp_plcfld3 == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fld_plcfld3 = (U8 *) malloc( (((lcbPlcffldFtn-4)/6)*2) * sizeof(U8));
		if ((fld_plcfld3 == NULL) && (lcbPlcffldFtn-4 > 0))
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcffldFtn,SEEK_SET);
		for(i=0;i<(((lcbPlcffldFtn-4)/6)+1);i++)
			{
			cp_plcfld3[i] =  read_32ubit(tablefd);
			error(erroroutput,"footnote field cps are %x\n",cp_plcfld3[i]);
			}
		for(i=0;i<(((lcbPlcffldFtn-4)/6)*2);i++)
			{
			fld_plcfld3[i] =  getc(tablefd);
			error(erroroutput,"field vals are %d\n",fld_plcfld3[i]);
			}

		footnote_fields.cps = cp_plcfld3;
		footnote_fields.flds = fld_plcfld3;
		footnote_fields.no = (lcbPlcffldFtn-4)/6;
		}

	annotation_fields.cps = NULL;
	annotation_fields.flds = NULL;
	annotation_fields.no = -1;

	if (lcbPlcffldAtn>0)
		{
		error(erroroutput,"guessing that no of entries is %d\n",(lcbPlcffldAtn-4)/6);
		cp_plcfld4 = (U32 *) malloc( (((lcbPlcffldAtn-4)/6)+1) * sizeof(U32));
		if (cp_plcfld4 == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fld_plcfld4 = (U8 *) malloc( (((lcbPlcffldAtn-4)/6)*2) * sizeof(U8));
		if ((fld_plcfld4 == NULL) && (lcbPlcffldAtn-4 > 0))
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcffldAtn,SEEK_SET);
		for(i=0;i<(((lcbPlcffldAtn-4)/6)+1);i++)
			{
			cp_plcfld4[i] =  read_32ubit(tablefd);
			error(erroroutput,"annotation field cps are %x\n",cp_plcfld4[i]);
			}
		for(i=0;i<(((lcbPlcffldAtn-4)/6)*2);i++)
			{
			fld_plcfld4[i] =  getc(tablefd);
			error(erroroutput,"field vals are %d\n",fld_plcfld4[i]);
			}

		annotation_fields.cps = cp_plcfld4;
		annotation_fields.flds = fld_plcfld4;
		annotation_fields.no = (lcbPlcffldAtn-4)/6;
		}


	decode_bookmarks(mainfd,tablefd,&portions);


	/*complex info bit*/
	fseek(mainfd,418,SEEK_SET);
	fcClx =  read_32ubit(mainfd);
	lcbClx = read_32ubit(mainfd);
	error(erroroutput,"complex bit begins at %X, and it %d long",fcClx,lcbClx);

	fseek(mainfd,442,SEEK_SET);
	fcGrpXstAtnOwners = read_32ubit(mainfd);
	lcbGrpXstAtnOwners = read_32ubit(mainfd);
	error(erroroutput,"fcGrpXstAtnOwners %x, lcbGrpXstAtnOwners %d\n",fcGrpXstAtnOwners,lcbGrpXstAtnOwners);
	portions.authors = extract_authors(tablefd,fcGrpXstAtnOwners,lcbGrpXstAtnOwners);
	decode_annotations(mainfd,tablefd,&portions);

	fseek(mainfd,474,SEEK_SET);
	fcPlcspaMom = read_32ubit(mainfd);
	lcbPlcspaMom = read_32ubit(mainfd);
	error(erroroutput,"error: pictures offset %x len %d\n",fcPlcspaMom,lcbPlcspaMom);

	portions.noofficedraw=0;
	portions.officedrawcps=NULL;
	portions.spids=NULL;
	portions.noofblipdata=0;

	if (lcbPlcspaMom!=0)
		{
		fseek(tablefd,fcPlcspaMom,SEEK_SET);
		portions.noofficedraw = (lcbPlcspaMom-4)/28;
		portions.officedrawcps = (U32 *) malloc(sizeof(U32) * (portions.noofficedraw+1));
		portions.spids=(U32 *) malloc(sizeof(U32) * portions.noofficedraw);
		if ((portions.officedrawcps == NULL) || ((portions.spids == NULL) && (portions.noofficedraw >0)) )
			{
			error(erroroutput,"no mem for spids\n");
			return(-1);
			}
		for(i=0;i<portions.noofficedraw+1;i++)
			{
			portions.officedrawcps[i] = read_32ubit(tablefd);
			error(erroroutput,"office draw cp is %x\n",portions.officedrawcps[i]);
			}
		for(i=0;i<portions.noofficedraw;i++)
			{
			portions.spids[i] = read_32ubit(tablefd);
			error(erroroutput,"spid id %x\n",portions.spids[i]);
			for (j=0;j<22;j++)
				getc(tablefd);
			}
		}

	/*begin endnote*/
	fseek(mainfd,522,SEEK_SET);
	fcPlcfendRef = read_32ubit(mainfd);
	lcbPlcfendRef = read_32ubit(mainfd);
	fcPlcfendTxt = read_32ubit(mainfd);
	lcbPlcfendTxt = read_32ubit(mainfd);

	error(erroroutput,"endnote: table offset of frd thingies (%x) of len %d\n",fcPlcfendRef,lcbPlcfendRef);
	error(erroroutput,"endnote: table offset for endnote text (%x) of len %d\n",fcPlcfendTxt,lcbPlcfendTxt);

	portions.endRef=NULL;
	portions.endFRD=NULL;
	portions.endTrueFRD=NULL;
	portions.endTxt=NULL;
	portions.list_end_no = 0;
	portions.auto_end=1;

	portions.endref_no=0;
	portions.endtxt_no=0;

	if (lcbPlcfendRef != 0)
		{
		portions.endref_no=(lcbPlcfendRef-4)/6;
		portions.endRef = (U32 *) malloc( (portions.endref_no+1) * sizeof(U32));
		if (portions.endRef == NULL)
			{
			fprintf(erroroutput,"NO MEM 1\n");
			return(-1);
			}

		portions.endFRD = (S16 *) malloc( portions.endref_no * sizeof(S16));
		portions.endTrueFRD = (S16 *) malloc( portions.endref_no * sizeof(S16));
		if ((portions.endFRD == NULL) || (portions.endTrueFRD == NULL))
			{
			fprintf(erroroutput,"NO MEM 2\n");
			return(-1);
			}
		fseek(tablefd,fcPlcfendRef,SEEK_SET);
		for(i=0;i<portions.endref_no+1;i++)
			{
			portions.endRef[i]=read_32ubit(tablefd);
			error(erroroutput,"endRef is %x\n",portions.endRef[i]);
			}
		for(i=0;i<portions.endref_no;i++)
			{
			portions.endFRD[i]=(S16)read_16ubit(tablefd);
			portions.endTrueFRD[i]=portions.endFRD[i];
			error(erroroutput,"endFRD is %d\n",portions.endFRD[i]);
			}
		}

	if (lcbPlcfendTxt!= 0)
		{
		portions.endtxt_no=lcbPlcfendTxt/4;
		portions.endTxt = (U32 *) malloc( portions.endtxt_no  * sizeof(U32) );
		if (portions.endTxt == NULL)
			{
			fprintf(erroroutput,"NO MEM 3\n");
			return(-1);
			}
		fseek(tablefd,fcPlcfendTxt,SEEK_SET);
		for (i=0;i<portions.endtxt_no;i++)
			{
			portions.endTxt[i]=read_32ubit(tablefd);
			error(erroroutput,"footnote->%x\n",portions.endTxt[i]);
			}
		}

	fcPlcffldEdn = read_32ubit(mainfd);
	lcbPlcffldEdn = read_32ubit(mainfd);


	endnote_fields.cps = NULL;
	endnote_fields.flds = NULL;
	endnote_fields.no = -1;

	if (lcbPlcffldEdn>0)
		{
		error(erroroutput,"guessing that no of entries is %d\n",(lcbPlcffldEdn-4)/6);
		cp_plcfld5 = (U32 *) malloc( (((lcbPlcffldEdn-4)/6)+1) * sizeof(U32));
		if (cp_plcfld5 == NULL)
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fld_plcfld5 = (U8 *) malloc( (((lcbPlcffldEdn-4)/6)*2) * sizeof(U8));
		if ((fld_plcfld5 == NULL) && (lcbPlcffldEdn-4 > 0))
			{
			fprintf(erroroutput,"no mem\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcffldEdn,SEEK_SET);
		for(i=0;i<(((lcbPlcffldEdn-4)/6)+1);i++)
			{
			cp_plcfld5[i] =  read_32ubit(tablefd);
			error(erroroutput,"endnote field cps are %x\n",cp_plcfld5[i]);
			}
		for(i=0;i<(((lcbPlcffldEdn-4)/6)*2);i++)
			{
			fld_plcfld5[i] =  getc(tablefd);
			error(erroroutput,"field vals are %d\n",fld_plcfld5[i]);
			}

		endnote_fields.cps = cp_plcfld5;
		endnote_fields.flds = fld_plcfld5;
		endnote_fields.no = (lcbPlcffldEdn-4)/6;
		}

	/*end endnote*/

	all_fields[0] = &main_fields;
	all_fields[1] = &header_fields;
	all_fields[2] = &footnote_fields;
	all_fields[3] = &annotation_fields;
	all_fields[4] = &endnote_fields;
	

	fseek(mainfd,554,SEEK_SET);
	fcDggInfo = read_32ubit(mainfd);
	lcbDggInfo = read_32ubit(mainfd);
	error(erroroutput,"fcDggInfo is %x and len is %d\n",fcDggInfo,lcbDggInfo);

	portions.ablipdata = get_blips(fcDggInfo,lcbDggInfo,tablefd,mainfd,&(portions.noofblipdata),0x08,NULL);

	fseek(mainfd,730,SEEK_SET);
	fcSttbFnm =  read_32ubit(mainfd);
	lcbSttbFnm = read_32ubit(mainfd);
	fcPlcfLst =  read_32ubit(mainfd);
	lcbPlcfLst = read_32ubit(mainfd);
	fcPlfLfo =  read_32ubit(mainfd);
	lcbPlfLfo = read_32ubit(mainfd);

	get_table_info(tablefd,&a_list_info,fcSttbFnm,lcbSttbFnm,fcPlcfLst,lcbPlcfLst,fcPlfLfo,lcbPlfLfo,masterstylesheet);

	error(erroroutput,"\n-----text------ %x\n",1024);
	if (core)
		spewString("<awml>\n");
	fseek(mainfd,1024,SEEK_SET);

	error(erroroutput,"endpoint for standard is %ld\n",fcMac);


	fseek(tablefd,portions.fcPlcfhdd,SEEK_SET);
	portions.headercpno = portions.lcbPlcfhdd/4;
	portions.headercplist = NULL;
	if (portions.headercpno > 0)
		{
		error(erroroutput,"head no is %d\n",portions.headercpno);
		portions.headercplist = malloc(sizeof(U32) * portions.headercpno);
		if (portions.headercplist == NULL)
			{
			error(erroroutput,"feck no mem\n");
			return(-1);
			}

		for (i=0;i<portions.headercpno;i++)
			{
			portions.headercplist[i]= read_32ubit(tablefd);
			error(erroroutput,"header fc ? is %x -> %x  %d",portions.headercplist[i],portions.fcMin+portions.ccpText+portions.ccpFtn+portions.headercplist[i],i);
			switch (i)
				{
				case 6:
					error(erroroutput,"even header\n");
					break;
				case 7:
					error(erroroutput,"odd header\n");
					break;
				case 8:
					error(erroroutput,"even footer\n");
					break;
				case 9:
					error(erroroutput,"odd footer\n");
					break;
				case 10:
					error(erroroutput,"page 1 special header\n");
					break;
				case 11:
					error(erroroutput,"page 1 special footer\n");
					break;
				case 12:
					error(erroroutput,"even header, section 2?\n");
					break;
				case 13:
					error(erroroutput,"odd header, section 2?\n");
					break;
				default:
					error(erroroutput,"\n");
					break;
				}
			}
		}

	if (iscomplex)
		{
		error(erroroutput,"complex writing complex table looking in %ld (len %ld)\n",fcClx,lcbClx);
		/*i believe that the intervals and plcfbtePapx are valid entities for
		fastsaved docs as well (i bloody hope so)*/
/*		spewString("\n<!-- complex document -->\n"); */
		error(erroroutput,"main ends at %x\n",portions.ccpText);
		decode_clx(0,0,portions.ccpText,tablefd,mainfd,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,&a_list_info,masterstylesheet,&portions,&fontnamelist,0);
		}
	else
		{
		error(erroroutput,"decoding simple\n");
/*		spewString("\n<!-- noncomplex document -->\n"); */

			
		decode_simple(mainfd,tablefd,data,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,&a_list_info,masterstylesheet,&portions,&fontnamelist,0);
		}


/* sterwill: looks like annotations and comments and things we can worry about later */
#if 0
	if ( (list_author_key) && (key_atrd!=NULL) )
		{
		spewString("\n<p>");
		spewString("<table border=1>\n");
		spewString("<tr><td colspan=2><b>Annotation Author Key</b></td></tr>");
		spewString("<tr><td>Initials</td><td>Full Name</td></tr>");

		for (i=0;i<portions.authors->noofstrings;i++)
			{
			spewString("<tr><td>");

			j = key_atrd[i].xstUsrInitl[0];
  			for (j=1;j<key_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    spewString("%c",key_atrd[i].xstUsrInitl[j]);
                    }
			j = 0;
			spewString("</td><td>");
			freegroup = portions.authors;
			while ((j<key_atrd[i].ibst) && (freegroup != NULL))
				{
				freegroup = freegroup->next;
				j++;
				}
			k=0;	

			if (freegroup->author != NULL)
				{
				while(freegroup->author[k] != '\0')
						{
						/*warning despite the possibility of being 16 bit nos ive done this*/
						spewString("%c",freegroup->author[k]);
						k++;
						}
				}
			spewString("</td></tr>");
			}
		spewString("</table>\n");
		}
#endif

	if (main_fields.cps != NULL)
		free(main_fields.cps);
	if (main_fields.flds != NULL)
		free(main_fields.flds);
	if (header_fields.cps != NULL)
		free(header_fields.cps);
	if (header_fields.flds != NULL)
		free(header_fields.flds);
	if (footnote_fields.cps != NULL)
		free(footnote_fields.cps);
	if (footnote_fields.flds != NULL)
		free(footnote_fields.flds);
	if (annotation_fields.cps != NULL)
		free(annotation_fields.cps);
	if (annotation_fields.flds != NULL)
		free(annotation_fields.flds);
	if (endnote_fields.cps != NULL)
		free(endnote_fields.cps);
	if (endnote_fields.flds != NULL)
		free(endnote_fields.flds);




	if (a_list_info.array != NULL)
		free(a_list_info.array);
	if (a_list_info.o_lst_ids != NULL)
		free(a_list_info.o_lst_ids);
	if (a_list_info.level != NULL)
		free(a_list_info.level);
	if (a_list_info.lstarray != NULL)
		free(a_list_info.lstarray);
	if (a_list_info.lst_ids != NULL)
		free(a_list_info.lst_ids);

	if (a_list_info.current_index_nos != NULL)
		{
		for (i=0;i<a_list_info.nooflsts+1;i++)
			free(a_list_info.current_index_nos[i]);
		free(a_list_info.current_index_nos);
		}

	if (a_list_info.o_list_def != NULL)
		{
		for (i=0;i<a_list_info.nooflfos;i++)
			{
			if (a_list_info.overridecount[i] > 0)
				{
				free_def = &(a_list_info.o_list_def[i]);
				if (free_def->list_string != NULL)
					free(free_def->list_string);
				free_def = free_def->sub_def_list;
				while (free_def != NULL)
					{
					free_def2 = free_def;
					free_def = free_def->sub_def_list;
					if (free_def2->list_string != NULL)
						free(free_def2->list_string);
					free(free_def2);
					}
				}
			}
		free(a_list_info.o_list_def);
		}       

	if (a_list_info.overridecount != NULL)
		free(a_list_info.overridecount);

	if (a_list_info.a_list_def != NULL)
		{
		for (i=0;i<a_list_info.nooflsts;i++)
			{
			free_def = &(a_list_info.a_list_def[i]);
			if (free_def->list_string != NULL)
				free(free_def->list_string);
			free_def = free_def->sub_def_list;
			while (free_def != NULL)
				{
				free_def2 = free_def;
				free_def = free_def->sub_def_list;
				if (free_def2->list_string != NULL)
					free(free_def2->list_string);
				free(free_def2);
				}
			}
		free(a_list_info.a_list_def);
		}

	
	if (masterstylesheet != NULL)
		free(masterstylesheet);
	
 	tempnames = &fontnamelist;
	tempnames = tempnames->next;
	while (tempnames != NULL)
		{
		freenames = tempnames;
		tempnames = tempnames->next;
		free(freenames);
		}

	free(plcfbtePapx);
	free(plcfbteChpx);

	while (portions.authors != NULL)
		{
		freegroup = portions.authors;
		portions.authors= portions.authors->next;
		if (freegroup->author!= NULL)
			free(freegroup->author);
		free(freegroup);
		}

	while (portions.ablipdata != NULL)
		{
		freeme = portions.ablipdata;
		portions.ablipdata = portions.ablipdata->next;
		free(freeme);
		}
	if (portions.endRef!=NULL)
		free(portions.endRef);
	if (portions.endFRD!=NULL)
		free(portions.endFRD);
	if (portions.endTrueFRD!=NULL)
		free(portions.endTrueFRD);
	if (portions.endTxt!=NULL)
		free(portions.endTxt);
	if (portions.fndRef != NULL)
		free(portions.fndRef);
	if (portions.fndFRD!=NULL);
		free(portions.fndFRD);
	if (portions.fndTrueFRD!=NULL);
		free(portions.fndTrueFRD);
	if (portions.fndTxt!=NULL);
		free(portions.fndTxt);

	if (portions.andTxt != NULL)
		free(portions.andTxt);
	if (portions.andRef != NULL)
		free(portions.andRef);
	if (portions.the_atrd != NULL)
		free(portions.the_atrd);
	

	if (portions.annotations.extra_bytes!=NULL)
		{
		for (i=0;i<portions.annotations.no_of_strings;i++)
			free(portions.annotations.extra_bytes[i]);
		free(portions.annotations.extra_bytes);
		}
	if (portions.a_bookmarks.bookmark_b_cps!=NULL)
		free(portions.a_bookmarks.bookmark_b_cps);
	if (portions.a_bookmarks.bookmark_b_bkfs!=NULL)
		free(portions.a_bookmarks.bookmark_b_bkfs);
	if (portions.a_bookmarks.bookmark_e_cps!=NULL)
		free(portions.a_bookmarks.bookmark_e_cps);
	if (portions.annotations.chars!=NULL)
		{
		for (i=0;i<portions.annotations.no_of_strings;i++)
			free(portions.annotations.chars[i]);
		free(portions.annotations.chars);
		}

		
	if (portions.headercplist != NULL)
		free(portions.headercplist);
	if (portions.officedrawcps != NULL)
		free(portions.officedrawcps);
	if (portions.spids != NULL)
		free(portions.spids);
	if (portions.section_cps != NULL)
		free(portions.section_cps);
	if (portions.section_fcs != NULL)
		free(portions.section_fcs);

	if (portions.l_bookmarks.bookmark_b_cps!=NULL)
		free(portions.l_bookmarks.bookmark_b_cps);
	if (portions.l_bookmarks.bookmark_b_bkfs!=NULL)
		free(portions.l_bookmarks.bookmark_b_bkfs);
	if (portions.l_bookmarks.bookmark_e_cps!=NULL)
		free(portions.l_bookmarks.bookmark_e_cps);
	if (portions.bookmarks.extra_bytes!=NULL)
		{
		for (i=0;i<portions.bookmarks.no_of_strings;i++)
			free(portions.bookmarks.extra_bytes[i]);
		free(portions.bookmarks.extra_bytes);
		}
	if (portions.bookmarks.chars!=NULL)
		{
		for (i=0;i<portions.bookmarks.no_of_strings;i++)
			free(portions.bookmarks.chars[i]);
		free(portions.bookmarks.chars);
		}
	return(0);
	}

U32 get_fc_from_cp(U32 acp,U32 *rgfc,U32 *avalrgfc,int nopieces)
	{
	int i=0;
	U32 distance;
	U32 fc2;

	/*given a cp find the piece*/
	while(i<nopieces)
        {
        if (rgfc[i+1] >= acp)
            {
			distance = acp-rgfc[i];	
			error(erroroutput,"cp distance is %d\n",distance);
			if (avalrgfc[i] & 0x40000000UL)
				{
				fc2 = avalrgfc[i];
				fc2 = fc2 & 0xbfffffffUL;
				fc2 = fc2/2;
				return(fc2+distance);
				}
			else
				return(avalrgfc[i]+(2*distance));
            }
        i++;
        }
	return(0);
	}

int decode_simple_endnote(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[4],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag)
	{
	U32 headerpos;
	U32 oldccpText;
	U32 begin=0;
	U32 len=0;
	int tempcp;
	int i;
	
	field_info *tempfields;
	chp achp;
	int nopieces;
	U32 *avalrgfc;
	U32 *rgfc;
	U16 *sprm;
	U32 clxcount=0;
	U32 fcvalue;
	pap indentpap;
	int anyendnotes=0;
	char roman[81];
	
	flushbreaks(0);
	init_pap(&indentpap);
	do_indent(&indentpap);

	if (noheaders)
		return(0);

/* sterwill: will do endnotes, footnotes later */
#if 0
	if (header == 0)
		{
		error(erroroutput,"doing endnotes page number is %d\n",pagenumber);

		for (i=0;i<portions->list_end_no;i++)
			{
			decode_endnote(&begin,&len,portions,i);

			if ((begin != -1) && (len != -1))
				{
				if (anyendnotes==0)
					spewString("\n<br><img src=\"%s/endnotebegin.gif\"><br>\n",patterndir());
				anyendnotes=1;
				init_chp(&achp);
				decode_s_chp(&achp,fontnamelist);
				error(erroroutput,"beginning endnote\n");
				spewString("<p>");
				/*test*/
				spewString("<font color=#330099>");
				inafont=1;
				inacolor=1;
				strcpy(incolor,"#330099");
				/*end test*/
				if (!insuper)
					{
					spewString("<sup>");
					insuper=2;
					}
				spewString("<a name=\"end%d\">",i);

				if (portions->endFRD[i] > 0)
					{
					error(erroroutput,"FRD is %d, end is %d, i is %d\n",portions->endTrueFRD[i],portions->auto_end,i);
					spewString("%s",ms_strlower(decimalToRoman(portions->endTrueFRD[i],roman)));
					spewString("</A>");
					if (insuper==2)
						{
						spewString("</sup>");
						insuper=0;
						}
					}
				else
					footnotehack=1;
					
				/*dont handling custom footnotes this way*/

				headerpos = ftell(mainfd);
				oldccpText = portions->ccpText;
				header++;

				fseek(tablefd,fcClx,SEEK_SET);
				getc(tablefd);
				nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);
				
				fcvalue = get_fc_from_cp(oldccpText+portions->ccpFtn+portions->ccpHdd+portions->ccpAtn+begin,rgfc,avalrgfc,nopieces);

				free(rgfc);
				free(avalrgfc);
				free(sprm);
				error(erroroutput,"footer fcvalue is %x, old one would be %x\n",fcvalue,portions->fcMin+oldccpText+begin);
				tempcp=cp;
				portions->ccpText = len;
				cp=begin;
				tempfields = all_fields[0];
				all_fields[0] = all_fields[4];
				realcp = oldccpText+portions->ccpFtn+portions->ccpHdd+portions->ccpAtn;
				inaheaderfooter=1;
				/*temp line*/
				fseek(mainfd,fcvalue,SEEK_SET);
				decode_simple(mainfd,tablefd,data,fcClx,fcvalue,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,2);
				inaheaderfooter=0;
				if (footnotehack == 1)
					{
					spewString("</A>");
					footnotehack=0;
					}
				all_fields[0] = tempfields;
				cp=tempcp;
				realcp = tempcp;
				fseek(mainfd,headerpos,SEEK_SET);
				portions->ccpText = oldccpText;
				header--;
				error(erroroutput,"ending endnote\n");
				}
			}
		
		portions->list_end_no=0; /*ready for next section endnotes*/
		portions->auto_end=1;

		
		
		init_chp(&achp);
		decode_s_chp(&achp,fontnamelist);
		}
#endif
	return(anyendnotes);
	}


int decode_simple_footer(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[4],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag)
	{
	U32 headerpos;
	U32 oldccpText;
	U32 begin=0;
	U32 len=0;
	int tempcp;
	int i,j;
	int footnoteflag=0;
	int annotationflag=0;
	field_info *tempfields;
	chp achp;
	int nopieces;
	U32 *avalrgfc;
	U32 *rgfc;
	U16 *sprm;
	U32 clxcount=0;
	U32 fcvalue;
	pap indentpap;
	int ret=0;

	flushbreaks(0);
	init_pap(&indentpap);
	do_indent(&indentpap);

	if (noheaders)
		return(ret);

/* sterwill: more footers, etc. can do later */
#if 0
	if (header == 0)
		{
		error(erroroutput,"doing footer page number is %d\n",pagenumber);

		for (i=portions->last_foot;i<portions->list_foot_no+portions->last_foot;i++)
			{
			decode_footnote(&begin,&len,portions,i);

			if ((begin != -1) && (len != -1))
				{
				init_chp(&achp);
				decode_s_chp(&achp,fontnamelist);
				if (footnoteflag == 0)
					{
					spewString("\n<br><img src=\"%s/footnotebegin.gif\"><br>\n",patterndir());
					footnoteflag=1;
					}
				error(erroroutput,"beginning footnote\n");
				spewString("<p>");
				/*test*/
				spewString("<font color=#330099>");
				inafont=1;
				inacolor=1;
				strcpy(incolor,"#330099");
				/*end test*/
				if (!insuper)
					{
					spewString("<sup>");
					insuper=2;
					}
				spewString("<a name=\"foot%d\">",i);

				if (portions->fndFRD[i] > 0)
					{
					spewString("%d",portions->fndFRD[i]);
					spewString("</A>");
					if (insuper==2)
						{
						spewString("</sup>");
						insuper=0;
						}
					}
				else
					footnotehack=1;
				/*dont handling custom footnotes this way*/

				headerpos = ftell(mainfd);
				oldccpText = portions->ccpText;
				header++;

				fseek(tablefd,fcClx,SEEK_SET);
				getc(tablefd);
				nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);
				
				fcvalue = get_fc_from_cp(oldccpText+begin,rgfc,avalrgfc,nopieces);

				free(rgfc);
				free(avalrgfc);
				free(sprm);
				/*			
				fseek(mainfd,portions->fcMin+oldccpText+begin,SEEK_SET);
				*/
				error(erroroutput,"footer fcvalue is %x, old one would be %x\n",fcvalue,portions->fcMin+oldccpText+begin);
				tempcp=cp;
				portions->ccpText = len;
				cp=begin;
				tempfields = all_fields[0];
				all_fields[0] = all_fields[2];
				realcp = oldccpText+begin;
				inaheaderfooter=1;
				/*temp line*/
				fseek(mainfd,fcvalue,SEEK_SET);
				decode_simple(mainfd,tablefd,data,fcClx,fcvalue,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,2);
				inaheaderfooter=0;
				if (footnotehack == 1)
					{
					spewString("</A>");
					footnotehack=0;
					}
				all_fields[0] = tempfields;
				cp=tempcp;
				realcp = tempcp;
				fseek(mainfd,headerpos,SEEK_SET);
				portions->ccpText = oldccpText;
				header--;
				error(erroroutput,"ending footnote\n");
				}
			}

		portions->last_foot+=portions->list_foot_no;
		portions->list_foot_no=0; /*ready for next page footnotes*/
		portions->auto_foot=1;
		if (footnoteflag != 0)
			{
			spewString("\n<br><img src=\"%s/footnoteend.gif\"><br>\n",patterndir());
			}
			
		for (i=portions->last_anno;i<portions->list_anno_no+portions->last_anno;i++)
			{
			decode_footanno(&begin,&len,portions,i);
			
			init_chp(&achp);
			decode_s_chp(&achp,fontnamelist);

			if ((begin != -1) && (len != -1))
				{
				if (annotationflag == 0)
					{
					spewString("\n<br><img src=\"%s/commentbegin.gif\"><br>\n",patterndir());
					annotationflag=1;
					}
				error(erroroutput,"beginning annotation\n");
				/*test*/
				spewString("<font color=#ff7777>");
				inafont=1;
				inacolor=1;
				strcpy(incolor,"#ff7777");
				/*end test*/
				if (!insuper)
					spewString("<sup>");
				/*
				spewString("<a name=\"anno%d\">",i);
				*/
				spewString("<a name=\"");
				j = portions->the_atrd[i].xstUsrInitl[0];
				for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    spewString("%c",portions->the_atrd[i].xstUsrInitl[j]);
					}
				spewString("%d",i);
				spewString("\">");
				list_author_key=1;
				j = portions->the_atrd[i].xstUsrInitl[0];
				for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    spewString("%c",portions->the_atrd[i].xstUsrInitl[j]);
					}
				spewString("%d",i+1);
				/*
				spewString("anno%d",i);
				*/
				spewString("</a>");
				if (!insuper)
					spewString("</sup>");

				headerpos = ftell(mainfd);
				oldccpText = portions->ccpText;
				header++;

				fseek(tablefd,fcClx,SEEK_SET);
				getc(tablefd);
				nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);
				
				fcvalue = get_fc_from_cp(oldccpText+portions->ccpFtn+portions->ccpHdd+begin,rgfc,avalrgfc,nopieces);

				free(rgfc);
				free(avalrgfc);
				free(sprm);
				fseek(mainfd,fcvalue,SEEK_SET);
				tempcp=cp;
				portions->ccpText = len;
				cp=begin;
				tempfields = all_fields[0];
				all_fields[0] = all_fields[3];
				realcp = oldccpText+portions->ccpFtn+portions->ccpHdd+begin;
				decode_simple(mainfd,tablefd,data,fcClx,fcvalue,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,3);
				all_fields[0] = tempfields;
				cp=tempcp;
				realcp = tempcp;
				fseek(mainfd,headerpos,SEEK_SET);
				portions->ccpText = oldccpText;
				header--;
				error(erroroutput,"ending annotation\n");
				}
			}

		if (annotationflag)
			spewString("\n<br><img src=\"%s/commentend.gif\"><br>\n",patterndir());
			
		portions->last_anno+=portions->list_anno_no;
		portions->list_anno_no=0; /*ready for next page footnotes*/
		
		decode_footer(&begin,&len,portions,asep);
		if ((begin != -1) && (len != -1))
			{
			init_chp(&achp);
			decode_s_chp(&achp,fontnamelist);
			headerpos = ftell(mainfd);
			oldccpText = portions->ccpText;
			header++;


			fseek(tablefd,fcClx,SEEK_SET);
			getc(tablefd);
			nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);

			
			fcvalue = get_fc_from_cp(portions->ccpFtn+oldccpText+begin,rgfc,avalrgfc,nopieces);

			free(rgfc);
			free(avalrgfc);
			free(sprm);
			error(erroroutput,"standard footer, fcvalue is %x, old method %d\n",fcvalue,portions->fcMin+portions->ccpFtn+oldccpText+begin);
			
		/*	
			fseek(mainfd,portions->fcMin+portions->ccpFtn+oldccpText+begin,SEEK_SET);
		*/
	
			fseek(mainfd,fcvalue,SEEK_SET);
	
			tempcp=cp;
			portions->ccpText = len;
			cp=begin;
			tempfields=all_fields[0];
			all_fields[0] = all_fields[1];
			realcp = oldccpText+portions->ccpFtn+begin;
			
			decode_simple(mainfd,tablefd,data,fcClx,fcvalue,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,1);
			
			all_fields[0] = tempfields;
			cp=tempcp;
			realcp = tempcp;
			fseek(mainfd,headerpos,SEEK_SET);
			portions->ccpText = oldccpText;
			header--;
			error(erroroutput,"ending footer\n");
			}

		ret = 1;
		pagenumber++;
		sectionpagenumber++;
		init_chp(&achp);
		decode_s_chp(&achp,fontnamelist);
		}
#endif
	return(ret);
	}

void decode_simple_header(FILE *mainfd,FILE *tablefd,FILE *data,sep *asep,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag)
	{
	U32 headerpos;
	U32 oldccpText;
	U32 begin=0;
	U32 len=0;
	int tempcp;
	int temprealcp;
	field_info *tempfields;
	int i;
	int nopieces;
	U32 *avalrgfc=NULL;
	U32 *rgfc=NULL;
	U16 *sprm=NULL;
	U32 fcvalue;
	U32 clxcount=0;

	if (noheaders)
		return;

/* sterwill: will do headers later */
#if 0
	
	if (header == 0)
		{
		error(erroroutput,"doing header\n");
		for (i=0;i<1;i++)
			{
			if (i==0)
				decode_header(&begin,&len,portions,asep);
#if 0
			/*
			im ignoring this for now, i think this is
			a header textbox, so ill hold off on this 
			until i get it right
			*/
			else
				decode_header2(&begin,&len,portions);
#endif
				
			if ((begin != -1) && (len != -1))
				{
				
				fseek(tablefd,fcClx,SEEK_SET);
				getc(tablefd);
				nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);
				

				headerpos = ftell(mainfd);
				oldccpText = portions->ccpText;
				header++;

				fcvalue = get_fc_from_cp(portions->ccpFtn+oldccpText+begin,rgfc,avalrgfc,nopieces);
				
				fseek(mainfd,fcvalue,SEEK_SET);

				free(avalrgfc);
				free(rgfc);
				free(sprm);

				tempcp=cp;
				portions->ccpText = len;
				cp=begin;
				tempfields = all_fields[0];
				all_fields[0] = all_fields[1];
				error(erroroutput,"overall header begin is %x\n",portions->fcMin+oldccpText+portions->ccpFtn+begin);
				error(erroroutput,"fcvalue is %x\n, ordinary seek is %x",fcvalue,portions->fcMin+portions->ccpFtn+oldccpText+begin);
				temprealcp = realcp;
				realcp = oldccpText+portions->ccpFtn+begin;
				inaheaderfooter=1;
				decode_simple(mainfd,tablefd,data,fcClx,fcvalue,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,1);
				inaheaderfooter=0;
				all_fields[0] = tempfields;
				cp=tempcp;
				realcp = temprealcp;
				error(erroroutput,"after header realcp is %x\n",realcp);
				fseek(mainfd,headerpos,SEEK_SET);
				portions->ccpText = oldccpText;
				header--;
				error(erroroutput,"ending header\n");
				}
			}

		}
#endif	
	}

void get_next_f_ref(textportions *portions,signed long *nextfootnote)
	{
	int i;
	if (portions->fndref_no > 0)
		{
		/*then there might be footnotes to look out for*/
		for(i=0;i<portions->fndref_no;i++)
			{
			error(erroroutput,"footnote blah %d, %ld, %ld",portions->fndFRD[i],*nextfootnote,portions->fndRef[i]);
			if ( (portions->fndFRD[i] <= 0) && (*nextfootnote < portions->fndRef[i]) )
				{
				/*not automatically numbered, so we have to look out for it manually*/
				*nextfootnote = portions->fndRef[i];
				error(erroroutput,"the next footnote is at cp %x\n",*nextfootnote);
				break;
				}
			}
		
		}
	}

void get_next_e_ref(textportions *portions,signed long *nextendnote)
	{
	int i;
	if (portions->endref_no > 0)
		{
		/*then there might be endnotes to look out for*/
		for(i=0;i<portions->endref_no;i++)
			{
			error(erroroutput,"endnote blah %d, %ld, %ld",portions->endFRD[i],*nextendnote,portions->endRef[i]);
			if ( (*nextendnote) < portions->endRef[i] )
				{
				/*not automatically numbered, so we have to look out for it manually*/
				*nextendnote = portions->endRef[i];
				error(erroroutput,"the next endnote is at cp %x\n",*nextendnote);
				break;
				}
			}
		
		}
	}

void decode_simple(FILE *mainfd,FILE *tablefd,FILE *data,U32 fcClx,U32 fcMin,U32 fcMac,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int flag)
	{
	
	U32 nextfc=0;
	U32 tapfc1=0;
	
	U32 nextchpfc=0;
	U32 lastfc=0;
	U32 lastchpfc=0;
	U32 internalcp;
	U32 limitcp;
	pap *apap;
	chp *achp;
	sep *asep=NULL;
	sep *tempsep=NULL;
	pap *tappap= NULL;
	pap *temppap = NULL;
	chp *tempchp = NULL;
	int i,j;
	int letter;
	int tempfck;
	int newpage=1;
	U32 thisfc;
	U32 sepxfc;
	

	U32 *rgfc=NULL;
	U32 *avalrgfc=NULL;
	
	U16 *sprm=NULL;
	U32 clxcount=0;
	int nopieces;
	signed long nextfootnote=0;
	signed long nextendnote=0;
	U32 nextbookmark_b=realcp; /*was 0*/
	U32 nextbookmark_e=realcp; /*was 0*/

	U32 untilfc=0;

	int flag_8_16=0;

	int d_count=0;

	U32 nextpiece=0;

	int issection=1;

	U32 sepcp;

	

	int charsetflag=0;


	apap = NULL;
	achp = NULL;
	nextfc=fcMin;
	nextchpfc=fcMin;


	error(erroroutput,"all text begins %ld %x ends at %ld %x, main text ends at %ld %x\n",fcMin,fcMin,fcMac,fcMac,portions->ccpText+fcMin,portions->ccpText+fcMin);


	fseek(tablefd,fcClx,SEEK_SET);
	getc(tablefd);
		
	error(erroroutput,"begin simple fcClx is (%x)",fcClx);
	/*
	hmm, simple files can be start off in 8 bit and go to 16
	bit and vice versa, is so the piecetable can be used to determine these
	limits
	*/
	nopieces = get_piecetable(tablefd,&rgfc,&avalrgfc,&sprm,&clxcount);
	if (nopieces > 1)
		error(erroroutput,"ah shit!, buggering microsoft over-complex toolkits!!!!, theres NO call for this dumb *piece* technology anyway\n");

	/*
	right so this is to guess (over safely) whether to go into unicode mode or not for
	the full html document, if i get it wrong then well ill get away with it wih utf-8 encoding
	coz i believe as ms only uses 8 bit for west european languages as far as i can tell
	*/
	if (header==0)
		{
		i=0;
		while(i<nopieces)
			{
			if (avalrgfc[i] & 0x40000000UL)
				{
				/*
				flag_8_16=0;
				untilfc=fcMin+portions->ccpText;
				*/
				error(erroroutput,"hokay, simple text is ascii 8 bit stuff\n");
				}
			else
				{
				/*
				flag_8_16=1;
				untilfc=fcMin+portions->ccpText+portions->ccpText;
				*/
				error(erroroutput,"hokay, simple text is unicode 16 bit stuff\n");
				charsetflag=1;
				/*
				if (header == 0)
					{
					spewString("\n<head>\n<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\">\n</head>\n");
					break;
					}
				*/
				}
			i++;
			}
	
/* sterwill: charset stuff, etc. */
#if 0
		if (charsetflag == 0)
			spewString("\n<head>\n<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">\n</head>\n");
		else
			spewString("\n<head>\n<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\">\n</head>\n");
#endif
		
		spewString("<section>\n<p>");  /* we need at least one paragraph */
		}

	/*now we have a problem as some simple (hah!!!) docs can go from 16 bit text to 
	8 bit and vice versa depending on the piece table, so keep an eye on that
	so as to know how to twiddle the flags*/
	/*find the smallest piece greater than or equal to the realcp*/
	i=0;

	while(i<nopieces)
		{
		error(erroroutput,"realcp is %x, rgfc[i+1] is %x\n",realcp,rgfc[i+1]);
		if (rgfc[i+1] > realcp)
			{
			nextpiece = rgfc[i+1];
			if (avalrgfc[i] & 0x40000000UL)
				{
				flag_8_16=0;
				untilfc=fcMin+portions->ccpText;
				}
			else
				{
				flag_8_16=1;
				untilfc=fcMin+portions->ccpText+portions->ccpText;
				}
			break;
			}
		i++;
		}


	error(erroroutput,"end simple fcClx is (%x)",fcClx);

	error(erroroutput,"end fc is %x\n",fcMin+portions->ccpText);

	if  ( (flag < 2) && (flag >0) )
		{
		untilfc=-1;  /*max it out for header/footers and rely on doubled 0x0d's*/
		limitcp=-1;
		}
	else
		limitcp=portions->ccpText;

	/*portions->cppText gives the amount of chars to handle*/
	i=fcMin;
	internalcp=0;


	error(erroroutput,"fcMin is %x,untilfc is %x, it would be %x, limitcp is %x, nextpiece is %x\n",fcMin,untilfc,untilfc-fcMin,limitcp,nextpiece);
	while(internalcp < limitcp)
		{
		error(erroroutput,"real cp is %x nextpiece is %x\n",realcp,nextpiece);
		if (nextpiece == realcp)
			{
			error(erroroutput,"have to get next piece for simple text mode\n");
			/*
			here we should actually do a seek to the fc, and see if this
			nextpiece thing is ok
			*/
			j=0;

			while(j<nopieces)
				{
				if (rgfc[j+1] > realcp)
					{
					nextpiece = rgfc[j+1];
					if (avalrgfc[j] & 0x40000000UL)
						flag_8_16=0;
						
					else
						flag_8_16=1;
						
					if (avalrgfc[j] & 0x40000000UL)
						{
						error(erroroutput,"THUS this fc is %x\n",(avalrgfc[j]&0xbfffffffUL)/2);
						thisfc = (avalrgfc[j]&0xbfffffffUL)/2;
						}
					else
						{
						error(erroroutput,"THUS fc is %x\n",avalrgfc[j]);
						thisfc = avalrgfc[j];
						}
					fseek(mainfd,thisfc,SEEK_SET);
					i = thisfc;
					/*force new paragraph and chp check*/
					nextfc=i;
					error(erroroutput,"SEEK: we're at %x, flag is %d\n",ftell(mainfd),flag_8_16);
					break;
					}
				j++;
				}
			}

		/*get the sep*/
		if (flag == 0)
			if (issection)
				{
				error(erroroutput,"getting sep, %x, piece is %d\n",realcp,i);
				sepxfc = find_FC_sepx(realcp,&sepcp,portions);
				tempsep = asep;
				asep = get_sep(sepxfc,mainfd);
				if (decode_simple_endnote(mainfd,tablefd,data,asep,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,flag))
				{
					/* sterwill: later */
					/* spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir()); */
				}
				
					
				if (pagenumber!=1)
					sectionbreak(asep);

				if (asep != NULL)
					{
					if (tempsep != NULL)
						free(tempsep);
					tempsep=NULL;
					}
				else
					{
					asep = tempsep;
					tempsep=NULL;
					}
					
				issection=0;
				}
		
		/*
		go down through all the text, when we hit a new page spit out the 
		header
		*/
		error(erroroutput,"fc is %ld (%X)\n",i,i);
		if (newpage)
			{
			if (!inatable)
				{
				newpage=0;
				decode_simple_header(mainfd,tablefd,data,asep,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,flag);
				error(erroroutput,"end: flag is %d\n",flag);
				/*force new paragraph and chp check*/
				nextfc=i;
				}
			else
				deferrednewpage=1;
			}


		/*get the pap*/
		if (i == nextfc) 
			{
			lastfc = i;
			temppap = apap;
			tempfck = find_FKPno_papx(i,plcfbtePapx,intervals);
			if (tempfck != -1)
				{
				apap = get_pap(tempfck,mainfd,i,&nextfc,sheet,a_list_info);
				error(erroroutput,"next para is at %x\n",nextfc);
				}
			
			if (apap == NULL)
				{
				error(erroroutput,"found no pap, reverting to previous\n");
				apap = temppap;
				temppap=NULL;
				}
			else if (temppap != NULL)
				{
				/*last gasp of the pap*/
				end_para(temppap,apap);
				free(temppap);
				temppap=NULL;
				}
			else
				end_para(NULL,apap);

			/*if table set search for tap*/
			if (apap->fInTable)
				{
				tapfc1 = i;
				error(erroroutput,"we search for the tap here\n");
				error(erroroutput,"tappap nexts are %x, i is %x\n",tapfc1,i);
				do
					{
					if (tappap != NULL)
						{
						free(tappap);
						tappap=NULL;
						}
						
					error(erroroutput,"tappap nexts are %x\n",tapfc1);
					tempfck = find_FKPno_papx(tapfc1,plcfbtePapx,intervals);
					if (tempfck != -1)
						tappap = get_pap(tempfck,mainfd,tapfc1,&tapfc1,sheet,a_list_info);
					else
						break;
					}
				while( (tappap != NULL) && (!tappap->fTtp) );
				error(erroroutput,"finished search for the tap here\n");
				/*
				at this stage tappap has the correct row structure stored in it
				that we want apap to have
				*/
				if (tappap != NULL)
					{
					copy_tap(&(apap->ourtap),&(tappap->ourtap));
					error(erroroutput,"no of cells is %d, %d\n",apap->ourtap.cell_no,tappap->ourtap.cell_no);
					free(tappap);
					tappap=NULL;
					}
				}
			}

		/* get the chp, using the paps istd for a basis*/
		if ((i == nextchpfc) || (lastfc == i)) 
			{
			lastchpfc=i;
			error(erroroutput,"looking for chp\n");
			tempchp = achp;
			tempfck = find_FKPno_chpx(i,plcfbteChpx,chpintervals);
			if (tempfck != -1)
				achp = get_chp(tempfck,mainfd,data,i,&nextchpfc,sheet,apap->istd);

			if (achp == NULL)
				{
				error(erroroutput,"found no chp, reverting to previous\n");
				achp = tempchp;
				}
			else if (tempchp != NULL)
				{
				free(tempchp);
				tempchp=NULL;
				}

			if (achp->color[0] == '\0')
				{
				if (flag == 1)
					{
					strcpy(achp->color,"#7f5555");
					error(erroroutput,"color set\n");
					}
				else if (flag == 2)
					{
					strcpy(achp->color,"#330099");
					error(erroroutput,"color set\n");
					}
				else if (flag == 3)
					{
					strcpy(achp->color,"#ff7777");
					error(erroroutput,"color set\n");
					}
				else 
					{
					error(erroroutput,"color not set\n");
					}
				}

			error(erroroutput,"the next char lim is %d (%x)\n",nextchpfc,nextchpfc);
			
			decode_e_list(apap,achp,a_list_info);
			decode_e_chp(achp);
			}
			
		/*after we have a new pap, do the special codes*/
		if (lastfc == i)
			{
			decode_e_specials(apap,achp,a_list_info);
			decode_e_table(apap,achp,a_list_info);
			decode_s_table(apap,achp,a_list_info);
/*			decode_s_specials(apap,achp,a_list_info); */
			decode_s_chp(achp,fontnamelist);
			}

		if (lastchpfc == i) 
			{
			decode_s_chp(achp,fontnamelist);
			lastchpfc = 0;
			}


		if (flag_8_16)
			letter = read_16ubit(mainfd);
		else
			letter = getc(mainfd);


		if (lastfc == i)
			{
			if ( ((letter != 12) && (letter != 13))  )
				{
				error(erroroutput,"doing list and next letter is %c %x\n",letter,letter);
				decode_s_list(apap,achp,a_list_info,fontnamelist,DONTIGNORENUM);
				}
			else
				error(erroroutput,"a special list\n");
				
				
			lastfc = 0;
			}

		if (flag)
			{
			if (letter == 13)
				{
				/*
				headers and footers seem to continue until 2 0x0d are reached, they dont
				appear after all to go until the len derived from the header table
				like i thought originally.
				*/
				d_count++;
				if (d_count == 2)
					break;
				}
			else
				d_count=0;
			}


		if ( (cp == nextfootnote) && (flag == 0) ) 
			{
			decode_f_reference(portions);
			get_next_f_ref(portions,&nextfootnote);
			}

		if ( (cp == nextendnote) && (flag == 0) ) 
			{
			decode_e_reference(portions);
			get_next_e_ref(portions,&nextendnote);
			}

		
			

		while (realcp == nextbookmark_b)
			nextbookmark_b = decode_b_bookmark(&(portions->l_bookmarks),&(portions->bookmarks));

		while (realcp == nextbookmark_e)
			nextbookmark_e = decode_e_bookmark(&(portions->l_bookmarks));

		newpage = decode_letter(letter,flag_8_16,apap,achp,all_fields[0],mainfd,data,fontnamelist,a_list_info,portions,&issection);

		if (newpage)
			{
			error(erroroutput,"this one\n");
			/*if we are in a table defer the pagebreaking until after the end of the row, and then pagebreak*/

			if ((inatable)  && newpage == 2)
				{
				/*in this case we have ended a row, so we should take the opportunity to halt the table*/
/*				spewString("</table>"); */
				inatable=0;
				newpage=1;
				}
			
			if ((!inatable) && (newpage == 1))
				{
				decode_simple_footer(mainfd,tablefd,data,asep,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,flag);
				if (!issection)
					pagebreak();
					
				}
			else if (newpage == 1)
				deferrednewpage=1;
				
			}


			
		if (flag_8_16)
			i+=2;
		else
			i++;
		internalcp++;
		}

	if (apap == NULL)
		{
		apap = (pap *)malloc(sizeof(pap));
		if (apap == NULL)
			{
			fprintf(erroroutput,"no mem available!\n");
			return;
			}
		}
	init_pap(apap);
	decode_e_list(apap,achp,a_list_info);
	decode_e_specials(apap,achp,a_list_info);
	decode_e_table(apap,achp,a_list_info);

	if (!newpage)
		{
		error(erroroutput,"the other one\n");
		decode_simple_footer(mainfd,tablefd,data,asep,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,flag);
		if (decode_simple_endnote(mainfd,tablefd,data,asep,fcClx,fcMin,fcMac,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,flag))
		{
			/* spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir()); */
		}
		
		}

	if (rgfc != NULL)
		free(rgfc);	
	if (avalrgfc != NULL)
		free(avalrgfc);
	if (sprm != NULL)
		free(sprm);

	if (achp != NULL)
		free(achp);
	if (apap != NULL)
		free(apap);
	if (asep != NULL)
		free(asep);
	}


void decode_header(U32 *begin,U32 *len,textportions *portions,sep *asep)
	{
	/*i dont know how these fields are working as of yet, the
	docs seem to be a bit dodgy*/
	int odd;
	int val,tval;
	*begin = -1;
	*len= -1;


	/*is this the first page of a section*/
	if ((sectionpagenumber==asep->pgnStart) && (asep->fTitlePage))
		{
		error(erroroutput,"checking for page 1 special header\n");
		if (portions->headercpno > 10)
			if (portions->headercplist[11] > portions->headercplist[10])
				{
				*begin = portions->headercplist[10];
				*len = portions->headercplist[11]-portions->headercplist[10];
				error(erroroutput,"header begin is %x end is %x\n",portions->headercplist[11],portions->headercplist[10]);
				return;
				}
		if (asep->fTitlePage)
			return;
				
		/*crazed bit of logic here :-)*/
		if (portions->headercpno > 13)
			if (portions->headercplist[portions->headercpno-1] > (portions->headercplist[10]+3))
				return;
		}
	
	/*if nothing was found continue, and pull out ordinary header*/

	if ((sectionpagenumber/2)*2 == sectionpagenumber)
		odd=0;
	else 
		odd=1;

	*len=0;


	/*if index 8 is larger than index 7 i think thats the boundaries
	of header for all pages*/

	tval = 6+(6*(sectionno-1)); /*hairy test*/

	val = tval;
	error(erroroutput,"header val is %d, odd is %d, pagenumber is %d\n",val,odd,pagenumber);

	if (!odd)
		{
		while ((*len==0) && (val>0))
			{
			if (portions->headercpno > val)
				{
				if (portions->headercplist[val+1] > portions->headercplist[val])
					{
					*begin = portions->headercplist[val];
					*len = portions->headercplist[val+1]-portions->headercplist[val];
					error(erroroutput,"header begin is %x end is %x\n",portions->headercplist[val],portions->headercplist[val+1]);
					}
				}
			else
				error(erroroutput,"well arse!!!, ive got the wrong idea about headers\n");
			val-=6;
			}
		}

	val=tval;
	
	if ((odd) || (*len == 0))
		{
		while ((*len==0) && (val>0))
			{
			if (portions->headercpno > val+1)
				{
				error(erroroutput,"val is %d headercplist[val+2] %x headercplist[val+1] %x\n",val,portions->headercplist[val+2],portions->headercplist[val+1]);
				if (portions->headercplist[val+2] > portions->headercplist[val+1])
					{
					*begin = portions->headercplist[val+1];
					*len = portions->headercplist[val+2]-portions->headercplist[val+1];
					error(erroroutput,"this header begin is %x end is %x\n",portions->headercplist[val+1],portions->headercplist[val+2]);
					}
				}
			else
				error(erroroutput,"well arse!!!, ive got the wrong idea about headers\n");
			val-=6;
			}
		}

	}

void decode_header2(U32 *begin,U32 *len,textportions *portions)
	{

	/*
	this is just a hacked together function, i have no basis for
	this at all except thats the output that word gets, dunno at all
	about this, im not seeing something obvious somewhere, hmm
	*/

	/*i dont know how these fields are working as of yet, the
	docs seem to be a bit dodgy*/
	*begin = -1;
	*len= -1;

	/*is this the first page of a section*/
	if (sectionpagenumber==1)
		return;
	
	/*if index 8 is larger than index 7 i think thats the boundaries
	of header for all pages*/
	if (portions->headercpno > 12)
		if (portions->headercplist[13] > portions->headercplist[12])
			{
			*begin = portions->headercplist[12];
			*len = portions->headercplist[13]-portions->headercplist[12];
			error(erroroutput,"header begin is %x end is %x\n",portions->headercplist[12],portions->headercplist[13]);
			}
	else
		error(erroroutput,"well arse!!!, ive got the wrong idea about headers\n");

	}


void decode_endnote(U32 *begin,U32 *len,textportions *portions,int i)
	{
	error(erroroutput,"endnote cp begin is %x end is %x\n",portions->endTxt[i],portions->endTxt[i+1]);
	*begin = portions->endTxt[i];
	*len = portions->endTxt[i+1]-*begin;
	}

void decode_footnote(U32 *begin,U32 *len,textportions *portions,int i)
	{
	error(erroroutput,"footnote cp begin is %x end is %x\n",portions->fndTxt[i],portions->fndTxt[i+1]);
	*begin = portions->fndTxt[i];
	*len = portions->fndTxt[i+1]-*begin;
	}

void decode_footanno(U32 *begin,U32 *len,textportions *portions,int i)
	{
	error(erroroutput,"annotation cp begin is %x end is %x\n",portions->andTxt[i],portions->andTxt[i+1]);
	*begin = portions->andTxt[i];
	*len = portions->andTxt[i+1]-*begin;
	}

void decode_footer(U32 *begin,U32 *len,textportions *portions,sep *asep)
	{
	/*i dont know how these fields are working as of yet, the
	docs seem to be a bit dodgy*/
	int odd;
	int val,tval;
	*begin = -1;
	*len= -1;
	
	error(erroroutput,"pagenumber is %d,sectionpagenumber is %d, pgnStart is %d, title is %d\n",pagenumber, sectionpagenumber,asep->pgnStart,asep->fTitlePage);

	if ((sectionpagenumber==asep->pgnStart) && (asep->fTitlePage))
		{
		error(erroroutput,"checking for page 1 special footer\n");
		if (portions->headercpno > 12)
			{
			if (portions->headercplist[12] > portions->headercplist[11])
				{
				*begin = portions->headercplist[11];
				*len = portions->headercplist[12]-portions->headercplist[11];
				/*  sterwill: later */
				/* spewString("\n<hr width=\"50%%\">\n"); */
				error(erroroutput,"header begin is %x end is %x\n",portions->headercplist[12],portions->headercplist[11]);
				return;
				}
			}
		if (asep->fTitlePage)
			return;

		/*crazed bit of logic here :-)*/
		if (portions->headercpno > 14)
			if (portions->headercplist[portions->headercpno-1] > (portions->headercplist[11]+3))
				return;
		}

	/*else contunue down the wire*/
	/*
	if its an odd page look at that, return the appropiate portion,
	if that is 0 len then return the even*/
	/*if index 10 is larger than index 9 i think thats the boundaries
	of header for odd(or all) pages*/


	if ((sectionpagenumber/2)*2 == sectionpagenumber)
		odd=0;
	else 
		odd=1;

/*
	spewString("\n<hr width=\"50%%\">\n");
*/

	*len=0;

	tval = 8+ ((sectionno-1)*6);

	error(erroroutput,"odd is %d, section no is %d, tval is %d, *len is %d\n",odd,sectionno,tval,*len);

	val=tval;
	if (!odd)
		{
		while ((*len == 0) && (val > 0))
			{
			if (portions->headercpno > val)
				{
				if (portions->headercplist[val+1] > portions->headercplist[val])
					{
					*begin = portions->headercplist[val];
					*len = portions->headercplist[val+1]-portions->headercplist[val];
					/*  sterwill: later */
/*					spewString("\n<hr width=\"50%%\">\n"); */
					error(erroroutput,"even begin is %x end is %x\n",portions->headercplist[val],portions->headercplist[val+1]);
					}
				}
			else
				error(erroroutput,"well arse!!!, ive got the wrong idea about footers val %d\n",portions->headercpno);
			val-=6;
			}
		}

	val = tval;

	if ( (odd) || (*len == 0))
		{
		while ((*len == 0) && (val > 0))
			{
			if (portions->headercpno > val+1)
				{
				if (portions->headercplist[val+2] > portions->headercplist[val+1])
					{
					*begin = portions->headercplist[val+1];
					*len = portions->headercplist[val+2]-portions->headercplist[val+1];
					/* sterwill: later */
/*					spewString("\n<hr width=\"50%%\">\n"); */
					error(erroroutput,"odd begin is %x end is %x\n",portions->headercplist[val+1],portions->headercplist[val+2]);
					}
				}
			else
				error(erroroutput,"well arse!!!, ive got the wrong idea about footers val %d\n",portions->headercpno);
			val-=6;
			}
		}
		
	}


void decode_s_chp(chp *achp, ffn *fontnamelist)
	{
	ffn *tempfont;
	int i;

	/* sterwill: kludly test to get around non-NULL PROPS restriction */
	int setupprops = 0;
	int morethanoneprop = 0;
			
	error(erroroutput,"in start chp\n");
	if (chps)
		return;
	error(erroroutput,"still in start chp\n");
	error(erroroutput,"incolor is %s,font =%d, current font is %d, currentfont is %d, achpfont is %d\n", incolor,achp->fontsize,currentfontsize,currentfontcode,achp->ascii_font);
#if 0
	if (achp->fStrike)
		strcpy(achp->color,"#ed32ff");		/*i just ensure that strike through text becomes this horrible shade of pink*/
	if (achp->fDStrike)
		strcpy(achp->color,"#ff7332");		
#endif
	if ((use_fontfacequery(achp) && (achp->ascii_font != currentfontcode)) || (achp->fontsize!=currentfontsize) || (strcmp(achp->color,incolor)) /* ( (achp->color[0] != '\0') && (!inacolor))  || ( (achp->color[0] == '\0' ) && (inacolor))*/   )
		{
		error(erroroutput,"b: font =%d %s\n", achp->fontsize,achp->color);
#if 0
		if (inunderline)
			{
			spewString("</c>");
			inunderline=0;
			}
		if (initalic)
			{
			spewString("</c>");
			initalic=0;
			}
		if (inbold)
			{
			spewString("</c>");
			inbold=0;
			}
		if (inblink)
			{
				/*spewString("</BLINK>");*/
			inblink=0;
			}
		if (inafont)
			{
			inacolor=0;
			incolor[0] = '\0';
			spewString("</c>");
			currentfontsize=NORMAL;
			inafont=0;
			}
#endif
		
		if ( (achp->color[0] != '\0') || (achp->fontsize!=currentfontsize) || ((use_fontfacequery(achp) && (achp->ascii_font != currentfontcode))) && no_default_props_hack == 1)
			{
			spewString("<c");

			if ( (((achp->fontsize/2)-NORMAL/2) != 0) && (achp->fontsize!=NORMAL) ||
				 achp->color[0] != '\0' ||
				 use_fontfacequery(achp))
			{
				spewString(" PROPS=\"");
				setupprops = 1;
			}
			if ( (((achp->fontsize/2)-NORMAL/2) != 0) && (achp->fontsize!=NORMAL))
				{
				spewString("font-size:%dpt", (int) currentfontsize + (achp->fontsize/2)-NORMAL/2);
				inafont=1;
				morethanoneprop = 1;
				}

			if (achp->color[0] != '\0')
				{
					if (morethanoneprop == 1)
					{
						/* must use semicolon to seperate */
						spewString("; ");
					}
					
				spewString("color:%s", achp->color);
				inafont=1;
				inacolor=1;
				morethanoneprop = 1;
				
				}

			strcpy(incolor,achp->color);

			if (use_fontfacequery(achp))
				{
				/*do diggery pokery to get fontface*/
				tempfont = get_fontnamefromcode(fontnamelist,achp->ascii_font, &i);
				if (tempfont != NULL)
					{
						/*
						  AbiWord does fonts better than your average browser (tee hee), so go straight for the real one.
						  Also, we don't do fallback font lists, so that's also why we don't try substitutions.  :)
						*/
						if (morethanoneprop == 1)
						{
							/* must use semicolon to seperate */
							spewString("; ");
						}

						spewString("font-family:%s",tempfont->name);
						morethanoneprop = 1;
#if 0						
					if (!strcmp(tempfont->name,"Times New Roman"))
						spewString(" face=\"%s,%s\"",tempfont->name,"Times");
					else if (!strcmp(tempfont->name,"Courier New"))
						spewString(" face=\"%s,%s\"",tempfont->name,"Courier");
					else 
						spewString(" face=\"%s,%s\"",tempfont->name,"Helvetica");
#endif
					}
				inafont=1;
				}

			if (setupprops)
				spewString("\"");
			
			/*set currentfontface to this chps*/
			currentfontcode = achp->ascii_font;

			spewString(">");
			currentfontsize=achp->fontsize;
			}
		}
	else
		no_default_props_hack = 1;
		


	if ((achp->supersubscript == 1) && (!insuper))
		{
		error(erroroutput,"superscript begins\n");
		insuper=1;
		}

	if ((achp->supersubscript == 2) && (!insub))
        {
        error(erroroutput,"subscript begins\n");
        insub=1;
        }

	error(erroroutput,"bold is %d\n",achp->fBold);

	if ((achp->animation) && (inblink == 0))
		{
		inblink=1;
		}

	if ((achp->fBold) && (inbold == 0))
		{
		spewString("<c PROPS=\"font-weight:bold\">");
		inbold=1;
		}
	
	if ((achp->fItalic) && (initalic==0))
		{
		spewString("<c PROPS=\"font-style:italic\">");
		initalic=1;
		}


	if ((achp->underline) && (inunderline == 0))
		{
		spewString("<c PROPS=\"text-decoration:underline\">");
		inunderline=1;
		}

	if ((achp->fStrike) && (instrike==0))
		{
			/* sterwill: hey!  We do this! */
			/* error(erroroutput,"STRIKETHROUGH"); */
/* sterwill: TESTING			 */
/*		spewString("<c PROPS=\"text-decoration:strikethrough\">"); */
		instrike=1;
		}
	}

void end_para(pap *apap,pap *newpap)
	{
	int height = 0;
	error(erroroutput,"THISend of para\n");
		
	if (apap != NULL)
		if (apap->fInTable == 1) 
			return;

	if (apap != NULL)
		{
		height = (apap->brcBottom>>24)&0x1f;
		error(erroroutput,"-->border depth is %d\n",height);
		
		if (newpap != NULL)
			{
			if (apap->brcLeft == newpap->brcLeft)
				if (apap->brcRight == newpap->brcRight)
					if (apap->fInTable == newpap->fInTable)
						if (apap->dxaWidth == newpap->dxaWidth)
							height = (apap->brcBetween>>24)&0x1f;	
			}
		}
	
	if (height > 1)
		{
		if (flushbreaks(0))
			{
			error(erroroutput,"<!--new paragraph-->");
			do_indent(apap);
			}
		error(erroroutput,"apap height\n");
		
		cutOffFormats();
		spewString("</p>\n<p>");
		}

	if (apap != NULL)
		{
		if (apap->dyaAfter > 0)
			{
			if (flushbreaks(0))
				{
				error(erroroutput,"<!--new paragraph-->");
				do_indent(apap);
				}
			if (apap->dyaAfter/TWIRPS_PER_V_PIXEL > 1)
				{
				error(erroroutput,"apap height\n");
				cutOffFormats();
				spewString("</p>\n<p>");
				}
			}
		}



	if (newpap != NULL)
		{
		if (newpap->fInTable == 1)  
			return;

		if (newpap->dyaBefore > 0)
			{
			if (flushbreaks(0))
				{
				error(erroroutput,"<!--new paragraph-->");
				do_indent(newpap);
				}
			if (newpap->dyaAfter/TWIRPS_PER_V_PIXEL >1)
				{
				error(erroroutput,"newpap height\n");
				cutOffFormats();
				spewString("</p>\n<p>");
				/*  sterwill: later				*/
/*				spewString("\n<img width=1 height=%d src=\"%s/clear.gif\"><br>\n",newpap->dyaAfter/TWIRPS_PER_V_PIXEL,patterndir()); */
				}
			}
		}
	}

void decode_e_chp(chp *achp)
	{
	int colorflag=0;
	error(erroroutput,"in end chp\n");
	if (chps)
		return;
#if 0
	if (achp->fStrike)
		strcpy(achp->color,"#ed32ff");		/*i just ensure that strike through text becomes this horrible shade of pink*/
	if (achp->fDStrike)
		strcpy(achp->color,"#ff7332");		
#endif
	if ((achp->fStrike==0) && (instrike==1))
		{
		spewString("</c>");
		instrike=0;
		}

	if ((inunderline== 1) && (achp->underline == 0))
		{
		inunderline=0;
		spewString("</c>");
		}

	if ((initalic == 1) && (achp->fItalic == 0))
		{
		if (inunderline== 1)
			{
			inunderline=0;
			spewString("</c>");
			}
		initalic=0;
		spewString("</c>");
		}

	if ((inbold==1) && (!achp->fBold))
		{
		if (inunderline== 1)
			{
			inunderline=0;
			spewString("</c>");
			}
		if (initalic == 1)
			{
			initalic=0;
			spewString("</c>");
			}
		inbold=0;
		spewString("</c>");
		}

	if ((inblink ==1) && (!achp->animation))
		{
		if (inunderline== 1)
			{
			inunderline=0;
			spewString("</c>");
			}
		if (initalic == 1)
			{
			initalic=0;
			spewString("</c>");
			}
		if (inbold== 1)
			{
			inbold=0;
			spewString("</c>");
			}
		/*  sterwill: never?  :)		 */
		}

	if ((insuper==1) && (achp->supersubscript != 1))
		{
		insuper=0;
		if (footnotehack==1)
			{

			footnotehack=0;
			}

		}

	if ((insub) && (achp->supersubscript != 2))
		{
		insub=0;

		}


	error(erroroutput,"the color is %s, %s\n",achp->color,incolor);
/*
	if ((inacolor) && (achp->color[0] == '\0'))
		colorflag=1;
	else*/
	if (strcmp(achp->color,incolor)) 
		colorflag=1;
		/*
	else if (achp->color[0] != '\0')
		if (incolor && (0 != strcmp(achp->color,incolor)) ) 
			colorflag=1;
		*/

	error(erroroutput,"inafont is %d\n,currentfontcode is %d\n,achpfntcode is %d, color flag iss %d\n",inafont,currentfontcode,achp->ascii_font,colorflag);

	if ( ((inafont) && (achp->fontsize!=currentfontsize)) || colorflag || (currentfontcode != achp->ascii_font))
		{
		error(erroroutput,"font ended\n");
		if (inunderline== 1)
			{
			inunderline=0;
			spewString("</c>");
			}
		if (initalic == 1)
			{
			initalic=0;
			spewString("</c>");
			}
		if (inbold == 1)
			{
			inbold=0;
			spewString("</c>");
			}
		if (inblink == 1)
			{
			inblink=0;

			}
		if (inafont || inacolor)
			{
			spewString("</c>");
			incolor[0] = '\0'; 
			currentfontcode=-1;
			inafont=0;
			inacolor=0;
			}

		if (instrike)
		{
/* sterwill: TESTING			 */
/* 			spewString("</c>"); */
			instrike=0;
		}

		inafont =0;
		inacolor=0;
		currentfontsize=NORMAL;
		}
	error(erroroutput,"left end chp sucessfully\n");
	}


/*
letter logic:
	in comes letter, check the font stuff, if its some fonts it appears
	to be fSpec by default, then run through the fSpec code , then
	if it gets through that then if its a special font decode the char
	otherwise run it through the ordinary letter handling
*/

int decode_letter(int letter,int flag,pap *apap, chp * achp,field_info *magic_fields,FILE *main,FILE *data,ffn *fontnamelist,list_info *a_list_info,textportions *portions, int *issection)
	{
	int ret=0;
	int i,j;
	static U8 fieldwas=-1;
	static int fieldeater=0;
	static long int swallowcp1=-1,swallowcp2=-1;
	static int spacecount;
	static int tabstop;
	static int silent=0;
	float tabbing;
	ffn *tempfont;
	time_t timep;
	struct tm *times;
	char date[1024];
	char *fontname=NULL;
	int fSpecflag=0;
	U16 temp=0;
	obj_by_spid *tempblip;
	char *tempname;
	
	static int fieldparse=0;
	U16 *array;
	U16 *array2;
	U16 *deleteme=NULL;

	long pictureoffset;
	U32 picturelen;
	U16 datatype;
	
	U8 target[7];
	int len;

	chp tempchp;

	int temp2;

	cellempty--;

	if (insuper==2)
		insuper=1;

	switch (fieldparse)	
		{
		case 37:
			realcp++;
			cp++;
			array = decode_crosslink(letter,&swallowcp1,&swallowcp2);
			if (array != NULL)
				{
/*		sterwill: never?  :) */
/*				spewString("<a href=\"#"); */
				array2 = array;
				while (*array2 != '\0')
					{ 
					/*warning despite the possibility of being 16 bit nos ive done this*/
/*					spewString("%c",*array2); */
					array2++;
					}
/*				spewString("\">*</a>"); */
				fflush(outputfile);
				error(erroroutput,"end of the mallocing\n");
				free(array);
				fieldparse=0;
				}
			return(0);
			break;
		case 88:
			array = decode_hyperlink(letter,&swallowcp1,&swallowcp2,&deleteme);		
			realcp++;
			cp++;

			if (array == NULL)		/*if theres no letters for the rest of this guy to parse*/
				return(0);
			else	
				{
				fieldparse=0;
				array2 = array;
				if (array2[0] != '\"')
				{
					/*				spewString("\""); */
				}
				
				
				while (*array2 != '\0')
					{
					/*warning despite the possibility of being 16 bit nos ive done this*/
						/*	spewString("%c",*array2);  */
					array2++;
					}
				array2--;
				
				if (*array2 != '\"')
				{
					
					/* spewString("\""); */
				}
				

				/* spewString(">"); */

				
				if (deleteme != NULL)
					free(deleteme);
					
				return(0);
				}
			break;
		default:
			break;
		}

	if  (letter != 13 || achp->fontcode !=0 ) 
		{
		if (flushbreaks(0))
			{
			error(erroroutput,"<!--new paragraph-->");
			do_indent(apap);
			}
		}

	if (fontnamelist != NULL)
		{
		if ( (achp->fontcode !=0) && (letter > 34) ) 
			{
			error(erroroutput,"asked for nonstandard font %d\n",achp->fontcode);
#if 0
			i=0;
			tempfont = fontnamelist;
			while (i < achp->fontcode)
				{
				tempfont = tempfont->next;
				if (tempfont == NULL)
					break;
				i++;
				}
#endif
			tempfont = get_fontnamefromcode(fontnamelist,achp->fontcode,&i);
			if (tempfont == NULL)
				{
				error(erroroutput,"warning!! fontcode incorrect, abandoning\n");
				return(0);
				}

			error(erroroutput,"i is %d and font is %s\n",i,tempfont->name);
			if (i == achp->fontcode)
				{
				if (!(strcmp("Symbol",tempfont->name)))
					{
					error(erroroutput,"must use symbol font for this letter\n");
					if (decode_symbol(achp->fontspec))
						{
						cp++;
						realcp++;
						return(ret);
						}
					}
				else if (!(strcmp("Wingdings",tempfont->name)))
					{
					error(erroroutput,"must use wingding font for this letter %c\n",letter);
					if (decode_wingding(achp->fontspec))
						{
						cp++;
						realcp++;
						return(ret);
						}
					}
				}
			}
		else 
			{
#if 0
			i=0;
			tempfont = fontnamelist;
			while (i < achp->ascii_font)
				{
				tempfont = tempfont->next;
				if (tempfont == NULL)
					break;
				i++;
				}
#endif
			tempfont = get_fontnamefromcode(fontnamelist,achp->ascii_font,&i);
			error(erroroutput,"i is %d and font is %s\n",i,tempfont->name);
			if ( (i == achp->ascii_font) && (letter > 34) ) 
				{
				if (!(strcmp("Symbol",tempfont->name)))
					{
					error(erroroutput,"must use symbol font for this letter here\n");
					fontname=tempfont->name;
					if (achp->fSpec==0)
						{
						achp->fSpec=1;
						fSpecflag=1;
						}
					}
				else if (!(strcmp("Wingdings",tempfont->name)))
					{
					error(erroroutput,"must use wingding font for this letter here %d fontcode is %d\n",letter,achp->fontcode);
					fontname=tempfont->name;
					if (achp->fSpec==0)
						{
						achp->fSpec=1;
						fSpecflag=1;
						}
					}
				}

			}
		}

	error(erroroutput,"cp is %x (%x) Trealcp is %x\n",cp,letter,realcp);
	if ( ( (fieldeater == 2) && (letter != 0x20) ) || ( (fieldeater > 0) && (letter == 0x20) ))
		{
		fieldeater--;
		if (fieldeater == 0)
			silent=0;
		}
#if 0
	else if ((cp > swallowcp1) && (cp < swallowcp2))
		{
		error(erroroutput,"swallowing %c\n",letter);
		}
#endif
	else
		{
		if ((letter != 32) && (spacecount == 1))
			{
			if (achp->underline == 2)
				{
				temp = achp->underline;
				achp->underline=0;
				decode_e_chp(achp);
				decode_s_chp(achp,fontnamelist);
				}
			spewString(" ");
			if (temp == 2)
				{
				achp->underline=temp;
				decode_e_chp(achp);
				decode_s_chp(achp,fontnamelist);
				}
			spacecount=0;
			tabstop++;
			}
		else if (letter != 32) 
			{
			if (i>0)
				{
				if (achp->underline == 2)
					{
					temp = achp->underline;
					achp->underline=0;
					decode_e_chp(achp);
					decode_s_chp(achp,fontnamelist);
					}
				for (i=0;i<spacecount;i++)
					{
					if (padding < 3)
						spewString(" ");	/*  nbsp */
					tabstop++;
					}
				if (temp == 2)
					{
					achp->underline=temp;
					decode_e_chp(achp);
					decode_s_chp(achp,fontnamelist);
					}
				}
			spacecount=0;
			}


		tabstop++;

		if (achp->fSpec==1)
			{
			switch (letter)
				{
				case 0:
/* 					spewString("%d",pagenumber); */
					error(erroroutput,"pagenumber is now %d\n",pagenumber);
					break;
				case 1:
					if ((achp->fData == 0) && (achp->fOle2 == 0))
						{
						error(erroroutput,"handle picture\n");
						error(erroroutput,"IN THIS CASE data offset is %x, ole2 is %x\n",achp->fcPic,achp->fOle2);

						pictureoffset = get_picture_header(achp->fcPic,data,&picturelen,&datatype);
						error(erroroutput,"the offset is %x, the len is %d\n",pictureoffset,picturelen);
						if (datatype != 0x6400)
							error(erroroutput,"data type isnt office draw, its %d (%x) ignoring\n",datatype,datatype);
						else
							{
							if (picblips != NULL)
								error(erroroutput,"here we have %s\n",picblips->filename);
							else
								error(erroroutput,"here we have NULL\n");
							fflush(outputfile);
							picblips = get_blips(pictureoffset,picturelen,data,data,&temp2,0x01,&picblips);
							tempblip = picblips;
							if (tempblip == NULL)
								error(erroroutput,"tempblip is NULL\n");
							if (tempblip != NULL)
								{
								tempname = tempblip->filename;
								while (tempblip->next != NULL)
									{
									tempblip = tempblip->next;
									error(erroroutput,"the filename is %s\n",tempblip->filename);
									error(erroroutput,"the spid is %x\n",tempblip->spid);
									/*
									if ((tempblip->filename != NULL) && (tempblip->filename[0] != '\0'))
									*/
									tempname = tempblip->filename;
									}
								outputimgsrc(tempname);
								}
							}
						}
					break;
				case 2:
					if (inaheaderfooter!=1)
						{
						error(erroroutput,"INSERT REF\n");
						decode_f_reference(portions);
						}
					break;
				case 3:
				case 4:
					error(erroroutput,"do these matter in html mode\n");
					break;
				case 5:
					error(erroroutput,"INSERT ANNOTATION\n");
					if (doannos)
						decode_annotation(portions,main);
					break;
				case 6:
					error(erroroutput,"line no\n");
/* 					spewString("UNKNOWN LINE NO\n"); */
					break;
				case 7:	
					error(erroroutput,"what is this !, pen windowsn\n");
					break;
				case 8:	
					error(erroroutput,"office draw thing, must get spid\n");
					output_draw(cp,portions);
					break;
/* sterwill: do these soon! */
#if 0					
				case 10:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%a, %b ",times);
					error(erroroutput,"output date as %s\n",date);
					spewString("%s",date);
					spewString("%d",times->tm_mday);
					strftime(date,1024,",%Y",times);
					error(erroroutput,"output date as %s\n",date);
					spewString("%s",date);
					break;
				case 30:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%x",times);
					error(erroroutput,"output date as %s\n",date);
					spewString("%s",date);
					break;
				case 11:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%H:%M:%S",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 28:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%X",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 12:
					spewString("--%d--\n",sectionno);
					error(erroroutput,"did section no\n");
					break;
				case 14:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%a",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 15:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%A",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 16:
					timep = time(NULL);
					times = localtime(&timep);
					/*
					strftime(date,1024,"%d",times);
					error(erroroutput,"output time as %s\n",date);
					*/
					spewString("%d",times->tm_mday);
					break;
				case 22:
					timep = time(NULL);
					times = localtime(&timep);
					/*
					strftime(date,1024,"%H",times);
					error(erroroutput,"output time as %s\n",date);
					*/
					spewString("%d",times->tm_hour);
					break;
				case 23:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%H",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 24:
					timep = time(NULL);
					times = localtime(&timep);
					/*
					strftime(date,1024,"%M",times);
					error(erroroutput,"output time as %s\n",date);
					*/
					spewString("%d",times->tm_min);
					break;
				case 25:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%M",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 26:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%S",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 27:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%p",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 29:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%B ",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					spewString("%d",times->tm_mday);
					strftime(date,1024,", %Y",times);
					spewString("%s",date);
					break;
				case 33:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%m",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 34:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%Y",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 35:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%y",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 36:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%b",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 37:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%B",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 38:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%H:%M",times);
					error(erroroutput,"output time as %s\n",date);
					spewString("%s",date);
					break;
				case 39:
					timep = time(NULL);
					times = localtime(&timep);
					strftime(date,1024,"%A, %B, ",times);
					spewString("%s",date);
					spewString("%d, ",times->tm_mday);
					strftime(date,1024,"%Y",times);
					spewString("%s",date);
					break;
#endif /* all these date formats */
				case 41:
					error(erroroutput,"print merge ?\n");
					break;
				case 19:
					error(erroroutput,"\nfield begins\n");
					init_chp(&tempchp);
					chpsoff();
					decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
					error(erroroutput,"cp vals are %x and %x\n",swallowcp1,swallowcp2);		/*if theres no letters for the rest of this guy to parse*/

					switch (fieldwas)
						{
						case 37:
							if ((swallowcp1 != -1) && (swallowcp2 != -1))
								fieldparse=37;
							silent=1;
							break;
						case 88:
							if ((swallowcp1 != -1) && (swallowcp2 != -1))
								fieldparse=88;
							break;
						default:
							silent=1;
							break;
						}
					break;
				case 21:
					error(erroroutput,"\nfield ends\n");
					decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
					chpson();
					silent=0;
					fieldparse=0;
					break;
				case 20:
					error(erroroutput,"\n field separator\n");
					decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
					if ( (fieldwas == 10) || (fieldwas == 12) || (fieldwas == 68) || (fieldwas == 17) || (fieldwas == 29) || (fieldwas == 13) || (fieldwas == 88) || (fieldwas == 3))
						{
						silent=0;
						chpson();
						}
					break;
				default:
					if (fontname==NULL)
					{
						if (flag)
						{
							len = our_wctomb(target,letter);
							error(erroroutput,"letter3: %x %d",letter,letter);
							/*
							  expand this in the ranges that we will have to handle 
							  ourselves, just for my own benefit for now, i might be
							  able to build a table with some word macros and luck*/
							if (letter == 0x2122)
								decode_symbol(0xf0e4);
							else if (letter != 0)
								for(i=0;i<len;i++)
								{
									if ( (achp->fSmallCaps || achp->fCaps)  && use_fontfacequery(achp) && (len == 1) )
									{
										if  ( isupper(target[i]) && achp->fSmallCaps)
										{
											temp = achp->fontsize;
											achp->fontsize+=2;
											decode_e_chp(achp);
											decode_s_chp(achp,fontnamelist);
											achp->fontsize = temp;
										}
										target[i] = toupper(target[i]);
									}
									/* sterwill: check for ASCII range */
									if (target[i] > 0 && target[i] < 128)
										spewString("%c",target[i]);

									error(erroroutput,"letter2: %c, silent is %d",target[i],silent);
								}
							else
								error(erroroutput,"given 0 as a letter !\n");
						}
						else
						{
							if (letter == 0xae)
								decode_symbol(61666);
							else if (letter != 0)
							{
								if ( ( achp->fSmallCaps || achp->fCaps) && use_fontfacequery(achp) )
								{
									if  ( letter && achp->fSmallCaps)
									{
										temp = achp->fontsize;
										achp->fontsize+=2;
										decode_e_chp(achp);
										decode_s_chp(achp,fontnamelist);
										achp->fontsize = temp;
									}
									letter = toupper(letter);
								}
								/* sterwill: check for ASCII range */
								if (letter > 0 && letter < 128)
									spewString("%c",letter);
							}
							error(erroroutput,"letter2: %c %d %x, silent is %d",letter,letter,letter,silent);
						}
					}
					else
					{
						if (!(strcmp("Wingdings",fontname)))
							decode_wingding(letter);
						else if (!(strcmp("Symbol",fontname)))
							decode_symbol(letter);
						realcp++;
						cp++;
						if (fSpecflag)
							achp->fSpec=0;
						return(ret);
					}
					break;
				}
			}
		else
		{
			switch(letter)
			{
			case 13:
				error(erroroutput,"\n<!--paragraph end-->\n");
				cutOffFormats();
				spewString("</p>\n<p>");
				if (!silent)
				{
					breakcount++;

					if (cellempty == 0)
						cellempty++;
						
					tabstop=0;
					newline=1;
				}
				break;
			case 11:
				error(erroroutput,"\n(--line break--)\n");
				spewString("<br/>");
				tabstop=0;
				break;
			case 45:
				error(erroroutput,"-");
				spewString("-"); 	/* is this a hyphen? We don't have ligatures for that yet.*/
				break;
			case 32:
				spacecount++;
				tabstop--;
				if (cellempty == 0)
					cellempty++;
				break;
			case 31:
				error(erroroutput,"\n(-nonrequired hyphen- ?)\n");
				spewString("-");
				break;
			case 30:
				error(erroroutput,"\n(-nonbreaking hyphen- ?)\n");
				spewString("-");
				break;
			case 160:
				error(erroroutput,"\n(-non breaking space)\n");
				spewString(" ");	/* nbsp */
				break;
			case 12:
				error(erroroutput,"\npage break (maybe section) at %x\n",cp);
				for (i=0;i<portions->section_nos+1;i++)
				{
					if (cp+1 == portions->section_cps[i])
					{
						/*sectionpagenumber=1;*/
						sectionno++;
						*issection=1;
					}
				}
				ret=1;
				tabstop=0;
				break;
			case 14:
				error(erroroutput,"\ncolumn break\n");
				columnbreak();
				break;
			case 9:
				spewString("\t");
#if 0					
				tabstop--;
				error(erroroutput,"\ntab\n");
				tabbing = ((float)tabstop)/8;
				error(erroroutput,"tabbing is %f, from %d\n",tabbing,tabstop);
				error(erroroutput,"tabsize is %f\n",tabsize);
				if (tabbing == tabstop/8)
				{
					if ( (padding == 0) || (padding == 3))
						spewString("\t");
					else if ( (padding == 1) || (padding == 4))
						for(j=0;j<(int)tabsize;j++)
							spewString(" ");	/* nbsp */
					else if ( (padding == 5) || (padding == 2))
						spewString(" ");
					tabstop = tabstop=8;
				}
				else 
				{
					temp2 = (8-((tabbing - (tabstop/8))*8))+1;
					error(erroroutput,"temp2 is %d\n",temp2);
					if ( (padding == 1) || (padding == 4))
					{
						for(i=0;i< (8-((tabbing - (tabstop/8))*8))+1;i++)
							for(j=0;j<(int)(tabsize/8);j++)
								spewString(" "); /* nbsp */
					}
					else if ( (padding == 0) || (padding == 3))
						spewString("\t");
					else if ((padding == 5) || (padding == 2))
						spewString(" ");
					tabstop = tabstop+(8-(tabbing - (tabstop/8))*8);
				}
				/*spewString("<TAB>");*/
#endif					
				break;
			case 19:
				error(erroroutput,"\nfield begins\n");
				init_chp(&tempchp);
				chpsoff();
				decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
				error(erroroutput,"cp vals are %x and %x\n",swallowcp1,swallowcp2);
				switch (fieldwas)
				{
				case 37:
					if ((swallowcp1 != -1) && (swallowcp2 != -1))
						fieldparse=37;
					break;
				case 88:
					if ((swallowcp1 != -1) && (swallowcp2 != -1))
						fieldparse=88;
					break;
				default:
					silent=1;
					break;
				}
				break;
			case 21:
				error(erroroutput,"\nfield ends\n");
				decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
				chpson();
				silent=0;
				fieldparse=0;
				break;
			case 20:
				error(erroroutput,"\n field separator\n");
				decode_field(main,magic_fields,&cp,&fieldwas,&swallowcp1,&swallowcp2);
				if ( (fieldwas == 10) || (fieldwas == 12) || (fieldwas == 68) || (fieldwas == 17) || (fieldwas == 29) || (fieldwas == 13) || (fieldwas == 88) || (fieldwas == 3))
				{
					silent=0;
					chpson();
				}
				break;
			case '<':
				spewString("&lt;");
				break;
			case '>':
				spewString("&gt;");
				break;
			case 7:
				if (cellempty == 0)
					cellempty++;
				apap->tableflag=1;
				decode_e_specials(apap,achp,a_list_info);
				if (decode_e_table(apap,achp,a_list_info)==1)
				{
					if (deferrednewpage)
					{
						deferrednewpage=0;
						ret=2;
					}
				}
				error(erroroutput,"\n");
				newline=1;
				break;
			case 0x26:
				spewString("&amp;");
				break;
			case 0x96:
			case 0x2013:
				spewString("-");
				break;
			case 145:
			case 146:
				spewString("'");
				break;
			case 132:
			case 147:
				error(erroroutput,"begin quot\n");
				spewString("\"");	/* &quot; */
				break;
			case 148:
				error(erroroutput,"end quot\n");
				spewString("\"");	/* &quot; */
				break;
			case 0x85:
				if (flag)
					letter = 0x2026;
				else
				{
					fontname = "Symbol";
					letter = 0xf0bc;
					/*156.gif*/
				}
				/*deliberate fallthrough*/
			default:
				if (fontname==NULL)
				{
					if (flag)
					{
						len = our_wctomb(target,letter);
						error(erroroutput,"letter3: %x %d",letter,letter);
						/*
						  expand this in the ranges that we will have to handle 
						  ourselves, just for my own benefit for now, i might be
						  able to build a table with some word macros and luck*/

						if (letter == 0x2122)
							decode_symbol(0xf0e4);
						else if (letter != 0)
							for(i=0;i<len;i++)
							{
								if ( (achp->fSmallCaps || achp->fCaps) && use_fontfacequery(achp) && (len == 1) )
								{
									if  ( isupper(target[i]) && achp->fSmallCaps)
									{
										temp = achp->fontsize;
										achp->fontsize+=2;
										decode_e_chp(achp);
										decode_s_chp(achp,fontnamelist);
										achp->fontsize = temp;
									}
									target[i] = toupper(target[i]);
								}
								/* sterwill: check for ASCII range */
								if (target[i] > 0 && target[i] < 128)
									spewString("%c",target[i]);
								error(erroroutput,"letter2: %c, silent is %d",target[i],silent);
							}
						else
							error(erroroutput,"given 0 as a letter !\n");
					}
					else
					{
						if (letter == 0xae)
							decode_symbol(61666);
						else if (letter != 0)
						{
							if ( (achp->fSmallCaps || achp->fCaps) && (use_fontfacequery(achp)) )
							{
								if  ( isupper(letter) && achp->fSmallCaps)
								{
									temp = achp->fontsize;
									achp->fontsize+=2;
									decode_e_chp(achp);
									decode_s_chp(achp,fontnamelist);
									achp->fontsize = temp;
								}
								letter = toupper(letter);
							}
							/* sterwill: check for ASCII range */
							if (letter > 0 && letter < 128)
								spewString("%c",letter);
						}
						else
							error(erroroutput,"silly 0 found\n");
							
/*								error(erroroutput,"letter2: %c %d %x silent is %d",letter,letter,letter,silent); */
					}
				}
				else
				{
					if (!(strcmp("Wingdings",fontname)))
						decode_wingding(letter);
					else if (!(strcmp("Symbol",fontname)))
						decode_symbol(letter);
					realcp++;
					cp++;
					if (fSpecflag)
						achp->fSpec=0;
					return(ret);
				}
				break;
			}
				
		}
		}
	cp++;
	realcp++;
	return(ret);
	}


int decode_clx_endnote(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag)
	{
	U32 oldtablepos;
	U32 oldmainpos;
	U32 tempcp;
	U32 begin=0;
	U32 len=0;
	int k;
	field_info *tempfields;
	int i;
	chp achp;
	pap indentpap;
	int anyendnotes=0;
	char roman[81];

	flushbreaks(0);
	init_pap(&indentpap);
	do_indent(&indentpap);

	if (noheaders)
		return(anyendnotes);

	error(erroroutput,"list_end_no is %d\n",portions->list_end_no);

/* sterwill: do this later */
#if 0
	if (header == 0)
		{
		for (i=0;i<portions->list_end_no;i++)
			{
			decode_endnote(&begin,&len,portions,i);

			if ((begin != -1) && (len != -1))
				{
				if (anyendnotes==0)
					spewString("\n<br><img src=\"%s/endnotebegin.gif\"><br>\n",patterndir());
				anyendnotes++;
				init_chp(&achp);
                decode_s_chp(&achp,fontnamelist);


				error(erroroutput,"beginning endnote\n");
				/*test*/
				spewString("<p>");
				spewString("<font color=\"#330099\">");
				inafont=1;
				inacolor=1;
				strcpy(incolor,"#330099");
				/*end test*/
				if (!insuper)
					{
					spewString("<sup>");
					insuper=2;
					}
				spewString("<a name=\"end%d\">",i);

				if (portions->endFRD[i] > 0)
					{
					/*
					spewString("%d",portions->endFRD[i]);
					*/
					spewString("%s",ms_strlower(decimalToRoman(portions->endTrueFRD[i],roman)));
					spewString("</A>");
					if (insuper == 2)
						{
						spewString("</sup>");
						insuper=0;
						}
					}
				else
					 footnotehack=1;


				oldmainpos = ftell(main);
				oldtablepos = ftell(in);
				header++;

				k=0;
				while (k< nopieces)
					{
					if (portions->ccpText+portions->ccpFtn+portions->ccpHdd+portions->ccpAtn+begin < rgfc[k]) 
						{
						error(erroroutput,"piece for footer is %d (%x) (%x)\n",k-1,rgfc[k-1],portions->ccpText+begin);
						break;
						}
					k++;
					}
				tempcp=cp;
				cp=begin;
				error(erroroutput,"the difference is %d\n",(portions->ccpText+begin)-rgfc[k-1]);
				cp = rgfc[k-1];
				error(erroroutput,"endnote len is %d\n",len);
				error(erroroutput,"endnote cp is %x\n",cp);
				tempfields = all_fields[0];
				all_fields[0] = all_fields[4];
				begin = portions->ccpText+portions->ccpFtn+portions->ccpHdd+portions->ccpAtn+begin;
				error(erroroutput,"endnote begin cp is %d, begin is %d, end is %d\n",cp,begin,begin+len);
				decode_clx(k-1,begin,begin+len,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,1);
				if (footnotehack == 1)
                    {
                    spewString("</A>");
                    footnotehack=0;
                    }
				all_fields[0] = tempfields;
				cp=tempcp;
				fseek(in,oldtablepos,SEEK_SET);
				fseek(main,oldmainpos,SEEK_SET);
				header--;
				}
			}
		portions->list_end_no=0; /*ready for next section endnotes*/
		error(erroroutput,"ending endnote\n");

		init_chp(&achp);
        decode_s_chp(&achp,fontnamelist);
		}
#endif	
	return(anyendnotes);
	}

void decode_clx_footer(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag)
	{
	U32 oldtablepos;
	U32 oldmainpos;
	U32 tempcp;
	U32 begin=0;
	U32 len=0;
	int k;
	field_info *tempfields;
	int i,j;
	int footnoteflag=0;
	int annotationflag=0;
	chp achp;
	pap indentpap;

	flushbreaks(0);
	init_pap(&indentpap);
	do_indent(&indentpap);

	if (noheaders)
		return;

/* sterwill: do this later */
#if 0	
	if (header == 0)
		{
		for (i=portions->last_foot;i<portions->list_foot_no+portions->last_foot;i++)
			{
			decode_footnote(&begin,&len,portions,i);

			if ((begin != -1) && (len != -1))
				{
				init_chp(&achp);
                decode_s_chp(&achp,fontnamelist);

				if (footnoteflag == 0)
					{
					spewString("\n<br><img src=\"%s/footnotebegin.gif\"><br>\n",patterndir());
					footnoteflag=1;
					}
				error(erroroutput,"beginning footnote\n");
				/*test*/
				spewString("<p>");
				spewString("<font color=\"#330099\">");
				inafont=1;
				inacolor=1;
				strcpy(incolor,"#330099");
				/*end test*/
				if (!insuper)
					{
					spewString("<sup>");
					insuper=2;
					}
					/*fish*/
				spewString("<a name=\"foot%d\">",i);
					/*
				spewString("<a name=\"foot%d\">",portions->list_footnotes[i]);
					*/

				if (portions->fndFRD[i] > 0)
					{
					spewString("%d",portions->fndFRD[i]);
					spewString("</A>");
					if (insuper == 2)
						{
						spewString("</Sup>");
						insuper=0;
						}
					}
				else
					 footnotehack=1;


				oldmainpos = ftell(main);
				oldtablepos = ftell(in);
				header++;

				k=0;
				while (k< nopieces)
					{
					if (portions->ccpText+begin < rgfc[k]) 
						{
						error(erroroutput,"piece for footer is %d (%x) (%x)\n",k-1,rgfc[k-1],portions->ccpText+begin);
						break;
						}
					k++;
					}
				tempcp=cp;
				cp=begin;
				error(erroroutput,"the difference is %d\n",(portions->ccpText+begin)-rgfc[k-1]);
				/*
				cp = cp-((portions->ccpText+begin)-rgfc[k-1]);
				*/
				cp = rgfc[k-1];
				error(erroroutput,"footer len is %d\n",len);
				error(erroroutput,"footer cp is %x\n",cp);
				tempfields = all_fields[0];
				all_fields[0] = all_fields[2];
				begin = portions->ccpText+begin;
				decode_clx(k-1,begin,begin+len,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,2);
				if (footnotehack == 1)
                    {
                    spewString("</A>");
                    footnotehack=0;
                    }
				all_fields[0] = tempfields;
				cp=tempcp;
				fseek(in,oldtablepos,SEEK_SET);
				fseek(main,oldmainpos,SEEK_SET);
				header--;
				}
			}
		portions->last_foot+=portions->list_foot_no;
		portions->list_foot_no=0; /*ready for next page footnotes*/
		portions->auto_foot=1;

		if (footnoteflag != 0)
			{
			spewString("\n<br><img src=\"%s/footnoteend.gif\"><br>\n",patterndir());
			}

/*begin annotation*/
		for (i=portions->last_anno;i<portions->list_anno_no+portions->last_anno;i++)
			{
			decode_footanno(&begin,&len,portions,i);

			init_chp(&achp);
            decode_s_chp(&achp,fontnamelist);

			if ((begin != -1) && (len != -1))
				{
				if (annotationflag == 0)
					{
					spewString("\n<br><img src=\"%s/commentbegin.gif\"><br>\n",patterndir());
					annotationflag=1;
					}
				error(erroroutput,"beginning annotation\n");
				/*test*/
                spewString("<font color=#ff7777>");
                inafont=1;
                inacolor=1;
                strcpy(incolor,"#ff7777");
                /*end test*/

				if (!insuper)
					spewString("<sup>");

				/*
				spewString("<a name=\"anno%d\">",i);
				*/
				spewString("<a name=\"");
				j = portions->the_atrd[i].xstUsrInitl[0];
				for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    spewString("%c",portions->the_atrd[i].xstUsrInitl[j]);
					}
				spewString("%d",i);
				spewString("\">");
				list_author_key=1;
				j = portions->the_atrd[i].xstUsrInitl[0];
				for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    spewString("%c",portions->the_atrd[i].xstUsrInitl[j]);
					}
				spewString("%d",i+1);
				/*
				spewString("anno%d",i);
				*/

				spewString("</a>");
				if (!insuper)
					spewString("</sup>");

				oldmainpos = ftell(main);
				oldtablepos = ftell(in);
				header++;

				k=0;
				while (k< nopieces)
					{
					if (portions->ccpText+portions->ccpFtn + portions->ccpHdd+begin < rgfc[k]) 
						{
						error(erroroutput,"piece for footer is %d (%x) (%x)\n",k-1,rgfc[k-1],portions->ccpText+portions->ccpFtn + portions->ccpHdd+begin);
						break;
						}
					k++;
					}
				tempcp=cp;
				cp=begin;
				error(erroroutput,"the difference is %d\n",(portions->ccpText+portions->ccpFtn + portions->ccpHdd+begin)-rgfc[k-1]);
				cp = cp-((portions->ccpText+portions->ccpFtn + portions->ccpHdd+begin)-rgfc[k-1]);
				error(erroroutput,"footer len is %d\n",len);
				error(erroroutput,"footer cp is %x\n",cp);
				tempfields = all_fields[0];
				all_fields[0] = all_fields[3];
				decode_clx(k-1,begin,begin+len,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,3);
				all_fields[0] = tempfields;
				cp=tempcp;
				fseek(in,oldtablepos,SEEK_SET);
				fseek(main,oldmainpos,SEEK_SET);
				header--;
				}
			}
		if (annotationflag)
			spewString("\n<br><img src=\"%s/commentbegin.gif\"><br>\n",patterndir());
			
		portions->last_anno+=portions->list_anno_no;
		portions->list_anno_no=0; /*ready for next page footnotes*/
/*end annotation*/		
		decode_footer(&begin,&len,portions,asep);
		if ((begin != -1) && (len != -1))
			{
			init_chp(&achp);
            decode_s_chp(&achp,fontnamelist);

			header++;
			error(erroroutput,"doing footer\n");

			oldmainpos = ftell(main);
			oldtablepos = ftell(in);
			error(erroroutput,"decoding footer clx 1\n");
			error(erroroutput,"the cp of begin of the header is probably cps (%x) to (%x)", portions->ccpText,portions->ccpText+portions->ccpFtn+portions->ccpHdd);
			k=0;
			while (k< nopieces)
				{
				if (portions->ccpText+portions->ccpFtn+begin < rgfc[k]) 
					{
					error(erroroutput,"piece for this footer is %d (%x) (%x)\n",k-1,rgfc[k-1],portions->ccpText+portions->ccpFtn+begin);
					break;
					}
				k++;
				}
			tempcp=cp;
			cp=begin;
			error(erroroutput,"the difference is %d\n",(portions->ccpText+portions->ccpFtn+begin)-rgfc[k-1]);
			cp = cp-((portions->ccpText+portions->ccpFtn+begin)-rgfc[k-1]);
			error(erroroutput,"footer len is %d\n",len);
			error(erroroutput,"footer cp is %x\n",cp);
			tempfields = all_fields[0];
			all_fields[0] = all_fields[1];
			inaheaderfooter=1;
			decode_clx(k-1,begin,begin+len,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,1);
			inaheaderfooter=0;
			all_fields[0] = tempfields;
			cp = tempcp;
			realcp = tempcp;
			fseek(in,oldtablepos,SEEK_SET);
			fseek(main,oldmainpos,SEEK_SET);
			header--;
			}
		error(erroroutput,"ending footer\n");
		pagenumber++;
		sectionpagenumber++;
		init_chp(&achp);
        decode_s_chp(&achp,fontnamelist);
		}
#endif	
	}

void decode_clx_header(U32 *rgfc,sep *asep,int nopieces,U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style *sheet,textportions *portions,ffn *fontnamelist,int headerfooterflag)
	{
	U32 oldtablepos;
	U32 oldmainpos;
	U32 tempcp;
	U32 begin=0;
	U32 len=0;
	int k;
	field_info *tempfields;

	if (noheaders)
		return;

/* sterwill: do this later	 */
#if 0	
	if (header == 0)
		{
		decode_header(&begin,&len,portions,asep);
		if ((begin != -1) && (len != -1))
			{
			header++;
			oldmainpos = ftell(main);
			oldtablepos = ftell(in);
			error(erroroutput,"decoding header\n");
			error(erroroutput,"the cp of begin of the header is probably cps (%x) to (%x)", portions->ccpText+portions->ccpFtn+begin,portions->ccpText+portions->ccpFtn+begin+len);
			k=0;
			while (k< nopieces)
				{
				if (portions->ccpText+portions->ccpFtn+begin < rgfc[k]) 
					{
					error(erroroutput,"piece for header is %d (%x) (%x)\n",k-1,rgfc[k-1],portions->ccpText+portions->ccpFtn+begin);
					break;
					}
				k++;
				}
			tempcp=cp;
			cp=begin;
			cp = rgfc[k-1];
			error(erroroutput,"header cp is %x\n",cp);
			tempfields = all_fields[0];
			all_fields[0] = all_fields[1];
			begin = portions->ccpText+portions->ccpFtn+begin;
			inaheaderfooter=1;
			decode_clx(k-1,begin,begin+len,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,1);
			inaheaderfooter=0;
			all_fields[0] = tempfields;
			cp = tempcp;
			fseek(in,oldtablepos,SEEK_SET);
			fseek(main,oldmainpos,SEEK_SET);
			header--;
			}
		}
#endif
	
	}

int find_piece_cp(U32 sepcp,U32  *rgfc,int nopieces)
	{
	int i;
	error(erroroutput,"find_piece_cp-> %x,\n",sepcp);
	for (i=0;i<nopieces;i++)
		{
		if (sepcp <= rgfc[i])
			{
			error(erroroutput,"sep piece is %d cp is %x, other is %x\n",i,sepcp,rgfc[i]);
			return(i);
			}
		}
	return(-1);
	}

void decode_clx(U32 startpiece,U32 begincp,U32 endcp,FILE *in,FILE *main,FILE *data,U32 fcClx,U32 lcbClx,U32 intervals,U32 chpintervals,U32 *plcfbtePapx,U32 *plcfbteChpx,field_info *all_fields[5],list_info *a_list_info,style * sheet,textportions *portions, ffn *fontnamelist,int headerfooterflag)
	{
	int paraendpiece;
	int paraendpiece2;
	int seppiece;
	int index=0;
	pap *apap=NULL;
	pap *tappap=NULL;
	pap *temppap=NULL;
	chp *achp=NULL;
	pap fakepap;
	chp fakechp;
	sep fakesep;
	long pos = ftell(in);
	long mainpos = ftell(main);
	int i;
	int k;
	U32 tapfc1=0;
	U32 tapfc2=0;
	
	U8 clxt;
	U16 cb;
	U16 *sprm=NULL;
	
	int j=0;
	int nopieces=0;
	U32 *rgfc=NULL;
	U32 *avalrgfc=NULL;
	int letter;
	int hack=0;
	U32 nextfc=0;
	U32 chpnextfc;
	U32 clxcount=0;
	Sprm asprmlist;
	Sprm *psprmlist=&asprmlist;
	Sprm *freesprm;
	U32 lastfc=0;
	U32 lastchpfc=0;
	static int metadone=0;

	signed long nextfootnote=0;
	signed long nextendnote=0;
	U32 nextbookmark_b=cp; /*was 0*/
	U32 nextbookmark_e=cp; /*was 0*/

	U32 sepxfc;
	U32 sepcp;
	sep *asep=NULL;
	sep *tempsep=NULL;

	int newpage=1;

	int notfinished=1;
	int fccount=0;
	int paraflag=0;
	U32 paraendfc=1;

	int d_count=0;

	int issection=1;

	long seek_val=0;

	fseek(in,fcClx,SEEK_SET);
	error(erroroutput,"seeking table to (%x) len %d\n",fcClx,lcbClx);
	error(erroroutput,"begincp is %x\n",begincp);
	while ((clxcount < lcbClx) && (notfinished))
		{
		clxt = getc(in);
		clxcount++;
		error(erroroutput,"clxt is %d\n",clxt);

		psprmlist->list=NULL;
		psprmlist->next=NULL;
		psprmlist->len=0;

		if (clxt == 1)
			{
			/*make a list of of these gprrls*/
			/*decode them when fetched in clxt==2*/
			/*use bitfields to skip ones i dont care about*/
			error(erroroutput,"contains  grpprl\n");
			cb = read_16ubit(in);
			error(erroroutput,"cb is %d\n",cb);
			psprmlist->list = (U8 *) malloc(cb);
			if (psprmlist->list == NULL)
				{
				fprintf(erroroutput,"mem barfoid\n");
				exit(-1);
				}

			clxcount+=(cb+2);
			psprmlist->len=cb;
			error(erroroutput,"here come the gpprl\n");
			for(i=0;i<cb;i++)
				{
				psprmlist->list[i] = getc(in);
				error(erroroutput,"%x",psprmlist->list[i]);
				}
			psprmlist->next = (Sprm *) malloc(sizeof(Sprm));
			if (psprmlist->next == NULL)
				{
				fprintf(erroroutput,"mem barfoid\n");
				exit(-1);
				}
			psprmlist = psprmlist->next;
			index++;
			error(erroroutput,"on gp no %d\n",index);
			}
		else if (clxt == 2)
			{
			index =0;
			error(erroroutput,"contains  Plcfpcd\n");
			
			nopieces = get_piecetable(in,&rgfc,&avalrgfc,&sprm,&clxcount);


			/*guess based on first piece whether to go utf 8 or not*/
			/*
			actually doing utf8 all the time on complex docs as it
			turns out not to be such a good test :-(
			*/

			if ( (header == 0) && (metadone == 0) )
				{
/*				spewString("\n<head>\n<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\">\n</head>\n"); */
				spewString("<section>\n<p>");
				metadone=1;
				}

			for(i=startpiece/*start piece*/;i<nopieces;i++)
				{

				if ((header == 0) && (metadone ==0))
					{
					spewString("<section>\n<p>");
					metadone=1;
					}

				if (notfinished == 0)
					break;

				/*get the sep*/
				if (headerfooterflag == 0)
					if (issection)
						{
						error(erroroutput,"getting sep, %x, piece is %d\n",cp,i);
						sepxfc = find_FC_sepx(cp,&sepcp,portions);
						tempsep = asep;
						asep = get_sep(sepxfc,main);
						seppiece = find_piece_cp(sepcp,rgfc,nopieces);
						init_chp(&fakechp);
						decode_gpprls(&fakepap,&fakechp,asep,sprm,seppiece,&asprmlist,sheet);
						if (decode_clx_endnote(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag))
						{
							/*spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir()); */
						}
						
						if ((pagenumber != 1) && (newpage))
							sectionbreak(asep);

						if (asep != NULL)
							{
							if (tempsep != NULL)
								free(tempsep);
							tempsep=NULL;
							}
						else
							{
							asep=tempsep;
							tempsep=NULL;
							}
						issection=0;
						}

				if (newpage)
					{
					if (!inatable)
						{
						newpage =0;
						decode_clx_header(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag);
						}
					else
						deferrednewpage=1;
					}			

				/*push this val back as its something else*/
				if (avalrgfc[i] & 0x40000000UL)
					{
					seek_val = avalrgfc[i] & 0xbfffffffUL;
					seek_val = seek_val/2;
					error(erroroutput,"tricky ? seek val (%x)\n",seek_val);
					error(erroroutput,"seeking main to (the FC %x) for len %d\n",seek_val,rgfc[i+1]-rgfc[i]);
					hack=1;
					}
				else
					{
					error(erroroutput,"seeking main to (the FC %x) for len %d piece %d\n",seek_val,2*(rgfc[i+1]-rgfc[i]),i);
					seek_val = avalrgfc[i];
					hack=0;
					}

				/*sometimes corrupt files have a few offsets in the file that are *past* the end of the file, so ive stuck 
				this in, which will sort them out for now*/
				if ((mainend < seek_val) || (0 != fseek(main,seek_val,SEEK_SET)))
					{
					error(erroroutput,"overran the end of the file !!\n");
					continue;
					}

				error(erroroutput,"piece entry fc is %x sprm is %x len is %d\n",seek_val,sprm[i],rgfc[i+1]-rgfc[i]);

				error(erroroutput,"1 getting pap HH nopieces is %d, current piece is %d\n",nopieces,i);
				temppap = apap;
				apap = NULL;
				error(erroroutput,"fccount is %x, paraendfc is %x\n",fccount,paraendfc);
				if (fccount == paraendfc-1)
					paraflag=1;
				apap = get_complex_pap(seek_val,plcfbtePapx,i,nopieces,intervals,rgfc,main,avalrgfc,&nextfc,&paraendfc,&paraendpiece,sheet,a_list_info);
				fflush(main);
				error(erroroutput," the next para is at (%x)\n",nextfc);
				if (apap == NULL)
					{
					error(erroroutput,"we failed in out attempt to get a paragraph\n");
					if (temppap == NULL)
						{
						apap = (pap *) malloc(sizeof(pap));
						if (apap == NULL)
							{
							fprintf(erroroutput,"no mem aieee\n");
							exit(-1);
							}
						init_pap(apap);
						}
					else
						{
						error(erroroutput,"using previous pap\n");
						apap = temppap;
						temppap = NULL;
						}
					}
				else
					{
					if (paraflag)
						{
						end_para(temppap,apap);
						paraflag=0;
						}
					if (temppap != NULL)
						free(temppap);
					temppap = NULL;
					}

				/*begin tappap search*/
				/*if we're in a table search for the one with fTtp*/
			
				decode_gpprls(apap,&fakechp,&fakesep,sprm,paraendpiece,&asprmlist,sheet);

				if (apap->fInTable)
					{
					k = i;
					do
						{
						tapfc1 = avalrgfc[k];
						error(erroroutput,"1. we search for the tap here\n");
						error(erroroutput,"tappap nexts are %x, k is %x\n",tapfc1,k);
						do
							{
							if (tappap != NULL)
								{
								free(tappap);
								tappap=NULL;
								}
							tapfc2 = tapfc1;
							tappap = get_complex_pap(tapfc1,plcfbtePapx,k,nopieces,intervals,rgfc,main,avalrgfc,&tapfc1,NULL,&paraendpiece2,sheet,a_list_info);
							if (tappap != NULL)
								decode_gpprls(tappap,&fakechp,&fakesep,sprm,paraendpiece2,&asprmlist,sheet);
							}
						while( (tappap != NULL) && (!tappap->fTtp)  && (tapfc1 != tapfc2)) ;
						k++;
						}
					while ( (tappap != NULL) && (!tappap->fTtp)  && (k<nopieces) );
					/*
					at this stage tappap has the correct row structure stored in it
					that we want apap to have
					*/
					if (tappap != NULL)
						{
						copy_tap(&(apap->ourtap),&(tappap->ourtap));
						error(erroroutput,"finished search for the tap here\n");
						error(erroroutput,"no of cells is %d, %d\n",apap->ourtap.cell_no,tappap->ourtap.cell_no);
						free(tappap);
						tappap=NULL;
						}
					}
				/*end tappap search*/
				
				error(erroroutput,"AT THIS POINT %d\n",apap->istd);
		
				if (achp != NULL)
					free(achp);
				achp = get_complex_chp(seek_val,plcfbteChpx,i,nopieces,chpintervals,rgfc,main,avalrgfc,&chpnextfc,sheet,apap->istd);
				error(erroroutput," CHP: the next para is at (%x)\n",chpnextfc);
				if (achp == NULL)
					{
					error(erroroutput,"we failed in out attempt to get a chp\n");
					achp = (chp *) malloc(sizeof(chp));
					if (achp == NULL)
						{
						fprintf(erroroutput,"no mem aieee\n");
						exit(-1);
						}
					init_chp(achp);

					}

				if (achp->color[0] == '\0')
					{
					if (headerfooterflag == 1)
						strcpy(achp->color,"#7f5555");
					else if (headerfooterflag == 2)
						strcpy(achp->color,"#330099");
					else if (headerfooterflag == 3)
						strcpy(achp->color,"#ff7777");
					}

				/*
				the gpprl that id added is the one that belongs to the 
				piece that has the paragraph mark
				*/
				
				error(erroroutput,"decoding gpprl with (%x) index %d\n",sprm[paraendpiece],paraendpiece);
				fakepap.istd = apap->istd;
				decode_gpprls(&fakepap,achp,&fakesep,sprm,paraendpiece,&asprmlist,sheet);
				error(erroroutput,"1) ilfo is %d\n",apap->ilfo);
				decode_e_list(apap,achp,a_list_info);
				decode_e_chp(achp);
				decode_e_specials(apap,achp,a_list_info);
				decode_e_table(apap,achp,a_list_info);
				decode_s_table(apap,achp,a_list_info);
				decode_s_specials(apap,achp,a_list_info);
				decode_s_chp(achp,fontnamelist);

				error(erroroutput,"2) ilfo is %d\n",apap->ilfo);
				j=0;

				/*this are the count of elements (can be both 8 and 16 bit)*/
				fccount=seek_val-1;
				error(erroroutput,"fcount is %x pos is %x\n",fccount,ftell(main));
				/*this is the actual file position, always in bytes*/
				while(j<rgfc[i+1]-rgfc[i])
					{
					/*get the sep*/
					if (headerfooterflag == 0)
						if (issection)
							{
							error(erroroutput,"getting sep, %x, piece is %d\n",cp,i);
							sepxfc = find_FC_sepx(cp,&sepcp,portions);
							tempsep=NULL;
							asep = get_sep(sepxfc,main);
							seppiece = find_piece_cp(sepcp,rgfc,nopieces);
							decode_gpprls(&fakepap,&fakechp,asep,sprm,seppiece,&asprmlist,sheet);
							if (decode_clx_endnote(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag))
							{
								/* spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir()); */
							}
							
							if ((pagenumber != 1) && (newpage))
								sectionbreak(asep);
							issection=0;

							if (asep != NULL)
								{
								if (tempsep != NULL)
									free(tempsep);
								tempsep=NULL;
								}
							else
								{
								asep=tempsep;
								tempsep=NULL;
								}
							}
					
					if (newpage)
						{
						if (header == 0)
							{
							if (!inatable)
								{
								newpage =0;
								decode_clx_header(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag);
								}
							else
								deferrednewpage=1;
							}
						}			

					if (!hack)
						{
						letter = read_16ubit(main);
						fccount+=2;	
						}
					else
						{
						letter = getc(main);
						fccount++;
						}

					if (j==0) 
						{
						if ( (letter != 12) && (letter!=13) )
							{
							decode_s_list(apap,achp,a_list_info,fontnamelist,DONTIGNORENUM);
							error(erroroutput,"finished list hwew letter was %d %x\n",letter,letter);
							}
						else
							{
							error(erroroutput,"finished list hwew letter was %d %x\n",letter,letter);
							}
						}


					if (fccount >= nextfc)
						{
						lastfc = nextfc;
						error(erroroutput,"the fc is (%x) and the first fc after end of para is (%x)",fccount,nextfc);
						error(erroroutput,"so im looking for the pap again\n");
						error(erroroutput,"2 getting pap HH, jc was %d\n",apap->justify);
						if (apap != NULL)
							{
							temppap = apap;
							apap = NULL;
							}
						if (fccount == paraendfc-1)
							paraflag=1;
						apap = get_complex_pap(fccount,plcfbtePapx,i,nopieces,intervals,rgfc,main,avalrgfc,&nextfc,&paraendfc,&paraendpiece,sheet,a_list_info);
						fflush(main);
						error(erroroutput,"the nextfc for pap one is at %x\n",nextfc);

						if (apap == NULL)
							{
							error(erroroutput,"we failed in out attempt to get a paragraph\n");
							if (temppap == NULL)
								{
								apap = (pap *) malloc(sizeof(pap));
								if (apap == NULL)
									{
									fprintf(erroroutput,"no mem aieee\n");
									exit(-1);
									}
								init_pap(apap);
								}
							else
								{
								apap = temppap;
								temppap = NULL;
								error(erroroutput,"using previous pap, jc is now %d\n",apap->justify);
								}
							}
						else
							{
							/*begin tappap search*/
							/*if we're in a table search for the one with fTtp*/
							decode_gpprls(apap,&fakechp,&fakesep,sprm,paraendpiece,&asprmlist,sheet);
							if (apap->fInTable)
								{
								k = i;
								do
									{
									if (k==i)
										tapfc1 = fccount;
									else
										tapfc1 = avalrgfc[k];
									error(erroroutput,"2. we search for the tap here\n");
									error(erroroutput,"tappap nexts are %x, k is %x\n",tapfc1,k);
									do
										{
										if (tappap != NULL)
											{
											free(tappap);
											tappap=NULL;
											}
										tapfc2 = tapfc1;
										tappap = get_complex_pap(tapfc1,plcfbtePapx,k,nopieces,intervals,rgfc,main,avalrgfc,&tapfc1,NULL,&paraendpiece2,sheet,a_list_info);
										if (tappap != NULL)
											decode_gpprls(tappap,&fakechp,&fakesep,sprm,paraendpiece2,&asprmlist,sheet);
										}
									while( (tappap != NULL) && (!tappap->fTtp) && (tapfc1 != tapfc2)) ;
									k++;
									}
								while ( (tappap != NULL) && (!tappap->fTtp) && (k<nopieces) );
								/*
								at this stage tappap has the correct row structure stored in it
								that we want apap to have
								*/
								if (tappap != NULL)
									{
									copy_tap(&(apap->ourtap),&(tappap->ourtap));
									error(erroroutput,"finished 2nd search for the tap here\n");
									error(erroroutput,"no of cells is %d, %d\n",apap->ourtap.cell_no,tappap->ourtap.cell_no);
									free(tappap);
									tappap=NULL;
									}
								}
							/*end tappap search*/

							/*last gasp of the pap*/
							if (paraflag)
								{
								end_para(temppap,apap);
								paraflag=0;
								}
							
							/*remove unneeded pap*/
							if (temppap != NULL)
								free(temppap);
								
							temppap = NULL;
							}

						}


					if ((fccount >= chpnextfc)  || (fccount >= lastfc))
						{
						lastchpfc=1;
						if (achp != NULL)
							{
							free(achp);
							achp=NULL;
							}
						achp = get_complex_chp(fccount,plcfbteChpx,i,nopieces,chpintervals,rgfc,main,avalrgfc,&chpnextfc,sheet,apap->istd);
						error(erroroutput," CHP: the next para is at (%x)\n",chpnextfc);

						if (achp == NULL)
							error(erroroutput,"we failed in out attempt to get a chp\n");
						else
							{
							if (achp->color[0] == '\0')
								{
								if (headerfooterflag == 1)
									strcpy(achp->color,"#7f5555");
								else if (headerfooterflag == 2)
									strcpy(achp->color,"#330099");
								else if (headerfooterflag == 3)
									strcpy(achp->color,"#ff7777");
								}
		
							error(erroroutput,"before gprl chp fontcode is %d\n",achp->fontcode);
							decode_gpprls(&fakepap,achp,&fakesep,sprm,paraendpiece,&asprmlist,sheet);
							error(erroroutput,"after gprl chp fontcode is %d\n",achp->fontcode);
							decode_e_list(apap,achp,a_list_info);
							decode_e_chp(achp);
							error(erroroutput,"after decode chp fontcode is %d\n",achp->fontcode);
							}
						}

					if (fccount >= lastfc) 
						{
						decode_e_specials(apap,achp,a_list_info);
						decode_e_table(apap,achp,a_list_info);
						decode_s_table(apap,achp,a_list_info);
						decode_s_specials(apap,achp,a_list_info);
						decode_s_chp(achp,fontnamelist);
						lastfc=-1;
						}

					if (lastchpfc) 
						{
						lastchpfc=-1;
						decode_s_chp(achp,fontnamelist);
						if ( ((letter != 12) && (letter != 13)) )
							{
							decode_s_list(apap,achp,a_list_info,fontnamelist,DONTIGNORENUM);
							error(erroroutput,"list started following letter is %d\n",letter);
							}
						else 
							error(erroroutput,"list started following letter is %d\n",letter);
						}


					if (cp >= begincp)
						{
						if ((headerfooterflag >0) && (headerfooterflag < 2))
							{
							if (letter == 13)
								{
								/*
								headers and footers seem to continue until 2 0x0d are reached, they dont
								appear after all to go until the len derived from the header table
								like i thought originally.
								*/
								d_count++;
								if (d_count == 2)
									{
									notfinished=0;
									break;
									}
								}
							else
								d_count=0;
							}
						
						error(erroroutput,"decoding letter: %c, fc is %x, nextfc is %x\n",letter,fccount,nextfc);

						if ( (realcp == nextfootnote) && (headerfooterflag == 0) ) 
							{
							decode_f_reference(portions);
							get_next_f_ref(portions,&nextfootnote);
							}
						
						if ( (realcp == nextendnote) && (headerfooterflag == 0) ) 
							{
							decode_e_reference(portions);
							get_next_e_ref(portions,&nextendnote);
							}

						while (realcp == nextbookmark_b) 
							nextbookmark_b = decode_b_bookmark(&(portions->l_bookmarks),&(portions->bookmarks));

						while (realcp == nextbookmark_e) 
							nextbookmark_e = decode_e_bookmark(&(portions->l_bookmarks));

						if (hack)
							newpage = decode_letter(letter,1,apap,achp,all_fields[0],main,data,fontnamelist,a_list_info,portions,&issection);
						else
							newpage = decode_letter(letter,1,apap,achp,all_fields[0],main,data,fontnamelist,a_list_info,portions,&issection);

						error(erroroutput,"W:sync, actuallt at %x, fccount is %x and aval is %xthough\n",ftell(main),fccount,fccount);
						}
					else
						{
						realcp++;
						cp++;
						}


					/*if newpage then decode header*/
					if (newpage)
						{

						if ((inatable)  && newpage == 2)
							{
							/*in this case we have ended a row, so we should take the opportunity to halt the table*/
/*							spewString("</table>"); */
							inatable=0;
							newpage=1;
							}
						
						error(erroroutput,"clx footer\n");

						if ((!inatable) && (newpage == 1))
							{
							decode_clx_footer(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag);
							if (!issection)
								pagebreak();
							}	
						else if (newpage == 1)
							deferrednewpage=1;
						}

			


					/*get the sep*/
					if (headerfooterflag == 0)
						if (issection)
							{
							error(erroroutput,"getting sep, %x\n",cp);
							sepxfc = find_FC_sepx(cp,&sepcp,portions);
							asep = get_sep(sepxfc,main);
							seppiece = find_piece_cp(sepcp,rgfc,nopieces);
							decode_gpprls(&fakepap,&fakechp,asep,sprm,seppiece,&asprmlist,sheet);
							if (decode_clx_endnote(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag))
							{
								/* spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir());*/
							}
							
							if ((pagenumber != 1) && (newpage))
								sectionbreak(asep);
							issection=0;

							if (asep!= NULL)
								{
								if (tempsep != NULL)
									free(tempsep);
								tempsep=NULL;
								}
							else
								{
								asep=tempsep;
								tempsep=NULL;
								}
							}

					if (cp >= endcp)
						{
						error(erroroutput,"reached end of header or main, headerfootflag is %d\n",headerfooterflag);
							notfinished=0;
							error(erroroutput,"end of header or main break\n");
							break;
						}

					j++;
					}/*end for/while j*/
					
				error(erroroutput,"here after header ?\n");

				if (achp != NULL)
					{
					free(achp);
					achp = NULL;
					}
				}
			}
		else
			fprintf(erroroutput,"some kind of error occured, byte count off\n");
		}


	if (apap == NULL)
		{
		apap = (pap *)malloc(sizeof(pap));
		if (apap == NULL)
			return;
		}
#if 0	
	apap->istd = 0;
	apap->ilvl=-1;
	apap->tableflag=0;
#endif
	init_pap(apap);
	decode_e_list(apap,achp,a_list_info);
	decode_e_specials(apap,achp,a_list_info);
	decode_e_table(apap,achp,a_list_info);
	if (apap !=NULL)
		free(apap);
	if (achp !=NULL)
		free(achp);

	if (!newpage)
		{
		error(erroroutput,"clx footer 2\n");
		decode_clx_footer(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag);
		if (decode_clx_endnote(rgfc,asep,nopieces,startpiece,begincp,endcp,in,main,data,fcClx,lcbClx,intervals,chpintervals,plcfbtePapx,plcfbteChpx,all_fields,a_list_info,sheet,portions,fontnamelist,headerfooterflag))
		{
			/* spewString("\n<br><img src=\"%s/endnoteend.gif\"><br>\n",patterndir());*/
		}
		
		}

	if (rgfc != NULL)
		free(rgfc);
	if (avalrgfc != NULL)
		free(avalrgfc);
	if (sprm != NULL)
		free(sprm);

	if (asep != NULL)
		free(asep);

	psprmlist = &asprmlist;
	if (psprmlist->list != NULL)
			free(psprmlist->list);
	psprmlist = psprmlist->next;
	while (psprmlist != NULL)
		{
		freesprm = psprmlist;
		psprmlist = psprmlist->next;
		if (freesprm->list != NULL)
			free(freesprm->list);
		free(freesprm);
		}

	fseek(in,pos,SEEK_SET);
	fseek(main,mainpos,SEEK_SET);
	}



		
int decode_ilfo(pap *retpap,list_info *a_list_info)
	{
	int j;
	
	/*
	else now we use our a_list_data to generate the paps list_data
	including the text to use, whether to prepend a index or not
	what format the index is in, and the start index
	*/
	/*
	first get the text, ) / . / string / whatever
	*/
	error(erroroutput,"ilfo %d of %d\n",retpap->ilfo,a_list_info->nooflfos);
	if (retpap->ilfo >= a_list_info->nooflfos)
		{
		if (retpap->ilfo != 2047)
			{
			error(erroroutput,"hmm problem\n");
			return(-1);
			}
		else
			error(erroroutput,"legacy anld\n");
		return(-1);
		}

	/*
	what should happen here, is after we get this base list data, 
	we add the overrides to a copy of our own
	*/

	retpap->list_data = &(a_list_info->o_list_def[retpap->ilfo]);
	
	retpap->list_data->id = a_list_info->lst_ids[retpap->ilfo];
	for (j=0;j<retpap->list_data->len;j++)
	            error(erroroutput,"--> %c",retpap->list_data->list_string[j]);


	error(erroroutput,"ilfo request is %d, the id in the lfo is %x",retpap->ilfo,a_list_info->lst_ids[retpap->ilfo]);
	j=0;
	while(j<a_list_info->nooflsts)
		{
		if (a_list_info->o_lst_ids[j] == retpap->list_data->id)
			{
			error(erroroutput,"and the returned index is %d\n",j);
			return(j);
			}
		j++;
		}

	return(0);
	}





void decode_s_table(pap *apap,chp *achp,list_info *a_list_info)
	{
	int redotable=0,i;
	float width;
	int clearedspecials=0;
	chp reset;
	pap resetpap;
	int resetchp=0;

	int tablewidth;

	init_chp(&reset);
	init_pap(&resetpap);
	reset.fontcode=-2;
	reset.ascii_font=currentfontcode+1;

	error(erroroutput,"decoding start specials istd is %d\n",apap->istd);

	if ((!(inatable)) && (apap->fInTable))
		{
		if (!resetchp)
			{
			decode_e_chp(&reset);
			resetchp=1;
			}
		if (!clearedspecials)
			{
			decode_e_specials(&resetpap,NULL,NULL);
			clearedspecials=1;
			}
		flushbreaks(0);
		do_indent(apap);
	
		if (notablewidth)
		{
			/* spewString("\n<table border=1>"); */
		}
		else
		{
			/* spewString("\n<table border=1 width=\"");*/
		}
		
		/*search for tap*/
		rowcount=0;
		inatable=1;
		inarow=0;
		inacell=0;
		lastrowlen=apap->ourtap.cell_no;
		error(erroroutput,"lastrowlen is %d\n",lastrowlen);
		tablewidth=0;
		for (i=0;i<apap->ourtap.cell_no+1;i++)
			{
			lastcellwidth[i] = apap->ourtap.cellwidth[i];
			error(erroroutput,"dx-->%d",lastcellwidth[i]);
			tablewidth+=lastcellwidth[i];
			}
		tablewidth-=lastcellwidth[0];
		error(erroroutput,"set table, tablewidth is twirps is %d\n",tablewidth);
		if (!notablewidth)
		{
			/*			spewString("%d\">\n",tablewidth/TWIRPS_PER_H_PIXEL); */
		}
		
		}

	if ( ((inatable) && (apap->fInTable) && (!apap->fTtp)) || ((inatable) && (!inacell) && (inarow)) )
		{
		if (inarow == 0)
			{
			if (lastrowlen != apap->ourtap.cell_no)
				{
				error(erroroutput,"well have to start a new table\n");
				redotable=1;
				}
			else 
				{
				for (i=0;i<apap->ourtap.cell_no+1;i++)
					{
					if (lastcellwidth[i] != apap->ourtap.cellwidth[i])
						redotable=1;
					}
				}

			if (redotable)
				{
				if (!clearedspecials)
					{
					/*decode_e_specials(&reset,achp,a_list_info);*/
					clearedspecials=1;
					}
				if (!resetchp)
					{
					decode_e_chp(&reset);
					resetchp=1;
					}
/* 				spewString("\n</table>\n"); */
#if 0
				flushbreaks(0);
#endif
#if 0
				if (notablewidth)
					spewString("\n<table border=1>\n");
				else
					spewString("\n<table border=1 width=\"100%%\">\n");
#endif				
				error(erroroutput,"redone table\n");
				/*no need to reset rowcount*/
				inatable=1;
				inarow=1;
				inacell=0;
				lastrowlen=apap->ourtap.cell_no;
				for (i=0;i<apap->ourtap.cell_no+1;i++)
					lastcellwidth[i] = apap->ourtap.cellwidth[i];

				}

			if (!clearedspecials)
				{
				/*decode_e_specials(&reset,achp,a_list_info);*/
				clearedspecials=1;
				}
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
/*			spewString("\n<tr>\n");*/
			rowcount++;
			colcount=0;
			inarow = 1;
			}
			
		if ( (inacell == 0) && (colcount < apap->ourtap.cell_no) )
			{
#if 0
			width = (apap->ourtap.cellwidth[colcount+1]-apap->ourtap.cellwidth[colcount])*100;
			width = rint(width/apap->ourtap.tablewidth);
#endif
/*
			if (notablewidth)
				spewString("\n<td valign=\"top\" "); 
			else
				spewString("\n<td valign=\"top\" width=\"%.0f%%\" ",width); 
			if (apap->ourtap.rowheight != 0)
				spewString("height=\"%d\"",abs(apap->ourtap.rowheight)/TWIRPS_PER_V_PIXEL);
*/
			chpson();

			output_tablebg(apap);

			cellempty=1;
			inacell=1;
			colcount++;
			}
		else if ( (inacell == 0) && (apap->ourtap.cell_no == 0) )
			{
			error(erroroutput,"dont have information as to width of cells\n");
#if 0
			if (!resetchp)
                {
                decode_e_chp(&reset);
                resetchp=1;
                }
#endif
/*
  spewString("\n<td valign=\"top\" ");
  if (apap->ourtap.rowheight != 0)
  spewString("height=\"%d\"",abs(apap->ourtap.rowheight)/TWIRPS_PER_V_PIXEL);
*/
			chpson();

			output_tablebg(apap);

            cellempty=1;
            inacell=1;
            colcount++;
			}

		check_auto_color(achp);
			
		/*weve come into another cell*/
		}
	}

void decode_s_specials(pap *apap,chp *achp,list_info *a_list_info)
	{
	
	int resetchp=0;
	
	chp reset;

	init_chp(&reset);
	reset.fontcode=-2;

	error(erroroutput,"decoding start specials istd is %d\n",apap->istd);

	do_indent(apap);

	if ((apap->justify == 1) && (incenter == 0))
		{
		incenter =1;
/* sterwill: do this later		 */
/*		spewString("\n<CENTER>"); */
		error(erroroutput,"begin of center\n");
		}

	if ((apap->justify == 2) && (inrightjust == 0))
		{
		inrightjust =1;
/* sterwill: do this later */
/*spewString("\n<DIV align=right>"); */
		error(erroroutput,"begin of right\n");
		}

	/* THESE ARE <H1> THROUGH <H9> */

	if (ignoreheadings == 0)
		{
		if ((apap->istd == 1) && (inah1 != 1))
			{
			inah1=1;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:24pt\">");
			}

		if ((apap->istd == 2) && (inah1 != 2))
			{
			inah1=2;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-size:20pt\">");
			}

		if ((apap->istd == 3) && (inah1 != 3))
			{
			inah1=3;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:16pt\">");
			}

		if ((apap->istd == 4) && (inah1 != 4))
			{
			inah1=4;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-style:italic; font-size:14pt\">");
			}

		if ((apap->istd == 5) && (inah1 != 5))
			{
			inah1=5;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-style:bold; font-size:12pt\">");
			}

		if ((apap->istd == 6) && (inah1 != 6))
			{
			inah1=6;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:10pt\">");
			}

		if ((apap->istd == 7) && (inah1 != 7))
			{
			inah1=7;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:8pt\">");
			}

		if ((apap->istd == 8) && (inah1 != 8))
			{
			inah1=8;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:6pt\">");
			}

		if ((apap->istd == 9) && (inah1 != 9))
			{
			inah1=9;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("<c PROPS=\"font-weight:bold; font-size:4pt\">");
			}
		}	

	error(erroroutput,"manually doing start chp\n");
	}

int decode_e_table(pap *apap,chp *achp,list_info *a_list_info)
	{
	int ret=0;
	pap reset;
	chp resetc;
	int clearedspecials=0;
	error(erroroutput,"decoding end table\n");

	init_pap(&reset);
	init_chp(&resetc);
	resetc.fontcode=-2;
	resetc.ascii_font=currentfontcode+1;

	if ((apap->fInTable == 1) && (apap->fTtp == 1) && apap->tableflag)
		{
		error(erroroutput,"cell and row end\n");
#if 0
		if (!clearedspecials)
			{
			decode_e_chp(&resetc);
			decode_e_specials(&reset,achp,a_list_info);
			clearedspecials=1;
			}
#endif
/*		spewString("\n</tr>\n"); */
		ret=1;
		inarow=0;
		inacell=0;
		apap->tableflag=0;
		}
	else if ((apap->fInTable == 1)  && (apap->fTtp != 1) && apap->tableflag )
		{
		if (!clearedspecials)
			{
			decode_e_chp(&resetc);
			decode_e_specials(&reset,achp,a_list_info);
			clearedspecials=1;
			}
		/*
		if (cellempty == 1)
			spewString("&nbsp;");
		spewString("\n</td>\n");
		*/		
		backgroundcolor[0] = '\0';
		inacell=0;
		apap->tableflag=0;
		chpsoff();
		error(erroroutput,"cell end\n");
		}

	if ( (inatable) && (!apap->fInTable) && (!inarow) )
		{
		error(erroroutput,"table ends here\n");
#if 0
		if (!clearedspecials)
			{
			decode_e_chp(&resetc);
			decode_e_specials(&reset,achp,a_list_info);
			clearedspecials=1;
			}
#endif
/*		spewString("\n</table>\n"); */
		lastrowlen=0;
		inatable=0;
		}

	return(ret);
	}
void decode_e_specials(pap *apap,chp *achp,list_info *a_list_info)
	{
	int resetchp=0;
	chp reset;
	error(erroroutput,"decoding specials istd is %d\n",apap->istd);
	/*before a para ends, well reset all chars to original defaults*/

	init_chp(&reset);
	reset.fontcode=-2;

	error(erroroutput,"manual end chp\n");

	if (ignoreheadings ==0)
		{
		if ((apap->istd != 9) && (inah1 == 9))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
				
			spewString("</c>");
			}
		else if ((apap->istd != 8) && (inah1 == 8))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 7) && (inah1 == 7))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 6) && (inah1 == 6))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 5) && (inah1 == 5))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 4) && (inah1 == 4))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 3) && (inah1 == 3))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 2) && (inah1 == 2))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		else if ((apap->istd != 1) && (inah1 == 1))
			{
			inah1=0;
			if (!resetchp)
				{
				decode_e_chp(&reset);
				resetchp=1;
				}
			spewString("</c>");
			}
		}
	error(erroroutput,"inah1 is now %d\n",inah1);
	if ((apap->justify != 2) && (inrightjust == 1))
		{
		inrightjust =0;
		error(erroroutput,"end of right\n");
/*		spewString("\n</DIV>"); */
		}

	if ((apap->justify != 1) && (incenter == 1))
		{
		incenter =0;
		error(erroroutput,"end of center\n");
/*		spewString("\n</CENTER>"); */
		}
#if 0
	do_indent(apap);
#endif


	}



void chpsoff()
	{
	chps=1;
	}

void chpson()
	{
	chps=0;
	}
