/*
Released under GPL, written by Caolan.McNamara@ul.ie.

Copyright (C) 1998 
	Caolan McNamara

Real Life: Caolan McNamara           *  Doing: MSc in HCI
Work: Caolan.McNamara@ul.ie          *  Phone: +353-61-202699
URL: http://skynet.csn.ul.ie/~caolan *  Sig: an oblique strategy
How would you have done it?
*/

/*warning: this software requires laola's lls to be installed*/

/*

this code is often all over the shop, being more of an organic entity
that a carefully planed piece of code, so no laughing there at the back!

and send me patches by all means, but think carefully before sending me
a patch that doesnt fix a bug or add a feature but instead just changes
the style of coding, i.e no more thousand line patches that fix my 
indentation.

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "config.h"
#include "mswordview.h"
#include "roman.h"
#include "utf.h"

extern char *symbolurl;
extern char *wingdingurl;
extern char *patternurl;
extern FILE *outputfile;
extern FILE *erroroutput;
extern char *imagesurl;
extern char *imagesdir;
extern int nofontfaces;
extern int incenter;
extern int inrightjust;
extern char backgroundcolor[8];
extern long int cp;
extern long int realcp;


/*
im only going to use fontfaces if the language is Ascii based, as there
would only be grief working out fonts for eastern fonts the way the
word system is at the moment
*/
int use_fontfacequery(chp *achp)
    {
    if (nofontfaces)
        return 0;
    error(erroroutput,"fontface query %d %d %d %d\n",achp->eastfont,achp->idctHint,achp->ascii_font,achp->noneastfont);
    if ((achp->eastfont == 0) && (achp->idctHint == 0) && (achp->ascii_font == achp->noneastfont))
        return 1;
    return 0;
    }


ffn *get_fontnamefromcode(ffn *fontnamelist,int fontcode, int *ndx)
    {
    ffn *tempfont;
    *ndx=0;
    tempfont = fontnamelist;
    while (*ndx < fontcode)
        {
        tempfont = tempfont->next;
        if (tempfont == NULL)
            break;
        (*ndx)++;
        }
    return(tempfont);
    }

void decode_list_nfc(int *value,int no_type)
    {
    char roman[81];
    switch(no_type)
        {
        case 0:
            fprintf(outputfile,"%d",(*value)++);
            break;
        case 1:
            fprintf(outputfile,"%s",decimalToRoman((*value)++,roman));
            break;
        case 2:
            fprintf(outputfile,"%s",ms_strlower(decimalToRoman((*value)++,roman)));
            break;
        case 3:
            fprintf(outputfile,"%c",64+(*value)++); /*uppercase letter*/
            break;
        case 4:
            fprintf(outputfile,"%c",96+(*value)++); /*lowercase letter*/
            break;
        default:
            break;
        }
    }

int decode_symbol(U16 fontspec)
	{
	error(erroroutput,"given symbol %ld, converting to %ld\n",fontspec,fontspec-61472);
	fontspec = fontspec-61472;
	if (fontspec < 95)
		fontspec+=34;
	fprintf(outputfile,"<img src=\"%s/%d.gif\">",symbolfontdir(),fontspec);
	return(1);
	}
	
char *symbolfontdir(void)
	{
	if (symbolurl != NULL)
		return(symbolurl);
	return(SYMBOLFONTDIR);
	}

