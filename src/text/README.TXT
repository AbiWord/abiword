/*
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
*/

/*! \page text_overview Text

The 'text' directory contains the text-editing engine used by AbiWord 
and other AbiSuite apps.  There is one subdirectory per module.

<dl>
<dt>Directory <tt>fmt/xp</tt> (\ref fmt_overview):
  <dd> Contains formatting and layout code, including views.</dd>

<dt>Directory <tt>ptbl/xp</tt> (\ref ptbl_overview):
  <dd> Contains the editable document, implemented using piece tables.</dd>

</dl>
*/
