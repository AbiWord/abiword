#ifndef lint
static char Rcs_Id[] =
	"$Id$";
#endif

/*
 * Copyright 1988, 1989, 1992, 1993, Geoff Kuenning, Granada Hills, CA
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
 * Revision 1.1  2001/04/15 16:01:24  tomas_f
 * moving to spell/xp
 *
 * Revision 1.6  1999/12/21 18:46:29  sterwill
 * ispell patch for non-English dictionaries by Henrik Berg <henrik@lansen.se>
 *
 * Revision 1.5  1999/10/20 03:19:35  paul
 * Hacked ispell code to ignore any characters that don't fit in the lookup tables loaded from the dictionary.  It ain't pretty, but at least we don't crash there any more.
 *
 * Revision 1.4  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.3  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.3  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.2  1998/12/28 23:11:30  eric
 *
 * modified spell code and integration to build on Windows.
 * This is still a hack.
 *
 * Actually, it doesn't yet WORK on Windows.  It just builds.
 * SpellCheckInit is failing for some reason.
 *
 * Revision 1.1  1998/12/28 18:04:43  davet
 * Spell checker code stripped from ispell.  At this point, there are
 * two external routines...  the Init routine, and a check-a-word routine
 * which returns a boolean value, and takes a 16 bit char string.
 * The code resembles the ispell code as much as possible still.
 *
 * Revision 1.45  1994/12/27  23:08:52  geoff
 * Add code to makedent to reject words that contain non-word characters.
 * This helps protect people who use ISO 8-bit characters when ispell
 * isn't configured for that option.
 *
 * Revision 1.44  1994/10/25  05:46:20  geoff
 * Fix some incorrect declarations in the lint versions of some routines.
 *
 * Revision 1.43  1994/09/16  03:32:34  geoff
 * Issue an error message for bad affix flags
 *
 * Revision 1.42  1994/02/07  04:23:43  geoff
 * Correctly identify the deformatter when changing file types
 *
 * Revision 1.41  1994/01/25  07:11:55  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ispell.h"
#include "msgs.h"

int		makedent P ((char * lbuf, int lbuflen, struct dent * ent));
#ifndef NO_CAPITALIZATION_SUPPORT
long		whatcap P ((ichar_t * word));
#endif
int		addvheader P ((struct dent * ent));
/*int		combinecaps P ((struct dent * hdr, struct dent * newent));
#ifndef NO_CAPITALIZATION_SUPPORT
static void	forcevheader P ((struct dent * hdrp, struct dent * oldp,
		  struct dent * newp));
#endif / * NO_CAPITALIZATION_SUPPORT * /
static int	combine_two_entries P ((struct dent * hdrp,
		  struct dent * oldp, struct dent * newp));
static int	acoversb P ((struct dent * enta, struct dent * entb));
*/
void		upcase P ((ichar_t * string));
void		lowcase P ((ichar_t * string));
void		chupcase P ((char * s));
/*static int	issubset P ((struct dent * ent1, struct dent * ent2));
static void	combineaffixes P ((struct dent * ent1, struct dent * ent2));*/

void		toutent P ((FILE * outfile, struct dent * hent,
		  int onlykeep));
/*static void	toutword P ((FILE * outfile, char * word,
		  struct dent * cent));
static void	flagout P ((FILE * outfile, int flag));
*/
int		stringcharlen P ((char * bufp, int canonical));
int		strtoichar P ((ichar_t * out, char * in, int outlen,
		  int canonical));
int		ichartostr P ((char * out, ichar_t * in, int outlen,
		  int canonical));
ichar_t *	strtosichar P ((char * in, int canonical));
char *		ichartosstr P ((ichar_t * in, int canonical));
char *		printichar P ((int in));
#ifndef ICHAR_IS_CHAR
ichar_t *	icharcpy P ((ichar_t * out, ichar_t * in));
int		icharlen P ((ichar_t * str));
int		icharcmp P ((ichar_t * s1, ichar_t * s2));
int		icharncmp P ((ichar_t * s1, ichar_t * s2, int n));
#endif /* ICHAR_IS_CHAR */
int		findfiletype P ((char * name, int searchnames,
		  int * deformatter));

