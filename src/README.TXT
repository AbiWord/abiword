
The 'src' directory contains all source code for all modules and applications
in the tree.  This directory contains the following subdirectories:


config
	Files related to the build system.  This contains no actual
	product source code.  Rather, it contains things like
	Makefiles and scripts.

af
	Source code for application-neutral cross-platform framework.

hello
	Source code for the "Hello World" sample application.

other
	This is where source code for forked third-party libraries is placed.
	For example, we use a librarified subset of ispell, which is
	stored here.  In general, put libraries in 'other' if we are making 
	substantial modifications to the code.  DO NOT DO THIS LIGHTLY!!

	Whenever possible, it's better to use existing libraries as peer 
	modules of the abi tree.  Minor bug fixes there are fine (please 
	send the fix back to the original author).  Customizations to make 
	the library fit into our build system are fine.  
	
	Source code in 'other' is not required to comply with every one of our 
	coding guidelines.  Don't make gratuitous sweeping changes just to fix 
	the indent level on someone else's library.

text
	Source code for the text-editing library used by various applications. 

wp
	Source code for the word processor application.


Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

