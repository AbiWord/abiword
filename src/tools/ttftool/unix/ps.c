
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include "types.h"
#include "proto.h"
#include "externs.h"

#define CHUNKSIZE 65534


void
printPSFont(FILE *psfontfile, struct HeadTable *ht,
            char **strings, int fd)
{
  printPSHeader(psfontfile,ht,strings);
  printPSData(psfontfile,fd);
  printPSTrailer(psfontfile);
}

void
printPSHeader(FILE *psfontfile, struct HeadTable *ht,
              char **strings)
{
  int i;

  fprintf(psfontfile,"%%!PS-TrueTypeFont\n");
  if(maxMemType42)
    fprintf(psfontfile,"%%%%VMUsage: %ld %ld\n",minMemType42, maxMemType42);
  fprintf(psfontfile,"%d dict begin\n",11);
  fprintf(psfontfile,"/FontName /%s def\n", strings[6]?strings[6]:"Unknown");
  if(!curr_encoding)
	  fprintf(psfontfile,"/Encoding %s def\n",cur_enc_name);
  else
  {
      fprintf(psfontfile,"/Encoding 256 array\n");

      msg(1,"printing encoding array");
      for (i = 0; i < 256; i++)
      {
		msg(3, "i 0x%02x, U 0x%04x, k %d\n", i,curr_encoding->vec[i], uni_to_glyph[curr_encoding->vec[i]]);
   		fprintf(psfontfile,
               "dup %d /%s put\n", i, mtx_tab[uni_to_glyph[curr_encoding->vec[i]]].name);
      }
      fprintf(psfontfile, "readonly def\n");
  }

  fprintf(psfontfile,"/PaintType 0 def\n/FontMatrix [1 0 0 1 0 0] def\n");
  fprintf(psfontfile,"/FontBBox [%ld %ld %ld %ld] def\n",
          ht->xMin*1000L/ht->unitsPerEm,
          ht->yMin*1000L/ht->unitsPerEm,
          ht->xMax*1000L/ht->unitsPerEm,
          ht->yMax*1000L/ht->unitsPerEm);
  fprintf(psfontfile,"/FontType 42 def\n");
  fprintf(psfontfile,"/FontInfo 8 dict dup begin\n");
  fprintf(psfontfile,"/version (%d.%d) def\n",
          ht->fontRevision.mantissa,
          ht->fontRevision.fraction);
  if(strings[0]) {
    fprintf(psfontfile,"/Notice (");
    fputpss(strings[0],psfontfile);
    fprintf(psfontfile,") def\n");
  }
  if(strings[4]) {
    fprintf(psfontfile,"/FullName (");
    fputpss(strings[4],psfontfile);
    fprintf(psfontfile,") def\n");
  }
  if(strings[1]) {
    fprintf(psfontfile,"/FamilyName (");
    fputpss(strings[1],psfontfile);
    fprintf(psfontfile,") def\n");
  }
  fprintf(psfontfile,"/isFixedPitch %s def\n", IsFixedPitch?"true":"false");
  fprintf(psfontfile,"/UnderlinePosition %ld def\n",
          UnderlinePosition*1000L/ht->unitsPerEm);
  fprintf(psfontfile,"/UnderlineThickness %ld def\n",
          UnderlineThickness*1000L/ht->unitsPerEm);
  fprintf(psfontfile,"end readonly def\n");
}

void 
printPSData(FILE *psfontfile, int fd)
{
  static char xdigits[]="0123456789ABCDEF";

  unsigned char *buffer;
  int i,j;

  surely_lseek(fd,0,SEEK_SET);

  buffer=mymalloc(CHUNKSIZE);

  fprintf(psfontfile,"/sfnts [");
  for(;;) {
    i=read(fd,buffer,CHUNKSIZE);
    if(i==0)
      break;
    fprintf(psfontfile,"\n<");
    for(j=0;j<i;j++) {
      if(j!=0 && j%36==0)
        putc('\n', psfontfile);
      /* fprintf(psfontfile,"%02X",(int)buffer[j]) is too slow */
      putc(xdigits[(buffer[j]&0xF0)>>4], psfontfile);
      putc(xdigits[buffer[j]&0x0F], psfontfile);
    }
    fprintf(psfontfile,"00>");         /* Adobe bug? */
    if(i<CHUNKSIZE)
      break;
  }
  fprintf(psfontfile,"\n] def\n");
  free(buffer);
}

void
printPSTrailer(FILE *psfontfile)
{
  int i,n;
  char *name;

  fprintf(psfontfile,"/CharStrings %d dict dup begin\n",nglyphs);
  switch(post_format) {
  case 0x00020000:
    for(n=i=0;i<nglyphs;i++) {
      if(n!=0 && n%4==0)
        fprintf(psfontfile,"\n");
      /*name=NAMEOF(i);*/
      if(name) {
        fprintf(psfontfile,"/%s %d def ",mtx_tab[i].name,i);
        n++;
      }
    }
    break;
  default:
    if(post_format != 0x00010000) {
      if(verbosity>-2)
        fprintf(stderr,"No glyph name table; assuming MacGlyphEncoding\n");
    }
    for(i=0; i<258 && i<nglyphs; i++) {
      fprintf(psfontfile,"/%s %d def ",mac_glyph_names[i],i);
      if(i!=0 && i%4==0)
        fprintf(psfontfile,"\n");
    }
    break;
  }
  fprintf(psfontfile,"end readonly def\n");
  fprintf(psfontfile,"FontName currentdict end definefont pop\n");
}


