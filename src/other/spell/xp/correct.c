#ifndef lint
static char Rcs_Id[] =
    "$Id$";
#endif

/*
 * correct.c - Routines to manage the higher-level aspects of spell-checking
 *
 * This code originally resided in ispell.c, but was moved here to keep
 * file sizes smaller.
 *
 * Copyright (c), 1983, by Pace Willisson
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
 * Revision 1.2  1999/10/05 16:17:28  paul
 * Fixed build, and other tidyness.
 * Spell dialog enabled by default, with keyboard binding of F7.
 *
 * Revision 1.1  1999/09/29 23:33:32  justin
 * Updates to the underlying ispell-based code to support suggested corrections.
 *
 * Revision 1.59  1995/08/05  23:19:43  geoff
 * Fix a bug that caused offsets for long lines to be confused if the
 * line started with a quoting uparrow.
 *
 * Revision 1.58  1994/11/02  06:56:00  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.57  1994/10/26  05:12:39  geoff
 * Try boundary characters when inserting or substituting letters, except
 * (naturally) at word boundaries.
 *
 * Revision 1.56  1994/10/25  05:46:30  geoff
 * Fix an assignment inside a conditional that could generate spurious
 * warnings (as well as being bad style).  Add support for the FF_ANYWORD
 * option.
 *
 * Revision 1.55  1994/09/16  04:48:24  geoff
 * Don't pass newlines from the input to various other routines, and
 * don't assume that those routines leave the input unchanged.
 *
 * Revision 1.54  1994/09/01  06:06:41  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.53  1994/08/31  05:58:38  geoff
 * Add code to handle extremely long lines in -a mode without splitting
 * words or reporting incorrect offsets.
 *
 * Revision 1.52  1994/05/25  04:29:24  geoff
 * Fix a bug that caused line widths to be calculated incorrectly when
 * displaying lines containing tabs.  Fix a couple of places where
 * characters were sign-extended incorrectly, which could cause 8-bit
 * characters to be displayed wrong.
 *
 * Revision 1.51  1994/05/17  06:44:05  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.50  1994/04/27  05:20:14  geoff
 * Allow compound words to be formed from more than two components
 *
 * Revision 1.49  1994/04/27  01:50:31  geoff
 * Add support to correctly capitalize words generated as a result of a
 * missing-space suggestion.
 *
 * Revision 1.48  1994/04/03  23:23:02  geoff
 * Clean up the code in missingspace() to be a bit simpler and more
 * efficient.
 *
 * Revision 1.47  1994/03/15  06:24:23  geoff
 * Fix the +/-/~ commands to be independent.  Allow the + command to
 * receive a suffix which is a deformatter type (currently hardwired to
 * be either tex or nroff/troff).
 *
 * Revision 1.46  1994/02/21  00:20:03  geoff
 * Fix some bugs that could cause bad displays in the interaction between
 * TeX parsing and string characters.  Show_char now will not overrun
 * the inverse-video display area by accident.
 *
 * Revision 1.45  1994/02/14  00:34:51  geoff
 * Fix correct to accept length parameters for ctok and itok, so that it
 * can pass them to the to/from ichar routines.
 *
 * Revision 1.44  1994/01/25  07:11:22  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ispell.h"
#include "msgs.h"

static int	posscmp P ((FIRST_ARG(istate) char * a, char * b));
int		casecmp P ((FIRST_ARG(istate) char * a, char * b, int canonical));
void		makepossibilities P ((FIRST_ARG(istate) ichar_t * word));
static int	insert P ((FIRST_ARG(istate) ichar_t * word));
#ifndef NO_CAPITALIZATION_SUPPORT
static void	wrongcapital P ((FIRST_ARG(istate) ichar_t * word));
#endif /* NO_CAPITALIZATION_SUPPORT */
static void	wrongletter P ((FIRST_ARG(istate) ichar_t * word));
static void	extraletter P ((FIRST_ARG(istate) ichar_t * word));
static void	missingletter P ((FIRST_ARG(istate) ichar_t * word));
static void	missingspace P ((FIRST_ARG(istate) ichar_t * word));
int		compoundgood P ((FIRST_ARG(istate) ichar_t * word, int pfxopts));
static void	transposedletter P ((FIRST_ARG(istate) ichar_t * word));
static int	ins_cap P ((FIRST_ARG(istate) ichar_t * word, ichar_t * pattern));
static int	save_cap P ((FIRST_ARG(istate) ichar_t * word, ichar_t * pattern,
		  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN]));
