/* AbiSource Application Framework
 * Copyright (C) 2003 Hubert Figuiere
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
/*
	Implements the text view that supports the NSTextInput protocol
 */
 
#include "ut_assert.h"
#include "ut_debugmsg.h" 
#include "ev_CocoaKeyboard.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "xap_Frame.h"
#include "xav_View.h"

#import "xap_CocoaTextView.h"


@implementation XAP_CocoaTextView


/* */
- (void)setView:(AV_View*)view
{
	m_pView = view;
}


- (AV_View*)view
{
	return m_pView;
}

/* standard methods */

- (BOOL)resignFirstResponder
{
	return YES;
}


/*!
	Invoke an Abi method
 */
- (void)invokeEditMethod:(const char*)method
{
	EV_EditMethod* pEM;
	pEM = XAP_App::getApp()->getEditMethodContainer()->findEditMethodByName(method);
	AV_View * pView = m_pFrame->getCurrentView();
	EV_Keyboard * pCocoaKeyboard = m_pFrame->getKeyboard();
	pCocoaKeyboard->invokeKeyboardMethod(pView,pEM,0,0);
}


/* NSResponder methods */
- (void)insertNewline:(id)sender
{
	[self invokeEditMethod:"insertParagraphBreak"];
}

- (void)insertTab:(id)sender
{
	[self invokeEditMethod:"insertTab"];
}

- (void)deleteBackward:(id)sender
{
	[self invokeEditMethod:"delLeft"];
}

- (void)deleteForward:(id)sender
{
	[self invokeEditMethod:"delRight"];
}

- (void)moveForward:(id)sender;
{
	[self invokeEditMethod:"warpInsPtRight"];
}

- (void)moveRight:(id)sender;
{
	[self invokeEditMethod:"warpInsPtRight"];
}

- (void)moveBackward:(id)sender;
{
	[self invokeEditMethod:"warpInsPtLeft"];
}

- (void)moveLeft:(id)sender;
{
	[self invokeEditMethod:"warpInsPtLeft"];
}

- (void)moveUp:(id)sender;
{
	[self invokeEditMethod:"warpInsPtPrevLine"];
}

- (void)moveDown:(id)sender;
{
	[self invokeEditMethod:"warpInsPtNextLine"];
}
- (void)moveWordForward:(id)sender
{
	[self invokeEditMethod:"warpInsPtEOW"];
}
- (void)moveWordBackward:(id)sender
{
	[self invokeEditMethod:"warpInsPtBOW"];
}

- (void)moveWordRight:(id)sender
{
	[self invokeEditMethod:"warpInsPtEOW"];
}

- (void)moveWordLeft:(id)sender
{
	[self invokeEditMethod:"warpInsPtBOW"];
}

- (void)moveToBeginningOfLine:(id)sender
{
	[self invokeEditMethod:"warpInsPtBOL"];
}

- (void)moveToEndOfLine:(id)sender
{
	[self invokeEditMethod:"warpInsPtEOL"];
}
- (void)moveToBeginningOfParagraph:(id)sender
{
	[self invokeEditMethod:"warpInsPtBOP"];
}
- (void)moveToEndOfParagraph:(id)sender
{
	[self invokeEditMethod:"warpInsPtEOP"];
}
#if 0
// implement the Edit method first
- (void)moveParagraphBackwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelBOP"];
}
- (void)moveParagraphForwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelEOP"];
}
#endif

- (void)moveToEndOfDocument:(id)sender
{
	[self invokeEditMethod:"warpInsPtEOD"];
}

- (void)moveToBeginningOfDocument:(id)sender
{
	[self invokeEditMethod:"warpInsPtBOD"];
}

- (void)moveToEndOfDocumentAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelEOD"];
}
- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelBOD"];
}

- (void)pageDown:(id)sender;
{
	[self invokeEditMethod:"warpInsPtNextScreen"];
}
- (void)pageUp:(id)sender;
{
	[self invokeEditMethod:"warpInsPtPrevScreen"];
}

- (void)moveBackwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelLeft"];
}
- (void)moveForwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelRight"];
}

- (void)moveWordForwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelEOW"];
}
- (void)moveWordRightAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelEOW"];
}

- (void)moveWordBackwardAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelBOW"];
}
- (void)moveWordLeftAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelBOW"];
}

- (void)moveUpAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelPrevLine"];
}
- (void)moveDownAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelNextLine"];
}
- (void)moveToBeginningOfLineAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelBOL"];
}
- (void)moveToEndOfLineAndModifySelection:(id)sender
{
	[self invokeEditMethod:"extSelEOL"];
}

- (void)scrollPageUp:(id)sender
{
	[self invokeEditMethod:"scrollPageUp"];
}
- (void)scrollPageDown:(id)sender
{
	[self invokeEditMethod:"scrollPageDown"];
}
- (void)scrollLineUp:(id)sender
{
	[self invokeEditMethod:"scrollLineUp"];
}
- (void)scrollLineDown:(id)sender
{
	[self invokeEditMethod:"scrollLineDown"];
}
- (void)scrollToBeginningOfDocument:(id)sender
{
	[self invokeEditMethod:"scrollToTop"];
}
- (void)scrollToEndOfDocument:(id)sender
{
	[self invokeEditMethod:"scrollToBottom"];
}


/* NSTextInput protocol */
- (NSAttributedString *)attributedSubstringFromRange:(NSRange)theRange
{
	UT_ASSERT_NOT_REACHED();
	return nil;
}


- (unsigned int)characterIndexForPoint:(NSPoint)thePoint
{
	UT_ASSERT_NOT_REACHED();
	return 0xffffffff;
}


- (long)conversationIdentifier
{
	return (long)self;
}

- (void)doCommandBySelector:(SEL)aSelector
{
	if ([self respondsToSelector:aSelector]) {
		[self performSelector:aSelector withObject:self];
		return;
	}
	UT_DEBUGMSG(("Unrecognized selector:%s\n", [NSStringFromSelector(aSelector) UTF8String]));
//	UT_ASSERT_NOT_REACHED();
}


- (NSRect)firstRectForCharacterRange:(NSRange)theRange
{
	UT_ASSERT_NOT_REACHED();
	return NSZeroRect;
}


- (BOOL)hasMarkedText
{
	return m_hasMarkedText;
}


- (void)insertText:(id)aString
{
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	AV_View * pView = m_pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(m_pFrame->getKeyboard());

	if (pView)
		pCocoaKeyboard->insertTextEvent(pView, aString);	
}


- (NSRange)markedRange
{
	UT_ASSERT_NOT_REACHED();
	return NSMakeRange (NSNotFound, 0);
}

- (NSRange)selectedRange
{
	return m_selectedRange;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
	m_selectedRange = selRange;
	m_hasMarkedText = (selRange.length != 0 ? YES : NO);
	UT_DEBUGMSG(("Hub TODO: handle -[XAP_CocoaTextView setMarkedText:selectedRange:]\n"));
	/*
		Steal code from the selection handling code in XP land. We have the AV_View
		so everything is here.
	 */
}

- (void)unmarkText
{	
	m_hasMarkedText = NO;
	UT_DEBUGMSG(("Hub TODO: handle -[XAP_CocoaTextView unmarkText]\n"));
}

- (NSArray*)validAttributesForMarkedText
{
	return [NSArray array];
}


@end
