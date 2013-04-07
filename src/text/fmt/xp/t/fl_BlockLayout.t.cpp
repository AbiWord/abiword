/* AbiSource Applications
 * Copyright (C) 2006 Hubert Figuiere
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

//#include "tf_test.h"

#include <iostream>

#error this is not a real unit test. Fix it by removing code below and using th e real code.

class fl_BlockLayout;


class FL_DocLayout
{
public:
	FL_DocLayout()
		: m_toSpellCheckHead(NULL),
		  m_toSpellCheckTail(NULL)
		{
		}
	fl_BlockLayout *spellQueueHead(void) const
		{
			return m_toSpellCheckHead;
		}
	void        setSpellQueueHead(fl_BlockLayout *h)
		{
			m_toSpellCheckHead = h;
		}
	void        setSpellQueueTail(fl_BlockLayout *t)
		{
			m_toSpellCheckTail = t;
		}

	fl_BlockLayout* m_toSpellCheckHead;
	fl_BlockLayout*	m_toSpellCheckTail;
};


class fl_BlockLayout
{
public:
	fl_BlockLayout(FL_DocLayout *l)
		: m_pLayout(l),
		  m_nextToSpell(NULL),
		  m_prevToSpell(NULL)
		{
		}
	void enqueueToSpellCheckAfter(fl_BlockLayout *prev);
	void dequeueFromSpellCheck(void);
	fl_BlockLayout *nextToSpell(void) const
	{
		return m_nextToSpell;
	}
	fl_BlockLayout *prevToSpell(void) const
	{
		return m_prevToSpell;
	}

private:
	FL_DocLayout *m_pLayout;
	fl_BlockLayout *m_nextToSpell;
	fl_BlockLayout *m_prevToSpell;
};


void fl_BlockLayout::enqueueToSpellCheckAfter(fl_BlockLayout *prev)
{
	if (prev != NULL) {
		m_nextToSpell = prev->m_nextToSpell;
		prev->m_nextToSpell = this;
	}
	else {
		m_nextToSpell = m_pLayout->spellQueueHead();
		m_pLayout->setSpellQueueHead(this);
	}
	if (m_nextToSpell != NULL) {
		m_nextToSpell->m_prevToSpell = this;
	}
	else {
		m_pLayout->setSpellQueueTail(this);
	}
	m_prevToSpell = prev;
}


void fl_BlockLayout::dequeueFromSpellCheck(void)
{
	if (m_prevToSpell != NULL) {
		m_prevToSpell->m_nextToSpell = m_nextToSpell;
	}
	else {
		m_pLayout->setSpellQueueHead(m_nextToSpell);
	}
	if (m_nextToSpell != NULL) {
		m_nextToSpell->m_prevToSpell = m_prevToSpell;
	}
	else {
		m_pLayout->setSpellQueueTail(m_prevToSpell);
	}
	m_nextToSpell = m_prevToSpell = NULL;
}



int
main(int argc, char **argv)
{
	FL_DocLayout layout;

	fl_BlockLayout *pHead = new fl_BlockLayout(&layout);;
	pHead->enqueueToSpellCheckAfter(NULL);
	std::cout << "4 checks to go" << std::endl;
	if(layout.m_toSpellCheckHead == pHead) {
		std::cout << " head checked" << std::endl;
	}
	if(layout.m_toSpellCheckTail == pHead) {
		std::cout << " tail checked" << std::endl;
	}
	if(pHead->nextToSpell() == NULL) {
		std::cout << " head->next link checked (NULL)" << std::endl;
	}
	if(pHead->prevToSpell() == NULL) {
		std::cout << " head->prev link checked (NULL)" << std::endl;
	}


	fl_BlockLayout * pBlock = new fl_BlockLayout(&layout);
	pBlock->enqueueToSpellCheckAfter(pHead);
	
	std::cout << "4 checks to go" << std::endl;
	if(layout.m_toSpellCheckHead == pHead) {
		std::cout << " head checked" << std::endl;
	}
	if(layout.m_toSpellCheckTail == pBlock) {
		std::cout << " tail checked" << std::endl;
	}

	if(pHead->nextToSpell() == pBlock) {
		std::cout << " next link checked" << std::endl;
	}

	if(pBlock->prevToSpell() == pHead) {
		std::cout << " prev link checked" << std::endl;
	}

	std::cout << "8 checks to go" << std::endl;

	fl_BlockLayout *pBlock2 = new fl_BlockLayout(&layout);
	pBlock2->enqueueToSpellCheckAfter(pHead);
	
	if(layout.m_toSpellCheckHead == pHead) {
		std::cout << " head checked" << std::endl;
	}
	if(layout.m_toSpellCheckTail == pBlock) {
		std::cout << " tail checked" << std::endl;
	}
	if(pHead->prevToSpell() == NULL) {
		std::cout << " pHead->prev link checked (NULL)" << std::endl;
	}
	if(pHead->nextToSpell() == pBlock2) {
		std::cout << " pHead->next link checked" << std::endl;
	}
	if(pBlock2->prevToSpell() == pHead) {
		std::cout << " block2->prev link checked" << std::endl;
	}
	if(pBlock2->nextToSpell() == pBlock) {
		std::cout << " block2->next link checked" << std::endl;
	}
	if(pBlock->prevToSpell() == pBlock2) {
		std::cout << " block->prev link checked" << std::endl;
	}
	if(pBlock->nextToSpell() == NULL) {
		std::cout << " block->next link checked (NULL)" << std::endl;
	}


	std::cout << "8 checks to go" << std::endl;
	pBlock2->dequeueFromSpellCheck();
	if(layout.m_toSpellCheckHead == pHead) {
		std::cout << " head checked" << std::endl;
	}
	if(layout.m_toSpellCheckTail == pBlock) {
		std::cout << " tail checked" << std::endl;
	}
	if(pHead->prevToSpell() == NULL) {
		std::cout << " pHead->prev link checked (NULL)" << std::endl;
	}
	if(pHead->nextToSpell() == pBlock) {
		std::cout << " pHead->next link checked" << std::endl;
	}
	if(pBlock->prevToSpell() == pHead) {
		std::cout << " block->prev link checked" << std::endl;
	}
	if(pBlock->nextToSpell() == NULL) {
		std::cout << " block->next link checked (NULL)" << std::endl;
	}
	if(pBlock2->nextToSpell() == NULL) {
		std::cout << " block2->next link checked (NULL)" << std::endl;
	}
	if(pBlock2->prevToSpell() == NULL) {
		std::cout << " block2->prev link checked (NULL)" << std::endl;
	}
	delete pBlock2;


	std::cout << "6 checks to go" << std::endl;
	pHead->dequeueFromSpellCheck();
	if(layout.m_toSpellCheckHead == pBlock) {
		std::cout << " head checked" << std::endl;
	}
	if(layout.m_toSpellCheckTail == pBlock) {
		std::cout << " tail checked" << std::endl;
	}
	if(pBlock->prevToSpell() == NULL) {
		std::cout << " block->prev link checked (NULL)" << std::endl;
	}
	if(pBlock->nextToSpell() == NULL) {
		std::cout << " block->next link checked (NULL)" << std::endl;
	}
	if(pHead->nextToSpell() == NULL) {
		std::cout << " head->next link checked (NULL)" << std::endl;
	}
	if(pHead->prevToSpell() == NULL) {
		std::cout << " head->prev link checked (NULL)" << std::endl;
	}
	delete pHead;

	std::cout << "4 checks to go" << std::endl;
	pBlock->dequeueFromSpellCheck();
	if(layout.m_toSpellCheckHead == NULL) {
		std::cout << " head checked (NULL)" << std::endl;
	}
	if(layout.m_toSpellCheckTail == NULL) {
		std::cout << " tail checked (NULL)" << std::endl;
	}
	if(pBlock->prevToSpell() == NULL) {
		std::cout << " block->prev link checked (NULL)" << std::endl;
	}
	if(pBlock->nextToSpell() == NULL) {
		std::cout << " block->next link checked (NULL)" << std::endl;
	}

	delete pBlock;
	return 0;
}
