#include "pfaedit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "psfont.h"

struct fontparse {
    FontDict *fd;
    /* always in font data */
    unsigned int infi:1;
    unsigned int inchars:1;
    unsigned int inprivate:1;
    unsigned int insubs:1;
    unsigned int inothersubs:1;
    unsigned int inencoding: 1;
};

static void copyenc(char *encoding[256],char *std[256]) {
    int i;
    for ( i=0; i<256; ++i )
	encoding[i] = strdup(std[i]);
}

char *AdobeStandardEncoding[] = {
/* 0000 */	".notdef",
/* 0001 */	".notdef",
/* 0002 */	".notdef",
/* 0003 */	".notdef",
/* 0004 */	".notdef",
/* 0005 */	".notdef",
/* 0006 */	".notdef",
/* 0007 */	".notdef",
/* 0008 */	".notdef",
/* 0009 */	".notdef",
/* 000a */	".notdef",
/* 000b */	".notdef",
/* 000c */	".notdef",
/* 000d */	".notdef",
/* 000e */	".notdef",
/* 000f */	".notdef",
/* 0010 */	".notdef",
/* 0011 */	".notdef",
/* 0012 */	".notdef",
/* 0013 */	".notdef",
/* 0014 */	".notdef",
/* 0015 */	".notdef",
/* 0016 */	".notdef",
/* 0017 */	".notdef",
/* 0018 */	".notdef",
/* 0019 */	".notdef",
/* 001a */	".notdef",
/* 001b */	".notdef",
/* 001c */	".notdef",
/* 001d */	".notdef",
/* 001e */	".notdef",
/* 001f */	".notdef",
/* 0020 */	"space",
/* 0021 */	"exclam",
/* 0022 */	"quotedbl",
/* 0023 */	"numbersign",
/* 0024 */	"dollar",
/* 0025 */	"percent",
/* 0026 */	"ampersand",
/* 0027 */	"quotesingle",
/* 0028 */	"parenleft",
/* 0029 */	"parenright",
/* 002a */	"asterisk",
/* 002b */	"plus",
/* 002c */	"comma",
/* 002d */	"hyphen",
/* 002e */	"period",
/* 002f */	"slash",
/* 0030 */	"zero",
/* 0031 */	"one",
/* 0032 */	"two",
/* 0033 */	"three",
/* 0034 */	"four",
/* 0035 */	"five",
/* 0036 */	"six",
/* 0037 */	"seven",
/* 0038 */	"eight",
/* 0039 */	"nine",
/* 003a */	"colon",
/* 003b */	"semicolon",
/* 003c */	"less",
/* 003d */	"equal",
/* 003e */	"greater",
/* 003f */	"question",
/* 0040 */	"at",
/* 0041 */	"A",
/* 0042 */	"B",
/* 0043 */	"C",
/* 0044 */	"D",
/* 0045 */	"E",
/* 0046 */	"F",
/* 0047 */	"G",
/* 0048 */	"H",
/* 0049 */	"I",
/* 004a */	"J",
/* 004b */	"K",
/* 004c */	"L",
/* 004d */	"M",
/* 004e */	"N",
/* 004f */	"O",
/* 0050 */	"P",
/* 0051 */	"Q",
/* 0052 */	"R",
/* 0053 */	"S",
/* 0054 */	"T",
/* 0055 */	"U",
/* 0056 */	"V",
/* 0057 */	"W",
/* 0058 */	"X",
/* 0059 */	"Y",
/* 005a */	"Z",
/* 005b */	"bracketleft",
/* 005c */	"backslash",
/* 005d */	"bracketright",
/* 005e */	"asciicircum",
/* 005f */	"underscore",
/* 0060 */	"grave",
/* 0061 */	"a",
/* 0062 */	"b",
/* 0063 */	"c",
/* 0064 */	"d",
/* 0065 */	"e",
/* 0066 */	"f",
/* 0067 */	"g",
/* 0068 */	"h",
/* 0069 */	"i",
/* 006a */	"j",
/* 006b */	"k",
/* 006c */	"l",
/* 006d */	"m",
/* 006e */	"n",
/* 006f */	"o",
/* 0070 */	"p",
/* 0071 */	"q",
/* 0072 */	"r",
/* 0073 */	"s",
/* 0074 */	"t",
/* 0075 */	"u",
/* 0076 */	"v",
/* 0077 */	"w",
/* 0078 */	"x",
/* 0079 */	"y",
/* 007a */	"z",
/* 007b */	"braceleft",
/* 007c */	"bar",
/* 007d */	"braceright",
/* 007e */	"asciitilde",
/* 007f */	".notdef",
/* 0080 */	".notdef",
/* 0081 */	".notdef",
/* 0082 */	".notdef",
/* 0083 */	".notdef",
/* 0084 */	".notdef",
/* 0085 */	".notdef",
/* 0086 */	".notdef",
/* 0087 */	".notdef",
/* 0088 */	".notdef",
/* 0089 */	".notdef",
/* 008a */	".notdef",
/* 008b */	".notdef",
/* 008c */	".notdef",
/* 008d */	".notdef",
/* 008e */	".notdef",
/* 008f */	".notdef",
/* 0090 */	".notdef",
/* 0091 */	".notdef",
/* 0092 */	".notdef",
/* 0093 */	".notdef",
/* 0094 */	".notdef",
/* 0095 */	".notdef",
/* 0096 */	".notdef",
/* 0097 */	".notdef",
/* 0098 */	".notdef",
/* 0099 */	".notdef",
/* 009a */	".notdef",
/* 009b */	".notdef",
/* 009c */	".notdef",
/* 009d */	".notdef",
/* 009e */	".notdef",
/* 009f */	".notdef",
/* 00a0 */	".notdef",
/* 00a1 */	"exclamdown",
/* 00a2 */	"cent",
/* 00a3 */	"sterling",
/* 00a4 */	"fraction",
/* 00a5 */	"yen",
/* 00a6 */	"florin",
/* 00a7 */	"section",
/* 00a8 */	"currency",
/* 00a9 */	"quotesingle",
/* 00aa */	"quotedblleft",
/* 00ab */	"guillemotleft",
/* 00ac */	"guilsinglleft",
/* 00ad */	"guilsinglright",
/* 00ae */	"fi",
/* 00af */	"fl",
/* 00b0 */	".notdef",
/* 00b1 */	"endash",
/* 00b2 */	"dagger",
/* 00b3 */	"dbldagger",
/* 00b4 */	"periodcentered",
/* 00b5 */	".notdef",
/* 00b6 */	"paragraph",
/* 00b7 */	"bullet",
/* 00b8 */	"quotesinglbase",
/* 00b9 */	"quotedblbase",
/* 00ba */	"quotedblright",
/* 00bb */	"guillemotright",
/* 00bc */	"elipsis",
/* 00bd */	"perthousand",
/* 00be */	".notdef",
/* 00bf */	"questiondown",
/* 00c0 */	".notdef",
/* 00c1 */	"grave",
/* 00c2 */	"acute",
/* 00c3 */	"circumflex",
/* 00c4 */	"tilde",
/* 00c5 */	"macron",
/* 00c6 */	"breve",
/* 00c7 */	"dotaccent",
/* 00c8 */	"dieresis",
/* 00c9 */	".notdef",
/* 00ca */	"ring",
/* 00cb */	"cedilla",
/* 00cc */	".notdef",
/* 00cd */	"hungarianumlaut",
/* 00ce */	"ogonek",
/* 00cf */	"caron",
/* 00d0 */	"emdash",
/* 00d1 */	".notdef",
/* 00d2 */	".notdef",
/* 00d3 */	".notdef",
/* 00d4 */	".notdef",
/* 00d5 */	".notdef",
/* 00d6 */	".notdef",
/* 00d7 */	".notdef",
/* 00d8 */	".notdef",
/* 00d9 */	".notdef",
/* 00da */	".notdef",
/* 00db */	".notdef",
/* 00dc */	".notdef",
/* 00dd */	".notdef",
/* 00de */	".notdef",
/* 00df */	".notdef",
/* 00e0 */	".notdef",
/* 00e1 */	"AE",
/* 00e2 */	".notdef",
/* 00e3 */	"ordfeminine",
/* 00e4 */	".notdef",
/* 00e5 */	".notdef",
/* 00e6 */	".notdef",
/* 00e7 */	".notdef",
/* 00e8 */	"Lslash",
/* 00e9 */	"Oslash",
/* 00ea */	"OE",
/* 00eb */	"ordmasculine",
/* 00ec */	".notdef",
/* 00ed */	".notdef",
/* 00ee */	".notdef",
/* 00ef */	".notdef",
/* 00f0 */	".notdef",
/* 00f1 */	"ae",
/* 00f2 */	".notdef",
/* 00f3 */	".notdef",
/* 00f4 */	".notdef",
/* 00f5 */	"dotlessi",
/* 00f6 */	".notdef",
/* 00f7 */	".notdef",
/* 00f8 */	"lslash",
/* 00f9 */	"oslash",
/* 00fa */	"oe",
/* 00fb */	"germandbls",
/* 00fc */	".notdef",
/* 00fd */	".notdef",
/* 00fe */	".notdef",
/* 00ff */	".notdef"
};
static void setStdEnc(char *encoding[256]) {
    copyenc(encoding,AdobeStandardEncoding);
}

