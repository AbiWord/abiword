/*
   OLEdecode - Decode Microsoft OLE files into its components.
   Copyright (C) 1998  Andrew Scriven

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
   Released under GPL, written by 
   Andrew Scriven <andy.scriven@research.natpower.co.uk>

   Copyright (C) 1998
   Andrew Scriven
 */
/*
   -----------------------------------------------------------------------
   Andrew Scriven
   Research and Engineering
   Electron Building, Windmill Hill, Whitehill Way, Swindon, SN5 6PB, UK
   Phone (44) 1793 896206, Fax (44) 1793 896251
   -----------------------------------------------------------------------
 */
/*
   Extremely modified by
   Arturo Tena <arturo@directmail.org> <filters@centauri.lci.ulsa.mx>
 */
/*
   The interface to OLEdecode now has
   int OLEdecode (char *OLEfilename, pps_entry ** stream_list, U32 * root,
		  U16 max_level);
   See the oledecod.h to see a description of the inputs and output
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/types.h>
#include <assert.h>

#include "config.h"
#include "mswordview.h"

#include "oledecod.h"
#include "support.h"


#define ENTRYCHUNK 20		/* number of entries in root_list and sbd_list
				   will be added each time. must be at least 1 */

	/* verbose pps tree */
static void verbosePPSTree (U32 root_pps, int level);
	/* reorder pps tree, from tree structure to a linear one,
	   and write the level numbers */
static void reorder_pps_tree (pps_entry * root_pps, U16 level);
	/* free memory used (except the pps tree) */
static void ends (void);


static FILE *input;
static U8 *Block;
static U8 *BDepot, *SDepot, *Root;
static pps_entry *pps_list = NULL;
static U32 num_of_pps = 0;
static FILE *sbfile = NULL;
static char sbfilename[L_tmpnam];
static U32 *sbd_list;
static U32 *root_list;