/*static int  	has_marker;*/

/*
 * Fill in a directory entry, including setting the capitalization flags, and
 * allocate and initialize memory for the d->word field.  Returns -1
 * if there was trouble.  The input word must be in canonical form.
int makedent (lbuf, lbuflen, d)
This function is not used by AbiWord.  I don't know if it'll be needed for 
other abi documents
 */
	
#ifndef NO_CAPITALIZATION_SUPPORT
/*
** Classify the capitalization of a sample entry.  Returns one of the
** four capitalization codes ANYCASE, ALLCAPS, CAPITALIZED, or FOLLOWCASE.
*/

long whatcap (word)
    register ichar_t *	word;
    {
    register ichar_t *	p;

    for (p = word;  *p;  p++)
	{
	if (mylower (*p))
	    break;
	}
    if (*p == '\0')
	return ALLCAPS;
    else
	{
	for (  ;  *p;  p++)
	    {
	    if (myupper (*p))
		break;
	    }
	if (*p == '\0')
	    {
	    /*
	    ** No uppercase letters follow the lowercase ones.
	    ** If there is more than one uppercase letter, it's
	    ** "followcase". If only the first one is capitalized,
	    ** it's "capitalize".  If there are no capitals
	    ** at all, it's ANYCASE.
	    */
	    if (myupper (word[0]))
		{
		for (p = word + 1;  *p != '\0';  p++)
		    {
		    if (myupper (*p))
			return FOLLOWCASE;
		    }
		return CAPITALIZED;
		}
	    else
		return ANYCASE;
	    }
	else
	    return FOLLOWCASE;	/* .../lower/upper */
	}
    }

/*
** Add a variant-capitalization header to a word.  This routine may be
** called even for a followcase word that doesn't yet have a header.
**
** Returns 0 if all was ok, -1 if allocation error.
*/
int addvheader (dp)
    register struct dent *	dp;	/* Entry to update */
    {
    register struct dent *	tdent; /* Copy of entry */

    /*
    ** Add a second entry with the correct capitalization, and then make
    ** dp into a special dummy entry.
    */
    tdent = (struct dent *) malloc (sizeof (struct dent));
    if (tdent == NULL)
	{
	(void) fprintf (stderr, MAKEDENT_C_NO_WORD_SPACE, dp->word);
	return -1;
	}
    *tdent = *dp;
    if (captype (tdent->flagfield) != FOLLOWCASE)
	tdent->word = NULL;
    else
	{
	/* Followcase words need a copy of the capitalization */
	tdent->word = malloc ((unsigned int) strlen (tdent->word) + 1);
	if (tdent->word == NULL)
	    {
	    (void) fprintf (stderr, MAKEDENT_C_NO_WORD_SPACE, dp->word);
	    free ((char *) tdent);
	    return -1;
	    }
	(void) strcpy (tdent->word, dp->word);
	}
    chupcase (dp->word);
    dp->next = tdent;
    dp->flagfield &= ~CAPTYPEMASK;
    dp->flagfield |= (ALLCAPS | MOREVARIANTS);
    return 0;
    }
#endif /* NO_CAPITALIZATION_SUPPORT */