static void setLatin1Enc(char *encoding[256]) {
    static char *latin1enc[] = {
/* 0000 */	".notdef",
/* 0001 */	".notdef",
/* 0002 */	".notdef",
/* 0003 */	".notdef",
/* 0004 */	".notdef",
/* 0005 */	".notdef",
/* 0006 */	".notdef",
/* 0007 */	".notdef",
/* 0008 */	".notdef",
/* 0009 */	".notdef",
/* 000a */	".notdef",
/* 000b */	".notdef",
/* 000c */	".notdef",
/* 000d */	".notdef",
/* 000e */	".notdef",
/* 000f */	".notdef",
/* 0010 */	".notdef",
/* 0011 */	".notdef",
/* 0012 */	".notdef",
/* 0013 */	".notdef",
/* 0014 */	".notdef",
/* 0015 */	".notdef",
/* 0016 */	".notdef",
/* 0017 */	".notdef",
/* 0018 */	".notdef",
/* 0019 */	".notdef",
/* 001a */	".notdef",
/* 001b */	".notdef",
/* 001c */	".notdef",
/* 001d */	".notdef",
/* 001e */	".notdef",
/* 001f */	".notdef",
/* 0020 */	"space",
/* 0021 */	"exclam",
/* 0022 */	"quotedbl",
/* 0023 */	"numbersign",
/* 0024 */	"dollar",
/* 0025 */	"percent",
/* 0026 */	"ampersand",
/* 0027 */	"quotesingle",
/* 0028 */	"parenleft",
/* 0029 */	"parenright",
/* 002a */	"asterisk",
/* 002b */	"plus",
/* 002c */	"comma",
/* 002d */	"hyphen",
/* 002e */	"period",
/* 002f */	"slash",
/* 0030 */	"zero",
/* 0031 */	"one",
/* 0032 */	"two",
/* 0033 */	"three",
/* 0034 */	"four",
/* 0035 */	"five",
/* 0036 */	"six",
/* 0037 */	"seven",
/* 0038 */	"eight",
/* 0039 */	"nine",
/* 003a */	"colon",
/* 003b */	"semicolon",
/* 003c */	"less",
/* 003d */	"equal",
/* 003e */	"greater",
/* 003f */	"question",
/* 0040 */	"at",
/* 0041 */	"A",
/* 0042 */	"B",
/* 0043 */	"C",
/* 0044 */	"D",
/* 0045 */	"E",
/* 0046 */	"F",
/* 0047 */	"G",
/* 0048 */	"H",
/* 0049 */	"I",
/* 004a */	"J",
/* 004b */	"K",
/* 004c */	"L",
/* 004d */	"M",
/* 004e */	"N",
/* 004f */	"O",
/* 0050 */	"P",
/* 0051 */	"Q",
/* 0052 */	"R",
/* 0053 */	"S",
/* 0054 */	"T",
/* 0055 */	"U",
/* 0056 */	"V",
/* 0057 */	"W",
/* 0058 */	"X",
/* 0059 */	"Y",
/* 005a */	"Z",
/* 005b */	"bracketleft",
/* 005c */	"backslash",
/* 005d */	"bracketright",
/* 005e */	"asciicircum",
/* 005f */	"underscore",
/* 0060 */	"grave",
/* 0061 */	"a",
/* 0062 */	"b",
/* 0063 */	"c",
/* 0064 */	"d",
/* 0065 */	"e",
/* 0066 */	"f",
/* 0067 */	"g",
/* 0068 */	"h",
/* 0069 */	"i",
/* 006a */	"j",
/* 006b */	"k",
/* 006c */	"l",
/* 006d */	"m",
/* 006e */	"n",
/* 006f */	"o",
/* 0070 */	"p",
/* 0071 */	"q",
/* 0072 */	"r",
/* 0073 */	"s",
/* 0074 */	"t",
/* 0075 */	"u",
/* 0076 */	"v",
/* 0077 */	"w",
/* 0078 */	"x",
/* 0079 */	"y",
/* 007a */	"z",
/* 007b */	"braceleft",
/* 007c */	"bar",
/* 007d */	"braceright",
/* 007e */	"asciitilde",
/* 007f */	".notdef",
/* 0080 */	".notdef",
/* 0081 */	".notdef",
/* 0082 */	".notdef",
/* 0083 */	".notdef",
/* 0084 */	".notdef",
/* 0085 */	".notdef",
/* 0086 */	".notdef",
/* 0087 */	".notdef",
/* 0088 */	".notdef",
/* 0089 */	".notdef",
/* 008a */	".notdef",
/* 008b */	".notdef",
/* 008c */	".notdef",
/* 008d */	".notdef",
/* 008e */	".notdef",
/* 008f */	".notdef",
/* 0090 */	"dotlessi",		/* Um, Adobe's Latin1 has some extra chars */
/* 0091 */	"grave",
/* 0092 */	"accute",		/* This is a duplicate... */
/* 0093 */	"circumflex",
/* 0094 */	"tilde",
/* 0095 */	"macron",
/* 0096 */	"breve",
/* 0097 */	"dotaccent",
/* 0098 */	"dieresis",
/* 0099 */	".notdef",
/* 009a */	"ring",
/* 009b */	"cedilla",
/* 009c */	".notdef",
/* 009d */	"hungarumlaut",
/* 009e */	"ogonek",
/* 009f */	"caron",
/* 00a0 */	"space",
/* 00a1 */	"exclamdown",
/* 00a2 */	"cent",
/* 00a3 */	"sterling",
/* 00a4 */	"currency",
/* 00a5 */	"yen",
/* 00a6 */	"brokenbar",
/* 00a7 */	"section",
/* 00a8 */	"dieresis",
/* 00a9 */	"copyright",
/* 00aa */	"ordfeminine",
/* 00ab */	"guillemotleft",
/* 00ac */	"logicalnot",
/* 00ad */	"hyphen",
/* 00ae */	"registered",
/* 00af */	"macron",
/* 00b0 */	"degree",
/* 00b1 */	"plusminus",
/* 00b2 */	"twosuperior",
/* 00b3 */	"threesuperior",
/* 00b4 */	"acute",
/* 00b5 */	"mu",
/* 00b6 */	"paragraph",
/* 00b7 */	"periodcentered",
/* 00b8 */	"cedilla",
/* 00b9 */	"onesuperior",
/* 00ba */	"ordmasculine",
/* 00bb */	"guillemotright",
/* 00bc */	"onequarter",
/* 00bd */	"onehalf",
/* 00be */	"threequarters",
/* 00bf */	"questiondown",
/* 00c0 */	"Agrave",
/* 00c1 */	"Aacute",
/* 00c2 */	"Acircumflex",
/* 00c3 */	"Atilde",
/* 00c4 */	"Adieresis",
/* 00c5 */	"Aring",
/* 00c6 */	"AE",
/* 00c7 */	"Ccedilla",
/* 00c8 */	"Egrave",
/* 00c9 */	"Eacute",
/* 00ca */	"Ecircumflex",
/* 00cb */	"Edieresis",
/* 00cc */	"Igrave",
/* 00cd */	"Iacute",
/* 00ce */	"Icircumflex",
/* 00cf */	"Idieresis",
/* 00d0 */	"Eth",
/* 00d1 */	"Ntilde",
/* 00d2 */	"Ograve",
/* 00d3 */	"Oacute",
/* 00d4 */	"Ocircumflex",
/* 00d5 */	"Otilde",
/* 00d6 */	"Odieresis",
/* 00d7 */	"multiply",
/* 00d8 */	"Oslash",
/* 00d9 */	"Ugrave",
/* 00da */	"Uacute",
/* 00db */	"Ucircumflex",
/* 00dc */	"Udieresis",
/* 00dd */	"Yacute",
/* 00de */	"Thorn",
/* 00df */	"germandbls",
/* 00e0 */	"agrave",
/* 00e1 */	"aacute",
/* 00e2 */	"acircumflex",
/* 00e3 */	"atilde",
/* 00e4 */	"adieresis",
/* 00e5 */	"aring",
/* 00e6 */	"ae",
/* 00e7 */	"ccedilla",
/* 00e8 */	"egrave",
/* 00e9 */	"eacute",
/* 00ea */	"ecircumflex",
/* 00eb */	"edieresis",
/* 00ec */	"igrave",
/* 00ed */	"iacute",
/* 00ee */	"icircumflex",
/* 00ef */	"idieresis",
/* 00f0 */	"eth",
/* 00f1 */	"ntilde",
/* 00f2 */	"ograve",
/* 00f3 */	"oacute",
/* 00f4 */	"ocircumflex",
/* 00f5 */	"otilde",
/* 00f6 */	"odieresis",
/* 00f7 */	"divide",
/* 00f8 */	"oslash",
/* 00f9 */	"ugrave",
/* 00fa */	"uacute",
/* 00fb */	"ucircumflex",
/* 00fc */	"udieresis",
/* 00fd */	"yacute",
/* 00fe */	"thorn",
/* 00ff */	"ydieresis"
    };
    copyenc(encoding,latin1enc);
}

