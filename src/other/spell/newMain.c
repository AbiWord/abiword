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


char     possibilities[MAXPOSSIBLE][INPUTWORDLEN + MAXAFFIXLEN];
                                /* Table of possible corrections */
int      pcount;         /* Count of possibilities generated */
int      maxposslen;     /* Length of longest possibility */
int      easypossibilities; /* Number of "easy" corrections found */
                                /* ..(defined as those using legal affixes) */

/*
 * The following array contains a list of characters that should be tried
 * in "missingletter."  Note that lowercase characters are omitted.
 */
int      Trynum;         /* Size of "Try" array */
ichar_t  Try[SET_SIZE + MAXSTRINGCHARS];



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

int SpellCheckSuggestWord16(unsigned short *word16, sp_suggestions *sg)
{
   register int x, c, l;
   
   if (!g_bSuccessfulInit) return 0;
   if (!word16) return 0;
   if (!sg) return 0;


   makepossibilities(word16);
   
   sg->count = pcount;
   sg->score = (short*)malloc(sizeof(short) * pcount);
   sg->word = (short**)malloc(sizeof(short**) * pcount);
   if (sg->score == NULL || sg->word == NULL) {
      sg->count = 0;
      return 0;
   }
   
   for (c = 0; c < pcount; c++) {
      sg->score[c] = 1000;
      l = 0;
      while (possibilities[c][l]) l++;
      l++;
      sg->word[c] = (short*)malloc(sizeof(short) * l);
      if (sg->word[c] == NULL) {
	 /* out of memory, but return what was copied so far */
	 sg->count = c;
	 return c;
      }
      for (x = 0; x < l; x++) sg->word[c][x] = (short)possibilities[c][x];
   }

   return sg->count;
}

int SpellCheckSuggestNWord16(const unsigned short *word16, int length, sp_suggestions *sg)
{
   ichar_t  *iWord;
   register ichar_t *p;
   register int x, c, l;
   
   if (!g_bSuccessfulInit) return 0;
   if (!word16) return 0;
   if (!sg) return 0;


   if (!(iWord = (ichar_t *) malloc( ( sizeof(ichar_t) * (length+1)))))
     {
	return 0;
     }
   
   /* copy and null terminate */
   for (x = 0, p = iWord; x < length; x++)
     {
	*p++ = *word16++;
     }
   *p = (ichar_t) 0;
   
   makepossibilities(iWord);
   free(iWord);
   
   sg->count = pcount;
   sg->score = (short*)malloc(sizeof(short) * pcount);
   sg->word = (short**)malloc(sizeof(short**) * pcount);
   if (sg->score == NULL || sg->word == NULL) {
      sg->count = 0;
      return 0;
   }
   
   for (c = 0; c < pcount; c++) {
      sg->score[c] = 1000;
      l = 0;
      while (possibilities[c][l]) l++;
      l++;
      sg->word[c] = (short*)malloc(sizeof(short) * l);
      if (sg->word[c] == NULL) {
	 /* out of memory, but return what was copied so far */
	 sg->count = c;
	 return c;
      }
      for (x = 0; x < l; x++) sg->word[c][x] = (short)possibilities[c][x];
   }

   return sg->count;
}

int SpellCheckSuggestWord(char *word, sp_suggestions *sg)
{
   char *pc;
   ichar_t  *pi, *iWord;
   int wordLength, x, c, l;

   if (!g_bSuccessfulInit)
     {
	return 1;
     }
	
   if (!word)
       return 0;
   if (!sg)
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

   makepossibilities(iWord);
   free(iWord);
   
   sg->count = pcount;
   sg->score = (short*)malloc(sizeof(short) * pcount);
   sg->word = (short**)malloc(sizeof(short**) * pcount);
   if (sg->score == NULL || sg->word == NULL) {
      sg->count = 0;
      return 0;
   }
   
   for (c = 0; c < pcount; c++) {
      sg->score[c] = 1000;
      l = 0;
      while (possibilities[c][l]) l++;
      l++;
      sg->word[c] = (short*)malloc(sizeof(short) * l);
      if (sg->word[c] == NULL) {
	 /* out of memory, but return what was copied so far */
	 sg->count = c;
	 return c;
      }
      for (x = 0; x < l; x++) sg->word[c][x] = (short)possibilities[c][x];
   }

   return sg->count;
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