/*
** Combine and resolve the entries describing two capitalizations of the same
** word.  This may require allocating yet more entries.
**
** Hdrp is a pointer into a hash table.  If the word covered by hdrp has
** variations, hdrp must point to the header.  Newp is a pointer to temporary
** storage, and space is malloc'ed if newp is to be kept.  The newp->word
** field must have been allocated with mymalloc, so that this routine may free
** the space if it keeps newp but not the word.
**
** Return value:  0 if the word was added, 1 if the word was combined
** with an existing entry, and -1 if trouble occurred (e.g., malloc).
** If 1 is returned, newp->word may have been be freed using myfree.
**
** Life is made much more difficult by the KEEP flag's possibilities.  We
** must ensure that a !KEEP word doesn't find its way into the personal
** dictionary as a result of this routine's actions.  However, a !KEEP
** word that has affixes must have come from the main dictionary, so it
** is acceptable to combine entries in that case (got that?).
**
** The net result of all this is a set of rules that is a bloody pain
** to figure out.  Basically, we want to choose one of the following actions:
**
**	(1) Add newp's affixes and KEEP flag to oldp, and discard newp.
**	(2) Add oldp's affixes and KEEP flag to newp, replace oldp with
**	    newp, and discard newp.
#ifndef NO_CAPITALIZATION_SUPPORT
**	(3) Insert newp as a new entry in the variants list.  If there is
**	    currently no variant header, this requires adding one.  Adding a
**	    header splits into two sub-cases:
**
**	    (3a) If oldp is ALLCAPS and the KEEP flags match, just turn it
**		into the header.
**	    (3b) Otherwise, add a new entry to serve as the header.
**		To ease list linking, this is done by copying oldp into
**		the new entry, and then performing (3a).
**
**	    After newp has been added as a variant, its affixes and KEEP
**	    flag are OR-ed into the variant header.
#endif
**
** So how to choose which?  The default is always case (3), which adds newp
** as a new entry in the variants list.  Cases (1) and (2) are symmetrical
** except for which entry is discarded.  We can use case (1) or (2) whenever
** one entry "covers" the other.  "Covering" is defined as follows:
**
**	(4) For entries with matching capitalization types, A covers B
**	    if:
**
**	    (4a) B's affix flags are a subset of A's, or the KEEP flags
**		 match, and
**	    (4b) either the KEEP flags match, or A's KEEP flag is set.
**		(Since A has more suffixes, combining B with it won't
**		cause any extra suffixes to be added to the dictionary.)
**	    (4c) If the words are FOLLOWCASE, the capitalizations match
**		exactly.
**
#ifndef NO_CAPITALIZATION_SUPPORT
**	(5) For entries with mismatched capitalization types, A covers B
**	    if (4a) and (4b) are true, and:
**
**	    (5a) B is ALLCAPS, or
**	    (5b) A is ANYCASE, and B is CAPITALIZED.
#endif
**
** For any "hdrp" without variants, oldp is the same as hdrp.  Otherwise,
** the above tests are applied using each variant in turn for oldp.
int combinecaps (hdrp, newp)
static void forcevheader (hdrp, oldp, newp)
static int combine_two_entries (hdrp, oldp, newp)
static int acoversb (enta, entb)
*/
void upcase (s)
    register ichar_t *	s;
    {

    while (*s)
	{
	*s = mytoupper (*s);
	s++;
	}
    }

void lowcase (s)
    register ichar_t *	s;
    {

    while (*s)
	{
	*s = mytolower (*s);
	s++;
	}
    }

/*
 * Upcase variant that works on normal strings.  Note that it is a lot
 * slower than the normal upcase.  The input must be in canonical form.
 */
void chupcase (s)
    char *	s;
    {
    ichar_t *	is;

    is = strtosichar (s, 1);
    upcase (is);
    (void) ichartostr (s, is, strlen (s) + 1, 1);
    }

/*
** See if one affix field is a subset of another.  Returns NZ if ent1
** is a subset of ent2.  The KEEP flag is not taken into consideration.
static int issubset (ent1, ent2)
static void combineaffixes (ent1, ent2)
*/

/*
** Write out a dictionary entry, including capitalization variants.
** If onlykeep is true, only those variants with KEEP set will be
** written.
Removed -- not used by Abiword
void toutent_ (toutfile, hent, onlykeep)
static void toutword (toutfile, word, cent)
static void flagout (toutfile, flag)
*/