static char *getstring(char *start) {
    char *end, *ret;
    int parencnt=0;

    while ( *start!='\0' && *start!='(' ) ++start;
    if ( *start=='(' ) ++start;
    for ( end = start; *end!='\0' && (*end!=')' || parencnt>0); ++end )
	if ( *end=='(' ) ++parencnt;
	else if ( *end==')' ) --parencnt;
    ret = malloc(end-start+1);
    if ( end>start )
	strncpy(ret,start,end-start);
    ret[end-start] = '\0';
return( ret );
}

static char *gettoken(char *start) {
    char *end, *ret;

    while ( *start!='\0' && *start!='/' ) ++start;
    if ( *start=='/' ) ++start;
    for ( end = start; *end!='\0' && !isspace(*end) && *end!='[' && *end!='/' && *end!='{' && *end!='(' ; ++end );
    ret = malloc(end-start+1);
    if ( end>start )
	strncpy(ret,start,end-start);
    ret[end-start] = '\0';
return( ret );
}

static int getbool(char *start) {

    while ( isspace(*start) ) ++start;
    if ( *start=='T' || *start=='t' )
return( 1 );

return( 0 );
}

static void fillintarray(int *array,char *start,int maxentries) {
    int i;
    char *end;

    while ( *start!='\0' && *start!='[' && *start!='{' ) ++start;
    if ( *start=='[' || *start=='{' ) ++start;
    for ( i=0; i<maxentries && *start!=']' && *start!='}'; ++i ) {
	array[i] = (int) strtod(start,&end);
	if ( start==end )
return;
	start = end;
	while ( isspace(*start) ) ++start;
    }
}

