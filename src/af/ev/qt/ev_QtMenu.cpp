/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include <stack>
#include <QWidget>
#include <QMenu>
#include <QString>
#include <QKeySequence>
#include <QActionGroup>
#include <QIcon>

#include "ev_QtMenu.h"
#include "xap_Frame.h"
#include "xap_QtApp.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_EditEventMapper.h"

// remove the first '&' character
static void _ev_convert(char * bufResult,
						const char * szString)
{
	UT_ASSERT(szString && bufResult);

	bool foundAmpersand = false;
	const char * src = szString;
	char * dest = bufResult;
	while (*src)
	{
		if (*src == '&' && !foundAmpersand)
		{
			src++;
			foundAmpersand = true;
		}
		else
		{
			*dest = *src;
			dest++;
			src++;
		}
	}
	*dest = 0;
}

/*
  Unlike the Win32 version, which uses a \t (tab) to seperate the
  feature from the mnemonic in a single label string, this
  function returns two strings, to be put into the two seperate
  labels in a QT menu item.

  Oh, and these are static buffers, don't call this function
  twice and expect previous return pointers to have the same
  values at their ends.
*/

static const char ** _ev_GetLabelName(XAP_QtApp * pQtApp,
									  XAP_Frame * /*pFrame*/,
									  const EV_Menu_Action * pAction,
									  const EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return data;	// which will be two nulls now

	static std::string accelbuf;
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();
		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pQtApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pQtApp->getEditEventMapper();
			UT_ASSERT(pEEM);

			const char * string = pEEM->getShortcutFor(pEM);
			if (string && *string)
				accelbuf = string;
			else
				// zero it out for this round
				accelbuf = "";
		}
	}

	// set shortcut mnemonic, if any
	if (!accelbuf.empty())
		data[1] = accelbuf.c_str();
	
	if (!pAction->raisesDialog())
	{
		data[0] = szLabelName;
		return data;
	}

	// append "..." to menu item if it raises a dialog
	static char buf[128];
	memset(buf,0,G_N_ELEMENTS(buf));
	strncpy(buf,szLabelName,G_N_ELEMENTS(buf)-4);
	strcat(buf,"...");

	data[0] = buf;
	
	return data;
}

