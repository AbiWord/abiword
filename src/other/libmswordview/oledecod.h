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
#ifndef OLEDECOD_H
#define OLEDECOD_H


#include <stdio.h>

#include "support.h"


struct pps_block
  {
    char name[0x20];
    char filename[L_tmpnam];	/* valid only if type == 2 */
    U8 type;			/* 5 == root, 1 == dir, 2 == file */
    U32 size;			/* the size of the file, valid only if type == 2 */
    U32 next;			/* next entry in this level, in this directory */
    U32 dir;			/* valid only if type != 2 */
    U16 level;			/* level in the ole tree */
    U32 seconds1;
    U32 seconds2;
    U32 days1;
    U32 days2;

    /* private fields, used only inside OLEdecoded and useless after */
    U32 previous;		/* previous pps, valid before reordering */
    U32 ppsnumber;		/* pps number */
    U32 start;			/* start block */
  };
typedef struct pps_block pps_entry;


/*
   Input: char *Olefilename        = File to be decoded (ie. .xsl, .doc, .ppt).
   .      pps_entry ** stream_list = The stream tree.
   .      U32 * root               = The number of root dir in stream_list.
   .      U16 max_level            = The maximum level on stream tree in which
   .                                 streams will be actually extracted
   .                                 to a file. 0 (zero) means extract all.
   Output: 0 = Sucess.
   .       4 = Couldn't open OLEfilename file (can use perror).
   .       8 = OLEfilename file seems to contain plain text, not OLE file.
   .       9 = OLEfilename file has a faulty OLEfile format.
   .       5 = Error reading from file, means OLEfilename file has a faulty
   .           OLE file format.
   .       6 = Error removing temporal files.
   .       7 = Error creating temporal files.
   .       10 = Error allocating memory, there's no more memory.
 */
int OLEdecode (char *OLEfilename, pps_entry ** stream_list, U32 * root,
	       U16 max_level);


/*
   Free all the memory allocated in the tree.
 */
void freeOLEtree (pps_entry * tree);


#endif /* OLEDECOD_H */
