#include <stdio.h>
#include "config.h"
#include "ispell.h"

#ifndef HASHPATH
#define HASHPATH "/home/davet/ispell-install/lib"
#endif

#ifndef DEFHASH
#define DEFHASH  "english.hash"
#endif

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





void myfree (ptr)
void*  ptr;
{
    free (ptr);
}

void *mymalloc (size)
unsigned int    size;
{
    return (void *) malloc (size);
}



int SpellCheckInit(char *hashname)
{

	if (linit(hashname) < 0)
	{
		printf("Couldn't load the hash table (dictionary)\n");
		exit(0);
	}

	return 1;

}


static int ICharTheWord(char *theWord, ichar_t *iWord)
{
    char *pc;
    ichar_t  *pi;

   /* convert from char string to ichar_t string */
    for (pi = iWord, pc = theWord; *pc; )
        {
            *pi++ = chartoichar(*pc++);
        }
    *pi = 0;

	return 1;
}


int SpellCheckWord16(unsigned short  *word16)
{
	int retVal;

	if (!word16) 
		return 0;

	retVal = good( (ichar_t *) word16, 0, 0, 1, 0, 0);

	return retVal;  /* returns 0 or 1 (boolean) */
}



int SpellCheckWord(char *word)
{
    char *pc;
    ichar_t  *pi, *iWord;
	int wordLength;
	int retVal;

	if (!word)
		return 0;

	wordLength = strlen(word);

	if (!wordLength)
		return 0;

	if (!(iWord = (ichar_t *) mymalloc( (wordLength+1) * 2 )))
		{
			return -1;
		}

    for (pi = iWord, pc = word; *pc; )
        {
            *pi++ = chartoichar(*pc++);
        }
    *pi = 0;

	retVal = good(iWord, 0, 0, 1, 0, 0);

	myfree(iWord);

	return retVal; /* return 0 not found, 1 on found */
	
}


/********************************************************************

	To use the Ispell checker at this stage, the following two calls
	are all that need be known externally:

	 SpellCheckInit(char *NameOfTheDictionaryHere)
	 Boolean SpellCheckWord( uint16 *WordHereIn16bitChars )

 ********************************************************************/


int main (argc, argv)
int argc;
char *argv[];
{
	char *w;
    ichar_t iword[256];
	int retVal;

#if 0
	char 			hashname[MAXPATHLEN];
	sprintf(hashname,"%s/%s",HASHPATH, DEFHASH);
#endif

	if (!SpellCheckInit("/usr/lib/ispell/english.hash"))
	{
		printf("Couldn't read in spell check dictionary....\n");
		exit(0);
	}


	w = "publish";
	printf("Looking up the word \"%s\"\n",w);
	retVal = SpellCheckWord(w);
	printf("returned %d\n",retVal);
	
	w = "Publish";
	printf("Looking up the word \"%s\"\n",w);
	retVal = SpellCheckWord(w);
	printf("returned %d\n",retVal);

	w = "boogyramma";
	printf("Looking up the word \"%s\"\n",w);
	retVal = SpellCheckWord(w);
	printf("returned %d\n",retVal);


	w = "costly";
	printf("Looking up the word \"%s\"\n",w);
	retVal = SpellCheckWord(w);
	printf("returned %d\n",retVal);

	w = "cost";
	printf("Looking up the word \"%s\"\n",w);
	retVal = SpellCheckWord(w);
	printf("returned %d\n",retVal);

}