static void filldoublearray(double *array,char *start,int maxentries) {
    int i;
    char *end;

    while ( *start!='\0' && *start!='[' && *start!='{' ) ++start;
    if ( *start=='[' || *start=='{' ) ++start;
    for ( i=0; i<maxentries && *start!=']' && *start!='}'; ++i ) {
	array[i] = strtod(start,&end);
	if ( start==end )
return;
	start = end;
	while ( isspace(*start) ) ++start;
    }
}

static void InitChars(struct chars *chars,char *line) {
    while ( *line!='/' && *line!='\0' ) ++line;
    while ( !isspace(*line) && *line!='\0' ) ++line;
    chars->cnt = strtol(line,NULL,10);
    if ( chars->cnt>0 ) {
	chars->keys = calloc(chars->cnt,sizeof(char *));
	chars->values = calloc(chars->cnt,sizeof(char *));
	chars->lens = calloc(chars->cnt,sizeof(int));
    }
}

static int mycmp(char *str,char *within, char *end ) {
    while ( within<end ) {
	if ( *str!=*within )
return( *str-*within );
	++str; ++within;
    }
return( *str=='\0'?0:1 );
}

static char *myfgets(char *str, int len, FILE *file) {
    char *pt, *end;
    int ch;

    for ( pt = str, end = str+len-1; pt<end && (ch=getc(file))!=EOF && ch!='\r' && ch!='\n';
	*pt++ = ch );
    if ( ch=='\n' )
	*pt++ = '\n';
    else if ( ch=='\r' ) {
	*pt++ = '\n';
	if ((ch=getc(file))!='\n' )
	    ungetc(ch,file);
    }
    if ( pt==str )
return( NULL );
    *pt = '\0';
return( str );
}

/* find the first occurence of a three letter token on the line;
   make sure we only find whole tokens
*/
static char * find_token3(char * line, char *tok, int start_of_line)
{
   char *d1, *d2, *d3, *d4;

   /* we will allow for space or new line to start/end the token */
   char t1[6] = "     ";
   char t2[6] = "    \n";
   char t3[5] = "    ";   /* the last two are only allowed at the start of a line */
   char t4[5] = "   \n";

   t1[1] = t2[1] = t3[0] = t4[0] = tok[0];
   t1[2] = t2[2] = t3[1] = t4[1] = tok[1];
   t1[3] = t2[3] = t3[2] = t4[2] = tok[2];

   /* make sure that not found strings do not figure in the comparison later on;
      and skip leading whitespace
   */

   d1 = strstr(line, t1);
   d1 = d1 ? d1 + 1 : (char *)0xFFFFFFFF;

   d2 = strstr(line, t2);
   d2 = d2 ? d2 + 1 : (char *)0xFFFFFFFF;

   d3 = strncmp(line, t3, 4)? (char*) 0xFFFFFFFF : line;

   d4 = strcmp(line, t4)? (char*) 0xFFFFFFFF : line;

   if(d2 < d1)
      d1 = d2;
   if(d3 < d1)
      d1 = d3;
   if(d4 < d1)
      d1 = d4;

   d1 = (d1 == (char *) 0xFFFFFFFF) ? 0 : d1;
   return d1;
}

/* retrieve the numerical index and character name starting at position *pos,
   and store the name in buff, returning the numerical value
*/
static int get_num_and_name(char ** pos, char *buff, char * line, FILE *in)
{
   char * pt;
   int indx;

   /* skip any whitespace */
   while ( isspace( **pos )) ++*pos;

   /* is this the end of line? */
   if(!**pos)
   {
      /* OK, the number is on the next line, get it */
      myfgets(line,256,in);
      *pos = line;
   }

   /* the number should follow */
   indx = strtol(*pos, pos,10);

   /* skip any futher whitespace */
   while ( isspace( **pos )) ++*pos;

   /* is this the end of line? */
   if(!**pos)
   {
      /* the name is on the next line, go and get it */
      myfgets(line,256,in);
      *pos = line;
   }

   /* skip any futher whitespace */
   while ( isspace( **pos )) ++*pos;

   /* the name should follow */
   if ( **pos=='/' ) ++*pos;
   else
   {
      /* this is a syntax error in the pfa file, just quit */
      printf("Error: Syntax error in the Encoding table. Quiting\n");
      exit(1);
   }

   /* copy the name into the buffer and null-terminate it */
   for ( pt = buff; !isspace(**pos); *pt++ = **pos, (*pos)++);

   *pt = '\0';

   return indx;
}

