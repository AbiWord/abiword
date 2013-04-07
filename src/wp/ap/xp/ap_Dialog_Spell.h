/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_SPELL_H
#define AP_DIALOG_SPELL_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_SectionLayout.h"
#include "fl_SelectionPreserver.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

#include "spell_manager.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_Spell : public XAP_Dialog_NonPersistent
{

 public:

   AP_Dialog_Spell(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
   virtual ~AP_Dialog_Spell(void);

   virtual void runModal(XAP_Frame * pFrame) = 0;

   bool isSelection(void) const { return m_bIsSelection; };
   bool isComplete(void) const { return !m_bCancelled; };

 protected:

	bool _spellCheckWord (const UT_UCSChar * word, UT_uint32 len);
	SpellChecker * _getDict();

   void _purgeSuggestions(void);

   PT_DocPosition m_iOrigInsPoint;

   // used to find misspelled words
   bool nextMisspelledWord(void);

   bool addIgnoreAll(void);
   void ignoreWord(void);

   bool inChangeAll(void);
   bool addChangeAll(const UT_UCSChar * newword);
   bool changeWordWith(const UT_UCSChar * newword);

   // make the word visible in the document behind the dialog
   bool makeWordVisible(void);
   // add the word to current user dictionary
   bool addToDict(void);

   // true if we're checking just a selction rather than entire doc
   bool m_bIsSelection;

   // change/ignore all hash tables
   UT_GenericStringMap<UT_UCSChar*> * m_pChangeAll;
   UT_GenericStringMap<UT_UCSChar*> * m_pIgnoreAll;

   // these variables keep track of the current
   // location/state of the search through the
   // document for misspelled words
   fl_BlockSpellIterator* m_pWordIterator;
   UT_sint32 m_iWordOffset;
   UT_sint32 m_iWordLength;
   const UT_UCSChar* m_pWord;
   bool m_bSkipWord;

   // section & block pairs for start, end,
   // and current position position within
   // the part of the document being checked
   fl_DocSectionLayout * m_pStartSection;
   fl_BlockLayout      * m_pStartBlock;
   UT_sint32             m_iStartIndex;
   fl_DocSectionLayout * m_pEndSection;
   fl_BlockLayout      * m_pEndBlock;
   UT_sint32             m_iEndLength;
   fl_DocSectionLayout * m_pCurrSection;
   fl_BlockLayout      * m_pCurrBlock;

   XAP_Frame * m_pFrame;
   FV_View * m_pView;
	FL_SelectionPreserver * m_pPreserver;
   PD_Document * m_pDoc;

   // current suggested corrections to the
   // most recently misspelled word
   UT_GenericVector<UT_UCSChar*> * m_Suggestions;

   bool	m_bCancelled;
   short m_iSelectedRow;
};

#endif /* AP_DIALOG_SPELL_H */
