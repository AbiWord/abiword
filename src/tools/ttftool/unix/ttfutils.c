#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "proto.h"

#include "externs.h"



/*
	numbers in the font file are big endian; the getnum function ensures
	that irrespective of architecture, we read them correctly
	However, sections of the code that came from ttfps use macros
	to fix endianness, and that requirest that the executable is
	compiled correctly, the next function is used to test this.
*/

long getnum(int s)
{
    long i = 0;
    int c;
    while (s > 0) {
        if ((c = xgetc(fontfile)) < 0)
            ttf_fail("unexpected EOF");
        i = (i << 8) + c;
        s--;
    }
    return i;
}

void endianness_test()
{
  union {
    TTF_BYTE b[4];
    TTF_ULONG l;
  } x;
  TTF_ULONG v;

  x.b[0]=1; x.b[1]=2; x.b[2]=3; x.b[3]=4;

  v=UL(x.l);

  if(v!=(((((1<<8)+2)<<8)+3)<<8)+4) {
    fprintf(stderr,"Error: Code badly compiled for this architecture\n");
    fprintf(stderr,"Please set SMALLENDIAN and recompile\n");
    exit(2);
  }
}

dirtab_entry *name_lookup(char *s)
{
    dirtab_entry *p;
    for (p = dir_tab; p - dir_tab < ntabs; p++)
        if (strncmp(p->tag, s, 4) == 0)
            break;
    if (p - dir_tab == ntabs)
        p = 0;
    return p;
}

void seek_tab(char *name, TTF_LONG offset)
{
    dirtab_entry *p = name_lookup(name);
    if (p == 0)
        ttf_fail("can't find table `%s'", name);
    if (fseek(fontfile, p->offset + offset, SEEK_SET) < 0)
        ttf_fail("fseek() failed while reading `%s' table", name);
}

void seek_off(char *name, TTF_LONG offset)
{
    if (fseek(fontfile, offset, SEEK_SET) < 0)
        ttf_fail("fseek() failed while reading `%s' table", name);
}

void store_kern_value(TTF_USHORT i, TTF_USHORT j, TTF_FWORD v)
{
    kern_entry *pk;
    for (pk = kern_tab + i; pk->next != 0; pk = pk->next);
    pk->next = ttf_alloc(1, kern_entry);
    pk = pk->next;
    pk->next = 0;
    pk->adjacent = j;
    pk->value = v;
}

TTF_FWORD get_kern_value(TTF_USHORT i, TTF_USHORT j)
{
    kern_entry *pk;
    for (pk = kern_tab + i; pk->next != 0; pk = pk->next)
        if (pk->adjacent == j)
            return pk->value;
    return 0;
}

void free_tabs()
{
    int i;
    kern_entry *p, *q, *r;
    free(ps_glyphs_buf);
    free(dir_tab);
    free(mtx_tab);
    for (i = 0; i <= MAX_CHAR_CODE; i++)
        if (enc_names[i] != notdef)
            free(enc_names[i]);
    for (p = kern_tab; p - kern_tab < nglyphs; p++)
        if (p->next != 0) {
            for (q = p->next; q != 0; q = r) {
                r = q->next;
                free(q);
            }
        }
    free(kern_tab);
}


char * ucs_to_uni(short unsigned int u)
{
   char* uni = (char*) mymalloc(8);
   sprintf(uni+3, "%04x",(unsigned int) u);
   * uni      = 'u';
   *(uni + 1) = 'n';
   *(uni + 2) = 'i';
   return uni;
}


int null_glyph(char *s)
{
    if (s != 0 &&
        (strcmp(s, ".null") == 0 ||
         strcmp(s, ".notdef") == 0))
        return 1;
    return 0;
}

#define fix_glyph_name(s)   ((s) != 0 ? (s) : null_name)
#define dont_print(s)       (!print_all && !print_index && null_glyph(s))
#define glyph_found(i)       (print_all || mtx_tab[i].found)

void print_glyph_name(FILE *f, int i)
{
   char *s = mtx_tab[i].name;

   switch (print_index) {
    case 0:
        fprintf(f, fix_glyph_name(s));
        break;
    case 1:
        fprintf(f, "%s%i", INDEXED_GLYPH_PREFIX, i);
        break;
    case 2:
        if (i < 0x0100)
            fprintf(f, "%s0x%.2X", INDEXED_GLYPH_PREFIX, i);
        else
            fprintf(f, "%s0x%.4X", INDEXED_GLYPH_PREFIX, i);
        break;
    }
}