int		ins_root_cap P ((FIRST_ARG(istate) ichar_t * word, ichar_t * pattern,
		  int prestrip, int preadd, int sufstrip, int sufadd,
		  struct dent * firstdent, struct flagent * pfxent,
		  struct flagent * sufent));
static void	save_root_cap P ((FIRST_ARG(istate) ichar_t * word, ichar_t * pattern,
		  int prestrip, int preadd, int sufstrip, int sufadd,
		  struct dent * firstdent, struct flagent * pfxent,
		  struct flagent * sufent,
		  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN],
		  int * nsaved));

//extern void upcase P ((ichar_t * string));
//extern void lowcase P ((ichar_t * string));
//extern ichar_t * strtosichar P ((char * in, int canonical));

int compoundflag = COMPOUND_CONTROLLED;

static int posscmp (FIRST_ARG(istate) char *a, char *b)
#if 0
    char *		a;
    char *		b;
#endif
    {

    return casecmp (DEREF_FIRST_ARG(istate) a, b, 0);
    }

int casecmp (FIRST_ARG(istate) char *a, char *b, int canonical)
#if 0
    char *		a;
    char *		b;
    int			canonical;	/* NZ for canonical string chars */
#endif
    {
    register ichar_t *	ap;
    register ichar_t *	bp;
    ichar_t		inta[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
    ichar_t		intb[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];

    (void) strtoichar (DEREF_FIRST_ARG(istate) inta, a, sizeof inta, canonical);
    (void) strtoichar (DEREF_FIRST_ARG(istate) intb, b, sizeof intb, canonical);
    for (ap = inta, bp = intb;  *ap != 0;  ap++, bp++)
	{
	if (*ap != *bp)
	    {
	    if (*bp == '\0')
		return DEREF(istate, hashheader.sortorder[*ap]);
	    else if (mylower (DEREF_FIRST_ARG(istate) *ap))
		{
		if (mylower (DEREF_FIRST_ARG(istate) *bp)  ||  mytoupper (DEREF_FIRST_ARG(istate) *ap) != *bp)
		    return (int) DEREF(istate, hashheader.sortorder[*ap])
		      - (int) DEREF(istate, hashheader.sortorder[*bp]);
		}
	    else
		{
		if (myupper (DEREF_FIRST_ARG(istate) *bp)  ||  mytolower (DEREF_FIRST_ARG(istate) *ap) != *bp)
		    return (int) DEREF(istate, hashheader.sortorder[*ap])
		      - (int) DEREF(istate, hashheader.sortorder[*bp]);
		}
	    }
	}
    if (*bp != '\0')
	return -(int) DEREF(istate, hashheader.sortorder[*bp]);
    for (ap = inta, bp = intb;  *ap;  ap++, bp++)
	{
	if (*ap != *bp)
	    {
	    return (int) DEREF(istate, hashheader.sortorder[*ap])
	      - (int) DEREF(istate, hashheader.sortorder[*bp]);
	    }
	}
    return 0;
    }

void makepossibilities (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    register int	i;

    for (i = 0; i < MAXPOSSIBLE; i++)
	DEREF(istate, possibilities[i][0]) = 0;
    DEREF(istate, pcount) = 0;
    DEREF(istate, maxposslen) = 0;
    DEREF(istate, easypossibilities) = 0;

#ifndef NO_CAPITALIZATION_SUPPORT
    wrongcapital (DEREF_FIRST_ARG(istate) word);
#endif

/* 
 * according to Pollock and Zamora, CACM April 1984 (V. 27, No. 4),
 * page 363, the correct order for this is:
 * OMISSION = TRANSPOSITION > INSERTION > SUBSTITUTION
 * thus, it was exactly backwards in the old version. -- PWP
 */

    if (DEREF(istate, pcount) < MAXPOSSIBLE)
	missingletter (DEREF_FIRST_ARG(istate) word);		/* omission */
    if (DEREF(istate, pcount) < MAXPOSSIBLE)
	transposedletter (DEREF_FIRST_ARG(istate) word);	/* transposition */
    if (DEREF(istate, pcount) < MAXPOSSIBLE)
	extraletter (DEREF_FIRST_ARG(istate) word);		/* insertion */
    if (DEREF(istate, pcount) < MAXPOSSIBLE)
	wrongletter (DEREF_FIRST_ARG(istate) word);		/* substitution */

    if ((compoundflag != COMPOUND_ANYTIME)  &&  DEREF(istate, pcount) < MAXPOSSIBLE)
	missingspace (DEREF_FIRST_ARG(istate) word);	/* two words */

    }

static int insert (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    register int	i;
    register char *	realword;

    realword = ichartosstr (DEREF_FIRST_ARG(istate) word, 0);
    for (i = 0; i < DEREF(istate, pcount); i++)
	{
	if (strcmp (DEREF(istate, possibilities[i]), realword) == 0)
	    return (0);
	}

    (void) strcpy (DEREF(istate, possibilities[DEREF(istate, pcount++)]), realword);
    i = strlen (realword);
    if (i > DEREF(istate, maxposslen))
	DEREF(istate, maxposslen) = i;
    if (DEREF(istate, pcount) >= MAXPOSSIBLE)
	return (-1);
    else
	return (0);
    }

#ifndef NO_CAPITALIZATION_SUPPORT
static void wrongcapital (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    /*
    ** When the third parameter to "good" is nonzero, it ignores
    ** case.  If the word matches this way, "ins_cap" will recapitalize
    ** it correctly.
    */
    if (good (DEREF_FIRST_ARG(istate) word, 0, 1, 0, 0))
	{
	(void) icharcpy (newword, word);
	upcase (DEREF_FIRST_ARG(istate) newword);
	(void) ins_cap (DEREF_FIRST_ARG(istate) newword, word);
	}
    }
#endif

static void wrongletter (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    register int	i;
    register int	j;
    register int	n;
    ichar_t		savechar;
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    n = icharlen (word);
    (void) icharcpy (newword, word);
#ifndef NO_CAPITALIZATION_SUPPORT
    upcase (DEREF_FIRST_ARG(istate) newword);
#endif

    for (i = 0; i < n; i++)
	{
	savechar = newword[i];
	for (j=0; j < DEREF(istate, Trynum); ++j)
	    {
	    if (DEREF(istate, Try[j]) == savechar)
		continue;
	    else if (isboundarych (DEREF_FIRST_ARG(istate) DEREF(istate, Try[j]))  &&  (i == 0  ||  i == n - 1))
		continue;
	    newword[i] = DEREF(istate, Try[j]);
	    if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
		{
		if (ins_cap (DEREF_FIRST_ARG(istate) newword, word) < 0)
		    return;
		}
	    }
	newword[i] = savechar;
	}
    }

static void extraletter (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t *	r;

    if (icharlen (word) < 2)
	return;

    (void) icharcpy (newword, word + 1);
    for (p = word, r = newword;  *p != 0;  )
	{
	if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (DEREF_FIRST_ARG(istate) newword, word) < 0)
		return;
	    }
	*r++ = *p++;
	}
    }