/* retrieve the encoding vector, allowing for it not being written a character
   per line
*/
static void parse_encoding(struct fontparse *fp, char *line, FILE *in)
{
   char * d;
   char * pos = line;
   int indx;
   int start_of_line = 1;
   char buff[200];
   int line_count = 0;

   while (1)
   {
      /* find the next "dup" token */
      d = find_token3(pos, "dup", start_of_line);

      if(d)
      {
         /* we found the dup token, so we move past the first whitespace */
         pos = d + 3;
         start_of_line = 0;
      }
      else
      {
         /*    no dup token, see if we can find a def token, in which
               case this is the last line of the encoding, and we quit
               otherwise get the next line and continue from the start
               but first see if this is not one of the weird fonts ...
         */
         if ( start_of_line && strncmp(line,"0 1 255",7)==0 )
         {
         	/* the T1 spec I've got doesn't allow for this, but I've seen it anyway*/
           	/* 0 1 255 {1 index exch /.notdef put} for */
      	   int i;
         	for ( i=0; i<256; ++i )
               fp->fd->encoding[i] = strdup(".notdef");
            return;
         }

         d = find_token3(pos, "def", start_of_line);
         if(d)
            break;
         else
         {
            /* OK, we did not find a dup token, nor a def token, so we will try a new line;
               however, to avoid endless loop in mallformed tables, we will keep count of
               the lines, there should be 258 at most.
            */
            myfgets(line,256,in);
            pos = line;
            start_of_line = 1;
            if(line_count++ > 258)
            {
               printf("Error: corrupt encoding table; bailing out after line %d\n", line_count);
               exit(1);
            }
            continue;
         }
      }

      /* now retrieve the number and name */
      indx = get_num_and_name(&pos, buff, line, in);

      /* if the returned value makes sense then store it */
     	if ( indx>=0 && indx<256 )
		{
         fp->fd->encoding[indx] = strdup(buff);
         /*printf("DEBUG: Enc. vector index [%03d], character [%s]\n", indx, buff);*/
		}
      	else
      	{
			printf("Error: encoding index out of bounds, value %d\n", indx);
			exit(1);
		}
	}
}


