#include "ispell_checker.h"
#include "ut_vector.h"

#include "ispell.h"
#include "sp_spell.h"
#include "iconv.h"


extern "C" {
extern int XAP_EncodingManager__swap_utos, XAP_EncodingManager__swap_stou;
}

/*this one fills ucs2 with values that iconv will treat as UCS-2. */
static void toucs2(const UT_uint16 *word16, UT_sint32 length)
{
	UT_sint32 i = 0;
	const UT_uint16* in = word16;
	UT_uint16* out = ucs2;
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_utos)
		    out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
		    out[i] = in[i];
	}
	out[i]= 0;
}

/*this one copies from 'ucs2' to UT_uint16 swapping bytes if necessary */
static void fromucs2(UT_uint16 *word16, UT_sint32 length)
{
	UT_sint32 i = 0;
	UT_uint16* in = ucs2,*out = word16;
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_stou)
			out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
			out[i] = in[i];
	}
	out[i]= 0;
}

#	define UCS_2_INTERNAL "UCS-2"

static void try_autodetect_charset(char* hashname)
{
	int len;
	char buf[3000];
	FILE* f;
	if (strlen(hashname)>(3000-15))
		return;
	sprintf(buf,"%s-%s",hashname,"encoding");
	f = fopen(buf,"r");
	if (!f)
		return;
	len = fread(buf,1,sizeof(buf),f);
	if (len<=0)
		return;
	buf[len]=0;
	fclose(f);
	{
		char* start, *p = buf;
		while (*p==' ' || *p=='\t' || *p=='\n')
			++p;
		start = p;
		while (!(*p==' ' || *p=='\t' || *p=='\n' || *p=='\0'))
			++p;
		*p = '\0';
		if (!*start) /* empty enc */
			return;
        	translate_in = iconv_open(start, UCS_2_INTERNAL);
        	translate_out = iconv_open(UCS_2_INTERNAL, start);
	}
	
}
/***************************************************************************/


ISpellChecker::ISpellChecker()
{
}

ISpellChecker::~ISpellChecker()
{
    lcleanup();
    if(translate_in != (iconv_t)-1)
        iconv_close(translate_in);
    translate_in = (iconv_t)-1;
    if(translate_out != (iconv_t)-1)
        iconv_close(translate_out);
    translate_out = (iconv_t)-1;
}

SpellChecker::SpellCheckResult
ISpellChecker::checkWord(const UT_UCSChar *word16, size_t length)
{
    SpellChecker::SpellCheckResult retVal;
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];

    if (!g_bSuccessfulInit)
        return SpellChecker::LOOKUP_SUCCEEDED;

    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
        return SpellChecker::LOOKUP_FAILED;

    if(translate_in == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register size_t x;

        for (x = 0, p = word8; x < length; x++)
            *p++ = (unsigned char)*word16++;
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
		/* TF CHANGE: Use the right types 
        unsigned int len_in, len_out; 
		*/
	size_t len_in, len_out;
        const char *In = (const char *)ucs2;
        char *Out = word8;

	toucs2(word16,length);
        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(translate_in, const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);
        *Out = '\0';
    }
    
/*UT_ASSERT(0);*/
    if( !strtoichar(iWord, word8, sizeof(iWord), 0) )
        retVal = (good(iWord, 0, 0, 1, 0) == 1 ? SpellChecker::LOOKUP_SUCCEEDED : SpellChecker::LOOKUP_FAILED);
    else
        retVal = SpellChecker::LOOKUP_ERROR;

    return retVal; /* 0 - not found, 1 on found, -1 on error */
}