int decode_wingding(U16 fontspec)
	{
	error(erroroutput,"given wingding %ld, converting to %ld\n",fontspec,fontspec-61472);
	fontspec = fontspec-61472;
	fontspec+=34;
	if (fontspec > 86) 
		fontspec+=4;
	if (fontspec == 114)
		fontspec = 153;
	else if (fontspec == 115)
		fontspec = 160;
	else if (fontspec == 116)
		fontspec = 163;
	else if (fontspec == 117)
		fontspec = 165;
	else if (fontspec == 118)
		fontspec = 164;
	else if (fontspec == 119)
		fontspec = 168;
	else if (fontspec == 120)
		fontspec = 167;
	else if (fontspec == 121)
		fontspec = 171;
	else if (fontspec == 122)
		fontspec = 172;
	else if (fontspec == 123)
		fontspec = 170;
	else if (fontspec == 124)
		fontspec = 184;
	else if (fontspec == 125)
		fontspec = 169;
	else if (fontspec == 129)
		fontspec = 116;
	else if (fontspec == 130)
		fontspec = 117;
	else if (fontspec == 131)
		fontspec = 114;
	else if (fontspec == 132)
		fontspec = 115;
	else if ( (fontspec > 132) && (fontspec < 156) )
		fontspec-=5;
	else if ( (fontspec >= 156) && (fontspec < 164 ) )
		fontspec-=38;
	else if (fontspec == 164)
		fontspec=151;
	else if (fontspec == 165)
		fontspec=152;
	else if (fontspec == 166)
		fontspec=161;
	else if  ((fontspec > 166) && (fontspec < 173 ) )
		fontspec-=13;
	else if (fontspec == 173)
		fontspec=162;
	else if (fontspec == 174)
		fontspec=164;
	else if ((fontspec > 174) && (fontspec < 186))
		fontspec-=2;
	else if ( (fontspec >= 186) && (fontspec < 257) )
		fontspec--;
	else if (fontspec > 256)
		fontspec-=170;
	
	fprintf(outputfile,"<img src=\"%s/%d.gif\">",wingdingfontdir(),fontspec);
	return(1);
	}

char *patterndir(void)
	{
	if (patternurl != NULL)
		return(patternurl);
	return(PATTERNDIR);
	}
	
char *wingdingfontdir(void)
	{
	if (wingdingurl != NULL)
		return(wingdingurl);
	return(WINGDINGFONTDIR);
	}


char *ms_basename(char *filename)
	{
	char *temppointer=NULL;
	if ((filename != NULL) && (filename[0] != '\0'))
		{
		temppointer = filename+strlen(filename);
#ifndef WINDOWS
		while ((temppointer != filename) && (*(temppointer-1) != '/'))
			temppointer--;
#else
		while ((temppointer != filename) && (*(temppointer-1) != '\\'))
			temppointer--;
#endif
		}
	return temppointer;
	}

void outputimgsrc(char *filename)
	{
	char *temppointer;


	if (filename!= NULL)
		{
		temppointer = ms_basename(filename);
		if ((imagesurl == NULL) && (imagesdir == NULL))
			fprintf(outputfile,"<img src=\"%s\"><br>",temppointer);
		else if ( (imagesurl == NULL) && (imagesdir != NULL) )
			fprintf(outputfile,"<img src=\"%s/%s\"><br>",imagesdir,temppointer);
		else
			fprintf(outputfile,"<img src=\"%s/%s\"><br>",imagesurl,temppointer);
		error(erroroutput,"success!\n");
		}
	else
		error(erroroutput,"filename was null!\n");
	}


char *ms_strlower(char *in)
	{
	char *useme = in;
	while(*useme != '\0')
		{
		*useme = tolower(*useme);
		useme++;
		}
	return(in);
	}


U32 read_32ubit(FILE *in)
	{
	U16 temp1,temp2;
	U32 ret;
	temp1 = read_16ubit(in);
	temp2 = read_16ubit(in); 
	ret = temp2;
	ret = ret << 16;
	ret += temp1;
	return(ret);
	}

U32 sread_32ubit(U8 *in)
	{
	U16 temp1,temp2;
	U32 ret;
	temp1 = sread_16ubit(in);
	temp2 = sread_16ubit(in+2);
	ret = temp2;
	ret = ret << 16;
	ret+=temp1;
	return(ret);
	}