/*
 * If the string under the given pointer begins with a string character,
 * return the length of that "character".  If not, return 0.
 * May be called any time, but it's best if "isstrstart" is first
 * used to filter out unnecessary calls.
 *
 * As a side effect, "laststringch" is set to the number of the string
 * found, or to -1 if none was found.  This can be useful for such things
 * as case conversion.
 */
int stringcharlen (bufp, canonical)
    char *		bufp;
    int			canonical;	/* NZ if input is in canonical form */
    {
#ifdef SLOWMULTIPLY
    static char *	sp[MAXSTRINGCHARS];
    static int		inited = 0;
#endif /* SLOWMULTIPLY */
    register char *	bufcur;
    register char *	stringcur;
    register int	stringno;
    register int	lowstringno;
    register int	highstringno;
    int			dupwanted;

#ifdef SLOWMULTIPLY
    if (!inited)
	{
	inited = 1;
	for (stringno = 0;  stringno < MAXSTRINGCHARS;  stringno++)
	    sp[stringno] = &hashheader.stringchars[stringno][0];
	}
#endif /* SLOWMULTIPLY */
    lowstringno = 0;
    highstringno = hashheader.nstrchars - 1;
    dupwanted = canonical ? 0 : defdupchar;
    while (lowstringno <= highstringno)
	{
	stringno = (lowstringno + highstringno) >> 1;
#ifdef SLOWMULTIPLY
	stringcur = sp[stringno];
#else /* SLOWMULTIPLY */
	stringcur = &hashheader.stringchars[stringno][0];
#endif /* SLOWMULTIPLY */
	bufcur = bufp;
	while (*stringcur)
	    {
#ifdef NO8BIT
	    if (((*bufcur++ ^ *stringcur) & 0x7F) != 0)
#else /* NO8BIT */
	    if (*bufcur++ != *stringcur)
#endif /* NO8BIT */
		break;
	    /*
	    ** We can't use autoincrement above because of the
	    ** test below.
	    */
	    stringcur++;
	    }
	if (*stringcur == '\0')
	    {
	    if (hashheader.dupnos[stringno] == dupwanted)
		{
		/* We have a match */
		laststringch = hashheader.stringdups[stringno];
#ifdef SLOWMULTIPLY
		return stringcur - sp[stringno];
#else /* SLOWMULTIPLY */
		return stringcur - &hashheader.stringchars[stringno][0];
#endif /* SLOWMULTIPLY */
		}
	    else
		--stringcur;
	    }
	/* No match - choose which side to search on */
#ifdef NO8BIT
	if ((*--bufcur & 0x7F) < (*stringcur & 0x7F))
	    highstringno = stringno - 1;
	else if ((*bufcur & 0x7F) > (*stringcur & 0x7F))
	    lowstringno = stringno + 1;
#else /* NO8BIT */
	if (*--bufcur < *stringcur)
	    highstringno = stringno - 1;
	else if (*bufcur > *stringcur)
	    lowstringno = stringno + 1;
#endif /* NO8BIT */
	else if (dupwanted < hashheader.dupnos[stringno])
	    highstringno = stringno - 1;
	else
	    lowstringno = stringno + 1;
	}
    laststringch = -1;
    return 0;			/* Not a string character */
    }

/*
 * Convert an external string to an ichar_t string.  If necessary, the parity
 * bit is stripped off as part of the process.
 *
 * Returns NZ if the output string overflowed.
 */
int strtoichar (out, in, outlen, canonical)
    register ichar_t *	out;		/* Where to put result */
    register char *	in;		/* String to convert */
    int			outlen;		/* Size of output buffer, *BYTES* */
    int			canonical;	/* NZ if input is in canonical form */
    {
    register int	len;		/* Length of next character */

    outlen /= sizeof (ichar_t);		/* Convert to an ichar_t count */
    for (  ;  --outlen > 0  &&  *in != '\0';  in += len)
	{
	if (l1_isstringch (in, len, canonical))
	    *out++ = SET_SIZE + laststringch;
	else
	    *out++ = (unsigned char)( *in );
	}
    *out = 0;
    return outlen <= 0;
    }

