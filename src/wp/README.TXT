
The 'wp' directory contains all source code specific to AbiWord.  With 
the exception of the samples subdirectory, there is one subdirectory 
per module.

docstrux
	The editable document, implemented using piece tables. 

fmt
	Formatting and layout code, including views.

guts
	Miscellaneous stuff, including platform-native GUIs.

gui
	A cross-platform GUI using wxWindows.

impexp
	Importers and exporters for various file formats. 

samples
	Sample files in AbiWord format.  (no source code here)

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