U16 read_16ubit(FILE *in)
	{
	/*im pretty insecure when it come to dealing with bits, i like
	 *to address them by their meaning not the reality
	 */
	U8 temp1,temp2;
	U16 ret;
	temp1 = getc(in);
	temp2 = getc(in);
	ret = temp2;
	ret = ret << 8;
	ret += temp1;
	return(ret);
	}


U16 sread_16ubit(U8 *in)
	{
	U8 temp1,temp2;
	U16 ret;
	temp1 = *in;
	temp2 = *(in+1);
	ret = temp2;
	ret = ret << 8;
	ret += temp1;
	return(ret);
	}

U32 dread_32ubit(FILE *in,U8 **list)
	{
	U8 *temp;
	U32 ret;
	if (list == NULL)
		return(read_32ubit(in));
	else
		{
		temp = *list;
		(*list)+=4;
		ret = sread_32ubit(temp);
		return(ret);
		}
	}

U16 dread_16ubit(FILE *in,U8 **list)
	{
	U8 *temp;
	U16 ret;
	if (list == NULL)
		return(read_16ubit(in));
	else
		{
		temp = *list;
		(*list)+=2;
		ret = sread_16ubit(temp);
		return(ret);
		}
	}

U8 dgetc(FILE *in,U8 **list)
	{
	U8 *temp;
	if (list == NULL)
		return(getc(in));
	else
		{
		temp = *list;
		(*list)++;
		return(sgetc(temp));
		}
	}

U8 sgetc(U8 *in)
	{
	return(*in);
	}


int isodd(int i)
	{
	if ( (i/2) == ((i+1)/2) )
		return 0;
	return 1;
	}

void oprintf(int silentflag,char *fmt, ...)
	{
    va_list argp;
	if (!silentflag)
		{
		va_start(argp, fmt);
		vfprintf(outputfile, fmt, argp);
		va_end(argp);
		}
    }

void error(FILE *stream,char *fmt, ...)
	{
/* sterwill: this slows things down	*/
#if 0		
	#ifdef DEBUG
    va_list argp;
    fprintf(stream, "error: ");
    va_start(argp, fmt);
    vfprintf(stream, fmt, argp);
    va_end(argp);
    fprintf(stream, "\n");
	fflush(stream);
	#endif
#endif
    }

RETSIGTYPE reaper (int ignored)
    {
#if 0
#ifdef MUST_REINSTALL_SIGHANDLERS
    signal_handle (SIGCHLD, reaper);
#endif
    while (WAITPID (-1, 0, WNOHANG) > 0)
        ;
#endif
    }

RETSIGTYPE timeingout(int ignored)
    {
	fprintf(erroroutput,"\nconversion took too long, assuming something wrong and aborting\n");
	fprintf(erroroutput,"\nset timeout value higher (or dont set it) to try for longer\n");
	exit(-1);
    }

#if defined (HAVE_POSIX_SIGNALS)
void signal_handle (int sig, SigHandler * handler)
    {
#if 0
    struct sigaction act, oact;

    act.sa_handler = handler;
    act.sa_flags = 0 | SA_NOCLDSTOP | SA_RESTART;
    sigemptyset (&act.sa_mask);
    sigemptyset (&oact.sa_mask);
    sigaction (sig, &act, &oact);
#endif
    }
#endif

