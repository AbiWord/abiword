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


<h2>AbiWord Doxygen Documentation</h2>

<a href="http://www.doxygen.org">Doxygen</a> is the tool used to
extract documentation from the sources and present it in the present
form.

The AbiWord sources are not fully doxygenized yet - please feel free
to add documentation to functions/variables you happen to hack
on. See below for details.

<h3>Generating The HTML Documentation</h3>

To regenerate documentation, run <tt>make dox</tt> from the top level
(<tt>abi</tt>) directory. This rule invokes the <tt>doxygen</tt> tool.
You can get Doxygen for many platforms at the Doxygen homepage <a
href="http://www.doxygen.org">www.doxygen.org</a>. The tool also
ships with Red Hat Linux and Debian.

If you want to use the documentation off-line for reference,
generating it locally is a good idea - it may become possible to
download it all in an archive eventually - but by generating it
locally, you can easily update it when you add comments yourself.

The HTML documentation on the AbiSource home page gets regenerated
hourly from the HEAD of the CVS repository.

<h3>AbiWord Doxygen Structure</h3>

The Doxygen configuration is kept in
<tt>abi/src/.doxygen.cfg</tt>. The <tt>INPUT</tt> variable contains
the list of directories to be scanned when generating documentation.
At present time only the text directory (the AbiWord backend) is
actually scanned - but it's simple to add other directories.

Each component of AbiWord has an overview description stored in a
<tt>%README.TXT</tt> file. This is where you want to put the grand
overview - and please add text if you gain insight on stuff not
presently documented in the <tt>%README.TXT</tt> files.

From the <tt>%README.TXT</tt> files you can refer to class/function
names and the outcome is nice guided tour where people can read the
overview description and dive into the code from there. It is of
course also possible to just go directly to the various hierarchies
and lists at the top of all pages.


<h3>AbiWord Doxygen Style Guide</h3>

Just a few guidelines for now. See fp_Container which adheres to these
(I think) and is comment complete.

Please try to adhere to these as it makes for more consistent
documentation (looks as well as content) - which gives a more
professional feel to it.  If you have ideas for other guidelines,
please post them to the developer list and we'll discuss it.

<ol>
<li> KISS! We don't want the source code to drown in fancy formatted
     comments.
<li> Comments should be kept in raw ASCII where possible. If you feel
     structure or typeface commands would help, use the HTML tags
     which most people understand.
<li> Use Doxygen default assumptions about documentation. That is,
     first line of a comment block is the brief description (do not
     use \\brief).
<li> Put the descriptions by the function definition, not the
     declaration (except inline accessors defined in the header
     files).
<li> In the brief line, describe what the function
     <em>does</em>. Leave the input/output details to the appropriate
     lines (accessors excepted again). See fp_Container::isEmpty.
<li> Always add input/output details for a function: \\param,
     \\retval (return value via pointer parameter), \\return (actual
     function return value).
<li> Add \\fixme, \\note, \\bug, etc as you feel necessary.
</ol>

A list of quick hints about doxygen syntax. Please see <a
href="http://www.doxygen.org">www.doxygen.org</a> for the full syntax.

<ul>
<li> Suppress links with \% (doxygen will add links to any function
     or filename).
<li> Note that "-" characters can be used for simple bullet list
     creation. I wonder if we should suppress that in favor of HTML
     tags.
<li> Sometimes you want to write a class name or similar in plural,
     but doxygen will not add a link. You can work around that by
     something like "\\link fp_Run fp_Runs \\endlink" but it's horrible
     to look at in the raw text. So do without it, rephrase, put the
     singular word in parenthesis "fp_Runs (fp_Run)", or assume the
     reader can find the class in the class list.
<li> Add references to named sections of the documentation with \\ref
     (e.g. \ref fmt_overview which links to the <tt>%README.TXT</tt>
     in the <tt>text/fmt</tt> directory on account of it having a
     \\page command).
</ul>

We may also want to discuss allowing simple figures for documenting
hairy code. I think it should be possible - but it should not be done
on account of comment text: the programmer should not be required to
look at the doxygen output to understand the code!

Do we want the brief descriptions and return/param text to be in a
certain language style? Would help make the doc look consistent, but
may be too much detail for people to bother with complying. Please see
fp_Container for a suggested style (i.e., compute vs. computes).

*/
