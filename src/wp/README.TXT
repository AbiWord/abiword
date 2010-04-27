/*
The 'wp' directory contains all source code specific to AbiWord.  
There is one subdirectory per module.

ap
	Source code for application-specific portion of the cross-platform
	framework defined in src/af/xap and src/af/ev.  This contains 
	application key bindings, mouse bindings, menu layouts, and toolbar 
	layouts.  It contains the menu string tables.  It contains the table 
	of application functions to which events may be bound.  It contains
	the code to manage the document window (rulers, scroll bars, and
	the actual document window itself).

impexp
	Importers and exporters for various file formats.

main
	Platform-specific Source code for main().

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.
*/

/*! \page wp_overview WP

The 'wp' directory contains all source code specific to AbiWord.  
There is one subdirectory per module.

<dl>
<dt>Directory <tt>ap</tt> (\ref ap_overview)</dt>
  <dd> Contains source code for application-specific portion of the
       cross-platform framework defined in <tt>src/af/xap</tt> and
       <tt>src/af/ev</tt>.  This contains application key bindings,
       mouse bindings, menu layouts, and toolbar layouts.  It contains
       the menu string tables.  It contains the table of application
       functions to which events may be bound.  It contains the code
       to manage the document window (rulers, scroll bars, and the
       actual document window itself).</dd>

<dt>Directory <tt>impexp</tt> (\ref impexp_overview)</dt>
  <dd> Contains importers and exporters for various file formats.</dd>

<dt>Directory <tt>main</tt> (\ref main_overview)</dt>
  <dd> Contains platform-specific source code for main().</dd>
</dl>

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.
*/