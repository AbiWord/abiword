/*
 * lookup.c - see if a word appears in the dictionary
 *
 * Pace Willisson, 1983
 *
 * Copyright 1987, 1988, 1989, 1992, 1993, Geoff Kuenning, Granada Hills, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Geoff Kuenning and
 *      other unpaid contributors.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Log$
 * Revision 1.12  2003/01/06 18:48:39  dom
 * ispell cleanup, start of using new 'add' save features
 *
 * Revision 1.11  2002/09/19 05:31:17  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.10  2002/09/17 03:03:30  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.9  2002/09/13 17:20:13  mpritchett
 * Fix more warnings for Linux build
 *
 * Revision 1.8  2002/05/03 09:49:43  fjfranklin
 * o hash downloader update (Gabriel Gerhardsson)
 * - Comment out the "Can't open <dictionary>" printf.
 * - Make the progressbar more clean at the begining of the download.
 * - Add support for tarballs that doesn't have the full path included
 * - Fix copyright headers on the newly added files (*HashDownloader.*)
 *
 * Revision 1.7  2001/08/27 19:06:30  dom
 * Lots of compilation fixes
 *
 * Revision 1.6  2001/08/10 18:32:40  dom
 * Spelling and iconv updates. god, i hate iconv
 *
 * Revision 1.5  2001/08/10 09:57:49  hub
 * Patch by sobomax@FreeBSD.org
 * #include "iconv.h" directive is missed from src/other/spell/xp/lookup.c and
 * src/wp/impexp/xp/ie_imp_RTF.cpp.
 * See bug 1823
 *
 * Revision 1.4  2001/07/18 17:46:01  dom
 * Module changes, and fix compiler warnings
 *
 * Revision 1.3  2001/06/12 21:32:49  dom
 * More ispell work...
 *
 * Revision 1.2  2001/05/12 16:05:42  thomasf
 * Big pseudo changes to ispell to make it pass around a structure rather
 * than rely on all sorts of gloabals willy nilly here and there.  Also
 * fixed our spelling class to work with accepting suggestions once more.
 * This code is dirty, gross and ugly (not to mention still not supporting
 * multiple hash sized just yet) but it works on my machine and will no
 * doubt break other machines.
 *
 * Revision 1.1  2001/04/15 16:01:24  tomas_f
 * moving to spell/xp
 *
 * Revision 1.7  1999/09/29 23:33:32  justin
 * Updates to the underlying ispell-based code to support suggested corrections.
 *
 * Revision 1.6  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.5  1999/01/07 01:07:48  paul
 * Fixed spell leaks.
 *
 * Revision 1.5  1999/01/07 01:07:48  paul
 * Fixed spell leaks.
 *
 * Revision 1.4  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.3  1998/12/28 23:11:30  eric
 *
 * modified spell code and integration to build on Windows.
 * This is still a hack.
 *
 * Actually, it doesn't yet WORK on Windows.  It just builds.
 * SpellCheckInit is failing for some reason.
 *
 * Revision 1.2  1998/12/28 22:16:22  eric
 *
 * These changes begin to incorporate the spell checker into AbiWord.  Most
 * of this is a hack.
 *
 * 1.  added other/spell to the -I list in config/abi_defs
 * 2.  replaced other/spell/Makefile with one which is more like
 * 	our build system.
 * 3.  added other/spell to other/Makefile so that the build will now
 * 	dive down and build the spell check library.
 * 4.  added the AbiSpell library to the Makefiles in wp/main
 * 5.  added a call to SpellCheckInit in wp/main/unix/UnixMain.cpp.
 * 	This call is a HACK and should be replaced with something
 * 	proper later.
 * 6.  added code to fv_View.cpp as follows:
 * 	whenever you double-click on a word, the spell checker
 * 	verifies that word and prints its status to stdout.
 *
 * Caveats:
 * 1.  This will break the Windows build.  I'm going to work on fixing it
 * 	now.
 * 2.  This only works if your dictionary is in /usr/lib/ispell/american.hash.
 * 	The dictionary location is currently hard-coded.  This will be
 * 	fixed as well.
 *
 * Anyway, such as it is, it works.
 *
 * Revision 1.1  1998/12/28 18:04:43  davet
 * Spell checker code stripped from ispell.  At this point, there are
 * two external routines...  the Init routine, and a check-a-word routine
 * which returns a boolean value, and takes a 16 bit char string.
 * The code resembles the ispell code as much as possible still.
 *
 * Revision 1.42  1995/01/08  23:23:42  geoff
 * Support MSDOS_BINARY_OPEN when opening the hash file to read it in.
 *
 * Revision 1.41  1994/01/25  07:11:51  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ispell.h"
#include "ut_iconv.h"
#include "msgs.h"

#ifdef INDEXDUMP
static void	dumpindex P ((struct flagptr * indexp, int depth));
#endif /* INDEXDUMP */
static void	clearindex P ((ispell_state_t *istate, struct flagptr * indexp));
static void     initckch P ((ispell_state_t *istate, char *));

