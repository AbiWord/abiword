#ifndef AP_DIALOG_GRAMMAR_H
#define AP_DIALOG_GRAMMAR_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_SectionLayout.h"
#include "fl_SelectionPreserver.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

#include "GrammarManager.h"
#include "GrammarUtil.h"

class XAP_Frame;

extern pt2Constructor DialogGrammar_Constructor;

class DialogGrammar : public XAP_Dialog_NonPersistent
{
public:

	DialogGrammar(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~DialogGrammar(void);
   
	virtual void runModal(XAP_Frame * pFrame) = 0;

	bool isSelection(void) const { return m_bIsSelection; }
	bool isComplete(void) const { return !m_bCancelled; };

protected:
	
	// TODO GPB remove!
	//bool _grammarCheckSentence (const UT_UCSChar * sentence, size_t len);
	GrammarChecker * _getChecker();
	void _updateSentenceMistakesPos(UT_sint32 iOffset);

	bool nextSentenceMistake();

	void _purgeMistakes(void);

	PT_DocPosition m_iOrigInsPoint;
	bool nextWrongSentence(void);
  
	//bool addIgnoreAll(void);
	void ignoreMistake(void);
   
	//bool inChangeAll(void);
	//bool addChangeAll(const UT_UCSChar * newword);
	bool changeMistakeWith(const UT_UCSChar * newword);

	// make the sentence visible in the document behind the dialog
	bool makeMistakeVisible();
	// add the word to current user dictionary
	//bool addToDict(void);

	// true if we're checking just a selection rather than entire doc
	bool m_bIsSelection;

	// these variables keep track of the current
	// location/state of the search through the
	// document for misspelled words

	fl_BlockSentenceIterator* m_pSentIterator;
	UT_sint32 m_iSentOffset;
	UT_sint32 m_iSentLength;
	const UT_UCSChar* m_pSent;
	bool m_bSkipMistake;
	 

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
	UT_GenericVector<GrammarMistake*> *m_mistakes;
	UT_uint32 nCurMistake;
	GrammarMistake * m_curMistake;
	
   
	bool	m_bCancelled;
	//short m_iSelectedRow;
};

#endif /* AP_DIALOG_GRAMMAR_H */
