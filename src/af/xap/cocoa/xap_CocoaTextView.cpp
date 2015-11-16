/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
/*
	Implements the text view that supports the NSTextInput protocol
 */
 
#include "ut_assert.h"
#include "ut_debugmsg.h" 

#include "ev_CocoaKeyboard.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"

#include "xap_App.h"
#import "xap_CocoaCompat.h"
#include "xap_CocoaTextView.h"
#include "xap_CocoaToolPalette.h"
#include "xap_Frame.h"
#include "xav_View.h"

#include "fv_View.h"

#include "pd_Document.h"

@implementation XAP_CocoaTextView

- (id)initWith:(XAP_Frame *)frame andFrame:(NSRect)windowFrame
{
	if (![super initWith:frame andFrame:windowFrame]) {
		return nil;
	}
	m_hasMarkedText = NO;
	return self;
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
	UT_UNUSED(sender);
	[self invokeEditMethod:"insertParagraphBreak"];
}

- (void)insertTab:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"insertTab"];
}

- (void)deleteBackward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"delLeft"];
}

- (void)deleteForward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"delRight"];
}

- (void)deleteToEndOfParagraph:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"delEOL"];
}

- (void)moveForward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtRight"];
}

- (void)moveRight:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtRight"];
}

- (void)moveBackward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtLeft"];
}

- (void)moveLeft:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtLeft"];
}

- (void)moveUp:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtPrevLine"];
}

- (void)moveDown:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtNextLine"];
}
- (void)moveWordForward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtEOW"];
}
- (void)moveWordBackward:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtBOW"];
}

- (void)moveWordRight:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtEOW"];
}

- (void)moveWordLeft:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtBOW"];
}

- (void)moveToBeginningOfLine:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtBOL"];
}

- (void)moveToEndOfLine:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtEOL"];
}
- (void)moveToBeginningOfParagraph:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtBOP"];
}
- (void)moveToEndOfParagraph:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtEOP"];
}
#if 0
// implement the Edit method first
- (void)moveParagraphBackwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelBOP"];
}
- (void)moveParagraphForwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelEOP"];
}
#endif

- (void)moveToEndOfDocument:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtEOD"];
}

- (void)moveToBeginningOfDocument:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtBOD"];
}

- (void)moveToEndOfDocumentAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelEOD"];
}
- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelBOD"];
}

- (void)pageDown:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtNextScreen"];
}
- (void)pageUp:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"warpInsPtPrevScreen"];
}

- (void)moveLeftAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelLeft"];
}
- (void)moveRightAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelRight"];
}

- (void)moveBackwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelLeft"];
}
- (void)moveForwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelRight"];
}

- (void)moveWordForwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelEOW"];
}
- (void)moveWordRightAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelEOW"];
}

- (void)moveWordBackwardAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelBOW"];
}
- (void)moveWordLeftAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelBOW"];
}

- (void)moveUpAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelPrevLine"];
}
- (void)moveDownAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelNextLine"];
}
- (void)moveToBeginningOfLineAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelBOL"];
}
- (void)moveToEndOfLineAndModifySelection:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"extSelEOL"];
}

- (void)scrollPageUp:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollPageUp"];
}
- (void)scrollPageDown:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollPageDown"];
}
- (void)scrollLineUp:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollLineUp"];
}
- (void)scrollLineDown:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollLineDown"];
}
- (void)scrollToBeginningOfDocument:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollToTop"];
}
- (void)scrollToEndOfDocument:(id)sender
{
	UT_UNUSED(sender);
	[self invokeEditMethod:"scrollToBottom"];
}


/* NSTextInput protocol */
- (NSAttributedString *)attributedSubstringFromRange:(NSRange)theRange
{
	UT_UNUSED(theRange);
	UT_ASSERT_NOT_REACHED();
	return nil;
}


- (NSUInteger)characterIndexForPoint:(NSPoint)thePoint
{
	UT_UNUSED(thePoint);
	UT_ASSERT_NOT_REACHED();
	return NSNotFound;
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
	// UT_ASSERT_NOT_REACHED();
	UT_DEBUG_ONLY_ARG(theRange);
	UT_DEBUGMSG(("characterRange=(location=%u,length=%u)\n",theRange.location,theRange.length));
	return NSZeroRect;
}


- (BOOL)hasMarkedText
{
	UT_DEBUGMSG(("m_hasMarkedText=%s\n",m_hasMarkedText ? "YES" : "NO"));
	return m_hasMarkedText;
}


- (void)insertText:(id)aString
{
	 /* UT_DEBUGMSG(("insertText '%s' in window '%s'\n", [str UTF8String], [[[self window] title] UTF8String]));
	 */
	UT_DEBUGMSG(("insertText: length=%u\n", [(NSString *)aString length]));
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	AV_View * pView = m_pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(m_pFrame->getKeyboard());

	if (pView)
		pCocoaKeyboard->insertTextEvent(pView, aString);	

	[XAP_CocoaToolPalette setPreviewText:@""];

	m_selectedRange = NSMakeRange(NSNotFound, 0);
	m_hasMarkedText = NO;
}


- (NSRange)markedRange
{
	// UT_ASSERT_NOT_REACHED();
	UT_DEBUGMSG(("markedRange (m_hasMarkedText=%s)\n",m_hasMarkedText ? "YES" : "NO"));

	/* This method gets called when you delete the current character sequence to 0 length,
	 * so let's clear the preview...
	 */
	[XAP_CocoaToolPalette setPreviewText:@""];

	m_selectedRange = NSMakeRange(NSNotFound, 0);
	m_hasMarkedText = NO;

	return m_selectedRange;
}

- (NSRange)selectedRange
{
	UT_DEBUGMSG(("selectedRange=(location=%u,length=%u)\n",m_selectedRange.location,m_selectedRange.length));
	return m_selectedRange;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
	[XAP_CocoaToolPalette setPreviewText:aString];

	m_selectedRange = selRange;
	m_hasMarkedText = (selRange.length != 0 ? YES : NO);

	UT_DEBUGMSG(("range=(location=%u,length=%u) [aString length]=%u\n", selRange.location, selRange.length, [(NSString*)aString length]));
	/*
		Steal code from the selection handling code in XP land. We have the AV_View
		so everything is here.
	 */

	if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView())) {
		NSString * str = 0;

		if ([aString isKindOfClass:[NSString class]]) {
			str = (NSString *) aString;
		}
		else if ([aString isKindOfClass:[NSAttributedString class]]) {
			NSAttributedString * attr_str = (NSAttributedString *) aString;
			str = [attr_str string];
		}
		if (str) {
			if ([str length]) {
				UT_UCS4String ucs4([str UTF8String], [str length]);

				PT_DocPosition oldPos = pView->getPoint();

				if (!pView->isSelectionEmpty())
					oldPos = pView->getSelectionAnchor();

				ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>(m_pFrame->getKeyboard());
				pCocoaKeyboard->insertTextEvent(pView, str);

			//	pView->cmdCharInsert(ucs4.ucs4_str(), ucs4.length());
				pView->cmdSelect(oldPos, pView->getPoint());
			}
		}
	}
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