UT_Vector *
ISpellChecker::suggestWord(const UT_UCSChar *word16, size_t length)
{
	sp_suggestions *sg = new sp_suggestions;
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];
    int  c;

    if (!g_bSuccessfulInit) 
        return 0;
    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
        return 0;
    if (!sg) 
        return 0;

    if(translate_in == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register size_t x;

        for (x = 0, p = word8; x < length; ++x)
		{
            *p++ = (unsigned char)*word16++;
		}
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
		/* TF CHANGE: Use the right types
        unsigned int len_in, len_out; 
		*/
        size_t len_in, len_out; 
        const char *In = (const char *)ucs2;
        char *Out = word8;
		toucs2(word16,length);	
        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(translate_in, const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);
        *Out = '\0';
    }
   
    if( !strtoichar(iWord, word8, sizeof(iWord), 0) )
        makepossibilities(iWord);

    sg->count = pcount;
	/* TF CHANGE: Use the right types
    sg->score = (unsigned short *)malloc(sizeof(unsigned short) * pcount); 
	*/
    sg->score = (short *)malloc(sizeof(short) * pcount);
    sg->word = (unsigned short**)malloc(sizeof(unsigned short**) * pcount);
    if (sg->score == NULL || sg->word == NULL) 
    {
        sg->count = 0;
        return 0;
    }

    for (c = 0; c < pcount; c++) 
    {
        int l;

        sg->score[c] = 1000;
        l = strlen(possibilities[c]);

        sg->word[c] = (unsigned short*)malloc(sizeof(unsigned short) * l + 2);
        if (sg->word[c] == NULL) 
        {
            /* out of memory, but return what was copied so far */
            sg->count = c;

			// BUG!!!!
			delete sg;
            return new UT_Vector();
        }

        if (translate_out == (iconv_t)-1)
        {
            /* copy to 16bit string and null terminate */
            register int x;

            for (x = 0; x < l; x++)
                sg->word[c][x] = (unsigned char)possibilities[c][x];
            sg->word[c][l] = 0;
        }
        else
        {
            /* convert to 16bit string and null terminate */
			/* TF CHANGE: Use the right types
			unsigned int len_in, len_out; 
			*/
			size_t len_in, len_out; 
            const char *In = possibilities[c];
            char *Out = (char *)ucs2;

            len_in = l;
            len_out = sizeof(unsigned short) * l;
            iconv(translate_out, const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);	    
            *((unsigned short *)Out) = 0;
			fromucs2(sg->word[c], (unsigned short*)Out-ucs2);
        }
    }

//    return sg->count;
	// TODO
	delete sg;
	return new UT_Vector();
}

bool
ISpellChecker::requestDictionary(const char *szLang)
{
	// WARNING: Brain-damaged implementation!
	// by now I just pick american.hash dictionary
	const char *hashname = "american.hash";

    if (linit(const_cast<char*>(hashname)) < 0)
    {
        /* TODO gripe -- could not load the dictionary */
        return false;
    }

    g_bSuccessfulInit = 1;

    /* Test for utf8 first */
    prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : (int *) NULL);
    if (prefstringchar >= 0)
    {
        translate_in = iconv_open("utf-8", UCS_2_INTERNAL);
        translate_out = iconv_open(UCS_2_INTERNAL, "utf-8");
    }

    /* Test for "latinN" */
    if(translate_in == (iconv_t)-1)
    {
        char teststring[64];
        int n1;

        /* Look for "altstringtype" names from latin1 to latin9 */
        for(n1 = 1; n1 <= 9; n1++)
        {
            sprintf(teststring, "latin%u", n1);
            prefstringchar = findfiletype(teststring, 1, deftflag < 0 ? &deftflag : (int *) NULL);
            if (prefstringchar >= 0)
            {
                translate_in = iconv_open(teststring, UCS_2_INTERNAL);
                translate_out = iconv_open(UCS_2_INTERNAL, teststring);
                break;
            }
        }
    }
    try_autodetect_charset(const_cast<char*>(hashname));

    /* Test for known "hashname"s */
    if(translate_in == (iconv_t)-1)
    {
        if( strstr( hashname, "russian.hash" ))
        {
            /* ISO-8859-5, CP1251 or KOI8-R */
            translate_in = iconv_open("KOI8-R", UCS_2_INTERNAL);
            translate_out = iconv_open(UCS_2_INTERNAL, "KOI8-R");
        }
    }

    /* If nothing found, use latin1 */
    if(translate_in == (iconv_t)-1)
    {
        translate_in = iconv_open("latin1", UCS_2_INTERNAL);
        translate_out = iconv_open(UCS_2_INTERNAL, "latin1");
    }

    if (prefstringchar < 0)
        defdupchar = 0;
    else
        defdupchar = prefstringchar;

	return true;
}

#ifndef NO_CAPITALIZATION_SUPPORT
UT_sint32 ISpellChecker::good (ichar_t *w, UT_sint32 ignoreflagbits, UT_sint32 allhits, UT_sint32 pfxopts, UT_sint32 sfxopts)
#else
UT_sint32 ISpellChecker::good (ichar_t *w, UT_sint32 ignoreflagbits, UT_sint32 dummy, UT_sint32 pfxopts, UT_sint32 sfxopts)
#define allhits	0	/* Never actually need more than one hit */
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

	}

    if (numhits  &&  !allhits)
	return 1;

    /* try stripping off affixes */

    chk_aff (w, nword, n, ignoreflagbits, allhits, pfxopts, sfxopts);

    return numhits;
    }

#ifndef NO_CAPITALIZATION_SUPPORT
UT_sint32 ISpellChecker::cap_ok (ichar_t *word, struct success *hit, UT_sint32 len)
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