int compare_name(const void *a, const void *b)
{
	mtx_entry * m1 = (mtx_entry*) a;
	mtx_entry * m2 = (mtx_entry*) b;
	return (strcmp(m1->name, m2->name));
}

void print_uni(char *date, char *fontname, FILE * unifile)
{
    mtx_entry *pm;

    fprintf(unifile, "# Generated at %s from font file `%s' \n# by AbiWord (www.abisource.com)\n# WARNING: THIS FILE MUST BE SORTED ALPHABETICALLY BY THE STRINGS\n"
                     "%d\n", date, fontname, nglyphs);

    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
        if(!pm->name)
            pm->name = notdef;

    qsort(mtx_tab, nglyphs, sizeof(mtx_entry), compare_name);

    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
        fprintf(unifile, "%s,0x%04x\n", pm->name, pm->uni);
}

void print_encodings()
{
   struct enc_vector ** ev = (struct enc_vector **) &known_encodings[0];
   printf("Known encodings:\n");
   printf("                   ISO-8895-1\n");
   while (*ev)
   {
      printf("                   %s\n", (*ev)->name);
      ev++;
   }
   printf("                   StandardEncoding\n\nWhen run without"
          " the -e parameter, Adobe StandardEncoding\nwill be used.\n");
   exit(0);
}

void set_encoding(char *e)
{
   struct enc_vector ** ev = (struct enc_vector **) &known_encodings[0];
   while (*ev)
      if(!strcmp((*ev)->name, e))
      {
         encoding = e;
         curr_encoding = *ev;
	 return;
      }
      else
	ev++;

   ttf_fail("unknown encoding %s; use `ttftool -e print' for list of\n"
            "supported encodings.\n", e);
}


void ttf_fail(char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\nError: ttftool");
    if (cur_file_name)
        fprintf(stderr, "(file %s)", cur_file_name);
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(-1);
}

void warn(int verb, char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    if(verb <= verbosity)
    {
      fprintf(stderr, "Warning ");
      if (cur_file_name)
         fprintf(stderr, "(%s)", cur_file_name);
      fprintf(stderr, ": ");
      vfprintf(stderr, fmt, args);
      fprintf(stderr, "\n");
    }
    va_end(args);
}

void msg(int verb, char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    if(verb <= verbosity)
    {
      vfprintf(stderr, fmt, args);
      fprintf(stderr, "\n");
    }
    va_end(args);
}


int xgetc(FILE *stream)
{
    int c = getc(stream);
    if (c < 0 && c != EOF)
        ttf_fail("getc() failed");
    return c;
}

void
*mymalloc(size_t size)
{
  void *p;
  if((p=malloc(size))==NULL)
    ttf_fail("Unable to allocate memory\n");
  return p;
}

void *
mycalloc(size_t nelem, size_t elsize)
{
  void *p;
  if((p=calloc(nelem, elsize))==NULL)
    ttf_fail("Unable to allocate memory\n");
  return p;
}

void *
myrealloc(void *ptr, size_t size)
{
  void *p;
  if((p=realloc(ptr, size))==NULL)
    ttf_fail("Unable to allocate memory");
  return p;
}

off_t
surely_lseek(int fildes, off_t offset, int whence)
{
  off_t result;
  if((result=lseek(fildes,offset,whence))<0)
    ttf_fail("Bad TTF file");
  return result;
}


void
syserror(char *string)
{
  perror(string);
  exit(3);
  /*NOTREACHED*/
}

size_t
surely_read(int fildes, void *buf, size_t nbyte)
{
  ssize_t n;
  if((n=read(fildes,buf,nbyte))<nbyte)
    ttf_fail("Bad TTF file");
  return n;
}

char *
unistrncpy(char *dst, char *str, size_t length)
{
  int i,j;

  for(i=j=0; i<length; i+=2)
    if(str[i]==0)
      dst[j++]=str[i+1];
  dst[j]='\0';
  return dst;
}

void
fputpss(char *s, FILE *stream)
{
  while(*s) {
    if((*s&0200)==0 && *s>=040 && *s!='(' && *s!=')')
      putc(*s,stream);
    else
      fprintf(stream,"\\%03o",(unsigned char)*s);
    s++;
  }
}