/*
 * Convert an ichar_t string to an external string.
 *
 * WARNING: the resulting string may wind up being longer than the
 * original.  In fact, even the sequence strtoichar->ichartostr may
 * produce a result longer than the original, because the output form
 * may use a different string type set than the original input form.
 *
 * Returns NZ if the output string overflowed.
 */
int ichartostr (out, in, outlen, canonical)
    register char *	out;		/* Where to put result */
    register ichar_t *	in;		/* String to convert */
    int			outlen;		/* Size of output buffer, bytes */
    int			canonical;	/* NZ for canonical form */
    {
    register int	ch;		/* Next character to store */
    register int	i;		/* Index into duplicates list */
    register char *	scharp;		/* Pointer into a string char */

    while (--outlen > 0  &&  (ch = *in++) != 0)
	{
	if (ch < SET_SIZE)
	    *out++ = (char) ch;
	else
	    {
	    ch -= SET_SIZE;
	    if (!canonical)
		{
		for (i = hashheader.nstrchars;  --i >= 0;  )
		    {
		    if (hashheader.dupnos[i] == defdupchar
		      &&  ((int) (hashheader.stringdups[i])) == ch)
			{
			ch = i;
			break;
			}
		    }
		}
	    scharp = hashheader.stringchars[(unsigned) ch];
	    while ((*out++ = *scharp++) != '\0')
		;
	    out--;
	    }
	}
    *out = '\0';
    return outlen <= 0;
    }

/*
 * Convert a string to an ichar_t, storing the result in a static area.
 */
ichar_t * strtosichar (in, canonical)
    char *		in;		/* String to convert */
    int			canonical;	/* NZ if input is in canonical form */
    {
    static ichar_t	out[STRTOSICHAR_SIZE / sizeof (ichar_t)];

    if (strtoichar (out, in, sizeof out, canonical))
	(void) fprintf (stderr, WORD_TOO_LONG (in));
    return out;
    }

/*
 * Convert an ichar_t to a string, storing the result in a static area.
 */
char * ichartosstr (in, canonical)
    ichar_t *		in;		/* Internal string to convert */
    int			canonical;	/* NZ for canonical conversion */
    {
    static char		out[ICHARTOSSTR_SIZE];

    if (ichartostr (out, in, sizeof out, canonical))
	(void) fprintf (stderr, WORD_TOO_LONG (out));
    return out;
    }

/*
 * Convert a single ichar to a printable string, storing the result in
 * a static area.
 */
char * printichar (in)
    int			in;
    {
    static char		out[MAXSTRINGCHARLEN + 1];

    if (in < SET_SIZE)
	{
	out[0] = (char) in;
	out[1] = '\0';
	}
    else
	(void) strcpy (out, hashheader.stringchars[(unsigned) in - SET_SIZE]);
    return out;
    }

#ifndef ICHAR_IS_CHAR
/*
 * Copy an ichar_t.
 */
ichar_t * icharcpy (out, in)
    register ichar_t *	out;		/* Destination */
    register ichar_t *	in;		/* Source */
    {
    ichar_t *		origout;	/* Copy of destination for return */

    origout = out;
    while ((*out++ = *in++) != 0)
	;
    return origout;
    }

/*
 * Return the length of an ichar_t.
 */
int icharlen (in)
    register ichar_t *	in;		/* String to count */
    {
    register int	len;		/* Length so far */

    for (len = 0;  *in++ != 0;  len++)
	;
    return len;
    }

/*
 * Compare two ichar_t's.
 */
int icharcmp (s1, s2)
    register ichar_t *	s1;
    register ichar_t *	s2;
    {

    while (*s1 != 0)
	{
	if (*s1++ != *s2++)
	    return *--s1 - *--s2;
	}
    return *s1 - *s2;
    }

/*
 * Strncmp for two ichar_t's.
 */