#if 0
olestream * divide_streams(char *filename,char **analyze,char **slashtmp,  char *argv0)
	{
	olestream *olelist;
	olestream *olelistptr;
	pid_t pid;
    int filedes[2];
    char *execut[4];
    char buffer[4096];
    char fullfilepath[PATH_MAX];
    char currentdir[PATH_MAX];
    int i,j,k=0;
    int scan=0;
    int scanbracket=0;
	int scanlevel=0;
	int level=0;
	int len;
	
	char numbers[4];
	
	char *ptr=NULL;
    char basename[PATH_MAX];

	memset(fullfilepath, 0, PATH_MAX);
	memset(currentdir, 0, PATH_MAX);
	memset(basename, 0, PATH_MAX);

    execut[0] = "lls-mswordview";
    execut[1] = "-s";
    execut[3] = NULL;


	if (pipe (filedes))
        {
        fprintf (erroroutput, "Pipe failed.\n");
        exit(-1);
        }

    if (filename[0] != '/')
        {
        if (NULL == getcwd(currentdir,PATH_MAX))
            {
            fprintf (erroroutput, "couldnt get current directory\n");
            exit(-1);
            }
        strcpy(fullfilepath,currentdir);
        }
    if (currentdir[strlen(currentdir)] != '/')
        strcat(currentdir,"/");
    if (fullfilepath[strlen(fullfilepath)] != '/')
        strcat(fullfilepath,"/");
    strncat(fullfilepath,filename,PATH_MAX-(strlen(fullfilepath)+1));
    error(erroroutput,"full filename is %s\n",fullfilepath);

    execut[2] = fullfilepath;

    *slashtmp = tmpnam(NULL);
    if (0 != mkdir((const char *)*slashtmp,S_IRWXU))
        {
        fprintf (erroroutput, "mkdir of %s failed\n",*slashtmp);
        exit(-1);
        }
    else
        {
        error(erroroutput, "mkdir of %s suceeded\n",*slashtmp);
        }

	*analyze =(char *) malloc (strlen(*slashtmp) + strlen("analyze") + 3);
    if (*analyze == NULL)
        {
        fprintf (erroroutput, "malloc failed\n");
        exit (-1);
        }
    strcpy(*analyze,*slashtmp);
    strcat(*analyze,"/analyze/");

	strcpy(basename,*analyze); 

	pid = fork ();
    if (pid == (pid_t) 0)  /*child*/
        {
        close (1);                      /* close stdout */
        dup (filedes[1]);               /* make pipe stdout */
        close (filedes[1]);
        chdir(*slashtmp);
        execvp(execut[0],(char* const *) execut);
#if defined(HAVE_ERRNO_H)
		fprintf( erroroutput, "Exec of %s failed with error code %d, trying a different tack\n", execut[0], errno );
#else
		fprintf( erroroutput, "Exec of %s failed, trying a different tack\n", execut[0]);
#endif
		/*if that failed try again, with zack@studioarchetype.com idea*/
		if (strchr(argv0,'/') != NULL) 
			{
			execut[0] = malloc((strlen(argv0) + strlen("/lls-mswordview") + 1) * sizeof(char));
			argv0[strlen(argv0) - strlen(strrchr(argv0,'/'))] = '\0';
			sprintf(execut[0],"%s/lls-mswordview",argv0);
		    } 
		else 
			{
			execut[0] = malloc((strlen(currentdir) + strlen("/lls-mswordview") + 1) * sizeof(char));
			sprintf(execut[0],"%s/lls-mswordview",currentdir);
			}
        execvp(execut[0],(char* const *) execut);
#if defined(HAVE_ERRNO_H)
		fprintf( erroroutput, "Exec of %s also failed with error code %d, trying one last tack\n", execut[0], errno );
#else
		fprintf( erroroutput, "Exec of %s also failed, trying one last tack\n", execut[0]);
#endif
		/*if that failed try again, with zack@studioarchetype.com idea*/
		if (strchr(argv0,'/') != NULL) 
			{
			execut[0] = malloc((strlen(argv0) + strlen("/laola/lls-mswordview") + 1) * sizeof(char));
			argv0[strlen(argv0) - strlen(strrchr(argv0,'/'))] = '\0';
			sprintf(execut[0],"%s/laola/lls-mswordview",argv0);
		    } 
		else 
			{
			execut[0] = malloc((strlen(currentdir) + strlen("/laola/lls-mswordview") + 1) * sizeof(char));
			sprintf(execut[0],"%s/laola/lls-mswordview",currentdir);
			}
        execvp(execut[0],(char* const *) execut);
#if defined(HAVE_ERRNO_H)
		fprintf( erroroutput, "Exec of %s also failed with error code %d\n", execut[0], errno );
#else
		fprintf( erroroutput, "Exec of %s also failed\n", execut[0]);
#endif
		exit(-1);
        }
    else if (pid < (pid_t) 0)
        {
        /* The fork failed. */
        fprintf (erroroutput, "Fork failed.\n");
        exit (-1);
        }

    close (filedes[1]);


	olelist = (olestream *) malloc(sizeof(olestream));
	if (olelist == NULL)
		{
		fprintf (erroroutput, "malloc failed.\n");
        exit (-1);
		}


	/*work backwards to the the first / or beginning*/

	k = strlen(basename);

	j=strlen(filename);
    while((filename[j] != '/') && (j!=0))
		j--;

	/*j now a beginning of basename*/
    while((filename[j] != '.') && (j<strlen(filename)))
		{
        basename[k++] = filename[j++];
		basename[k] = '\0';
		}

	basename[k++] = '.';
	basename[k] = '\0';
	error(erroroutput,"basename is %s\n",basename);

    error(erroroutput,"filename base is %s\n",basename);

    olelistptr = olelist;


	while ((len = read (filedes[0],buffer,4095)) > 0)
        {
        for (i=0;i<len;i++)
            {
            if ((buffer[i] == '\'') && (scan == 0))
                {
                scan=1;
                ptr = olelistptr->streamname;
                j=0;
                }
            else if ((buffer[i] == '\'') && (scan == 1))
                {
                scan=0;
                ptr[j] = '\0';
                }
            else if ((buffer[i] == '(') && (scanbracket == 0))
                {
				error(erroroutput,"streamname is %s\n",olelistptr->streamname);
                scanbracket=1;
				ptr = numbers;
				j = 0;
                }
            else if ((buffer[i] == ')') && (scanbracket == 1))
                {
                scanbracket=0;
                ptr[j] = '\0';
				error(erroroutput,"filename nos %s\n",numbers);
				sprintf(olelistptr->filename,"%s%.2x",basename,(int)strtol(numbers,(char **)NULL,16));
				error(erroroutput,"filename is %s\n",olelistptr->filename);
				olelistptr->level = level/3;
				error(erroroutput,"level is %d\n",level/3);
                olelistptr->next = (olestream *) malloc(sizeof(olestream));
                if (olelistptr == NULL)
                    {
                    fprintf (erroroutput, "malloc failed.\n");
                    exit (-1);
                    }
                olelistptr = olelistptr->next;
                olelistptr->next=NULL;
                }
            else if (scan == 1)
                {
                ptr[j++] = buffer[i];
                }
            else if (scanbracket==1)
                {
                if (isxdigit(buffer[i]))
                    ptr[j++] = buffer[i];
                }
			else if (buffer[i] == ':')
				{
				scanlevel=1;
				level=0;
				}
			else if (scanlevel == 1)
				{
				if (isspace(buffer[i]))
					level++;
				else
					scanlevel=0;
				}
            }
        }

	return(olelist);
	}
