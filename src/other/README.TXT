
This directory contains source code which was obtained from outside
sources.  In general, we do not modify or otherwise maintain any code
which is contained herein.  It's only here because it's being
incorporated into our applications, or because we intend to
incorporate it.

expat
	An XML parser by James Clark.  Covered under the Mozilla Public License.

js
	The JavaScript interpreter engine from Mozilla.  Covered under the Netscape
	Public License.

nsprpub
	NSPR, a portability library from Mozilla.  Covered under the Netscape
	Public License.

--

NOTE: Currently, js and nsprpub are not actually used by AbiWord.
This will change very soon.  They have incorporated into our build
process now, in anticipation of their future integration into the
application itself.  We'll use js as the scripting language for
AbiWord and our other apps.  NSPR is required because js requires it,
but we also intend to make use of parts of NSPR in our apps.