EV_QtMenu::EV_QtMenu(XAP_QtApp * pQtApp, 
						 XAP_Frame *pFrame, 
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pQtApp, pQtApp->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	  m_pQtApp(pQtApp),
	  m_pFrame(pFrame)
{
}

EV_QtMenu::~EV_QtMenu()
{
	m_vecMenuWidgets.clear();
}

XAP_Frame * EV_QtMenu::getFrame()
{
	return m_pFrame;
}

bool EV_QtMenu::menuEvent(XAP_Menu_Id id)
{
	//TODO
	return true;
}

bool EV_QtMenu::synthesizeMenu(QMenuBar * menuRoot, bool isPopup)
{
	// create a QT menu from the info provided.
	const EV_Menu_ActionSet * pMenuActionSet = m_pQtApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	UT_uint32 nrLabelItemsInLayout = m_pMenuLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	// we keep a stack of the widgets so that we can properly
	// parent the menu items and deal with nested pull-rights.
	std::stack<QWidget*> stack;
	stack.push(menuRoot);

	std::stack<QActionGroup*> groupStack;

	for (UT_uint32 k = 0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Menu_LayoutItem * pLayoutItem = m_pMenuLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Menu_Id id = pLayoutItem->getMenuId();
		// VERY BAD HACK!  It will be here until I fix the const correctness of all the functions
		// using EV_Menu_Action
		const EV_Menu_Action * pAction = pMenuActionSet->getAction(id);
		UT_ASSERT(pAction);
		const EV_Menu_Label * pLabel = m_pMenuLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		// get the name for the menu item
		const char * szLabelName;
		const char * szMnemonicName;

		switch (pLayoutItem->getMenuLayoutFlags())
		{
			case EV_MLF_Normal:
			{
				const char ** data = getLabelName(m_pQtApp, pAction, pLabel);
				szLabelName = data[0];
				szMnemonicName = data[1];
				QAction *w;

				if (szLabelName && *szLabelName)
				{
					// find parent menu item
					QWidget * wParent = stack.top();
					UT_ASSERT(wParent);

					char buf[1024];
					// remove the first '&' character of the label
					_ev_convert(buf, szLabelName);
					QString str = buf;
					w = new QAction(str, wParent);

					// set mnemonic
					if (szMnemonicName && *szMnemonicName && !isPopup)
					{
						w->setShortcut(QKeySequence(QString(szMnemonicName)));
					}

					// an item can't be both a checkable and a radio option
					UT_return_val_if_fail(!(pAction->isCheckable() && pAction->isRadio()), NULL);

					if (pAction->isCheckable())
					{
						w->setCheckable(true);
						if(!groupStack.empty())
						{
							groupStack.pop(); // radio buton items should be consecutive 
						}
					}
					else if (pAction->isRadio()) 
					{
						QActionGroup * top;
						if(groupStack.empty())
						{
							top = new QActionGroup(wParent); 
							groupStack.push(top);
						}
						else
						{
							top = groupStack.top();
						}
						w->setCheckable(true);
						w->setActionGroup(top);
					} 
					else
					{
						if(!groupStack.empty())
						{
							groupStack.pop(); // radio buton items should be consecutive 
						}
						//TODO: Set icon here if necessary
						//w->setIcon(QIcon("image"));
					}

					// bury in parent
					// There is always one menubar
					if(wParent != menuRoot)
					{
						static_cast<QMenu *>(wParent)->addAction(w);
					}
					else
					{
						static_cast<QMenuBar *>(wParent)->addAction(w);
					}
				}
				// give it a fake, with no label, to make sure it passes the
				// test that an empty (to be replaced) item in the vector should
				// have no children
				else
				{
					w = new QAction("", NULL);
	 				UT_ASSERT(w);
				}

				m_vecMenuWidgets.push_front(w);
				break;
			}
			case EV_MLF_BeginSubMenu:
			{
				const char ** data = _ev_GetLabelName(m_pQtApp, m_pFrame, pAction, pLabel);
				szLabelName = data[0];
				QMenu * w;

				if(!groupStack.empty())
				{
					groupStack.pop(); // radio buton items should be consecutive 
				}

				if (szLabelName && *szLabelName)
				{
					char buf[1024];
					// remove the first '&' character of the label
					_ev_convert(buf, szLabelName);

					// create the item widget
					QString str = buf;
					w = new QMenu(str);

					// find parent menu item
					QWidget * wParent = stack.top();
					UT_ASSERT(wParent);

					// bury the widget in parent menu
					if(wParent != menuRoot)
					{
						static_cast<QMenu *>(wParent)->addMenu(w);
					}
					else
					{
						static_cast<QMenuBar *>(wParent)->addMenu(w);
					}

					// add to menu bar
					stack.push(w);

					// item is created, add to vector
					m_vecMenuWidgets.push_front(w);
					break;
				}

				break;
			}
			case EV_MLF_EndSubMenu:
			{
				// pop and inspect
				QWidget * w;
				w = stack.top();
				stack.pop();
				UT_ASSERT(w);

				if(!groupStack.empty())
				{
					groupStack.pop(); // radio buton items should be consecutive 
				}

				// item is created (albeit empty in this case), add to vector
				m_vecMenuWidgets.push_front(w);
				break;

			}
			case EV_MLF_Separator:
			{
				if(!groupStack.empty())
				{
					groupStack.pop(); // radio buton items should be consecutive 
				}

				QWidget * wParent = stack.top();
				UT_ASSERT(wParent);

				if(wParent != menuRoot)
				{
					static_cast<QMenu *>(wParent)->addSeparator();
				}
				else
				{
					static_cast<QMenuBar *>(wParent)->addSeparator();
				}
				m_vecMenuWidgets.push_front(NULL);	// reserve slot in vector so indexes will be in sync
				break;
			}
			case EV_MLF_BeginPopupMenu:
			{
				m_vecMenuWidgets.push_front(NULL);	// reserve slot in vector so indexes will be in sync
				break;
			}
			case EV_MLF_EndPopupMenu:
			{
				m_vecMenuWidgets.push_front(NULL);	// reserve slot in vector so indexes will be in sync
				break;
			}
			default:
			{
				UT_ASSERT(0);
				break;
			}
		}
	}
	return true;
}

bool EV_QtMenu::_refreshMenu(AV_View * pView, QMenuBar * menuRoot)
{
	//TODO
	return true;
}


bool EV_QtMenu::_doAddMenuItem(UT_uint32 layout_pos)
{
	//TODO
	return true;
}

