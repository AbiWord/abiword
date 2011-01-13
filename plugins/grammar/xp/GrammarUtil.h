
#ifndef GRAMMAR_UTIL_H
#define GRAMMAR_UTIL_H

#include "GrammarManager.h"
#include "fl_BlockLayout.h"

class fl_BlockSentenceIterator;
class fl_PieceOfText;
class GrammarChecker;

class fl_BlockSentenceIterator
{
	friend class fl_BlockLayout;

	bool _breakBlockInSentences();
	
	fl_BlockLayout* m_pBL;

	bool	bBlockBroke;
	UT_uint32	m_iStartIndex;
	UT_uint32	iLastSentence;
	UT_uint32	iCurSentence;
	UT_uint32	nSentences;
	UT_uint32	iLength;
	
	UT_GenericVector<fl_PieceOfText *> 	m_vecSentences;

public:
	fl_BlockSentenceIterator(fl_BlockLayout* pBL, UT_sint32 iPos = 0);
	~fl_BlockSentenceIterator();

	bool				nextSentForGrammarChecking(const UT_UCSChar*& pWord,
											 UT_sint32& iLength,
											 UT_sint32& iBlockPos,
											 UT_sint32& iPTLength);
	void              	updateBlock(UT_sint32 iOffset);
	void              	updateSentenceBoundaries(void);

	UT_sint32         	getBlockLength(void);

	void              	revertToPreviousWord(void);

    const UT_UCSChar* 	getCurrentSentence(UT_sint32& iLength);
    const UT_UCSChar* 	getPreSent(UT_sint32& iLength);
    const UT_UCSChar* 	getPostSent(UT_sint32& iLength);
};

class fl_PieceOfText
{
public:
	fl_PieceOfText(void);
	virtual ~fl_PieceOfText(void);
	UT_sint32 iInLow;
	UT_sint32 iInHigh;
	UT_sint32 nWords;
	bool      bHasStop;
	UT_UTF8String sLang;
	UT_UTF8String sText;
	bool      m_bGrammarChecked;
	bool      m_bGrammarOK;
	UT_sint32 countWords(void);
};

//bool	_grammarCheckSentence(const UT_UCSChar * sentence, UT_uint32 len, UT_uint32 blockPos);
//GrammarChecker * _getGrammarChecker (UT_uint32 blockPos);

#endif