int
OLEdecode (char *OLEfilename, pps_entry ** stream_list, U32 * root,
	   U16 max_level)
{
  int c;
  U32 num_bbd_blocks;
  U32 i, len;
  U8 *s, *p, *t;
  long FilePos;
  /* FilePos is long, not U32, because second argument of fseek is long */

  /* initialize static variables */
  input = sbfile = NULL;
  Block = BDepot = SDepot = Root = NULL;
  pps_list = NULL;
  num_of_pps = 0;
  sbfilename[0] = 0;
  root_list = sbd_list = NULL;

  /* open input file */
  verbose ("open input file");
  input = fopen (OLEfilename, "rb");
  test (input != NULL, 4, ends ());

  /* fast check type of file */
  verbose ("fast testing type of file");
  test ((c = getc (input)) != EOF, 5, ends ());
  test (ungetc (c, input) != EOF, 5, ends ());
  test (!isprint (c), 8, ends ());
  test (c == 0xd0, 9, ends ());

  /* read header block */
  verbose ("read header block");
  Block = (U8 *) malloc (0x0200);
  test (Block != NULL, 10, ends ());
  fread (Block, 0x0200, 1, input);
  test (!ferror (input), 5, ends ());

  /* really check type of file */
  rewind (input);
  verbose ("testing type of file");
  test (fil_sreadU32 (Block) != 0xd0cf11e0UL, 9, ends ());
  test (fil_sreadU32 (Block + 0x04) != 0xa1b11ae1UL, 9, ends ());


  /* read big block depot */
  verbose ("read big block depot (bbd)");
  num_bbd_blocks = fil_sreadU32 (Block + 0x2c);
  verboseU32 (num_bbd_blocks);
  BDepot = malloc (0x0200 * num_bbd_blocks);
  test (BDepot != NULL, 10, ends ());
  s = BDepot;
  for (i = 0; i < num_bbd_blocks; i++)
    {
      /* note: next line may be needed to be cast to long in right side */
      FilePos = 0x0200 * (1 + fil_sreadU32 (Block + 0x4c + (i * 4)));
      assert (FilePos >= 0);
      test (!fseek (input, FilePos, SEEK_SET), 5, ends ());
      fread (s, 0x0200, 1, input);
      test (!ferror (input), 5, ends ());
      s += 0x0200;
    }
  verboseU8Array (BDepot, num_bbd_blocks, 0x0200);


  /* extract the sbd block list */
  verbose ("extract small block depot (sbd) block list");
  sbd_list = malloc (ENTRYCHUNK * 4);
  test (sbd_list != NULL, 10, ends ());
  sbd_list[0] = fil_sreadU32 (Block + 0x3c);
  /* -2 signed long int == 0xfffffffe unsinged long int */
  for (len = 1; sbd_list[len - 1] != 0xfffffffeUL; len++)
    {
      test (len != 0, 5, ends ());	/* means file is too big */
      /* if memory allocated in sbd_list is all used, allocate more memory */
      if (!(len % ENTRYCHUNK))
	{
	  U32 *newspace;
	  newspace = realloc (sbd_list,
			      (1 + len / ENTRYCHUNK) * ENTRYCHUNK * 4);
	  test (newspace != NULL, 10, ends ());
	  sbd_list = newspace;
	}
	  if ( (sbd_list[len - 1] * 4) >= (0x0200 * num_bbd_blocks))
		return(5);
      sbd_list[len] = fil_sreadU32 (BDepot + (sbd_list[len - 1] * 4));
      test (sbd_list[len] != 0xfffffffdUL && sbd_list[len] != 0xffffffffUL,
	    5, ends ());
    }
  len--;
  verboseU32Array (sbd_list, len);
  /* read in small block depot, if there's any small block */
  if (len == 0)
    {
      SDepot = NULL;
      verbose ("not read small block depot (sbd): there's no small blocks");
    }
  else
    {
      verbose ("read small block depot (sbd)");
      SDepot = malloc (0x0200 * len);
      test (SDepot != NULL, 10, ends ());
      s = SDepot;
      for (i = 0; i < len; i++)
	{
	  FilePos = 0x0200 * (1 + sbd_list[i]);
	  assert (FilePos >= 0);
	  test (!fseek (input, FilePos, SEEK_SET), 5, ends ());
	  fread (s, 0x0200, 1, input);
	  test (!ferror (input), 5, ends ());
	  s += 0x200;
	}
      verboseU8Array (SDepot, len, 0x0200);
    }


  /* extract the root block list */
  verbose ("extract root block depot (root) block list");
  root_list = malloc (ENTRYCHUNK * 4);
  test (root_list != NULL, 10, ends ());
  root_list[0] = fil_sreadU32 (Block + 0x30);
  for (len = 1; root_list[len - 1] != 0xfffffffeUL; len++)
    {
      test (len != 0, 5, ends ());	/* means file is too long */
      /* if memory allocated in root_list is all used, allocate more memory */
      if (!(len % ENTRYCHUNK))
	{
	  U32 *newspace;
	  newspace = realloc (root_list,
			      (1 + len / ENTRYCHUNK) * ENTRYCHUNK * 4);
	  test (newspace != NULL, 10, ends ());
	  root_list = newspace;
	}
      root_list[len] = fil_sreadU32 (BDepot + (root_list[len - 1] * 4));
      test (root_list[len] != 0xfffffffdUL && root_list[len] != 0xffffffffUL,
	    5, ends ());
    }
  len--;
  verboseU32Array (root_list, len);
  /* read in root block depot */
  verbose ("read in root block depot (Root)");
  Root = malloc (0x0200 * len);
  test (Root != NULL, 10, ends ());
  s = Root;
  for (i = 0; i < len; i++)
    {
      FilePos = 0x0200 * (root_list[i] + 1);
      assert (FilePos >= 0);
      test (!fseek (input, FilePos, SEEK_SET), 5, ends ());
      fread (s, 0x0200, 1, input);
      test (!ferror (input), 5, ends ());
      s += 0x200;
    }
  verboseU8Array (Root, len, 0x0200);


  /* assign space for pps list */
  verbose ("read pps list");
  num_of_pps = len * 4;		/* each sbd block have 4 pps */
  *stream_list = pps_list = malloc (num_of_pps * sizeof (pps_entry));
  test (pps_list != NULL, 10, ends ());
  /* read pss entry details and look out for "Root Entry" */
  verbose ("read pps entry details");
  for (i = 0; i < num_of_pps; i++)
    {
      U16 size_of_name;

      s = Root + (i * 0x80);

      /* read the number */
      pps_list[i].ppsnumber = i;

      /* read the name */
      size_of_name = fil_sreadU16 (s + 0x40);
      pps_list[i].name[0] = 0;
      if (size_of_name == 0)
	continue;
      for (p = (unsigned char *) pps_list[i].name, t = s;
	   t < s + size_of_name; t++)
	*p++ = *t++;
      /* makes visible the non printable first character */
      if (!isprint (pps_list[i].name[0]) && pps_list[i].name[0])
	pps_list[i].name[0] += 'a';

      /* read the type */
      pps_list[i].type = fil_sreadU8 (s + 0x42);
      if (pps_list[i].type == 5)
	{
	  assert (i == 0);
	  *root = i;		/* this pps is the root */
	}

      /* read the others fields */
      pps_list[i].previous = fil_sreadU32 (s + 0x44);
      pps_list[i].next = fil_sreadU32 (s + 0x48);
      pps_list[i].dir = fil_sreadU32 (s + 0x4c);
      pps_list[i].start = fil_sreadU32 (s + 0x74);
      pps_list[i].size = fil_sreadU32 (s + 0x78);
      pps_list[i].seconds1 = fil_sreadU32 (s + 0x64);
      pps_list[i].seconds2 = fil_sreadU32 (s + 0x6c);
      pps_list[i].days1 = fil_sreadU32 (s + 0x68);
      pps_list[i].days2 = fil_sreadU32 (s + 0x70);
    }

  /* NEXT IS VERBOSE verbose */
#ifdef VERBOSE
  {
    U32 i;
    printf ("before reorder pps tree\n");
    printf ("pps    type    prev     next      dir start   level size     name\n");
    for (i = 0; i < num_of_pps; i++)
      {
	if (!pps_list[i].name[0])
	  continue;
	printf ("%08lx ", pps_list[i].ppsnumber);
	printf ("%d ", pps_list[i].type);
	printf ("%08lx ", pps_list[i].previous);
	printf ("%08lx ", pps_list[i].next);
	printf ("%08lx ", pps_list[i].dir);
	printf ("%08lx ", pps_list[i].start);
	printf ("%04x ", pps_list[i].level);
	printf ("%08lx ", pps_list[i].size);
	printf ("%s\n", pps_list[i].name);
      }
  }
#endif

  /* go through the tree made with pps entries, and reorder it so only the
     next link is used (move the previous-link-children to the last visited
     next-link-children) */
  reorder_pps_tree (&pps_list[*root], 0);

  /* NEXT IS VERBOSE verbose */
#ifdef VERBOSE
  {
    U32 i;
    printf ("after reorder pps tree\n");
    printf ("pps    type    prev     next      dir start   level size     name\n");
    for (i = 0; i < num_of_pps; i++)
      {
	if (!pps_list[i].name[0])
	  continue;
	printf ("%08lx ", pps_list[i].ppsnumber);
	printf ("%d ", pps_list[i].type);
	printf ("%08lx ", pps_list[i].previous);
	printf ("%08lx ", pps_list[i].next);
	printf ("%08lx ", pps_list[i].dir);
	printf ("%08lx ", pps_list[i].start);
	printf ("%04x ", pps_list[i].level);
	printf ("%08lx ", pps_list[i].size);
	printf ("%s\n", pps_list[i].name);
      }
  }
#endif
#ifdef VERBOSE
  /* NEXT IS VERBOSE verbose */
  verbosePPSTree (*root, 0);
#endif


  /* generates pps real files */
  /* NOTE: by this moment, the pps tree,
     wich is made with pps_list entries, is reordered */
  verbose ("create pps files");
  {
    U8 *Depot;
    FILE *OLEfile, *infile;
    U16 BlockSize, Offset;
    size_t bytes_to_read;
    U32 pps_size, pps_start;
#define THEMIN(a,b) ((a)<(b) ? (a) : (b))

    for (i = 0; i < num_of_pps; i++)
      {
	/* storage pps and non-valid-pps does not need files */
	if (pps_list[i].type == 1 || !pps_list[i].name[0])
	  {
	    pps_list[i].filename[0] = 0;
	    continue;
	  }
	/* pps that have level > max_level will not be extracterd */
	if (max_level != 0 && pps_list[i].level > max_level)
	  {
	    pps_list[i].filename[0] = 0;
	    continue;
	  }

	pps_start = pps_list[i].start;
	pps_size = pps_list[i].size;

	pps_list[i].filename[0] = 0;
	/* create the new file */
	if (pps_list[i].type == 5)
	  /* root entry, sbfile must be generated */
	  {
	    assert (i == *root);
	    assert (i == 0);
	    tmpnam (sbfilename);
	    test (sbfilename[0], 7, ends ());
	    sbfile = OLEfile = fopen (sbfilename, "wb+");
	    test (OLEfile != NULL, 7, ends ());
	    verboseS (sbfilename);
	  }
	else
	  /* other entry, save in a file */
	  {
	    tmpnam (pps_list[i].filename);
	    test (pps_list[i].filename[0], 7, ends ());
	    verbose (pps_list[i].name);
	    OLEfile = fopen (pps_list[i].filename, "wb");
	    test (OLEfile != NULL, 7, ends ());
	    verbose (pps_list[i].filename);
	  }

	if (pps_size >= 0x1000 /*is in bbd */  ||
	    OLEfile == sbfile  /*is root */ )
	  {
	    /* read from big block depot */
	    Offset = 1;
	    BlockSize = 0x0200;
	    infile = input;
	    Depot = BDepot;
	  }
	else
	  {
	    /* read from small block file */
	    Offset = 0;
	    BlockSize = 0x40;
	    infile = sbfile;
	    Depot = SDepot;
	  }

	/* -2 signed long int == 0xfffffffe unsinged long int */
	while (pps_start != 0xfffffffeUL)
	  {
#ifdef VERBOSE
	    printf ("reading pps %08lx block %08lx from %s\n",
		    pps_list[i].ppsnumber, pps_start,
		 Depot == BDepot ? "big block depot" : "small block depot");
#endif
	    FilePos = (pps_start + Offset) * BlockSize;
	    assert (FilePos >= 0);
	    bytes_to_read = THEMIN (BlockSize, pps_size);
	    fseek (infile, FilePos, SEEK_SET);
	    fread (Block, bytes_to_read, 1, infile);
	    test (!ferror (infile), 5, ends ());
	    fwrite (Block, bytes_to_read, 1, OLEfile);
	    test (!ferror (infile), 5, ends ());
	    assert (Depot != NULL);	/* may be sbd no contains blocks */
	    pps_start = fil_sreadU32 (Depot + (pps_start * 4));
	    pps_size -= THEMIN (BlockSize, pps_size);
	    if (pps_size == 0)
	      pps_start = 0xfffffffeUL;

	  }
	if (OLEfile == sbfile)
	  /* if small block file generated */
	  rewind (OLEfile);	/* rewind because we will reader later */
	else if (!fclose (OLEfile))	/* close the pps file */
	  /* don't know what to do here */
	  ;
      }
    if (sbfile != NULL)
      {
	fclose (sbfile);
	if (!remove (sbfilename))
	  /* don't know what to do here */
	  ;
	sbfile = NULL;
      }
  }
  ends ();
  return 0;
}


