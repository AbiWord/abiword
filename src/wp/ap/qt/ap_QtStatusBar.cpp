/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2013 Serhat Kiyak <serhatkiyak91@gmail.com>
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

#include <QVBoxLayout>
#include <QLabel>
#include <Qt>
#include <QProgressBar> 

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "ap_QtStatusBar.h"

AP_QtStatusBar::AP_QtStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_wProgressFrame = NULL;
}

AP_QtStatusBar::~AP_QtStatusBar(void)
{
}

// FIXME: we need more sanity checking here to make sure everything allocates correctly
QStatusBar * AP_QtStatusBar::createWidget(void)
{
	UT_ASSERT(!m_wStatusBar);

	m_wStatusBar = new QStatusBar();

	for (UT_sint32 k=0; k<getFields()->getItemCount(); k++) 
	{
		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements

		// set up a frame for status bar elements so they look like status bar elements, 
		// and not just normal widgets
		QWidget *pStatusBarElement = NULL;

		if (pf->getFillMethod() == REPRESENTATIVE_STRING || (pf->getFillMethod() == MAX_POSSIBLE)){
			AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);
			pStatusBarElement = new QWidget();
			QVBoxLayout *layout = new QVBoxLayout();
			pStatusBarElement->setLayout(layout);

			QLabel *pStatusBarElementLabel = new QLabel(pf_TextInfo->getRepresentativeString());
			layout->addWidget(pStatusBarElementLabel);

			// align
			if (pf_TextInfo->getAlignmentMethod() == LEFT) {
				pStatusBarElementLabel->setAlignment(Qt::AlignLeft);
			}

			m_wStatusBar->addWidget(pStatusBarElement);
			pStatusBarElementLabel->clear();
			pStatusBarElementLabel->setText("");
		}
		else if(pf->getFillMethod() == PROGRESS_BAR)
		{
			pStatusBarElement = new QWidget();
			QVBoxLayout *layout = new QVBoxLayout();
			pStatusBarElement->setLayout(layout);	

			m_wStatusBar->addWidget(pStatusBarElement);	
			QProgressBar *pProgress = new QProgressBar();
			layout->addWidget(pProgress);
			pProgress->setOrientation(Qt::Horizontal);
			m_wProgressFrame = pStatusBarElement;
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}
	hideProgressBar();
	return m_wStatusBar;
}

void AP_QtStatusBar::hideProgressBar(void)
{
	m_wProgressFrame->hide();
}