#define fix_glyph_name(s)   ((s) != 0 ? (s) : null_name)
#define dont_print(s)       (!print_all && !print_index && null_glyph(s))
#define glyph_found(i)       (print_all || mtx_tab[i].found)

void print_afm(char *date, char *fontname, FILE *outfile)
{
    int /*i,*/ ncharmetrics;
    mtx_entry *pm;
    int new_nkernpairs;
    /*short mtx_index[MAX_CHAR_CODE + 1], *idx;*/
    /*char **pe;*/
    kern_entry *pk, *qk;
    fprintf(outfile, "Comment Converted at %s from font file `%s'\nComment by AbiWord (www.abisource.com)\n", date, fontname);
    fputs("\nStartFontMetrics 2.0\n", outfile);
    print_str(FontName);
    print_str(FullName);
    print_str(Notice);
    fprintf(outfile, "ItalicAngle %i", (int)(ItalicAngle/0x10000));
    if (ItalicAngle%0x10000 > 0)
        fprintf(outfile, ".%i", (int)((ItalicAngle%0x10000)*1000)/0x10000);
    fputs("\n", outfile);
    fprintf(outfile, "IsFixedPitch %s\n", IsFixedPitch ? "true" : "false");
    fprintf(outfile, "FontBBox %i %i %i %i\n",
            (int)get_ttf_funit(FontBBox1),
            (int)get_ttf_funit(FontBBox2),
            (int)get_ttf_funit(FontBBox3),
            (int)get_ttf_funit(FontBBox4));
    print_dimen(UnderlinePosition);
    print_dimen(UnderlineThickness);
    print_dimen(CapHeight);
    print_dimen(XHeight);
    print_dimen(Ascender);
    print_dimen(Descender);
    ncharmetrics = nglyphs;
    if (!print_all && !print_index)
        for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
            if (null_glyph(pm->name))
                ncharmetrics--;
    if (names_count == 0) { /* external encoding vector not given */
        fprintf(outfile, "\nStartCharMetrics %u\n", ncharmetrics);
        for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
            if (dont_print(pm->name))
                continue;
            pm->found = 1;
            fprintf(outfile, "C -1 ; WX %i ; N ", (int)get_ttf_funit(pm->wx));
            print_glyph_name(outfile, pm - mtx_tab);
            fprintf(outfile, " ; B %i %i %i %i ;\n",
                       (int)get_ttf_funit(pm->bbox[0]),
                       (int)get_ttf_funit(pm->bbox[1]),
                       (int)get_ttf_funit(pm->bbox[2]),
                       (int)get_ttf_funit(pm->bbox[3]));
        }
    }
    else
#if 1
	ttf_fail("External encoding vectors not supported");
#else
    { /* external encoding vector given */
        for (idx = mtx_index; idx - mtx_index <= MAX_CHAR_CODE; *idx++ = 0);
        if (!print_all)
            ncharmetrics = 0;
        for (pe = enc_names; pe - enc_names < names_count; pe++) {
            if (*pe == notdef)
                continue;
            if (read_index) {
                if (sscanf(*pe, INDEXED_GLYPH_PREFIX "%i", &i) == 1) {
                    if (i < 0 || i > nglyphs)
                        ttf_fail("`%s' out of valid range (0..%i)",
                             *pe, nglyphs);
                    pm = mtx_tab + i;
                }
                else
                    ttf_fail("`%s<num>' expected instead of `%s'",
                         INDEXED_GLYPH_PREFIX, *pe);
            }
            else
                for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
                    if (pm->name != 0 && strcmp(*pe, pm->name) == 0)
                            break;
            if (pm - mtx_tab < nglyphs) {
                mtx_index[pe - enc_names] = pm - mtx_tab;
                pm->found = 1;
                if (!print_all)
                    ncharmetrics++;
            }
            else
                warn(1,"glyph `%s' not found", *pe);
        }

        fprintf(outfile, "\nStartCharMetrics %u\n", ncharmetrics);
        for (idx = mtx_index; idx - mtx_index <= MAX_CHAR_CODE; idx++) {
            if (dont_print(mtx_tab[*idx].name))
                continue;
            if (*idx != 0)
                if (mtx_tab[*idx].found == 1) {
                    fprintf(outfile, "C %d ; WX %i ; N ",
                           idx - mtx_index,
                           (int)get_ttf_funit(mtx_tab[*idx].wx));
                    print_glyph_name(outfile, *idx);
                    fprintf(outfile, " ; B %i %i %i %i ;\n",
                           (int)get_ttf_funit(mtx_tab[*idx].bbox[0]),
                           (int)get_ttf_funit(mtx_tab[*idx].bbox[1]),
                           (int)get_ttf_funit(mtx_tab[*idx].bbox[2]),
                           (int)get_ttf_funit(mtx_tab[*idx].bbox[3]));
                }
        }
        if (!print_all)
            goto end_metrics;
        for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
            if (dont_print(pm->name))
                continue;
            if (pm->found == 0) {
                fprintf(outfile, "C -1 ; WX %i ; N ",
                       (int)get_ttf_funit(pm->wx));
                print_glyph_name(outfile, pm - mtx_tab);
                fprintf(outfile, " ; B %i %i %i %i ;\n",
                       (int)get_ttf_funit(pm->bbox[0]),
                       (int)get_ttf_funit(pm->bbox[1]),
                       (int)get_ttf_funit(pm->bbox[2]),
                       (int)get_ttf_funit(pm->bbox[3]));
            }
        }
    }
