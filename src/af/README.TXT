
The 'af' directory contains all source code for the cross-platform 
application framework.  This directory contains the following 
subdirectories:

ev
	Source code for the event mechanism for the cross-platform application
	framework. This code contains the machinery to do key bindings, mouse
	bindings, menu bars, and tool bars.  This code is currently used by
	the word processor, but we expect to also use it for the
	spreadsheet application.

gr
	Source code for graphics (drawing code, font code, etc.)

util
	Source code for general-purpose utility functions.

xap
	Source code for application-neutral portion of the
	cross-platform framework defined in src/ev.

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

