#include <stdlib.h>
#include <string.h>

#include "ispell.h"
#include "sp_spell.h"

/***************************************************************************/
/* Reduced Gobals needed by ispell code.                                   */
/***************************************************************************/
       int 				numhits;
struct success 			hits[MAX_HITS];

struct flagptr          pflagindex[SET_SIZE + MAXSTRINGCHARS];/*prefix index*/ 
struct flagent          *pflaglist; /* prefix flag control list */
       int				numpflags;  /* Number of prefix flags in table*/
struct flagptr          sflagindex[SET_SIZE + MAXSTRINGCHARS];/*suffix index*/
struct flagent          *sflaglist; /* suffix flag control list */
       int				numsflags;  /* Number of prefix flags in table*/

struct hashheader		hashheader; /* Header of hash table */
       int    			hashsize;   /* Size of main hash table */
       char 			*hashstrings = NULL; /* Strings in hash table */
struct dent 			*hashtbl;    /* Main hash table, for dictionary */

struct strchartype 		*chartypes;  /* String character type collection */
	   int     			defdupchar;   /* Default duplicate string type */
       unsigned int		laststringch; /* Number of last string character */


/***************************************************************************/


static int g_bSuccessfulInit = 0;

int SpellCheckInit(char *hashname)
{
	if (linit(hashname) < 0)
	{
		/* TODO gripe -- could not load the dictionary */
		
		return 0;
	}

	g_bSuccessfulInit = 1;
	
	return 1;
}

void SpellCheckCleanup(void)
{
	lcleanup();
}


int SpellCheckWord16(unsigned short  *word16)
{
	int retVal;

	if (!g_bSuccessfulInit)
	{
		return 1;
	}
	
	if (!word16) 
		return 0;

	retVal = good( (ichar_t *) word16, 0, 0, 1, 0);

	return retVal;  /* returns 0 or 1 (boolean) */
}


int SpellCheckNWord16(const unsigned short *word16, int length)
{
	int retVal;
    ichar_t  *iWord;
	register ichar_t *p;
	register int x;

	if (!g_bSuccessfulInit)
	{
		return 1;
	}

	if (!word16)
		return 0;

	/* TODO: modify good() to take a non-null terminated string so
		we don't have to malloc() for this check */

	if (!(iWord = (ichar_t *) malloc( ( sizeof(ichar_t) * (length+1)))))
	{
			return -1;
	}

	/* copy and null terminate */
	for (x = 0, p = iWord; x < length; x++)
	{
		*p++ = *word16++;
	}
	*p = (ichar_t) 0;

	
	retVal = good(iWord, 0, 0, 1, 0);
	free(iWord);

	return retVal; /* 0 - not found, 1 on found, -1 on error */
	
}



int SpellCheckWord(char *word)
{
    char *pc;
    ichar_t  *pi, *iWord;
	int wordLength;
	int retVal;

	if (!g_bSuccessfulInit)
	{
		return 1;
	}
	
	if (!word)
		return 0;

	wordLength = strlen(word);

	if (!wordLength)
		return 0;

	if (!(iWord = (ichar_t *) malloc( (wordLength+1) * 2 )))
	{
			return -1;
	}

    for (pi = iWord, pc = word; *pc; )
    {
            *pi++ = chartoichar(*pc++);
    }
    *pi = 0;

	retVal = good(iWord, 0, 0, 1, 0);

	free(iWord);

	return retVal; /* return 0 not found, 1 on found */
	
}