static void missingletter (FIRST_ARG(istate) ichar_t *word)
#if 0
    ichar_t *		word;
#endif
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    register ichar_t *	p;
    register ichar_t *	r;
    register int	i;

    (void) icharcpy (newword + 1, word);
    for (p = word, r = newword;  *p != 0;  )
	{
	for (i = 0;  i < DEREF(istate, Trynum);  i++)
	    {
	    if (isboundarych (DEREF_FIRST_ARG(istate) DEREF(istate, Try[i]))  &&  r == newword)
		continue;
	    *r = DEREF(istate, Try[i]);
	    if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
		{
		if (ins_cap (DEREF_FIRST_ARG(istate) newword, word) < 0)
		    return;
		}
	    }
	*r++ = *p++;
	}
    for (i = 0;  i < DEREF(istate, Trynum);  i++)
	{
	if (isboundarych (DEREF_FIRST_ARG(istate) DEREF(istate, Try[i])))
	    continue;
	*r = DEREF(istate, Try[i]);
	if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (DEREF_FIRST_ARG(istate) newword, word) < 0)
		return;
	    }
	}
    }

static void missingspace (FIRST_ARG(istate) ichar_t *word)
#if 0
    ichar_t *		word;
#endif
    {
    ichar_t		firsthalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			firstno;	/* Index into first */
    ichar_t *		firstp;		/* Ptr into current firsthalf word */
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    int			nfirsthalf;	/* No. words saved in 1st half */
    int			nsecondhalf;	/* No. words saved in 2nd half */
    register ichar_t *	p;
    ichar_t		secondhalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			secondno;	/* Index into second */

    /*
    ** We don't do words of length less than 3;  this keeps us from
    ** splitting all two-letter words into two single letters.  We
    ** also don't do maximum-length words, since adding the space
    ** would exceed the size of the "possibilities" array.
    */
    nfirsthalf = icharlen (word);
    if (nfirsthalf < 3  ||  nfirsthalf >= INPUTWORDLEN + MAXAFFIXLEN - 1)
	return;
    (void) icharcpy (newword + 1, word);
    for (p = newword + 1;  p[1] != '\0';  p++)
	{
	p[-1] = *p;
	*p = '\0';
	if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
	    {
	    /*
	     * Save_cap must be called before good() is called on the
	     * second half, because it uses state left around by
	     * good().  This is unfortunate because it wastes a bit of
	     * time, but I don't think it's a significant performance
	     * problem.
	     */
	    nfirsthalf = save_cap (DEREF_FIRST_ARG(istate) newword, word, firsthalf);
	    if (good (DEREF_FIRST_ARG(istate) p + 1, 0, 1, 0, 0))
		{
		nsecondhalf = save_cap (DEREF_FIRST_ARG(istate) p + 1, p + 1, secondhalf);
		for (firstno = 0;  firstno < nfirsthalf;  firstno++)
		    {
		    firstp = &firsthalf[firstno][p - newword];
		    for (secondno = 0;  secondno < nsecondhalf;  secondno++)
			{
			*firstp = ' ';
			(void) icharcpy (firstp + 1, secondhalf[secondno]);
			if (insert (DEREF_FIRST_ARG(istate) firsthalf[firstno]) < 0)
			    return;
			*firstp = '-';
			if (insert (DEREF_FIRST_ARG(istate) firsthalf[firstno]) < 0)
			    return;
			}
		    }
		}
	    }
	}
    }

