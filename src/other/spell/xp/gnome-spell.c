/* AbiSuite
 * Copyright (C) 2000 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sp_spell.h"

#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>

/* TODO: autogenerate this from <prefix>/share/idl/Spell.idl */
#include "Spell.h"

#define WORD_SIZE 256 /* or whatever */

/**********************************************************************/
/*       encoding manager method hack so we can use it in C code      */
/**********************************************************************/
extern const char * xap_encoding_manager_get_language_iso_name (void);

/**********************************************************************/
/*                  Globals we'll need for gnome-spell                */
/**********************************************************************/

/* TODO: Try to reduce the number of globals needed */
static	BonoboObjectClient *checker_client, *dictionary_client;
static	GNOME_Spell_Checker checker;
static	GNOME_Spell_Dictionary dict;
static  CORBA_Environment   ev;

static gboolean successful_init = FALSE;

#define WORD_SIZE 256 /* or whatever */

/*
 * Pspell's author tells me that we probably don't need these
 * Two methods because Pspell can detect and handle utf16
 * Character strings, so long as they're *double null terminated*
 * These are defined in ut_string.[cpp,h]
 */
typedef unsigned short U16;
extern char * UT_UCS_strcpy_to_char (char * dest, const U16 * src);
extern U16  * UT_UCS_strcpy_char (U16 * dest, const char * src);

static gboolean
check_obj (BonoboObjectClient *obj, const gchar *id)
{
	if (!obj)
		fprintf (stderr, "Could not create an instance of the %s component", id);
	return obj == NULL;
}

/*
 * Should return 1 on success, 0 on failure
 */
int SpellCheckInit (char *unused_ispell_hashfile_name)
{
  char *checker_id, *dictionary_id;

  checker_id    = "OAFIID:GNOME_Spell_Checker:0.1";
  dictionary_id = "OAFIID:GNOME_Spell_Dictionary:0.1";

  checker_client = bonobo_object_activate (checker_id, 0);
  dictionary_client = bonobo_object_activate (dictionary_id, 0);

  if (check_obj (checker_client, checker_id) || 
      check_obj (dictionary_client, dictionary_id))
    return -1;

  CORBA_exception_init (&ev);

  checker = bonobo_object_corba_objref (BONOBO_OBJECT (checker_client));
  dict    = bonobo_object_corba_objref (BONOBO_OBJECT (dictionary_client));

  GNOME_Spell_Dictionary_setTag (dict, "language-tag",
				 xap_encoding_manager_get_language_iso_name (),
				 &ev);

  successful_init = TRUE;
}

void SpellCheckCleanup (void)
{
  if (!successful_init) 
    return;

  CORBA_exception_free (&ev);
  bonobo_object_unref (BONOBO_OBJECT (checker_client));
  bonobo_object_unref (BONOBO_OBJECT (dictionary_client));
}

/*
 * These next 2 functions should return 0 if not found,
 * > 1 if found, -1 on error
 */
int SpellCheckNWord16 (const unsigned short *word16, int length)
{
  unsigned char word8[WORD_SIZE];

  if (!successful_init)
    return -1;

  /* trying to spell-check a 0 length word will (rightly) cause pspell to segfault */
  if (word16 == NULL || length == 0)
    return 0;
  
  UT_UCS_strcpy_to_char (word8, word16);
  return GNOME_Spell_Dictionary_checkWord (dict, (char*)word8, &ev);
}

#define SPELL_ERROR_CLEANUP() { \
                               sg->count=0; \
                               CORBA_free (seq); \
                               return 0; \
                              }

int SpellCheckSuggestNWord16 (const unsigned short *word16, 
			      int length, sp_suggestions *sg)
{
  GNOME_Spell_StringSeq *seq;
  unsigned char word8[WORD_SIZE];
  int count = 0, i = 0;

  if (!successful_init || sg == NULL)
      return -1;

  if (word16 == NULL || length == 0)
    {
      sg->count = 0;
      return 0;
    }

  UT_UCS_strcpy_to_char (word8, word16);

  seq = GNOME_Spell_Dictionary_getSuggestions (dict, (char*)word8, &ev);
  count = seq->_length;

  if (count == 0)
    SPELL_ERROR_CLEANUP ();

  sg->score = (short *)malloc (sizeof(short) * count);
  sg->word = (U16**)malloc (sizeof(U16**) * count);
  if (sg->score == NULL || sg->word == NULL) 
    SPELL_ERROR_CLEANUP ();

  for (i = 0; i < count; i++)
    {
      char * new_word = seq->_buffer [i];
      int len = strlen (new_word);
      sg->word[i] = (U16*)malloc (sizeof (U16) * len + 2);
      if (sg->word[i] == NULL) 
        {
	  /* out of memory, but return what was copied so far */
	  sg->count = i;
	  CORBA_free (seq);
	  return i;
        }

      UT_UCS_strcpy_char (sg->word[i], new_word);
      sg->score[i] = 1000;
    }

  sg->count = count;
  CORBA_free (seq);
  return count;
}
