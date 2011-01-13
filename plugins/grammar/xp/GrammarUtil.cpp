#include "GrammarUtil.h"

#include <unicode/brkiter.h> // break iterator
#include <unicode/ucnv.h>

/*!
  Constructor for iterator
  
  Use the iterator to find words for spell-checking in the block.

  \param pBL BlockLayout this iterator should work on
  \param iPos Position the iterator should start from
*/
fl_BlockSentenceIterator::fl_BlockSentenceIterator(fl_BlockLayout* pBL, UT_sint32 iPos)
	: m_pBL(pBL), m_iStartIndex(iPos), iLastSentence(0), iCurSentence(0), nSentences(0)
{
	//iLength = m_pgb->getLength();
	bBlockBroke = false;
}

/*!
  Destructor for iterator
*/
fl_BlockSentenceIterator::~fl_BlockSentenceIterator()
{
	for (UT_uint32 i = 0; i < m_vecSentences.getItemCount(); i++)
	{
		fl_PieceOfText * pT = m_vecSentences.getNthItem(i);
		if (pT)
			FREEP(pT);
	}
	m_vecSentences.clear();
}


/*!
  Update block information for this iterator

  This method must be called whenever the block this iterator is
  associated with changes.
*/
void
fl_BlockSentenceIterator::updateBlock(UT_sint32 iOffset)
{
	for (UT_uint32 i= iCurSentence; i < m_vecSentences.getItemCount(); i++)
	{
		fl_PieceOfText * pPT = m_vecSentences.getNthItem(i);
		fl_PieceOfText * pNewPT = pPT;
		pNewPT->iInLow += iOffset;
		pNewPT->iInHigh += iOffset;
		m_vecSentences.setNthItem(i, pNewPT, &pPT);
	}
}

bool
fl_BlockSentenceIterator::nextSentForGrammarChecking(const UT_UCSChar*& pWord, UT_sint32& iLength,
												UT_sint32& iBlockPos, UT_sint32& iPTLength)
{
	if (!bBlockBroke) {
		if (!_breakBlockInSentences())
			return false;
		bBlockBroke = true;
	}
	
	if (iLastSentence >= nSentences)
		return false;
	
	fl_PieceOfText *pSent = m_vecSentences.getNthItem(iLastSentence); 
	UT_UCS4Char * str = 0;
    UT_UCS4String ucs4(pSent->sText.utf8_str());
    UT_UCS4_cloneString(&str, ucs4.ucs4_str());
	
	UT_DEBUGMSG(("fl_BlockSentenceIterator: got piece of text\n"));
	
	pWord = str;
	iLength = pSent->iInHigh - pSent->iInLow;
	iBlockPos = pSent->iInLow;
	
	iLastSentence++;

	return true;
	
}

bool 
fl_BlockSentenceIterator::_breakBlockInSentences()
{
	UT_return_val_if_fail( m_pBL, false );

	for (UT_uint32 i = 0; i < m_vecSentences.getItemCount(); i++)
	{
		fl_PieceOfText * pPT = m_vecSentences.getNthItem(i);
		if (pPT)
			FREEP(pPT);
	}
	m_vecSentences.clear();
	
	UT_UTF8String sText;
	BreakIterator * m_bIterator;
	UErrorCode status = U_ZERO_ERROR;
	
	m_pBL->appendUTF8String(sText);
	m_bIterator = BreakIterator::createSentenceInstance(Locale::getDefault(), status);
	if (U_FAILURE(status)) 
	{
		return false;
	}
	m_bIterator->setText(sText.utf8_str());
	int32_t iStart = m_bIterator->first();
	//int32_t iLastStart = 0;
	//bool bIsAbbrev = false;

	for (int32_t iEnd = m_bIterator->next();
       	 iEnd != BreakIterator::DONE;
         iStart = iEnd, iEnd = m_bIterator->next())
	{
		CharacterIterator *pStrIter = m_bIterator->getText().clone();
	
		
		UT_UTF8String sSentence;
		//UT_UTF8String sLastWord;
		UT_UCS4Char cUCS4;
		sSentence += static_cast<UT_UCS4Char>(pStrIter->setIndex(iStart));
		// gets the sentence in UTF8 char by char.
		for (int32_t iChar = iStart; iChar < iEnd - 1; iChar++)
		{
			cUCS4 = static_cast<UT_UCS4Char>(pStrIter->next());
			sSentence += cUCS4;
		}
		
		delete pStrIter;
		
	
		fl_PieceOfText * pCurSent = new fl_PieceOfText();
		m_vecSentences.addItem(pCurSent);
		pCurSent->iInLow = iStart;
		pCurSent->iInHigh = iEnd;
		pCurSent->sText = sSentence;
		//pCurSent->sLang = sLang; 
	}

	nSentences = m_vecSentences.getItemCount();
	delete m_bIterator;
	return true;
}

fl_PieceOfText::fl_PieceOfText(void):
	iInLow(0),
	iInHigh(0),
	nWords(0),
	bHasStop(false),
	sText(""),
	m_bGrammarChecked(false),
	m_bGrammarOK(false)
{
}

fl_PieceOfText::~fl_PieceOfText(void)
{
}

UT_sint32 fl_PieceOfText::countWords(void)
{
	const char * szSent = sText.utf8_str();
	UT_sint32 totlen = strlen(szSent);
	bool bNewWord = false;
	UT_sint32 i = 0;
	for (i=0; i< totlen; i++) 
	{
	       bool bFoundSpace = false;
	       while(((szSent[i] == ' ') || szSent[i]==';' || szSent[i] ==':' 
		      || szSent[i]==','  || szSent[i] ==static_cast<char>(UCS_TAB)) && (i < totlen))
	       {
		     i++;
		     bFoundSpace = true;
	       }
	       if(szSent[i] == '.')
	       {
		     if( (i> 0)&&((szSent[i-1] >= '0')&&(szSent[i-1] <= '9')))
		     {
		           continue;
		     }
		     bHasStop = true;
		     continue;
	       }
	       if(bFoundSpace)
	       {
	             nWords++;
		     bNewWord = true;
	       }
	       if(bNewWord && (szSent[i] >= '0' && szSent[i] <= '9'))
	       {
		     nWords--;
		     bNewWord = false;
	       }
	}

	return nWords;
}

/*!
  Get current word
  \result iLength Length of string.
  \return Pointer to word.
*/
// FIXME GPB variable name conflict
const UT_UCSChar*
fl_BlockSentenceIterator::getCurrentSentence(UT_sint32& iLength1)
{	
	fl_PieceOfText *pSent = m_vecSentences.getNthItem(iLastSentence-1); //fix!!!
	UT_UCS4Char * str = 0;
    UT_UCS4String ucs4(pSent->sText.utf8_str());
    UT_UCS4_cloneString(&str, ucs4.ucs4_str());
	
	iLength1 = pSent->iInHigh - pSent->iInLow;
	return str;
}