int compoundgood (FIRST_ARG(istate) ichar_t *word, int pfxopts)
#if 0
    ichar_t *		word;
    int			pfxopts;	/* Options to apply to prefixes */
#endif
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	savech;
    long		secondcap;	/* Capitalization of 2nd half */

    /*
    ** If compoundflag is COMPOUND_NEVER, compound words are never ok.
    */
    if (compoundflag == COMPOUND_NEVER)
	return 0;
    /*
    ** Test for a possible compound word (for languages like German that
    ** form lots of compounds).
    **
    ** This is similar to missingspace, except we quit on the first hit,
    ** and we won't allow either member of the compound to be a single
    ** letter.
    **
    ** We don't do words of length less than 2 * compoundmin, since
    ** both halves must at least compoundmin letters.
    */
    if (icharlen (word) < 2 * DEREF(istate, hashheader.compoundmin))
	return 0;
    (void) icharcpy (newword, word);
    p = newword + DEREF(istate, hashheader.compoundmin);
    for (  ;  p[DEREF(istate, hashheader.compoundmin) - 1] != 0;  p++)
	{
	savech = *p;
	*p = 0;
	if (good (DEREF_FIRST_ARG(istate) newword, 0, 0, pfxopts, FF_COMPOUNDONLY))
	    {
	    *p = savech;
	    if (good (DEREF_FIRST_ARG(istate) p, 0, 1, FF_COMPOUNDONLY, 0)
	      ||  compoundgood (DEREF_FIRST_ARG(istate) p, FF_COMPOUNDONLY))
		{
		secondcap = whatcap (DEREF_FIRST_ARG(istate) p);
		switch (whatcap (DEREF_FIRST_ARG(istate) newword))
		    {
		    case ANYCASE:
		    case CAPITALIZED:
		    case FOLLOWCASE:	/* Followcase can have l.c. suffix */
			return secondcap == ANYCASE;
		    case ALLCAPS:
			return secondcap == ALLCAPS;
		    }
		}
	    }
	else
	    *p = savech;
	}
    return 0;
    }

