
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <string.h>
#include "types.h"
#include "proto.h"
#include "externs.h"

struct TableDirectoryEntry
*readDirectory(int fd, struct OffsetTable *ot)
{
  unsigned n;
  int i;

  struct TableDirectoryEntry *td;
  surely_read(fd,ot,sizeof(struct OffsetTable));
  FIX_OffsetTable(*ot);
  msg(2,"%d tables\n",ot->numTables);
  n=sizeof(struct TableDirectoryEntry)*ot->numTables;
  td=mymalloc(n);
  surely_read(fd,td,n);
  for(i=0; i<ot->numTables; i++)
    FIX_TableDirectoryEntry(td[i]);
  return td;
}

char **
readNamingTable(int fd)
{
  TTF_USHORT format;
  TTF_USHORT nrecords;
  off_t position;
  TTF_USHORT offset;
  int i,index,maxIndex;
  struct NameRecord *records;
  char *data;
  char **strings;

  position=surely_lseek(fd,0,SEEK_CUR);

  surely_read(fd,&format,sizeof(TTF_USHORT));
  FIX_UH(format);
  if(format!=0)
    ttf_fail("Bad TTF file\n");
  surely_read(fd,&nrecords,sizeof(TTF_USHORT));
  FIX_UH(nrecords);
  surely_read(fd,&offset,sizeof(TTF_USHORT));
  FIX_UH(offset);
  records=mymalloc(nrecords*sizeof(struct NameRecord));
  surely_read(fd,records,nrecords*sizeof(struct NameRecord));

  for(i=0,maxIndex=-1;i<nrecords;i++) {
    FIX_NameRecord(records[i]);
    index=records[i].offset+records[i].length;
    maxIndex=maxIndex>index?maxIndex:index;
  }
  data=mymalloc(maxIndex);
  surely_lseek(fd,position+offset,SEEK_SET);
  surely_read(fd,data,maxIndex);

  strings=mymalloc(8*sizeof(char*));
  for(i=0;i<8;i++)
    strings[i]=NULL;

  for(i=0; i<nrecords; i++) {
    if(records[i].platformID==3 && /* Microsoft */
       records[i].encodingID==1 && /* UGL */
       records[i].languageID==0x0409 && /* US English */
       records[i].nameID<=7) {
      strings[records[i].nameID]=mymalloc(records[i].length/2+1);
      unistrncpy(strings[records[i].nameID],
                 data+records[i].offset,
                 records[i].length);
      msg(2,"%d: %s\n",records[i].nameID,
                strings[records[i].nameID]);
    }
  }
  free(records);
  free(data);
  return strings;
}

void
readHeadTable(int fd, struct HeadTable *ht)
{
  surely_read(fd,ht,sizeof(struct HeadTable));
  FIX_HeadTable(*ht);
  msg(2,"  version %d.%d", ht->version.mantissa,ht->version.fraction);
  msg(2,"  font revision %d.%d",ht->fontRevision.mantissa, ht->fontRevision.fraction);

  if(ht->magicNumber!=0x5F0F3CF5)
    ttf_fail("Bad magic number in TTF file");
  msg(2,"  %d units per Em\n",ht->unitsPerEm);
}