/* reorder pps tree and write levels */
/* not sure if it is safe declare last_next_link_visited inside reorder_pps_tree */
static U32 *last_next_link_visited;
void
reorder_pps_tree (pps_entry * node, U16 level)
{
	static int depth;
	depth++;
  /* NOTE: in next, previous and dir link,
     0xffffffff means point to nowhere (NULL) */

  if (depth == 50)
  	{
  	fprintf(stderr,"this ole tree appears far too deep\n");
	depth--;
  	return;
	}

  node->level = level;

  /* reorder subtrees, if there's any */
  if (node->dir != 0xffffffffUL)
  	{
	fprintf(stderr,"1here , %d\n",node->dir);
	fprintf(stderr,"2here , %d\n",num_of_pps);
    reorder_pps_tree (&pps_list[node->dir], level+1);
	}

  /* reorder next-link subtree, saving the most next link visited */
  if (node->next != 0xffffffffUL)
    reorder_pps_tree (&pps_list[node->next], level);
  else
    last_next_link_visited = &node->next;

  /* move the prev child to the next link and reorder it, if there's any */
  if (node->previous != 0xffffffffUL)
    {
      *last_next_link_visited = node->previous;
      node->previous = 0xffffffffUL;
      reorder_pps_tree (&pps_list[*last_next_link_visited], level);
    }
	depth--;
}