int		gnMaskBits = 64;

/*!
 * \param hashname name of the hash file (dictionary)
 *
 * \return
 */
int linit (ispell_state_t *istate, char *hashname)
{
	FILE*	fpHash;
		
    register int	i;
    register struct dent * dp;
    struct flagent *	entry;
    struct flagptr *	ind;
    int			nextchar, x;
    int			viazero;
    register ichar_t *	cp;

    if ((fpHash = fopen (hashname, "rb")) == NULL)
	{
		return (-1);
	}

    istate->hashsize = fread ((char *) &istate->hashheader, 1, sizeof istate->hashheader, fpHash);
    if (istate->hashsize < (int)sizeof(istate->hashheader))
	{
		if (istate->hashsize < 0)
			(void) fprintf (stderr, LOOKUP_C_CANT_READ, hashname);
		else if (istate->hashsize == 0)
			(void) fprintf (stderr, LOOKUP_C_NULL_HASH, hashname);
		else
			(void) fprintf (stderr,
			  LOOKUP_C_SHORT_HASH (istate->hashname, istate->hashsize,
				(int) sizeof istate->hashheader));
		return (-1);
	}
    else if (istate->hashheader.magic != MAGIC)
	{
		(void) fprintf (stderr,
		  LOOKUP_C_BAD_MAGIC (hashname, (unsigned int) MAGIC,
			(unsigned int) istate->hashheader.magic));
		return (-1);
	}
    else if (istate->hashheader.magic2 != MAGIC)
	{
		(void) fprintf (stderr,
		  LOOKUP_C_BAD_MAGIC2 (hashname, (unsigned int) MAGIC,
			(unsigned int) istate->hashheader.magic2));
		return (-1);
	}
/*  else if (hashheader.compileoptions != COMPILEOPTIONS*/
    else if ( 1 != 1
      ||  istate->hashheader.maxstringchars != MAXSTRINGCHARS
      ||  istate->hashheader.maxstringcharlen != MAXSTRINGCHARLEN)
	{
		(void) fprintf (stderr,
		  LOOKUP_C_BAD_OPTIONS ((unsigned int) istate->hashheader.compileoptions,
			istate->hashheader.maxstringchars, istate->hashheader.maxstringcharlen,
			(unsigned int) COMPILEOPTIONS, MAXSTRINGCHARS, MAXSTRINGCHARLEN));
		return (-1);
	}

	{
		istate->hashtbl =
		 (struct dent *)
			calloc ((unsigned) istate->hashheader.tblsize, sizeof (struct dent));
		istate->hashsize = istate->hashheader.tblsize;
		istate->hashstrings = (char *) malloc ((unsigned) istate->hashheader.stringsize);
	}
    istate->numsflags = istate->hashheader.stblsize;
    istate->numpflags = istate->hashheader.ptblsize;
    istate->sflaglist = (struct flagent *)
      malloc ((istate->numsflags + istate->numpflags) * sizeof (struct flagent));
    if (istate->hashtbl == NULL  ||  istate->hashstrings == NULL  ||  istate->sflaglist == NULL)
	{
		(void) fprintf (stderr, LOOKUP_C_NO_HASH_SPACE);
		return (-1);
	}
    istate->pflaglist = istate->sflaglist + istate->numsflags;

	{
		if( fread ( istate->hashstrings, 1, (unsigned)istate->hashheader.stringsize, fpHash) 
			!= ((size_t)(istate->hashheader.stringsize)) )
	    {
		    (void) fprintf (stderr, LOOKUP_C_BAD_FORMAT);
			(void) fprintf (stderr, "stringsize err\n" );
	    	return (-1);
	    }
		if ( istate->hashheader.compileoptions & 0x04 )
		{
			if(  fread ((char *) istate->hashtbl, 1, (unsigned)istate->hashheader.tblsize * sizeof(struct dent), fpHash)
		    	!= ((size_t) (istate->hashheader.tblsize * sizeof (struct dent))))
		    {
			    (void) fprintf (stderr, LOOKUP_C_BAD_FORMAT);
		    	return (-1);
		    }
		}
		else
		{
			for( x=0; x<istate->hashheader.tblsize; x++ )
			{
				if(  fread ( (char*)(istate->hashtbl+x), sizeof( struct dent)-sizeof( MASKTYPE ), 1, fpHash)
			    	!= 1)
			    {
				    (void) fprintf (stderr, LOOKUP_C_BAD_FORMAT);
			    	return (-1);
			    }
			}	/*for*/
		}	/*else*/
	}
    if (fread ((char *) istate->sflaglist, 1,
	(unsigned) (istate->numsflags + istate->numpflags) * sizeof (struct flagent), fpHash)
      != (istate->numsflags + istate->numpflags) * sizeof (struct flagent))
	{
		(void) fprintf (stderr, LOOKUP_C_BAD_FORMAT);
		return (-1);
	}
    (void) fclose (fpHash);

	{
		for (i = istate->hashsize, dp = istate->hashtbl;  --i >= 0;  dp++)
		{
			if (dp->word == (char *) -1)
				dp->word = NULL;
			else
				dp->word = &istate->hashstrings [ (int)(dp->word) ];
			if (dp->next == (struct dent *) -1)
				dp->next = NULL;
			else
				dp->next = &istate->hashtbl [ (int)(dp->next) ];
	    }
	}

    for (i = istate->numsflags + istate->numpflags, entry = istate->sflaglist; --i >= 0; entry++)
	{
		if (entry->stripl)
			entry->strip = (ichar_t *) &istate->hashstrings[(int) entry->strip];
		else
			entry->strip = NULL;
		if (entry->affl)
			entry->affix = (ichar_t *) &istate->hashstrings[(int) entry->affix];
		else
			entry->affix = NULL;
	}
    /*
    ** Warning - 'entry' and 'i' are reset in the body of the loop
    ** below.  Don't try to optimize it by (e.g.) moving the decrement
    ** of i into the loop condition.
    */
    for (i = istate->numsflags, entry = istate->sflaglist;  i > 0;  i--, entry++)
	{
		if (entry->affl == 0)
		{
			cp = NULL;
			ind = &istate->sflagindex[0];
			viazero = 1;
		}
		else
		{
			cp = entry->affix + entry->affl - 1;
			ind = &istate->sflagindex[*cp];
			viazero = 0;
			while (ind->numents == 0  &&  ind->pu.fp != NULL)
			{
				if (cp == entry->affix)
				{
					ind = &ind->pu.fp[0];
					viazero = 1;
				}
				else
				{
					ind = &ind->pu.fp[*--cp];
					viazero = 0;
				}
			}
		}
		if (ind->numents == 0)
			ind->pu.ent = entry;
		ind->numents++;
		/*
		** If this index entry has more than MAXSEARCH flags in
		** it, we will split it into subentries to reduce the
		** searching.  However, the split doesn't make sense in
		** two cases:  (a) if we are already at the end of the
		** current affix, or (b) if all the entries in the list
		** have identical affixes.  Since the list is sorted, (b)
		** is true if the first and last affixes in the list
		** are identical.
		*/
		if (!viazero  &&  ind->numents >= MAXSEARCH
		  &&  icharcmp (entry->affix, ind->pu.ent->affix) != 0)
		{
			/* Sneaky trick:  back up and reprocess */
			entry = ind->pu.ent - 1; /* -1 is for entry++ in loop */
			i = istate->numsflags - (entry - istate->sflaglist);
			ind->pu.fp =
			  (struct flagptr *)
			calloc ((unsigned) (SET_SIZE + istate->hashheader.nstrchars),
			  sizeof (struct flagptr));
			if (ind->pu.fp == NULL)
			{
				(void) fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
				return (-1);
			}
			ind->numents = 0;
		}
	}
    /*
    ** Warning - 'entry' and 'i' are reset in the body of the loop
    ** below.  Don't try to optimize it by (e.g.) moving the decrement
    ** of i into the loop condition.
    */
    for (i = istate->numpflags, entry = istate->pflaglist;  i > 0;  i--, entry++)
	{
		if (entry->affl == 0)
	    {
			cp = NULL;
			ind = &istate->pflagindex[0];
			viazero = 1;
	    }
		else
		{
			cp = entry->affix;
			ind = &istate->pflagindex[*cp++];
			viazero = 0;
			while (ind->numents == 0  &&  ind->pu.fp != NULL)
			{
				if (*cp == 0)
				{
					ind = &ind->pu.fp[0];
					viazero = 1;
				}
				else
				{
					ind = &ind->pu.fp[*cp++];
					viazero = 0;
				}
			}
		}
		if (ind->numents == 0)
			ind->pu.ent = entry;
		ind->numents++;
		/*
		** If this index entry has more than MAXSEARCH flags in
		** it, we will split it into subentries to reduce the
		** searching.  However, the split doesn't make sense in
		** two cases:  (a) if we are already at the end of the
		** current affix, or (b) if all the entries in the list
		** have identical affixes.  Since the list is sorted, (b)
		** is true if the first and last affixes in the list
		** are identical.
		*/
		if (!viazero  &&  ind->numents >= MAXSEARCH
		  &&  icharcmp (entry->affix, ind->pu.ent->affix) != 0)
		{
			/* Sneaky trick:  back up and reprocess */
			entry = ind->pu.ent - 1; /* -1 is for entry++ in loop */
			i = istate->numpflags - (entry - istate->pflaglist);
			ind->pu.fp =
			  (struct flagptr *) calloc (SET_SIZE + istate->hashheader.nstrchars,
				sizeof (struct flagptr));
			if (ind->pu.fp == NULL)
			{
				(void) fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
				return (-1);
			}
			ind->numents = 0;
		}
	}
#ifdef INDEXDUMP
    (void) fprintf (stderr, "Prefix index table:\n");
    dumpindex (istate->pflagindex, 0);
    (void) fprintf (stderr, "Suffix index table:\n");
    dumpindex (istate->sflagindex, 0);
#endif
    if (istate->hashheader.nstrchartype == 0)
		istate->chartypes = NULL;
    else
	{
		istate->chartypes = (struct strchartype *)
		  malloc (istate->hashheader.nstrchartype * sizeof (struct strchartype));
		if (istate->chartypes == NULL)
		{
			(void) fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
			return (-1);
		}
		for (i = 0, nextchar = istate->hashheader.strtypestart;
		  i < istate->hashheader.nstrchartype;
		  i++)
		{
			istate->chartypes[i].name = &istate->hashstrings[nextchar];
			nextchar += strlen (istate->chartypes[i].name) + 1;
			istate->chartypes[i].deformatter = &istate->hashstrings[nextchar];
			nextchar += strlen (istate->chartypes[i].deformatter) + 1;
			istate->chartypes[i].suffixes = &istate->hashstrings[nextchar];
			while (istate->hashstrings[nextchar] != '\0')
				nextchar += strlen (&istate->hashstrings[nextchar]) + 1;
			nextchar++;
		}
	}

    initckch(istate, NULL);   
       
    return (0);
}

