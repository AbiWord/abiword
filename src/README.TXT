
The 'src' directory contains all source code for all modules and applications
in the tree.  This directory contains the following subdirectories:


config
	Files related to the build system.  This contains no actual
	product source code.  Rather, it contains things like
	Makefiles and scripts.

ev
	Source code for the event mechanizm for the cross-platform application
	framework. This code contains the machinery to do key bindings, mouse
	bindings, menu bars, and tool bars.  This code is currently used by
	the word processor, but we expect to also use it for the
	spreadsheet application.

other
	This is where source code for all third-party libraries is placed.

	For example, we use James Clark's XML parser, expat, which is
	stored here.  In general, don't put libraries in 'other' if
	we are making substantial modifications to the code.  Minor bug
	fixes are fine (please send the fix back to the original author).
	Customizations to make the library fit into our build system are
	fine.  Source code in 'other' is not required to comply with
	every one of our coding guidelines.  Don't make gratuitous sweeping
	changes just to fix the indent level on someone else's library.

util
	Source code for general-purpose utility functions.

wp
	Source code for the word processor application.

xap
	Source code for application-neutral portion of the
	cross-platform framework defined in src/ev.


Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

