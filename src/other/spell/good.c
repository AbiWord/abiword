#ifndef lint
static char Rcs_Id[] =
    "$Id$";
#endif

/*
 * good.c - see if a word or its root word
 * is in the dictionary.
 *
 * Pace Willisson, 1983
 *
 * Copyright 1992, 1993, Geoff Kuenning, Granada Hills, CA
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
 * Revision 1.5  2000/02/09 22:35:25  sterwill
 * Clean up some warnings
 *
 * Revision 1.4  1998/12/29 14:55:32  eric
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
 * Revision 1.43  1994/11/02  06:56:05  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.42  1994/10/25  05:45:59  geoff
 * Add support for an affix that will work with any word, even if there's
 * no explicit flag.
 *
 * Revision 1.41  1994/05/24  06:23:06  geoff
 * Let tgood decide capitalization questions, rather than doing it ourselves.
 *
 * Revision 1.40  1994/05/17  06:44:10  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.39  1994/01/25  07:11:31  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ispell.h"

int		good P ((ichar_t * word, int ignoreflagbits, int allhits,
		  int pfxopts, int sfxopts));
#ifndef NO_CAPITALIZATION_SUPPORT
int		cap_ok P ((ichar_t * word, struct success * hit, int len));
static int	entryhasaffixes P ((struct dent * dent, struct success * hit));
#endif /* NO_CAPITALIZATION_SUPPORT */
void		flagpr P ((ichar_t * word, int preflag, int prestrip,
		  int preadd, int sufflag, int sufadd));

#if 0 /* DELETE_ME */
static ichar_t *	orig_word;
#endif /* DELETE_ME */

#ifndef NO_CAPITALIZATION_SUPPORT
int good (w, ignoreflagbits, allhits, pfxopts, sfxopts)
    ichar_t *		w;		/* Word to look up */
    int			ignoreflagbits;	/* NZ to ignore affix flags in dict */
    int			allhits;	/* NZ to ignore case, get every hit */
    int			pfxopts;	/* Options to apply to prefixes */
    int			sfxopts;	/* Options to apply to suffixes */
#else
/* ARGSUSED */
int good (w, ignoreflagbits, dummy, pfxopts, sfxopts)
    ichar_t *		w;		/* Word to look up */
    int			ignoreflagbits;	/* NZ to ignore affix flags in dict */
    int			dummy;
#define allhits	0	/* Never actually need more than one hit */
    int			pfxopts;	/* Options to apply to prefixes */
    int			sfxopts;	/* Options to apply to suffixes */
#endif
    {
    ichar_t		nword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t *	q;
    register int	n;
    register struct dent * dp;

    /*
    ** Make an uppercase copy of the word we are checking.
    */
    for (p = w, q = nword;  *p;  )
	*q++ = mytoupper (*p++);
    *q = 0;
    n = q - nword;

    numhits = 0;

#if 0 /* DELETE_ME */
    if (cflag)
	{
	(void) printf ("%s", ichartosstr (w, 0));
	orig_word = w;
	}
    else 
#endif /* DELETE_ME */
    if ((dp = ispell_lookup (nword, 1)) != NULL)
	{
	hits[0].dictent = dp;
	hits[0].prefix = NULL;
	hits[0].suffix = NULL;
#ifndef NO_CAPITALIZATION_SUPPORT
	if (allhits  ||  cap_ok (w, &hits[0], n))
	    numhits = 1;
#else
	numhits = 1;
#endif
	/*
	 * If we're looking for compounds, and this root doesn't
	 * participate in compound formation, undo the hit.
	 */

#if 0 /*  DELETE_ME */
	if (compoundflag == COMPOUND_CONTROLLED
	  &&  ((pfxopts | sfxopts) & FF_COMPOUNDONLY) != 0
	  &&  hashheader.compoundbit >= 0
	  &&  TSTMASKBIT (dp->mask, hashheader.compoundbit) == 0)
	    numhits = 0;
#endif /* DELETE_ME */
	}

    if (numhits  &&  !allhits)
	return 1;

    /* try stripping off affixes */

#if 0
    numchars = icharlen (nword);
    if (numchars < 4)
	{
	if (cflag)
	    (void) putchar ('\n');
	return numhits  ||  (numchars == 1);
	}
#endif

    chk_aff (w, nword, n, ignoreflagbits, allhits, pfxopts, sfxopts);

#if 0 /* DELETE_ME */
    if (cflag)
	(void) putchar ('\n');

#endif /* DELETE_ME */

    return numhits;
    }