static void transposedletter (FIRST_ARG(istate) ichar_t *word)
#if 0
    register ichar_t *	word;
#endif
    {
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	temp;

    (void) icharcpy (newword, word);
    for (p = newword;  p[1] != 0;  p++)
	{
	temp = *p;
	*p = p[1];
	p[1] = temp;
	if (good (DEREF_FIRST_ARG(istate) newword, 0, 1, 0, 0))
	    {
	    if (ins_cap (DEREF_FIRST_ARG(istate) newword, word) < 0)
		return;
	    }
	temp = *p;
	*p = p[1];
	p[1] = temp;
	}
    }

/* Insert one or more correctly capitalized versions of word */
static int ins_cap (FIRST_ARG(istate) ichar_t *word, ichar_t *pattern)
#if 0
    ichar_t *		word;
    ichar_t *		pattern;
#endif
    {
    int			i;		/* Index into savearea */
    int			nsaved;		/* No. of words saved */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];

    nsaved = save_cap (DEREF_FIRST_ARG(istate) word, pattern, savearea);
    for (i = 0;  i < nsaved;  i++)
	{
	if (insert (DEREF_FIRST_ARG(istate) savearea[i]) < 0)
	    return -1;
	}
    return 0;
    }

/* Save one or more correctly capitalized versions of word */
static int save_cap (FIRST_ARG(istate) ichar_t *word, ichar_t *pattern, 
					ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN])
#if 0
    ichar_t *		word;		/* Word to save */
    ichar_t *		pattern;	/* Prototype capitalization pattern */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
					/* Room to save words */
#endif
    {
    int			hitno;		/* Index into hits array */
    int			nsaved;		/* Number of words saved */
    int			preadd;		/* No. chars added to front of root */
    int			prestrip;	/* No. chars stripped from front */
    int			sufadd;		/* No. chars added to back of root */
    int			sufstrip;	/* No. chars stripped from back */

    if (*word == 0)
	return 0;

    for (hitno = DEREF(istate, numhits), nsaved = 0;  --hitno >= 0  &&  nsaved < MAX_CAPS;  )
	{
	if (DEREF(istate, hits[hitno].prefix))
	    {
	    prestrip = DEREF(istate, hits[hitno].prefix->stripl);
	    preadd = DEREF(istate, hits[hitno].prefix->affl);
	    }
	else
	    prestrip = preadd = 0;
	if (DEREF(istate, hits[hitno].suffix))
	    {
	    sufstrip = DEREF(istate, hits[hitno].suffix->stripl);
	    sufadd = DEREF(istate, hits[hitno].suffix->affl);
	    }
	else
	    sufadd = sufstrip = 0;
	save_root_cap (DEREF_FIRST_ARG(istate) word, pattern, prestrip, preadd,
	    sufstrip, sufadd,
	    DEREF(istate, hits[hitno].dictent), DEREF(istate, hits[hitno].prefix), DEREF(istate, hits[hitno].suffix),
	    savearea, &nsaved);
	}
    return nsaved;
    }

int ins_root_cap (FIRST_ARG(istate) ichar_t *word, ichar_t *pattern, 
				 int prestrip, int preadd, int sufstrip, int sufadd,
  				 struct dent *firstdent, struct flagent *pfxent, struct flagent *sufent)
