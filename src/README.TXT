/* Raw ASCII version. Doxygen version below. Should the ASCII version be kept?

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
*/

/*! \mainpage

<h2>Sources</h2>

The <tt>src</tt> directory contains all source code for all modules
and applications in the tree.  This directory contains the following
subdirectories:

<dl>
<dt>Directory <tt>config</tt>:</dt>
  <dd> Contains files related to the build system.  This contains no actual
       product source code.  Rather, it contains things like
       Makefiles and scripts.

<dt>Directory <tt>af</tt>:</dt>
  <dd> Contains source code for application-neutral cross-platform framework.


<dt>Directory <tt>hello</tt>:</dt>
  <dd> Contains source code for the "Hello World" sample application.

<dt>Directory <tt>other</tt>:</dt>
  <dd> Contains source code for forked third-party libraries.
       For example, we use a librarified subset of ispell, which is
       stored here.  In general, put libraries in 'other' if we are making 
       substantial modifications to the code.  DO NOT DO THIS LIGHTLY!!
  <dd> Whenever possible, it's better to use existing libraries as peer 
       modules of the abi tree.  Minor bug fixes there are fine (please 
       send the fix back to the original author).  Customizations to make 
       the library fit into our build system are fine.  
  <dd> Source code in 'other' is not required to comply with every one of our 
       coding guidelines.  Don't make gratuitous sweeping changes just to fix 
       the indent level on someone else's library.

<dt>Directory <tt>text</tt> (\ref text_overview):</dt>
  <dd> Contains source code for the text-editing library used by
       various applications.

<dt>Directory <tt>wp</tt></dt>
  <dd> Contains source code for the word processor application.

</dl>

Subdirectories below may have additional hierarchy to further break
things down by module.  However, eventually, source code should find
itself in a directory which indicates the portability of the code
within it.  For example, cross-platform code should always be placed
in a subdirectory called 'xp'.  Win32-specific code should be in a
subdirectory called 'win'.

<h2>Issues / TODO</h2>

This is a list of issues and/or problems that need to be addressed in
the code. Some of these are probably also registered in the bug
database described by their various symptoms.

This however, is an attempt to keep track of things that should be
cleaned up / rewritten, even if it's not clear if there are actual
bugs caused by the existing implementation.

When adding / removing entries, be sure to go through the bug database
to add references to bugs representing symptoms or remove bugs fixed
by changes. When adding a new entry, please put your name after it,
optionally linking to a descriptive posting on the mailing list.


<h3>Text</h3>

<ul>
<li> Importers rely on superfluous use of fl_BlockLayout::format() to
work around (assumed) problems in
fl_BlockLayout::_stuffAllRunsOnALine() and the line breaker code. <a
href="http://www.abisource.com/mailinglists/abiword-dev/00/December/0540.html">(Martin
Sevior)</a>

<li> Existing code should be changed to provide view (non-mutating)
and controller (mutating) accessors to the PieceTable.  <a
href="http://www.abisource.com/mailinglists/abiword-dev/00/May/0643.html">(Sam
Th)</a>

</ul>


<h2>Doxygen Information</h2>

To regenerate documentation, run <tt>make dox</tt> from the top level
(<tt>abi</tt>) directory.

The Doxygen configuration is kept in
<tt>abi/src/.doxygen.cfg</tt>. The INPUT variable contains the list of
directories to be scanned when generating documentation.

Code overviews are kept in <tt>%README.TXT</tt> files in the various
modules. 

Sources may not be fully doxygenized yet - please feel free to add
documentation to functions/variables you happen to hack on. Note that
most of the HTML tags can be used for formatting clues which is
preferable to the Doxygen specific keywords for the simple reason that
most people know HTML already.

For details on the Doxygen syntax, please see <a
href="http://www.doxygen.org">www.doxygen.org</a>.

*/