#ifndef FREEP
#define FREEP(p)	do { if (p) free(p); } while (0)
#endif

/*!
 * \param wchars Characters in -w option, if any
 */
static void initckch (ispell_state_t *istate, char *wchars)
{
	register ichar_t    c;
	char                num[4];

	for (c = 0; c < (ichar_t) (SET_SIZE + istate->hashheader.nstrchars); ++c)
    {
		if (iswordch (istate, c))
		{
			if (!mylower (istate, c))
			{
				istate->Try[istate->Trynum] = c;
				++istate->Trynum;
			}
		}
		else if (isboundarych (istate, c))
		{
			istate->Try[istate->Trynum] = c;
			++istate->Trynum;
		}
	}
	if (wchars != NULL)
    {
		while (istate->Trynum < SET_SIZE  &&  *wchars != '\0')
		{
			if (*wchars != 'n'  &&  *wchars != '\\')
			{
				c = *wchars;
				++wchars;
			}
			else
			{
			    ++wchars;
			    num[0] = '\0';
			    num[1] = '\0';
			    num[2] = '\0';
			    num[3] = '\0';
			    if (isdigit (wchars[0]))
				{
				    num[0] = wchars[0];
				    if (isdigit (wchars[1]))
				    {
						num[1] = wchars[1];
						if (isdigit (wchars[2]))
							num[2] = wchars[2];
					}
				}
				if (wchars[-1] == 'n')
				{
				    wchars += strlen (num);
				    c = atoi (num);
				}
				else
				{
				    wchars += strlen (num);
				    c = 0;
				    if (num[0])
						c = num[0] - '0';
				    if (num[1])
				    {
						c <<= 3;
						c += num[1] - '0';
					}
					if (num[2])
					{
						c <<= 3;
						c += num[2] - '0';
					}
				}
			}
/*	    	c &= NOPARITY;*/
			if (!istate->hashheader.wordchars[c])
			{
				istate->hashheader.wordchars[c] = 1;
				istate->hashheader.sortorder[c] = istate->hashheader.sortval++;
				istate->Try[istate->Trynum] = c;
				++istate->Trynum;
			}
		}
    }
}