#if 0
    register ichar_t *	word;
    register ichar_t *	pattern;
    int			prestrip;
    int			preadd;
    int			sufstrip;
    int			sufadd;
    struct dent *	firstdent;
    struct flagent *	pfxent;
    struct flagent *	sufent;
#endif
    {
    int			i;		/* Index into savearea */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			nsaved;		/* Number of words saved */

    nsaved = 0;
    save_root_cap (DEREF_FIRST_ARG(istate) word, pattern, prestrip, preadd, sufstrip, sufadd,
      firstdent, pfxent, sufent, savearea, &nsaved);
    for (i = 0;  i < nsaved;  i++)
	{
	if (insert (DEREF_FIRST_ARG(istate) savearea[i]) < 0)
	    return -1;
	}
    return 0;
    }

/* ARGSUSED */
static void save_root_cap (FIRST_ARG(istate) ichar_t *word, ichar_t *pattern, 
						  int prestrip, int preadd, int sufstrip, int sufadd,
						  struct dent *firstdent, struct flagent *pfxent, struct flagent *sufent, 
						  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN], 
					      int * nsaved)
#if 0
    register ichar_t *	word;		/* Word to be saved */
    register ichar_t *	pattern;	/* Capitalization pattern */
    int			prestrip;	/* No. chars stripped from front */
    int			preadd;		/* No. chars added to front of root */
    int			sufstrip;	/* No. chars stripped from back */
    int			sufadd;		/* No. chars added to back of root */
    struct dent *	firstdent;	/* First dent for root */
    struct flagent *	pfxent;		/* Pfx-flag entry for word */
    struct flagent *	sufent;		/* Sfx-flag entry for word */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
					/* Room to save words */
    int *		nsaved;		/* Number saved so far (updated) */