#endif

/*end_metrics:*/
    fputs("EndCharMetrics\n", outfile);
    if (nkernpairs == 0)
        goto end_kerns;
    new_nkernpairs = 0;
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++)
        if (!dont_print(mtx_tab[pk - kern_tab].name) &&
            glyph_found(pk - kern_tab))
            for (qk = pk; qk != 0; qk = qk->next)
                if (qk->value != 0 &&
                    !dont_print(mtx_tab[qk->adjacent].name) &&
                    glyph_found(qk->adjacent))
                    new_nkernpairs++;
    if (new_nkernpairs == 0)
        goto end_kerns;
    fprintf(outfile, "\nStartKernData\nStartKernPairs %i\n", new_nkernpairs);
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++)
        if (!dont_print(mtx_tab[pk - kern_tab].name) &&
            glyph_found(pk - kern_tab))
            for (qk = pk; qk != 0; qk = qk->next)
                if (qk->value != 0 &&
                    !dont_print(mtx_tab[qk->adjacent].name) &&
                    glyph_found(qk->adjacent)) {
                    fputs("KPX ", outfile);
                    print_glyph_name(outfile, pk - kern_tab);
                    fputs(" ", outfile);
                    print_glyph_name(outfile, qk->adjacent);
                    fprintf(outfile, " %i\n", get_ttf_funit(qk->value));
                }
    fputs("EndKernPairs\nEndKernData\n", outfile);
end_kerns:
    fputs("EndFontMetrics\n", outfile);
}

void create_type42(FILE * psfontfile)
{
   int fd = 0;
   int i;
   struct OffsetTable ot;
   struct HeadTable *ht;
   struct TableDirectoryEntry *td;
   char **strings=NULL;

   TTF_ULONG headOff=0, maxpOff=0, postOff=0, nameOff=0,
   locaOff=0, glyfOff=0, hheaOff=0, hmtxOff=0, kernOff=0;

   msg(1, "Reading font file (phase 2).");

   if((fd=open(cur_file_name,O_RDONLY))<0)
      ttf_fail("cannot open font file [%s] for reading", cur_file_name);

   td=readDirectory(fd,&ot);

   msg(2,"True type version %d.%u", ot.version.mantissa,ot.version.fraction);

   for(i=0;i<ot.numTables;i++) {
      msg(2,"Found `%c%c%c%c' table",
         (char)(td[i].tag>>24),
         (char)(td[i].tag>>16)&255,
         (char)(td[i].tag>>8)&255,
         (char)td[i].tag&255);

       switch(td[i].tag) {
          case MAKE_ULONG('m','a','x','p'):
            maxpOff=td[i].offset;
            break;
         case MAKE_ULONG('h','e','a','d'):
            headOff=td[i].offset;
            break;
         case MAKE_ULONG('p','o','s','t'):
            postOff=td[i].offset;
            break;
         case MAKE_ULONG('n','a','m','e'):
            nameOff=td[i].offset;
            break;
         case MAKE_ULONG('l','o','c','a'):
            locaOff=td[i].offset;
            break;
         case MAKE_ULONG('g','l','y','f'):
            glyfOff=td[i].offset;
            break;
         case MAKE_ULONG('h','h','e','a'):
            hheaOff=td[i].offset;
            break;
         case MAKE_ULONG('h','m','t','x'):
            hmtxOff=td[i].offset;
            break;
         case MAKE_ULONG('k','e','r','n'):
            kernOff=td[i].offset;
            break;
         }
      }
      if(maxpOff==0 || headOff==0 || postOff==0 || nameOff==0)
         ttf_fail("Incomplete TTF file");

     msg(1,"Processing `head' table");
     surely_lseek(fd,headOff,SEEK_SET);
     ht=mymalloc(sizeof(struct HeadTable));
     readHeadTable(fd,ht);

     msg(1,"Processing `name' table\n");
     surely_lseek(fd,nameOff,SEEK_SET);
     strings=readNamingTable(fd);

     msg(1,"Generating PS file\n");
     printPSFont(psfontfile,ht,strings,fd);
     fclose(psfontfile);

     close(fd);

}

