
The 'wp' directory contains all source code specific to AbiWord.  With 
the exception of the samples subdirectory, there is one subdirectory 
per module.

ap
	Source code for application-specific portion of the
	cross-platform framework defined in src/ev.  This contains
	application key bindings, mouse bindings, menu layouts,
	etc.  It contains the menu string tables.  It contains the
	table of application functions to which events may be bound.

fmt
	Formatting and layout code, including views.

gr
	Source code for graphics (drawing code, font code, etc.)

impexp
	Importers and exporters for various file formats. 

main
	Platform-specific Source code for main().

ptbl
	The editable document, implemented using piece tables. 

samples
	Sample files in AbiWord format.  (no source code here)

tests
	Test harness/framework.

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