void lcleanup(ispell_state_t *istate)
{
	clearindex (istate, istate->pflagindex);
	clearindex (istate, istate->sflagindex);

	FREEP(istate->hashtbl);
	FREEP(istate->hashstrings);
	FREEP(istate->sflaglist);
	FREEP(istate->chartypes);
}

/*
 * \param indexp
 */
static void clearindex (ispell_state_t *istate, struct flagptr *indexp)
{
    register int		i;
    for (i = 0;  i < SET_SIZE + istate->hashheader.nstrchars;  i++, indexp++)
	{
		if (indexp->numents == 0 && indexp->pu.fp != NULL)
		{
		    clearindex(istate, indexp->pu.fp);
			free(indexp->pu.fp);
		}
	}
}
	
#ifdef INDEXDUMP
static void dumpindex (indexp, depth)
    register struct flagptr *	indexp;
    register int		depth;
{
    register int		i;
    int				j;
    int				k;
    char			stripbuf[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];

    for (i = 0;  i < SET_SIZE + hashheader.nstrchars;  i++, indexp++)
	{
		if (indexp->numents == 0  &&  indexp->pu.fp != NULL)
	    {
			for (j = depth;  --j >= 0;  )
				(void) putc (' ', stderr);
			if (i >= ' '  &&  i <= '~')
				(void) putc (i, stderr);
			else
				(void) fprintf (stderr, "0x%x", i);
			(void) putc ('\n', stderr);
			dumpindex (indexp->pu.fp, depth + 1);
	    }
		else if (indexp->numents)
		{
			for (j = depth;  --j >= 0;  )
				(void) putc (' ', stderr);
			if (i >= ' '  &&  i <= '~')
				(void) putc (i, stderr);
			else
				(void) fprintf (stderr, "0x%x", i);
			(void) fprintf (stderr, " -> %d entries\n", indexp->numents);
			for (k = 0;  k < indexp->numents;  k++)
			{
				for (j = depth;  --j >= 0;  )
					(void) putc (' ', stderr);
				if (indexp->pu.ent[k].stripl)
				{
					(void) ichartostr (stripbuf, indexp->pu.ent[k].strip,
					  sizeof stripbuf, 1);
					(void) fprintf (stderr, "     entry %d (-%s,%s)\n",
					  &indexp->pu.ent[k] - sflaglist,
					  stripbuf,
					  indexp->pu.ent[k].affl
						? ichartosstr (indexp->pu.ent[k].affix, 1) : "-");
				}
				else
					(void) fprintf (stderr, "     entry %d (%s)\n",
					  &indexp->pu.ent[k] - sflaglist,
					  ichartosstr (indexp->pu.ent[k].affix, 1));
			}
		}
	}
}
#endif