#ifndef NO_CAPITALIZATION_SUPPORT
int cap_ok (word, hit, len)
    register ichar_t *		word;
    register struct success *	hit;
    int				len;
    {
    register ichar_t *		dword;
    register ichar_t *		w;
    register struct dent *	dent;
    ichar_t			dentword[INPUTWORDLEN + MAXAFFIXLEN];
    int				preadd;
    int				prestrip;
    int				sufadd;
    ichar_t *			limit;
    long			thiscap;
    long			dentcap;

    thiscap = whatcap (word);
    /*
    ** All caps is always legal, regardless of affixes.
    */
    preadd = prestrip = sufadd = 0;
    if (thiscap == ALLCAPS)
	return 1;
    else if (thiscap == FOLLOWCASE)
	{
	/* Set up some constants for the while(1) loop below */
	if (hit->prefix)
	    {
	    preadd = hit->prefix->affl;
	    prestrip = hit->prefix->stripl;
	    }
	else
	    preadd = prestrip = 0;
	sufadd = hit->suffix ? hit->suffix->affl : 0;
	}
    /*
    ** Search the variants for one that matches what we have.  Note
    ** that thiscap can't be ALLCAPS, since we already returned
    ** for that case.
    */
    dent = hit->dictent;
    for (  ;  ;  )
	{
	dentcap = captype (dent->flagfield);
	if (dentcap != thiscap)
	    {
	    if (dentcap == ANYCASE  &&  thiscap == CAPITALIZED
	     &&  entryhasaffixes (dent, hit))
		return 1;
	    }
	else				/* captypes match */
	    {
	    if (thiscap != FOLLOWCASE)
		{
		if (entryhasaffixes (dent, hit))
		    return 1;
		}
	    else
		{
		/*
		** Make sure followcase matches exactly.
		** Life is made more difficult by the
		** possibility of affixes.  Start with
		** the prefix.
		*/
		(void) strtoichar (dentword, dent->word, INPUTWORDLEN, 1);
		dword = dentword;
		limit = word + preadd;
		if (myupper (dword[prestrip]))
		    {
		    for (w = word;  w < limit;  w++)
			{
			if (mylower (*w))
			    goto doublecontinue;
			}
		    }
		else
		    {
		    for (w = word;  w < limit;  w++)
			{
			if (myupper (*w))
			    goto doublecontinue;
			}
		    }
		dword += prestrip;
		/* Do root part of word */
		limit = dword + len - preadd - sufadd;
		while (dword < limit)
		    {
		    if (*dword++ != *w++)
		      goto doublecontinue;
		    }
		/* Do suffix */
		dword = limit - 1;
		if (myupper (*dword))
		    {
		    for (  ;  *w;  w++)
			{
			if (mylower (*w))
			    goto doublecontinue;
			}
		    }
		else
		    {
		    for (  ;  *w;  w++)
			{
			if (myupper (*w))
			    goto doublecontinue;
			}
		    }
		/*
		** All failure paths go to "doublecontinue,"
		** so if we get here it must match.
		*/
		if (entryhasaffixes (dent, hit))
		    return 1;
doublecontinue:	;
		}
	    }
	if ((dent->flagfield & MOREVARIANTS) == 0)
	    break;
	dent = dent->next;
	}

    /* No matches found */
    return 0;
    }

/*
** See if this particular capitalization (dent) is legal with these
** particular affixes.
*/
static int entryhasaffixes (dent, hit)
    register struct dent *	dent;
    register struct success *	hit;
    {

    if (hit->prefix  &&  !TSTMASKBIT (dent->mask, hit->prefix->flagbit))
	return 0;
    if (hit->suffix  &&  !TSTMASKBIT (dent->mask, hit->suffix->flagbit))
	return 0;
    return 1;			/* Yes, these affixes are legal */
    }
#endif

