/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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
 



#ifndef EV_EDITEVENTMAPPER_H
#define EV_EDITEVENTMAPPER_H

/****************************************************************
*****************************************************************
** EditEventMapper, EditBinding, and EditMethod form the basis for
** all editing operations.  All keystrokes, mouse events, menu
** picks, and toolbar picks are directed thru here.  These are
** defined as classes outside of the document so that we may switch
** between different implementations as we want.
**
** EditEventMapper is in charge.  It receives each event, applies
** any policy decisions, and then uses the EditBindings to decide
** what to do.  (An example policy decision might be whether or
** not to do sticky-control and -shift keys for one-handed users;
** we may have 2 different implementations of EditEventMapper --
** a normal one and a sticky one, for example.  (Another policy
** decision might be whether or not to support Control- and
** Shift- toolbar events.)
**
** EditBindings provides an emacs-like single/multiple key/event
** sequence to action mapping.  Each EditBinding table provides
** a mapping from a single event (key, mouse, etc.) to an action.
** If the action maps to a EditMethod, the key/event sequence is
** considered complete and the indicated EditMethod is returned
** to allow the caller to invoke it.  If the action maps to another
** EditBinding table, the key/event sequence is considered to be
** a prefix to a longer sequence; in this case, the EditEventMapper
** updates it state and waits for the next key/event.  We can
** think of these EditBinding tables as forming a tree.
**
** An EditMethod (and EditMethodContainer) provides a handle to
** a specific editing operation (such as InsertCharacter, NextLine,
** etc).  The EditMethodContainer should contain a static set of
** builtin methods and a set of dynamically created ones (dare
** I say JavaScript macros).  Provision has been made to allow
** the implementation of the static methods to be switched based
** upon user language or some other criteria; this allows us to 
** define multiple sets of the basic primitives, in case we want
** to simplify the selection of various WordProcessor emmulations
** or language quirks.  For example, does NextWord() go to the 
** first whitespace at the end of the current word or go to the
** beginning of the next word and for those languages which don't
** use whitespace, where does it go or is it even defined.
**
** The EditEventMapper may have more than one set of EditBinding
** trees -- much like emacs's global bindings and mode bindings.
** A key/event sequence will searched for in the mode bindings
** and then in the global bindings.
**
** The EditEventMapper returns the EditMethod rather than invoking
** it directly.  This allows our caller to query the EditMethod
** for it's properties (such as a short description for the 
** status bar when the menu item is highlighted).
**
** The EditEventMapper has seperate event methods for keystrokes,
** mouse operations, menu (both menu-bar and pop-up) selections,
** and toolbar selections.
**
** Keystroke Events:
**
** Mouse Events:
**   We allow the following sequences to be interpreted:
**     [SingleClick,DoubleClick] [Drag]* Release
**
**   Mouse buttons are numbered from 1 to n.  It is upto the GUI/OS
**   to decide which physical button is mapped to 1 and which to n.
**
**   Keyboard modifiers are respected.
**
**   Depending upon the GUI, a Double click may also generate Single
**   (and Double) click and release events, so be aware.
**
**   A drag is processed as a sequence of mouse movements.
**   The quantity and granularity of the movement is GUI/OS
**   dependent; we will simply map each of them as we see
**   them.  We allow the keyboard modifiers to change during
**   the drag, so the mapping may change between successive
**   drag events.  Again, be aware.
**
**   We do not attempt to support sequences where a second
**   mouse button is pressed during a drag (while the first
**   mouse button is still down).
**
**   TODO Question: If another event (keystroke, mouse click)
**   TODO           occurs during a drag, do we:
**   TODO           (1) end the drag with or without issuing an
**   TODO           implicit release and then process the new
**   TODO           event (and ignore the eventual actual 
**   TODO           release), or
**   TODO           (2) ignore the new event and keep the
**   TODO           drag active until the actual release, or
**   TODO           (3) ??
**
** Menu Events:
**   We allow menus (both the menu-bar and pop-ups) to register
**   the EditMethods to be invoked when menu item is selected.
**   We allow menus to be registered by name.  For cascaded
**   (ie nested (ie pull-right)) menus we allow a hierarchy
**   of names.  Since menu selections hit in a single event
**   (rather than a sequence of selections in the case of a
**   cascaded menu), we will use the menu "pathname" to generate
**   the complete mapping in a single call (or return bogus);
**   that is, it would be an error for a menu selection to
**   return an incomplete status.
**
**   We use a standard (unix) pathname model for the menu
**   naming.  For example:
**     result = pEEM->Menu("MenuBar/File/Open",&pEM);
**     result = pEEM->Menu("Popup-Foo/xyzzy/Open",&pEM);
**
**   We allow a menu event to complete an incomplete sequence
**   of events -- just as any other type of event, an incomming
**   menu event is interpreted in the context of the inprogress
**   sequence.  For example, the user could press Control-x and
**   then hit "MenuBar/File/Open" (assuming that Control-x was a
**   defined prefix key).
**
** Toolbar Events:
**   Toolbars operate in much the same fashion as menus in that
**   we only receive a single selection.  Although cascaded
**   toolbars are not common, we will use the same naming scheme
**   to allow multiple toolbars to maintain their own namespaces.
**   For example, the "Find" button on the "Normal" toolbar may
**   map to a different EditMethod than the "Find" button on the
**   "Advanced" toolbar.
**     result = pEEM->Toolbar("Normal/Find",&pEM);
**
**   Like with menus, we allow a toolbar event to complete an
**   incomplete sequence.  For example, Control-x and then "Normal/Find".
**
** TODO Question: Should Menus and Toolbars recognize keyboard
** TODO           modifiers (control, shift, etc) like keystroke
** TODO           and mouse events ??
**
** TODO Issue: We should use the EditEventMapper mechanism to
** TODO        handle all keystrokes and not use the GUI native
** TODO        menu keys (accelerators) in order to allow us to map
** TODO        Alt-F to NextWord() rather to raise the File menu.  We
** TODO        should provide a set of Win32 compatibility bindings
** TODO        to map Alt-F to something like RaiseMenu("MenuBar/File")
**
** TODO Question: Should we distinguish between left- and right-versions
** TODO           of SHFIT, CONTROL, ALT, etc ??
**
******************************************************************
*****************************************************************/