#endif

#if 0
void cleanupstreams(char *analyze,char *slashtmp)
	{
	DIR *adir;
	struct dirent *thedir;
	char currentdir[PATH_MAX] ="/";

 	if (NULL == getcwd(currentdir,PATH_MAX))
            fprintf (erroroutput, "minor warning: couldnt get current directory path,continuing.\n");

	chdir(analyze);
	adir = opendir(analyze);	
	if (adir == NULL)
		error (erroroutput, "couldnt open %s\n",analyze);
	else
		{
		while (NULL != (thedir = readdir(adir)))
			{
			if (strcmp(thedir->d_name,"..") && strcmp(thedir->d_name,"."))
				{
				error(erroroutput,"removing %s\n",thedir->d_name);
				remove(thedir->d_name);
				}
			}
		closedir(adir);
		}
	chdir(currentdir);

	if (0 != rmdir((const char *)analyze))
		error(erroroutput, "rmdir of %s failed\n",analyze);
	else
		error(erroroutput, "rmdir of %s suceeded\n",analyze);

	if (0 != rmdir((const char *)slashtmp))
		error(erroroutput, "rmdir of %s failed\n",slashtmp);
	else
		error (erroroutput, "rmdir of %s suceeded\n",slashtmp);
	}
#endif

int setdecom(void)
	{
#ifdef SYSTEM_ZLIB
	return(1);
#endif
	fprintf(erroroutput,"Warning: mswordview was not compiled against zlib, so wmf files cannot be\ndecompressed\n");
	return(0);
	}