static void parseline(struct fontparse *fp,char *line, FILE *in) {
    char *endtok;
   /* the original implementation had some code here dealing with the
      encoding vector, but it was now replaced by parse_encoding()
      as a result we need to be able to pass the input file pointer
      through, but it is only used if the call is from withing doubledecrypt
	  so we can safely default it to 0
   */

    fp->inencoding = 0;

    while ( isspace(*line)) ++line;
    endtok = NULL;
    if ( *line=='/' )
	for ( endtok=line+1; !isspace(*endtok) && *endtok!='(' &&
		*endtok!='{' && *endtok!='[' && *endtok!='\0'; ++endtok );
    if ( fp->infi ) {
	if ( endtok==NULL && strncmp(line,"end", 3)==0 ) {
	    fp->infi=0;
return;
	} else if ( endtok==NULL )
return;
	if ( mycmp("version",line+1,endtok)==0 )
	    fp->fd->fontinfo->version = getstring(endtok);
	else if ( mycmp("Notice",line+1,endtok)==0 )
	    fp->fd->fontinfo->notice = getstring(endtok);
	else if ( mycmp("FullName",line+1,endtok)==0 )
	    fp->fd->fontinfo->fullname = getstring(endtok);
	else if ( mycmp("FamilyName",line+1,endtok)==0 )
	    fp->fd->fontinfo->familyname = getstring(endtok);
	else if ( mycmp("Weight",line+1,endtok)==0 )
	    fp->fd->fontinfo->weight = getstring(endtok);
	else if ( mycmp("ItalicAngle",line+1,endtok)==0 )
	    fp->fd->fontinfo->italicangle = strtod(endtok,NULL);
	else if ( mycmp("UnderlinePosition",line+1,endtok)==0 )
	    fp->fd->fontinfo->underlineposition = strtod(endtok,NULL);
	else if ( mycmp("UnderlineThickness",line+1,endtok)==0 )
	    fp->fd->fontinfo->underlinethickness = strtod(endtok,NULL);
	else if ( mycmp("isFixedPitch",line+1,endtok)==0 )
	    fp->fd->fontinfo->isfixedpitch = getbool(endtok);
	else if ( mycmp("em",line+1,endtok)==0 )
	    fp->fd->fontinfo->em = strtol(endtok,NULL,10);
	else if ( mycmp("ascent",line+1,endtok)==0 )
	    fp->fd->fontinfo->ascent = strtol(endtok,NULL,10);
	else if ( mycmp("descent",line+1,endtok)==0 )
	    fp->fd->fontinfo->descent = strtol(endtok,NULL,10);
	else
	    fprintf( stderr, "Didn't understand |%s", line );
    } else if ( fp->inprivate ) {
	if ( strstr(line,"/CharStrings")!=NULL ) {
	    InitChars(fp->fd->chars,line);
	    fp->inchars = 1;
	    fp->insubs = fp->inothersubs = 0;
return;
	} else if ( strstr(line,"/Subrs")!=NULL ) {
	    InitChars(fp->fd->private->subrs,line);
	    fp->insubs = 1;
	    fp->inchars = fp->inothersubs = 0;
return;
	} else if ( strstr(line,"/OtherSubrs")!=NULL ) {
	    InitChars(fp->fd->private->othersubrs,line);
	    fp->inothersubs = 1;
	    fp->inchars = fp->insubs = 0;
return;
	}
	if ( endtok==NULL )
return;
	if ( mycmp("BlueFuzz",line+1,endtok)==0 )
	    fp->fd->private->bluefuzz = strtol(endtok,NULL,10);
	else if ( mycmp("BlueScale",line+1,endtok)==0 )
	    fp->fd->private->bluescale = strtod(endtok,NULL);
	else if ( mycmp("BlueShift",line+1,endtok)==0 )
	    fp->fd->private->blueshift = strtol(endtok,NULL,10);
	else if ( mycmp("BlueValues",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->bluevalues,endtok,14);
	else if ( mycmp("ExpansionFactor",line+1,endtok)==0 )
	    fp->fd->private->expansionfactor = strtod(endtok,NULL);
	else if ( mycmp("FamilyBlues",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->familyblues,endtok,14);
	else if ( mycmp("FamilyOtherBlues",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->familyotherblues,endtok,10);
	else if ( mycmp("ForceBold",line+1,endtok)==0 )
	    fp->fd->private->forcebold = getbool(endtok);
	else if ( mycmp("LanguageGroup",line+1,endtok)==0 )
	    fp->fd->private->languagegroup = strtol(endtok,NULL,10);
	else if ( mycmp("lenIV",line+1,endtok)==0 )
	    fp->fd->private->leniv = strtol(endtok,NULL,10);
	else if ( mycmp("MinFeature",line+1,endtok)==0 )
	    fp->fd->private->minfeature = strdup("{16 16}");
	else if ( mycmp("ND",line+1,endtok)==0 || mycmp("|-",line+1,endtok)==0 )
	    fp->fd->private->nd = strdup("{noaccess def}");
	else if ( mycmp("NP",line+1,endtok)==0 || mycmp("|",line+1,endtok)==0 )
	    fp->fd->private->np = strdup("{noaccess put}");
	else if ( mycmp("OtherBlues",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->otherblues,endtok,10);
	else if ( mycmp("password",line+1,endtok)==0 )
	    fp->fd->private->password = 5839;
	else if ( mycmp("RD",line+1,endtok)==0 || mycmp("-|",line+1,endtok)==0 )
	    fp->fd->private->rd = strdup("{string currentfile exch readstring pop}");
	else if ( mycmp("RndStemUp",line+1,endtok)==0 )
	    fp->fd->private->rndstemup = getbool(endtok);
	else if ( mycmp("StdHW",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->stdhw,endtok,1);
	else if ( mycmp("StdVW",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->stdvw,endtok,1);
	else if ( mycmp("StemSnapH",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->stemsnaph,endtok,12);
	else if ( mycmp("StemSnapV",line+1,endtok)==0 )
	     filldoublearray(fp->fd->private->stemsnapv,endtok,12);
	else if ( mycmp("UniqueID",line+1,endtok)==0 )
	    fp->fd->private->uniqueid = strtol(endtok,NULL,10);
	else
	    fprintf( stderr, "Didn't understand |%s", line );
    } else {
	if ( strstr(line,"/Private")!=NULL ) {
	    fp->inprivate = 1;
return;
	} else if ( strstr(line,"/FontInfo")!=NULL ) {
	    fp->infi = 1;
return;
	} else if ( strstr(line,"/CharStrings")!=NULL ) {
	    InitChars(fp->fd->chars,line);
	    fp->inchars = 1;
	    fp->insubs = fp->inothersubs = 0;
return;
	}
	if ( endtok==NULL )
return;
	if ( mycmp("FontName",line+1,endtok)==0 )
	    fp->fd->fontname = gettoken(endtok);
	else if ( mycmp("Encoding",line+1,endtok)==0 ) {
	    if ( strstr(endtok,"StandardEncoding")!=NULL )
		setStdEnc(fp->fd->encoding);
	    else if ( strstr(endtok,"ISOLatin1Encoding")!=NULL )
		setLatin1Enc(fp->fd->encoding);
	    else
         /* unfortunately the original implementation assumed that the encoding
            vector is one character per line and this is not always so; the
            function below will take care of this
         */
         parse_encoding(fp, line, in);
		   /*fp->inencoding = 1;*/
	} else if ( mycmp("PaintType",line+1,endtok)==0 )
	    fp->fd->painttype = strtol(endtok,NULL,10);
	else if ( mycmp("FontType",line+1,endtok)==0 )
	    fp->fd->fonttype = strtol(endtok,NULL,10);
	else if ( mycmp("FontMatrix",line+1,endtok)==0 )
	     filldoublearray(fp->fd->fontmatrix,endtok,6);
	else if ( mycmp("LanguageLevel",line+1,endtok)==0 )
	    fp->fd->languagelevel = strtol(endtok,NULL,10);
	else if ( mycmp("WMode",line+1,endtok)==0 )
	    fp->fd->wmode = strtol(endtok,NULL,10);
	else if ( mycmp("FontBBox",line+1,endtok)==0 )
	     filldoublearray(fp->fd->fontbb,endtok,4);
	else if ( mycmp("UniqueID",line+1,endtok)==0 )
	    fp->fd->uniqueid = strtol(endtok,NULL,10);
	else if ( mycmp("XUID",line+1,endtok)==0 )
	     fillintarray(fp->fd->xuid,endtok,4);
	else if ( mycmp("StrokeWidth",line+1,endtok)==0 )
	    fp->fd->strokewidth = strtod(endtok,NULL);
	else
	    fprintf( stderr, "Didn't understand |%s", line );
    }
}


static int hex(int ch1, int ch2) {
    if ( ch1>='0' && ch1<='9' )
	ch1 -= '0';
    else if ( ch1>='a' )
	ch1 -= 'a'-10;
    else
	ch1 -= 'A'-10;
    if ( ch2>='0' && ch2<='9' )
	ch2 -= '0';
    else if ( ch2>='a' )
	ch2 -= 'a'-10;
    else
	ch2 -= 'A'-10;
return( (ch1<<4)|ch2 );
}

unsigned short r;
#define c1	52845
#define c2	22719

static void initcode() {
    r = 55665;
}

static int decode(unsigned char cypher) {
    unsigned char plain = ( cypher ^ (r>>8));
    r = (cypher + r) * c1 + c2;
return( plain );
}

static void dumpzeros(FILE *out, unsigned char *zeros, int zcnt) {
    while ( --zcnt >= 0 )
	fputc(*zeros++,out);
}

static void decodestr(unsigned char *str, int len) {
    unsigned short r = 4330;
    unsigned char plain, cypher;

    while ( len-->0 ) {
	cypher = *str;
	plain = ( cypher ^ (r>>8));
	r = (cypher + r) * c1 + c2;
	*str++ = plain;
    }
}

static void addinfo(struct fontparse *fp,char *line,char *tok,char *binstart,int binlen) {
    decodestr((unsigned char *)binstart,binlen);
    binstart += fp->fd->private->leniv;
    binlen -= fp->fd->private->leniv;

    if ( fp->insubs || fp->inothersubs ) {
	struct chars *chars = fp->insubs ? fp->fd->private->subrs : fp->fd->private->othersubrs;
	while ( isspace(*line)) ++line;
	if ( strncmp(line,"dup ",4)==0 ) {
	    int i = strtol(line+4,NULL,10);
	    if ( i<chars->cnt ) {
		chars->lens[i] = binlen;
		chars->values[i] = malloc(binlen);
		memcpy(chars->values[i],binstart,binlen);
	    } else
		fprintf( stderr, "Index too big (must be <%d) |%s", chars->cnt, line);
	} else
	    fprintf( stderr, "Didn't understand |%s", line );
    } else if ( fp->inchars ) {
	struct chars *chars = fp->fd->chars;
	if ( *tok=='\0' )
	    fprintf( stderr, "No name for CharStrings dictionary |%s", line );
	else if ( chars->next>=chars->cnt )
	    fprintf( stderr, "Too many entries in CharStrings dictionary |%s", line );
	else {
	    int i = chars->next;
	    chars->lens[i] = binlen;
	    chars->keys[i] = strdup(tok);
	    chars->values[i] = malloc(binlen);
	    memcpy(chars->values[i],binstart,binlen);
	    ++chars->next;
	}
    } else
	fprintf( stderr, "Shouldn't be in addinfo |%s", line );
}

/* In the book the token which starts a character description is always RD but*/
/*  it's just the name of a subroutine which is defined in the private diction*/
/*  and it could be anything. in one case it was "-|" (hyphen bar) so we can't*/
/*  just look for RD we must be a bit smarter and figure out what the token is*/
/* (oh. I see now. it's allowed to be either one "RD" or "-|", but nothing else*/
/*  right) */
/* It's defined as {string currentfile exch readstring pop} so look for that */
static int glorpline(struct fontparse *fp, FILE *temp, char *rdtok, FILE *in) {
    char buffer[3000], *pt, *binstart;
    int binlen;
    int ch;
    int innum, val, inbinary, cnt, inr, wasspace, nownum, nowr, nowspace, sptok;
    char *rdline = "{string currentfile exch readstring pop}", *rpt;
    char *tokpt = NULL, *rdpt;
    char temptok[255];
    int intok, first;

    ch = getc(temp);
    if ( ch==EOF )
return( 0 );
    ungetc(ch,temp);

    innum = inr = 0; wasspace = 0; inbinary = 0; rpt = NULL; rdpt = NULL;
    pt = buffer; binstart=NULL; binlen = 0; intok=0; sptok=0; first=1;
    temptok[0] = '\0';
    while ( (ch=getc(temp))!=EOF ) {
	*pt++ = ch;
	nownum = nowspace = nowr = 0;
	if ( inbinary ) {
	    if ( --cnt==0 )
		inbinary = 0;
	} else if ( ch=='/' ) {
	    intok = 1;
	    tokpt = temptok;
	} else if ( intok && !isspace(ch) && ch!='{' && ch!='[' ) {
	    *tokpt++ = ch;
	} else if ( (intok||sptok) && (ch=='{' || ch=='[')) {
	    *tokpt = '\0';
	    rpt = rdline+1;
	    intok = 0;
	} else if ( intok ) {
	    *tokpt = '\0';
	    intok = 0;
	    sptok = 1;
	} else if ( sptok && isspace(ch)) {
	    nowspace = 1;
	    if ( ch=='\n' || ch=='\r' )
    break;
	} else if ( sptok && !isdigit(ch))
	    sptok = 0;
	else if ( rpt!=NULL && ch==*rpt ) {
	    if ( *++rpt=='\0' ) {
		/* it matched the character definition string so this is the */
		/*  token we want to search for */
		strcpy(rdtok,temptok);
	    }
	} else if ( isdigit(ch)) {
	    sptok = 0;
	    nownum = 1;
	    if ( innum )
		val = 10*val + ch-'0';
	    else
		val = ch-'0';
	} else if ( isspace(ch)) {
	    nowspace = 1;
	    if ( ch=='\n' || ch=='\r' )
    break;
	} else if ( wasspace && ch==*rdtok ) {
	    nowr = 1;
	    rdpt = rdtok+1;
	} else if ( inr && ch==*rdpt ) {
	    if ( *++rdpt =='\0' ) {
		ch = getc(temp);
		*pt++ = ch;
		if ( isspace(ch) && val!=0 ) {
		    inbinary = 1;
		    cnt = val;
		    binstart = pt;
		    binlen = val;
		    if ( binlen>sizeof(buffer)) {
			fprintf(stderr, "Buffer overflow needs to be at least %d\ndying gracefully.\n", binlen);
			exit(1);
		    }
		}
	    } else
		nowr = 1;
	}
	innum = nownum; wasspace = nowspace; inr = nowr;
	first = 0;
    }
    *pt = '\0';
    if ( binstart==NULL ) {
	parseline(fp,buffer,in);
    } else {
	addinfo(fp,buffer,temptok,binstart,binlen);
    }
return( 1 );
}

static int nrandombytes[4];
#define EODMARKLEN	16

static void decrypteexec(FILE *in,FILE *temp, int hassectionheads) {
    int ch1, ch2, ch3, ch4, binary;
    int zcnt;
    unsigned char zeros[EODMARKLEN+6+1];

    while ( (ch1=getc(in))!=EOF && isspace(ch1));
    if ( ch1==0200 && hassectionheads ) {
	/* skip the 6 byte section header in pfb files that follows eexec */
	ch1 = getc(in);
	ch1 = getc(in);
	ch1 = getc(in);
	ch1 = getc(in);
	ch1 = getc(in);
	ch1 = getc(in);
    }
    ch2 = getc(in); ch3 = getc(in); ch4 = getc(in);
    binary = 0;
    if ( ch1<'0' || (ch1>'9' && ch1<'A') || ( ch1>'F' && ch1<'a') || (ch1>'f') ||
	     ch2<'0' || (ch2>'9' && ch2<'A') || (ch2>'F' && ch2<'a') || (ch2>'f') ||
	     ch3<'0' || (ch3>'9' && ch3<'A') || (ch3>'F' && ch3<'a') || (ch3>'f') ||
	     ch4<'0' || (ch4>'9' && ch4<'A') || (ch4>'F' && ch4<'a') || (ch4>'f') )
	binary = 1;
    if ( ch1==EOF || ch2==EOF || ch3==EOF || ch4==EOF ) {
return;
    }

    initcode();
    if ( binary ) {
	nrandombytes[0] = decode(ch1);
	nrandombytes[1] = decode(ch2);
	nrandombytes[2] = decode(ch3);
	nrandombytes[3] = decode(ch4);
	zcnt = 0;
	while (( ch1=getc(in))!=EOF ) {
	    if ( hassectionheads ) {
		if ( (zcnt>=1 && zcnt<6) || ( ch1==0200 && zcnt==0 ))
		    zeros[zcnt++] = decode(ch1);
		else if ( zcnt>=6 && ch1=='0' ) {
		    zeros[zcnt++] = decode(ch1);
		    if ( zcnt>EODMARKLEN+6 )
	break;
		} else {
		    dumpzeros(temp,zeros,zcnt);
		    zcnt = 0;
		    putc(decode(ch1),temp);
		}
	    } else {
		if ( ch1=='0' ) ++zcnt; else {dumpzeros(temp,zeros,zcnt); zcnt = 0; }
		if ( zcnt>EODMARKLEN )
	break;
		if ( zcnt==0 )
		    putc(decode(ch1),temp);
		else
		    zeros[zcnt-1] = decode(ch1);
	    }
	}
    } else {
	nrandombytes[0] = decode(hex(ch1,ch2));
	nrandombytes[1] = decode(hex(ch3,ch4));
	ch1 = getc(in); ch2 = getc(in); ch3 = getc(in); ch4 = getc(in);
	nrandombytes[2] = decode(hex(ch1,ch2));
	nrandombytes[3] = decode(hex(ch3,ch4));
	zcnt = 0;
	while (( ch1=getc(in))!=EOF ) {
	    while ( ch1!=EOF && isspace(ch1)) ch1 = getc(in);
	    while ( (ch2=getc(in))!=EOF && isspace(ch2));
	    if ( ch1=='0' && ch2=='0' ) ++zcnt; else { dumpzeros(temp,zeros,zcnt); zcnt = 0;}
	    if ( zcnt>EODMARKLEN )
	break;
	    if ( zcnt==0 )
		putc(decode(hex(ch1,ch2)),temp);
	    else
		zeros[zcnt-1] = decode(hex(ch1,ch2));
	}
    }
    while (( ch1=getc(in))=='0' || isspace(ch1) );
    if ( ch1!=EOF ) ungetc(ch1,in);
}

static void decryptagain(struct fontparse *fp,FILE *temp, FILE *in) {
    char rdtok[255];
    strcpy(rdtok,"RD");
    while ( glorpline(fp,temp,rdtok,in));
}

static void doubledecrypt(struct fontparse *fp,FILE *in, FILE *temp) {
    char buffer[256];
    int first, hassectionheads;

    first = 1; hassectionheads = 0;
    while ( myfgets(buffer,sizeof(buffer),in)!=NULL ) {
	if ( first && buffer[0]=='\200' ) {
	    hassectionheads = 1;
	    parseline(fp,buffer+6,in);
	} else
	    parseline(fp,buffer,in);
	first = 0;
	if ( strstr(buffer,"currentfile")!=NULL && strstr(buffer, "eexec")!=NULL )
    break;
    }

    decrypteexec(in,temp,hassectionheads);
    rewind(temp);
    decryptagain(fp,temp,in);
    while ( myfgets(buffer,sizeof(buffer),in)!=NULL ) {
	if ( buffer[0]!='\200' || !hassectionheads )
	    parseline(fp,buffer,in);
    }
}

static struct fontdict *MakeEmptyFont(void) {
    struct fontdict *ret;

    ret = calloc(1,sizeof(struct fontdict));
    ret->fontinfo = calloc(1,sizeof(struct fontinfo));
    ret->chars = calloc(1,sizeof(struct chars));
    ret->private = calloc(1,sizeof(struct private));
    ret->private->subrs = calloc(1,sizeof(struct chars));
    ret->private->othersubrs = calloc(1,sizeof(struct chars));
    ret->private->leniv = 4;
return( ret );
}

FontDict *ReadPSFont(char *fontname) {
    FILE *in, *temp;
    struct fontparse fp;

    in = fopen(fontname,"r");
    if ( in==NULL ) {
	fprintf( stderr, "Cannot open %s\n", fontname );
return(NULL);
    }

    temp = tmpfile();
    if ( temp==NULL ) {
	fprintf( stderr, "Cannot open temporary file\n" );
	fclose(in);
return(NULL);
    }

    memset(&fp,'\0',sizeof(fp));
    fp.fd = MakeEmptyFont();
    doubledecrypt(&fp,in,temp);

    fclose(in); fclose(temp);
return( fp.fd );
}

void PSFontFree(FontDict *fd) {
}

#if 0
int main( int argc, char **argv) {
    int i;

    for ( i=1; i<argc; ++i ) {
	WritePSFont("recrypt.pfa",ReadPSFont(argv[i]),0);
    }
return( 0 );
}
#endif