/* n is length of s */

/*
 * \param s
 * \param dotree
 *
 * \return
 */
struct dent * ispell_lookup (ispell_state_t *istate, ichar_t *s, int dotree)
{
    register struct dent *	dp;
    register char *		s1;
    char			schar[INPUTWORDLEN + MAXAFFIXLEN];

    dp = &istate->hashtbl[hash (istate, s, istate->hashsize)];
    if (ichartostr (istate, schar, s, sizeof schar, 1))
		(void) fprintf (stderr, WORD_TOO_LONG (schar));
    for (  ;  dp != NULL;  dp = dp->next)
	{
		/* quick strcmp, but only for equality */
		s1 = dp->word;
		if (s1  &&  s1[0] == schar[0]  &&  strcmp (s1 + 1, schar + 1) == 0)
			return dp;
#ifndef NO_CAPITALIZATION_SUPPORT
		while (dp->flagfield & MOREVARIANTS)	/* Skip variations */
			dp = dp->next;
#endif
	}
	return NULL;
}

ispell_state_t *alloc_ispell_struct()
{
	ispell_state_t *ret;
	ret = (ispell_state_t *)calloc(1, sizeof(ispell_state_t));
	ret->translate_in = 
	ret->translate_out = (UT_iconv_t)-1;

	return ret;
}

void free_ispell_struct(ispell_state_t *istate)
{
	if (istate)
	{
		if (UT_iconv_isValid(istate->translate_in))
			UT_iconv_close (istate->translate_in);
		if (UT_iconv_isValid(istate->translate_out))
			UT_iconv_close (istate->translate_out);
	  
		free(istate);
	}
}