void sectionbreak(sep *asep)
	{
	/*
	i may need to put code here to add 1 to a number based
	on the even odd section breaks, or maybe that only affects
	what side of a book the page appears ?
	*/
	if (incenter)
    	fprintf(outputfile,"</center>");
    if (inrightjust)
    	fprintf(outputfile,"</div>");
	if (asep != NULL)
		{
		switch (asep->bkc)
			{
			case 0:
				fprintf(outputfile,"\n<br><img src=\"%s/sectionendcontinous.gif\"><br>\n",patterndir());
				break;
			case 1:
				fprintf(outputfile,"\n<br><img src=\"%s/sectionendcolumn.gif\"><br>\n",patterndir());
				break;
			case 2:
				fprintf(outputfile,"\n<br><img src=\"%s/sectionendnewpage.gif\"><br>\n",patterndir());
				break;
			case 3:
				fprintf(outputfile,"\n<br><img src=\"%s/sectionendeven.gif\"><br>\n",patterndir());
				break;
			case 4:
				fprintf(outputfile,"\n<br><img src=\"%s/sectionendodd.gif\"><br>\n",patterndir());
				break;
			}
		}
	else
		error(erroroutput,"agh, null sep\n");
    if (incenter)
    	fprintf(outputfile,"<center>");
    if (inrightjust)
    	fprintf(outputfile,"<div>");
	}

void pagebreak(void)
	{
	if (incenter)
    	fprintf(outputfile,"</center>");
    if (inrightjust)
    	fprintf(outputfile,"</div>");
    fprintf(outputfile,"\n<br><img src=\"%s/pagebreak.gif\"><br>\n",patterndir());
    if (incenter)
    	fprintf(outputfile,"<center>");
    if (inrightjust)
    	fprintf(outputfile,"<div>");
	}

void columnbreak(void)
	{
	if (incenter)
    	fprintf(outputfile,"</center>");
    if (inrightjust)
    	fprintf(outputfile,"</div>");
    fprintf(outputfile,"\n<br><img src=\"%s/columnbreak.gif\"><br>\n",patterndir());
    if (incenter)
    	fprintf(outputfile,"<center>");
    if (inrightjust)
    	fprintf(outputfile,"<div>");
	}

void check_auto_color(chp *achp)
    {
    /*
    if the foreground color is auto (black basically) then see if the bg is a conflicting
    color
    */
	if ( (!strcmp(backgroundcolor,"#000000")) && ( (achp->color[0] == '\0') || (!(strcmp(achp->color,"#000000"))))	)
		{
		error(erroroutput,"black on black\n");
		strcpy(achp->color,"#ffffff");
		}
	else if ( (!strcmp(backgroundcolor,"#000078")) && ( (achp->color[0] == '\0') || (!(strcmp(achp->color,"#000000"))))     )
		{
		error(erroroutput,"black on blue\n");
		strcpy(achp->color,"#ffffff");
		}
    }

