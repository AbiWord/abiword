
/* These are global variables used outside the main file ttftool.c */

extern FILE * fontfile;
extern char * encoding;
extern struct enc_verctor * known_encodings[];
extern dirtab_entry * dir_tab;
extern TTF_USHORT ntabs;
extern kern_entry * kern_tab;
extern mtx_entry * mtx_tab;
extern char *ps_glyphs_buf;
extern char *enc_names[MAX_CHAR_CODE + 1];
extern char notdef[];
extern char *null_name;
extern int print_index;
extern int print_all;
extern int read_index;
extern struct enc_vector * curr_encoding;
extern char * cur_file_name;
extern char * cur_enc_name;
extern int verbosity;
extern int broken_cmap;

extern TTF_LONG ItalicAngle;
extern TTF_LONG IsFixedPitch;
extern TTF_LONG FontBBox1;
extern TTF_LONG FontBBox2;
extern TTF_LONG FontBBox3;
extern TTF_LONG FontBBox4;
extern TTF_LONG UnderlinePosition;
extern TTF_LONG UnderlineThickness;
extern TTF_LONG CapHeight;
extern TTF_LONG XHeight;
extern TTF_LONG Ascender;
extern TTF_LONG Descender;

extern TTF_ULONG minMemType42;
extern TTF_ULONG maxMemType42;
extern TTF_ULONG minMemType1;
extern TTF_ULONG maxMemType1;

extern unsigned short int uni_map[];
extern unsigned short int uni_to_glyph[];
extern int post_format;
extern int loca_format;
extern int nglyphs;
extern int nkernpairs;
extern int names_count;

extern char * mac_glyph_names[];

extern char *FontName;
extern char *FullName;
extern char *Notice;

extern TTF_USHORT upem;
extern TTF_USHORT ntabs;
extern int nhmtx;