int icharncmp (s1, s2, n)
    register ichar_t *	s1;
    register ichar_t *	s2;
    register int	n;
    {

    while (--n >= 0  &&  *s1 != 0)
	{
	if (*s1++ != *s2++)
	    return *--s1 - *--s2;
	}
    if (n < 0)
	return 0;
    else
	return *s1 - *s2;
    }

#endif /* ICHAR_IS_CHAR */

int findfiletype (name, searchnames, deformatter)
    char *		name;		/* Name to look up in suffix table */
    int			searchnames;	/* NZ to search name field of table */
    int *		deformatter;	/* Where to set deformatter type */
    {
    char *		cp;		/* Pointer into suffix list */
    int			cplen;		/* Length of current suffix */
    register int	i;		/* Index into type table */
    int			len;		/* Length of the name */

    /*
     * Note:  for now, the deformatter is set to 1 for tex, 0 for nroff.
     * Further, we assume that it's one or the other, so that a test
     * for tex is sufficient.  This needs to be generalized.
     */
    len = strlen (name);
    if (searchnames)
	{
	for (i = 0;  i < hashheader.nstrchartype;  i++)
	    {
	    if (strcmp (name, chartypes[i].name) == 0)
		{
		if (deformatter != NULL)
		    *deformatter =
		      (strcmp (chartypes[i].deformatter, "tex") == 0);
		return i;
		}
	    }
	}
    for (i = 0;  i < hashheader.nstrchartype;  i++)
	{
	for (cp = chartypes[i].suffixes;  *cp != '\0';  cp += cplen + 1)
	    {
	    cplen = strlen (cp);
	    if (len >= cplen  &&  strcmp (&name[len - cplen], cp) == 0)
		{
		if (deformatter != NULL)
		    *deformatter =
		      (strcmp (chartypes[i].deformatter, "tex") == 0);
		return i;
		}
	    }
	}
    return -1;
    }

/*
 * The following routines are all dummies for the benefit of lint.
 */
#ifdef lint
int TSTMASKBIT (mask, bit) MASKTYPE * mask; int bit;
    { return bit + (int) *mask; }
void CLRMASKBIT (mask, bit) MASKTYPE * mask; int bit; { bit += (int) *mask; }
void SETMASKBIT (mask, bit) MASKTYPE * mask; int bit; { bit += (int) *mask; }
int BITTOCHAR (bit) int bit; { return bit; }
/*int CHARTOBIT (ch) int ch; { return ch; }*/
int myupper (ch) unsigned int ch; { return (int) ch; }
int mylower (ch) unsigned int ch; { return (int) ch; }
int myspace (ch) unsigned int ch; { return (int) ch; }
int iswordch (ch) unsigned int ch; { return (int) ch; }
int isboundarych (ch) unsigned int ch; { return (int) ch; }
int isstringstart (ch) unsigned int ch; { return ch; }
ichar_t mytolower (ch) unsigned int ch; { return (ichar_t) ch; }
ichar_t mytoupper (ch) unsigned int ch; { return (ichar_t) ch; }
#else
/*
	HACK: macros replaced with function implementations 
	so we could do a side-effect-free check for unicode
	characters which aren't in hashheader

	TODO: this is just a workaround to keep us from crashing. 
	more sophisticated logic needed here. 
*/
char myupper(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.upperchars[c];
	else
		return 0;
}

char mylower(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.lowerchars[c];
	else
		return 0;
}

int myspace(ichar_t c)
{
	return ((c > 0)  &&  (c < 0x80) &&  isspace((unsigned char) c));
}

char iswordch(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.wordchars[c];
	else
		return 0;
}

char isboundarych(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.boundarychars[c];
	else
		return 0;
}

char isstringstart(ichar_t c)
{
	if (c < (SET_SIZE))
		return hashheader.stringstarts[(unsigned char) c];
	else
		return 0;
}

ichar_t mytolower(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.lowerconv[c];
	else
		return c;
}

ichar_t mytoupper (ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return hashheader.upperconv[c];
	else
		return c;
}

#endif /* lint */
