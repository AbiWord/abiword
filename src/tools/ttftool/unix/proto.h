#ifndef TTF_PROTO_H
#define TTF_PROTO_H
#include "types.h"
/* function prototypes */

#include <sys/types.h>

void endianness_test(void);

struct TableDirectoryEntry *readDirectory(int fd, struct OffsetTable *ot);
char **readNamingTable(int fd);
void readHeadTable(int fd, struct HeadTable *ht);

void printPSFont(FILE *out, struct HeadTable *ht,
                 char **strings,int fd);

void printPSHeader(FILE *out, struct HeadTable *ht,
                   char **strings);
void printPSData(FILE *out, int fd);
void printPSTrailer(FILE *out);

int xgetc(FILE *stream);
long getnum(int s);

void *mymalloc(size_t size);
void *mycalloc(size_t nelem, size_t elsize);
void *myrealloc(void *ptr, size_t size);

void ttf_fail(char *fmt,...);
void warn(int verb, char *fmt,...);
void msg(int verb, char *fmt,...);

void syserror(char *string);
size_t surely_read(int fildes, void *buf, size_t nbyte);
char *unistrncpy(char *dst, char *str, size_t length);
void fputpss(char *s, FILE *stream);
off_t surely_lseek(int fildes, off_t offset, int whence);
unsigned hash(char *string);
struct hashtable *make_hashtable(int size);
int puthash(struct hashtable *t, char *key, int value);
int gethash(struct hashtable *t, char *key);

/*---------------------------------------------------------*/
long getnum(int s);
dirtab_entry *name_lookup(char *s);
void seek_tab(char *name, TTF_LONG offset);
void seek_off(char *name, TTF_LONG offset);
void store_kern_value(TTF_USHORT i, TTF_USHORT j, TTF_FWORD v);
TTF_FWORD get_kern_value(TTF_USHORT i, TTF_USHORT j);
void free_tabs();
char * ucs_to_uni(short unsigned int u);
void read_font();
int null_glyph(char *s);
void print_glyph_name(FILE *f, int i);
int compare_name(const void *a, const void *b);
void print_uni(char *date, char *fontname, FILE * unifile);
void print_afm(char *date, char *fontname, FILE * afmfile);
int print_sep(FILE *file);
void create_type42(FILE * psfontfile);

void print_encodings(void);
void set_encoding(char *e);


#endif