void read_font()
{
    long i, j, k, l, n, m, platform_id, encoding_id;
    TTF_FWORD kern_value;
    char buf[1024], *p;
    dirtab_entry *pd;
    kern_entry *pk;
    mtx_entry *pm;

    long int netabs, length;
    cmap_entry *cmap_tab, *e;
    seg_entry *seg_tab, *s;
    long cmap_offset;
    TTF_USHORT *glyphId, format, segCount;

    msg(3, "Reading font");
    ttf_skip(TTF_FIXED_L_SIZE);
    ntabs = get_ushort();
    ttf_skip(3*TTF_USHORT_SIZE);
    msg(3, "Reading directory table [%d entries]", ntabs);
    dir_tab = ttf_alloc(ntabs, dirtab_entry);
    for (pd = dir_tab; pd - dir_tab < ntabs; pd++) {
        pd->tag[0] = get_char();
        pd->tag[1] = get_char();
        pd->tag[2] = get_char();
        pd->tag[3] = get_char();
        ttf_skip(TTF_ULONG_SIZE);
        pd->offset = get_ulong();
        pd->length = get_ulong();
    }

    msg(3, "Reading head table");
    seek_tab("head", 2*TTF_FIXED_L_SIZE + 2*TTF_ULONG_SIZE + TTF_USHORT_SIZE);
    upem = get_ushort();
    ttf_skip(16);
    FontBBox1 = get_fword();
    FontBBox2 = get_fword();
    FontBBox3 = get_fword();
    FontBBox4 = get_fword();
    ttf_skip(TTF_USHORT_SIZE);
    ttf_skip(TTF_USHORT_SIZE + TTF_SHORT_SIZE);
    loca_format = get_short();
    msg(3, "loca table format %d", loca_format);

    msg(3,"Reading maxp table");
    seek_tab("maxp", TTF_FIXED_L_SIZE);
    nglyphs = get_ushort();
    msg(3, "%d entries", nglyphs);
    mtx_tab = ttf_alloc(nglyphs + 1, mtx_entry); /*need 1 for the loca table*/
    /* set initially to 0, so that we can test whether an entry was used
       by testing for 0
    memset(mtx_tab, 0, nglyphs*sizeof(mtx_entry)); */

    for (pm = mtx_tab; pm - mtx_tab < nglyphs; pm++) {
        pm->name = 0; /* notdef */
        pm->found = 0;
    }
    msg(3, "Reading hhea table");
    seek_tab("hhea", TTF_FIXED_L_SIZE);
    Ascender = get_fword();
    Descender = get_fword();
    ttf_skip(TTF_FWORD_SIZE + TTF_UFWORD_SIZE + 3*TTF_FWORD_SIZE + 8*TTF_SHORT_SIZE);
    nhmtx = get_ushort();

    msg(3, "Reading hmtx table");
    seek_tab("hmtx", 0);
    for (pm = mtx_tab; pm - mtx_tab < nhmtx; pm++) {
        pm->wx = get_ufword();
        ttf_skip(TTF_FWORD_SIZE);
    }
    i = pm[-1].wx;
    for (; pm - mtx_tab < nglyphs; pm++)
        pm->wx = i;

    msg(3, "Reading post table");
    seek_tab("post", 0);
    post_format = get_fixed();
    msg(3, "post format %d", post_format);
    ItalicAngle = get_fixed();
    UnderlinePosition = get_fword();
    UnderlineThickness = get_fword();
    IsFixedPitch = get_ulong();
    /*ttf_skip(4*TTF_ULONG_SIZE);*/
    minMemType42 = get_ulong();
    maxMemType42 = get_ulong();
    minMemType1  = get_ulong();
    maxMemType1  = get_ulong();

    switch (post_format) {
    case 0x00010000:
	for (pm = mtx_tab; pm - mtx_tab < NMACGLYPHS; pm++)
            pm->name = mac_glyph_names[pm - mtx_tab];
        break;
    case 0x00020000:
        l = get_ushort(); /* some fonts have this value different from nglyphs */
	msg(3, "post table contains %d entries", l);

        for (pm = mtx_tab; pm - mtx_tab < l; pm++)
            pm->index = get_ushort();
        if ((pd = name_lookup("post")) == 0)
            ttf_fail("can't find table `post'");
        n = pd->length - (ftell(fontfile) - pd->offset);
        ps_glyphs_buf = ttf_alloc(n + 1, char);
        for (m = 0, p = ps_glyphs_buf; p - ps_glyphs_buf < n;) {
            for (i = get_byte(); i > 0; i--)
                *p++ = get_char();
            *p++ = 0;
            m++;

        }

        /* now we retrieve the names that are in the font table*/
        for (pm = mtx_tab; pm - mtx_tab < l; pm++) {
          if (pm->index < NMACGLYPHS) {
                pm->name = mac_glyph_names[pm->index];
          }
          else {
       	   k = pm->index - NMACGLYPHS;
           if (k <= m) {
                for (p = ps_glyphs_buf; k > 0; k--)
                    p = (char *)strend(p) + 1;
                pm->name = p;
            }
            else {
                warn(1,"index 0x%04x out of range (glyphs: 0x%04x, m=0x%04x)", k,l,m);
                pm->name = 0; /* index out of valid range, fix name to notdef */
           }
          }
        }


		/*here comes the ucs->uni hack*/
	msg(3, "Reading cmap table");
        seek_tab("cmap", TTF_USHORT_SIZE); /* skip the table vesrion number (0) */
        netabs = get_ushort();
	msg(3, "%d entries", netabs);

        cmap_offset = ftell(fontfile) - 2*TTF_USHORT_SIZE;
        cmap_tab = ttf_alloc(netabs, cmap_entry);
        for (e = cmap_tab; e - cmap_tab < netabs; e++) {
            e->platform_id = get_ushort();
            e->encoding_id = get_ushort();
            e->offset = get_ulong();
        }
        for (e = cmap_tab; e - cmap_tab < netabs; e++) {
            seek_off("cmap", cmap_offset + e->offset);
            format = get_ushort();
            if (format != 4) {
            	msg(4, "found table in unsupported format %d, skipping.", format);
                continue;
            }

	msg(3, "format=%d (should be 4)", format);

        /*uni_to_glyph = 1;*/
        length = get_ushort(); /* length of subtable in bytes */
        get_ushort(); /* skip the version number */
        segCount = get_ushort();
        segCount /= 2;
	msg(3, "segment count %d, length %d", segCount, length);

        get_ushort(); /* skip searchRange */
        get_ushort(); /* skip entrySelector */
        get_ushort(); /* skip rangeShift */
        seg_tab = ttf_alloc(segCount, seg_entry);
        for (s = seg_tab; s - seg_tab < segCount; s++)
        {
            s->endCode = get_ushort();
            msg(4, "segment end 0x%04x", s->endCode);
        }
        get_ushort(); /* skip reversedPad */
        for (s = seg_tab; s - seg_tab < segCount; s++)
        {
            s->startCode = get_ushort();
            msg(4, "segment start 0x%04x", s->startCode);
        }
        for (s = seg_tab; s - seg_tab < segCount; s++)
        {
            s->idDelta = get_ushort();
            msg(4, "segment delta 0x%04x", s->idDelta);
        }
        for (s = seg_tab; s - seg_tab < segCount; s++)
        {
            s->idRangeOffset = get_ushort();
            msg(4, "segment range offset 0x%04x", s->idRangeOffset);
        }
        length -= 8*TTF_USHORT_SIZE + 4*segCount*TTF_USHORT_SIZE;
        length /= TTF_USHORT_SIZE; /* conver byte length to number if items */

	msg(3, "should contain %d glyphs", length);

        glyphId = ttf_alloc(length, TTF_USHORT);
        for (i = 0; i < length; i++)
        {
        	msg(5, "i=%d", i);
            glyphId[i] = get_ushort();
        }

	msg(3, "retrieved glyphs");

        /* set uni_map to 0, so that we can test for duplicate usage */
        for (i = 0; i <= MAX_CHAR_CODE; i++)
        {
        	uni_map[i] = 0;
                uni_to_glyph[i] = 0;
        }

        /* the following loop translates the unicode value i into the glyph index k*/
        for (i = 0; i <= MAX_CHAR_CODE; i++)
        {
            for (s = seg_tab; s - seg_tab < segCount; s++)
                if (s->endCode >= i)
                    break;

            if (s - seg_tab < segCount && s->startCode <= i) {
                if (s->idRangeOffset != 0) {
                    k = glyphId[(i-s->startCode) + s->idRangeOffset/2 - (segCount-(s-seg_tab))];
                    if (k != 0)
                        k = (k + s->idDelta) % 0xFFFF;
                }
                else
                    k = (s->idDelta + i) % 0xFFFF;
            }
            else
            	k = 0; /*not found*/
			
			if(uni_map[k] == 0 || k == 0)
			{
				uni_map[k] = i;
			}
	  		else
	  			warn(1,"warning: duplicate ucs; index %d, ucs 0x%04x, prev. ucs 0x%04x\n", k, i, uni_map[k]);

			uni_to_glyph[i] = k;


        }

        for (i = 0; i < nglyphs; i++)
			mtx_tab[i].uni = uni_map[i];
     }

        break;
    case 0x00030000:
        if (print_index == 0) {
            warn(1,"no names available in `post' table, printing by index forced");
            print_index = 2;
        }
        break;
    default:
        ttf_fail("unsupported format (%.8X) of `post' table", post_format);
    }
    msg(3, "Reading loca table");
    seek_tab("loca", 0);
    for (pm = mtx_tab; pm - mtx_tab <= nglyphs; pm++) /*loca table contains nglyphs+1 entries*/
        pm->offset = (loca_format == 1 ? get_ulong() : get_ushort() << 1);

    msg(3, "Reading glyf table");

    if ((pd = name_lookup("glyf")) == 0)
        ttf_fail("can't find table `glyf'");
    for (n = pd->offset, pm = mtx_tab; pm - mtx_tab < nglyphs; pm++)
    {
    	msg(5, "indx %d of %d [off 0x%04x, next off 0x%04x]", pm - mtx_tab, nglyphs-1,n + pm->offset, n + (pm+1)->offset);
        if (pm->offset != (pm + 1)->offset) {
            seek_off("glyf", n + pm->offset);
            ttf_skip(TTF_SHORT_SIZE);
            pm->bbox[0] = get_fword();
            msg(6, "got bbox[0]");
            pm->bbox[1] = get_fword();
            msg(6, "got bbox[1]");
            pm->bbox[2] = get_fword();
            msg(6, "got bbox[2]");
            pm->bbox[3] = get_fword();
            msg(6, "got bbox[3]");
        }
        else { /* get the BBox from .notdef */
            pm->bbox[0] = mtx_tab[0].bbox[0];
            pm->bbox[1] = mtx_tab[0].bbox[1];
            pm->bbox[2] = mtx_tab[0].bbox[2];
            pm->bbox[3] = mtx_tab[0].bbox[3];
        }
    }
    msg(3, "Reading name table");
    seek_tab("name", TTF_USHORT_SIZE);
    i = ftell(fontfile);
    n = get_ushort();
    j = get_ushort() + i - TTF_USHORT_SIZE;
    i += 2*TTF_USHORT_SIZE;
    while (n-- > 0) {
        seek_off("name", i);
        platform_id = get_ushort();
        encoding_id = get_ushort();
        get_ushort(); /* skip language_id */
        k = get_ushort();
        l = get_ushort();
        if ((platform_id == 1 && encoding_id == 0) &&
            (k == 0 || k == 4 || k == 6)) {
            seek_off("name", j + get_ushort());
            for (p = buf; l-- > 0; p++)
                *p = get_char();
            *p++ = 0;
	    p = (char *) mymalloc(strlen(buf)+1);
            p = strcpy(p,buf);
            switch (k) {
            case 0:  Notice = p; break;
            case 4:  FullName = p; break;
            case 6:  FontName = p; break;
            }
            if (Notice != 0 && FullName != 0 && FontName != 0)
                break;
        }
        i += 6*TTF_USHORT_SIZE;
    }
    
    msg(3, "Reading PCLT table");
    if ((pd = name_lookup("PCLT")) != 0) {
        seek_off("PCLT", pd->offset + TTF_FIXED_L_SIZE + TTF_ULONG_SIZE + TTF_USHORT_SIZE);
        XHeight = get_ushort();
        ttf_skip(2*TTF_USHORT_SIZE);
        CapHeight = get_ushort();
    }
    msg(3, "Reading kern table");
    if ((pd = name_lookup("kern")) == 0)
        return;
    kern_tab = ttf_alloc(nglyphs, kern_entry);
    for (pk = kern_tab; pk - kern_tab < nglyphs; pk++) {
        pk->next = 0;
        pk->value = 0;
    }
    seek_off("kern", pd->offset + TTF_USHORT_SIZE);
    for (n = get_ushort(); n > 0; n--) {
        ttf_skip(2*TTF_USHORT_SIZE);
        k = get_ushort();
        if (!(k & 1) || (k & 2) || (k & 4))
            return;
        if (k >> 8 != 0) {
            warn(1, "warning: only format 0 supported of `kern' \
                 subtables, others are ignored\n");
            continue;
        }
        k = get_ushort();
        ttf_skip(3*TTF_USHORT_SIZE);
        while (k-- > 0) {
            i = get_ushort();
            j = get_ushort();
            kern_value = get_fword();
            if (kern_value != 0) {
                store_kern_value(i, j, kern_value);
                nkernpairs++;
            }
        }
    }
}

