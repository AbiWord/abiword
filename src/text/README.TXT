
The 'text' directory contains the text-editing engine used by AbiWord 
and other AbiSuite apps.  There is one subdirectory per module.

fmt
	Formatting and layout code, including views.

ptbl
	The editable document, implemented using piece tables.

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.