#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditMethod.h"

typedef UT_uint32 EV_EditEventMapperResult;
#define EV_EEMR_BOGUS_START		((EV_EditEventMapperResult) 1) /* start of unknown event sequence */
#define EV_EEMR_BOGUS_CONT		((EV_EditEventMapperResult) 2) /* unknown continuation event sequence */
#define EV_EEMR_INCOMPLETE		((EV_EditEventMapperResult) 3) /* accumulating valid prefix */
#define EV_EEMR_COMPLETE		((EV_EditEventMapperResult) 4) /* complete sequence */

/****************************************************************/
/****************************************************************/

class EV_EditEventMapper
{
public:
	EV_EditEventMapper(EV_EditBindingMap * pebm);

	EV_EditEventMapperResult Keystroke(EV_EditBits eb,
									   EV_EditMethod ** ppEM,
									   UT_uint32 * piPrefixCount);

	EV_EditEventMapperResult Mouse(EV_EditBits eb,
								   EV_EditMethod ** ppEM,
								   UT_uint32 * piPrefixCount);

	EV_EditEventMapperResult Menu(const char * szName,
								  EV_EditMethod ** ppEM,
								  UT_uint32 * piPrefixCount);

	EV_EditEventMapperResult Toolbar(const char * szName,
									 EV_EditMethod ** ppEM,
									 UT_uint32 * piPrefixCount);

	const char *			getShortcutFor(const EV_EditMethod * pEM) const;

protected:
	EV_EditBindingMap *		m_pebmTopLevel;
	EV_EditBindingMap *		m_pebmInProgress;
};

#endif /* EV_EDITEVENTMAPPER_H */