void extract_sttbf(STTBF *bookmarks,FILE *tablefd,U32 fcSttbf,U32 lcbSttbf)
	{
	/*extract_sttbf(portions->bookmarks,tablefd,fcSttbfbkmk,lcbSttbfbkmk);*/
	int i,j;
	U16 temp,charlen;

	bookmarks->extra_bytes=NULL;
	bookmarks->chars = NULL;
	bookmarks->no_of_strings=0;

	if (lcbSttbf>0)
		{
		fseek(tablefd,fcSttbf,SEEK_SET);
		error(erroroutput,"sttbf strinf offset is %x\n",fcSttbf);
		temp = read_16ubit(tablefd);
		if (temp == 0xFFFF)
			{
			bookmarks->exflag = temp;
			bookmarks->no_of_strings = read_16ubit(tablefd);
			}
		else
			{
			bookmarks->exflag =0;
			bookmarks->no_of_strings = temp;
			}
		bookmarks->extra_byte_flag = read_16ubit(tablefd);
		error(erroroutput,"there are %d strings\n",bookmarks->no_of_strings);

		bookmarks->chars = (U16 **) malloc(bookmarks->no_of_strings*sizeof(U16*));
		bookmarks->extra_bytes = (U8 **) malloc (bookmarks->no_of_strings*sizeof(U8*));
		if ((bookmarks->chars == NULL) || (bookmarks->extra_bytes == NULL))
			{
			fprintf(erroroutput,"mem problem in bookmarks\n");
			exit(-1);
			}

		for (i=0;i<bookmarks->no_of_strings;i++)
			{
			if (bookmarks->extra_byte_flag != 0)
				{
				bookmarks->extra_bytes[i] = malloc(bookmarks->extra_byte_flag * sizeof(U8));
				if (bookmarks->extra_bytes[i] == NULL)
					{
					fprintf(erroroutput,"mem problem in bookmarks\n");
					exit(-1);
					}
				}
			else
				bookmarks->extra_bytes[i]=NULL;

			if (bookmarks->exflag)
				{
				charlen = read_16ubit(tablefd);
				error(erroroutput,"got to here 1, charlen is %d\n",charlen);
				bookmarks->chars[i] = (U16*) malloc((charlen +1) * sizeof(U16));

				if (bookmarks->chars[i] == NULL)
					{
					fprintf(erroroutput,"mem problem in bookmarks\n");
					exit(-1);
					}

				for (j=0;j<charlen;j++)
					bookmarks->chars[i][j] = read_16ubit(tablefd);
				}
			else
				{
				error(erroroutput,"got to here 2\n");
				charlen = getc(tablefd);
				bookmarks->chars[i] = (U16*) malloc((charlen +1) * sizeof(U16));

				if (bookmarks->chars[i] == NULL)
					{
					fprintf(erroroutput,"mem problem in bookmarks\n");
					exit(-1);
					}
				for (j=0;j<charlen;j++)
					bookmarks->chars[i][j] = getc(tablefd);
				}

			bookmarks->chars[i][j] = '\0';
			for(j=0;j<bookmarks->extra_byte_flag;j++)
				bookmarks->extra_bytes[i][j] = getc(tablefd);
			}
		}
	}

