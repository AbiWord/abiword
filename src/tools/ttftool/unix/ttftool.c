#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "encoding.h"
#include "proto.h"

/*
	This pogram was created by joining together bits of code and data from
   ttf2afm, ttfps1 and ttfps. The bits of code contain some degree of overlap,
   if anyone wishes to beautify it, please go ahead ...
*/

char *FontName = 0;
char *FullName = 0;
char *Notice = 0;

TTF_LONG ItalicAngle = 0;
TTF_LONG IsFixedPitch = 0;
TTF_LONG FontBBox1 = 0;
TTF_LONG FontBBox2 = 0;
TTF_LONG FontBBox3 = 0;
TTF_LONG FontBBox4 = 0;
TTF_LONG UnderlinePosition = 0;
TTF_LONG UnderlineThickness = 0;
TTF_LONG CapHeight = 0;
TTF_LONG XHeight = 0;
TTF_LONG Ascender = 0;
TTF_LONG Descender = 0;

TTF_ULONG minMemType42 = 0;
TTF_ULONG maxMemType42 = 0;
TTF_ULONG minMemType1 = 0;
TTF_ULONG maxMemType1 = 0;

char *cur_file_name = 0;
/*char *b_name = 0;*/
FILE *fontfile, *afmfile, *unifile, *psfontfile = 0;
char enc_line[ENC_BUF_SIZE];
int print_all = 0;      /* print all glyphs? */
int print_index = 0;    /* print glyph names by index? */
int read_index = 0;     /* read names in encoding file as index? */
int print_cmap = 0;
/*int uni_to_glyph = 0;*/
char *null_name = "UNDEFINED";
char latin1enc[] = "ISOLatin1Encoding";
char standardEnc[] = "StandardEncoding";
char *cur_enc_name = standardEnc;


TTF_USHORT upem;
TTF_USHORT ntabs;
int nhmtx;
int post_format;
int loca_format;
int nglyphs;
int nkernpairs = 0;
int names_count = 0;
char *ps_glyphs_buf = 0;
dirtab_entry *dir_tab;
mtx_entry *mtx_tab;
kern_entry *kern_tab;
char *enc_names[MAX_CHAR_CODE + 1];
unsigned short int uni_map[MAX_CHAR_CODE + 1]; /* will use this to translate glyph names to unicode values to */
unsigned short int uni_to_glyph[MAX_CHAR_CODE + 1]; /* will use this to translate unicode values to glyph index*/

char notdef[] = ".notdef";

char *encoding = NULL;
extern struct enc_vector * known_encodings[];
struct enc_vector * curr_encoding = NULL;

int verbosity = 0;

void usage()
{
    cur_file_name = 0;
    fprintf(stderr,
        "Usage: ttftool [-v[v[v]]] -f fontfile [-a afm_file] [-u u2g_file] [-p psfont] [-e encoding]\n"
        "    -v             verbosity level\n"
        "    -f fontfile:   the ttf font to process\n"
        "    -a afm_file:   output afm file to `afm_file' instead of stdout\n"
        "    -u u2g-file:   output unicode to glyph maping table to file `u2g_file'\n"
        "    -p fsfont  :   output postcript font\n"
        "    -e encoding:   encoding, for instance ISO-8859-2; use `ttftool -e print'\n"
        "                   for list of known encodings; without the -e parameter\n"
        "                   Adobe StandardEncoding will be used.\n"
        );
    exit(-1);
}

int main(int argc, char **argv)
{
    char date[128];
    time_t t = time(&t);
    int i;

    /*first of all that the architecture matches*/
    endianness_test();

/*----------------------------------------------------------------------------
process the command line
------------------------------------------------------------------------------*/
    for(i = 1; i < argc; i++)
    {
		if(!strcmp(argv[i], "-f"))
		{
    		cur_file_name = argv[++i];
    		fontfile = fopen(cur_file_name, "r");
    		if(!fontfile)
        		ttf_fail("cannot open font file [%s] for reading", cur_file_name);
			continue;
		}

		if(!strcmp(argv[i], "-a"))
		{
            afmfile = fopen(argv[++i], "w");
            if (afmfile == 0)
                ttf_fail("cannot open file [%s] for writting", argv[i]);
			continue;
		}

		if(!strcmp(argv[i], "-u"))
		{
            unifile = fopen(argv[++i], "w");
            if (unifile == 0)
                ttf_fail("cannot open file [%s] for writting",argv[i]);
			continue;
		}

      	    if(!strcmp(argv[i], "-e"))
      	    {
               if(!strcmp(argv[++i], "print"))
                  print_encodings();
               else if(!strcmp(argv[i], "ISO-8859-1"))
               {
               	  cur_enc_name = latin1enc;
               	  continue;
               }
               else if(!strcmp(argv[i], "StandardEncoding"))
                  continue;
               set_encoding(argv[i]);
               continue;
             }

   	if(!strcmp(argv[i], "-p"))
	   {
		   if((psfontfile=fopen(argv[++i],"w"))==NULL)
			   ttf_fail("cannot open file [%s] for writing",argv[i]);
   		continue;
	   }

   	if(!strncmp(argv[i], "-v",2))
	   {
		   char * p = argv[i] + 1;
		   while(*p++ == 'v')
			   verbosity++;
		   continue;
	   }

		usage();
    }

    if(!fontfile)
	usage();

/*----------------------------------------------------------------------------
Let's get things going ...
------------------------------------------------------------------------------*/

    sprintf(date, "%s\n", ctime(&t));
    *(char *)strchr(date, '\n') = 0;

    /* first we will generate the afm file*/
    msg(1, "Reading font file (phase 1).");
    read_font();
    fclose(fontfile);

    if (afmfile)
    {
      msg(1, "Generating afm file.");
    	print_afm(date, cur_file_name, afmfile);
    }

   free(Notice);
   free(FullName);
   free(FontName);


   /* now we need to generate the t42 font, since the generation of u2g file
      reorders the mtx table, and we need to be able to index it using the
      uni_map look up table */
   if(psfontfile)
   {
      msg(1, "Generating type 42 font file.");
      create_type42(psfontfile);
   }

   /* now we generate the u2g file */
   if (unifile)
   {
      msg(1, "Generating u2g file.");
      print_uni(date, cur_file_name, unifile);
   }

   /*free(b_name);*/
   msg(1,"Done.\n");
    return 0;
}