/* verbose pps tree */
void
verbosePPSTree (U32 start_entry, int level)
{
  U32 entry;
  int i;

  for (entry = start_entry; entry != 0xffffffffUL; entry = pps_list[entry].next)
    {
      if (pps_list[entry].type == 2)
	{
	  for (i = 0; i < level * 3; i++)
	    printf (" ");
	  printf ("FILE %02lx %5ld %s\n", pps_list[entry].ppsnumber,
		  pps_list[entry].size, pps_list[entry].name);
	}
      else
	{
	  for (i = 0; i < level * 3; i++)
	    printf (" ");
	  printf ("DIR  %02lx %s\n", pps_list[entry].ppsnumber,
		  pps_list[entry].name);
	  verbosePPSTree (pps_list[entry].dir, level + 1);
	}
    }
}


#define freeNoNULL(x) { if ((x) != NULL) free (x); }

void
closeOLEtreefiles (pps_entry * tree, U32 root)
{
  if (tree[root].previous != 0xffffffffUL)
    closeOLEtreefiles (tree, tree[root].previous);
  if (tree[root].next != 0xffffffffUL)
    closeOLEtreefiles (tree, tree[root].next);
  if ((tree[root].type != 2) && (tree[root].dir != 0xffffffffUL))
    closeOLEtreefiles (tree, tree[root].dir);
  else if (!remove (tree[root].filename))
    /* I don't know what to do here, may be print a message? */
    ;
}

void
freeOLEtree (pps_entry * tree)
{
  closeOLEtreefiles (tree, 0);
  freeNoNULL (tree);
}


/* free memory used (except the pps tree) */
void
ends (void)
{
  if (input != NULL)
    fclose (input);
  freeNoNULL (Block);
  freeNoNULL (BDepot);
  freeNoNULL (SDepot);
  freeNoNULL (Root);
  freeNoNULL (sbd_list);
  freeNoNULL (root_list);
  if (sbfile != NULL)
    {
      fclose (sbfile);
      if (!remove (sbfilename))
	/* I don't know what to do here, may be print a message? */
	;
    }
}