void extract_bookm_limits(bookmark_limits *l_bookmarks,FILE *tablefd,U32 fcPlcfbkf,U32 lcbPlcfbkf, U32 fcPlcfbkl,U32 lcbPlcfbkl)
	{
	int i;
	l_bookmarks->bookmark_b_no=0;
	l_bookmarks->bookmark_b_cps=NULL;
	l_bookmarks->bookmark_b_bkfs=NULL;
	l_bookmarks->bookmark_e_no=0;
	l_bookmarks->bookmark_e_cps=NULL;
	if (lcbPlcfbkf > 0)
		{
		l_bookmarks->bookmark_b_no = (lcbPlcfbkf-4)/8;
		l_bookmarks->bookmark_b_cps = (U32 *) malloc ((l_bookmarks->bookmark_b_no+1) * sizeof(U32));
		l_bookmarks->bookmark_b_bkfs = (BKF *) malloc (l_bookmarks->bookmark_b_no * sizeof(BKF));

		if ((l_bookmarks->bookmark_b_bkfs==NULL) || (l_bookmarks->bookmark_b_cps ==NULL))
			{
			fprintf(erroroutput,"arrch, no mem for bookmarks\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcfbkf,SEEK_SET);
		for (i=0;i<l_bookmarks->bookmark_b_no+1;i++)
			{
			l_bookmarks->bookmark_b_cps[i] = read_32ubit(tablefd);
			error(erroroutput,"the bookmark b cp is %x\n",l_bookmarks->bookmark_b_cps[i]);
			}
		for (i=0;i<l_bookmarks->bookmark_b_no;i++)
			{
			l_bookmarks->bookmark_b_bkfs[i].ibkl = read_16ubit(tablefd);
			l_bookmarks->bookmark_b_bkfs[i].flags = read_16ubit(tablefd);
			}
		}

	if (lcbPlcfbkl> 0)
		{
		l_bookmarks->bookmark_e_no = (lcbPlcfbkl-4)/4;
		l_bookmarks->bookmark_e_cps = (U32 *) malloc ((l_bookmarks->bookmark_e_no+1) * sizeof(U32));

		if (l_bookmarks->bookmark_e_cps ==NULL)
			{
			fprintf(erroroutput,"arrch, no mem for bookmarks\n");
			exit(-1);
			}

		fseek(tablefd,fcPlcfbkl,SEEK_SET);
		for (i=0;i<l_bookmarks->bookmark_e_no+1;i++)
			{
			l_bookmarks->bookmark_e_cps[i] = read_32ubit(tablefd);
			error(erroroutput,"the bookmark e cp is %x\n",l_bookmarks->bookmark_e_cps[i]);
			}
		}
	}



/*this finds the beginning of a bookmark given a particular cp, adds
the beginning tag and returns the next bookmark cp*/
U32 decode_b_bookmark(bookmark_limits *l_bookmarks, STTBF *bookmarks)
	{
	int i=0;
	U16 *letter;
	while (i<l_bookmarks->bookmark_b_no)
		{
		if (l_bookmarks->bookmark_b_cps[i] == realcp)
			{
			l_bookmarks->bookmark_b_cps[i] = 0xffff;		/*mark it off the list*/
			fprintf(outputfile,"<a name=\"");
			letter = bookmarks->chars[i];
			while (*letter != '\0')
				fprintf(outputfile,"%c",*letter++);
			fprintf(outputfile,"\">");
			if (i == l_bookmarks->bookmark_b_no-1)
				return(-1);
			else
				return(l_bookmarks->bookmark_b_cps[i+1]);
			}
		i++;
		}

	i=0;
	while (i<l_bookmarks->bookmark_b_no)
		{
		if ( (l_bookmarks->bookmark_b_cps[i] != 0xffff) && (l_bookmarks->bookmark_b_cps[i] > realcp))
			return(l_bookmarks->bookmark_b_cps[i]);
		i++;
		}

	if ((l_bookmarks->bookmark_b_no) > 0)
		return(l_bookmarks->bookmark_b_cps[0]);
		
	return(-1);
	}

U32 decode_e_bookmark(bookmark_limits *l_bookmarks)
	{
	int i=0;
	
	while (i<l_bookmarks->bookmark_e_no)
		{
		if (l_bookmarks->bookmark_e_cps[i] == realcp)
			{
			l_bookmarks->bookmark_e_cps[i] = 0xffff;		/*mark it off the list*/
			fprintf(outputfile,"</A>");
			if (i == l_bookmarks->bookmark_e_no-1)
				return(-1);
			else
				return(l_bookmarks->bookmark_e_cps[i+1]);
			}
		i++;
		}

	i=0;
	while (i<l_bookmarks->bookmark_e_no)
		{
		if ((l_bookmarks->bookmark_e_cps[i] != 0xffff) && (l_bookmarks->bookmark_e_cps[i] > realcp))
			return(l_bookmarks->bookmark_e_cps[i]);
		i++;
		}
		
	if ((l_bookmarks->bookmark_e_no) > 0)
		return(l_bookmarks->bookmark_e_cps[0]);
		
	return(-1);
	}