#endif
    {
#ifndef NO_CAPITALIZATION_SUPPORT
    register struct dent * dent;
#endif /* NO_CAPITALIZATION_SUPPORT */
    int			firstisupper;
    ichar_t		newword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
#ifndef NO_CAPITALIZATION_SUPPORT
    register ichar_t *	p;
    int			len;
    int			i;
    int			limit;
#endif /* NO_CAPITALIZATION_SUPPORT */

    if (*nsaved >= MAX_CAPS)
	return;
    (void) icharcpy (newword, word);
    firstisupper = myupper (DEREF_FIRST_ARG(istate) pattern[0]);
#ifdef NO_CAPITALIZATION_SUPPORT
    /*
    ** Apply the old, simple-minded capitalization rules.
    */
    if (firstisupper)
	{
	if (myupper (pattern[1]))
	    upcase (newword);
	else
	    {
	    lowcase (newword);
	    newword[0] = mytoupper (newword[0]);
	    }
	}
    else
	lowcase (newword);
    (void) icharcpy (savearea[*nsaved], newword);
    (*nsaved)++;
    return;
#else /* NO_CAPITALIZATION_SUPPORT */
#define flagsareok(dent)    \
    ((pfxent == NULL \
	||  TSTMASKBIT (dent->mask, pfxent->flagbit)) \
      &&  (sufent == NULL \
	||  TSTMASKBIT (dent->mask, sufent->flagbit)))

    dent = firstdent;
    if ((dent->flagfield & (CAPTYPEMASK | MOREVARIANTS)) == ALLCAPS)
	{
	upcase (DEREF_FIRST_ARG(istate) newword);	/* Uppercase required */
	(void) icharcpy (savearea[*nsaved], newword);
	(*nsaved)++;
	return;
	}
    for (p = pattern;  *p;  p++)
	{
	if (mylower (DEREF_FIRST_ARG(istate) *p))
	    break;
	}
    if (*p == 0)
	{
	upcase (DEREF_FIRST_ARG(istate) newword);	/* Pattern was all caps */
	(void) icharcpy (savearea[*nsaved], newword);
	(*nsaved)++;
	return;
	}
    for (p = pattern + 1;  *p;  p++)
	{
	if (myupper (DEREF_FIRST_ARG(istate) *p))
	    break;
	}
    if (*p == 0)
	{
	/*
	** The pattern was all-lower or capitalized.  If that's
	** legal, insert only that version.
	*/
	if (firstisupper)
	    {
	    if (captype (dent->flagfield) == CAPITALIZED
	      ||  captype (dent->flagfield) == ANYCASE)
		{
		lowcase (DEREF_FIRST_ARG(istate) newword);
		newword[0] = mytoupper (DEREF_FIRST_ARG(istate) newword[0]);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
		}
	    }
	else
	    {
	    if (captype (dent->flagfield) == ANYCASE)
		{
		lowcase (DEREF_FIRST_ARG(istate) newword);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
		}
	    }
	while (dent->flagfield & MOREVARIANTS)
	    {
	    dent = dent->next;
	    if (captype (dent->flagfield) == FOLLOWCASE
	      ||  !flagsareok (dent))
		continue;
	    if (firstisupper)
		{
		if (captype (dent->flagfield) == CAPITALIZED)
		    {
		    lowcase (DEREF_FIRST_ARG(istate) newword);
		    newword[0] = mytoupper (DEREF_FIRST_ARG(istate) newword[0]);
		    (void) icharcpy (savearea[*nsaved], newword);
		    (*nsaved)++;
		    return;
		    }
		}
	    else
		{
		if (captype (dent->flagfield) == ANYCASE)
		    {
		    lowcase (DEREF_FIRST_ARG(istate) newword);
		    (void) icharcpy (savearea[*nsaved], newword);
		    (*nsaved)++;
		    return;
		    }
		}
	    }
	}
    /*
    ** Either the sample had complex capitalization, or the simple
    ** capitalizations (all-lower or capitalized) are illegal.
    ** Insert all legal capitalizations, including those that are
    ** all-lower or capitalized.  If the prototype is capitalized,
    ** capitalized all-lower samples.  Watch out for affixes.
    */
    dent = firstdent;
    p = strtosichar (DEREF_FIRST_ARG(istate) dent->word, 1);
    len = icharlen (p);
    if (dent->flagfield & MOREVARIANTS)
	dent = dent->next;	/* Skip place-holder entry */
    for (  ;  ;  )
	{
	if (flagsareok (dent))
	    {
	    if (captype (dent->flagfield) != FOLLOWCASE)
		{
		lowcase (DEREF_FIRST_ARG(istate) newword);
		if (firstisupper  ||  captype (dent->flagfield) == CAPITALIZED)
		    newword[0] = mytoupper (DEREF_FIRST_ARG(istate) newword[0]);
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		if (*nsaved >= MAX_CAPS)
		    return;
		}
	    else
		{
		/* Followcase is the tough one. */
		p = strtosichar (DEREF_FIRST_ARG(istate) dent->word, 1);
		(void) memmove (
		  (char *) (newword + preadd),
		  (char *) (p + prestrip),
		  (len - prestrip - sufstrip) * sizeof (ichar_t));
		if (myupper (DEREF_FIRST_ARG(istate) p[prestrip]))
		    {
		    for (i = 0;  i < preadd;  i++)
			newword[i] = mytoupper (DEREF_FIRST_ARG(istate) newword[i]);
		    }
		else
		    {
		    for (i = 0;  i < preadd;  i++)
			newword[i] = mytolower (DEREF_FIRST_ARG(istate) newword[i]);
		    }
		limit = len + preadd + sufadd - prestrip - sufstrip;
		i = len + preadd - prestrip - sufstrip;
		p += len - sufstrip - 1;
		if (myupper (DEREF_FIRST_ARG(istate) *p))
		    {
		    for (p = newword + i;  i < limit;  i++, p++)
			*p = mytoupper (DEREF_FIRST_ARG(istate) *p);
		    }
		else
		    {
		    for (p = newword + i;  i < limit;  i++, p++)
		      *p = mytolower (DEREF_FIRST_ARG(istate) *p);
		    }
		(void) icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		if (*nsaved >= MAX_CAPS)
		    return;
		}
	    }
	if ((dent->flagfield & MOREVARIANTS) == 0)
	    break;		/* End of the line */
	dent = dent->next;
	}
    return;
#endif /* NO_CAPITALIZATION_SUPPORT */
    }

