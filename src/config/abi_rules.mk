#! gmake
 
##
## The contents of this file are subject to the AbiSource Public
## License Version 1.0 (the "License"); you may not use this file
## except in compliance with the License. You may obtain a copy
## of the License at http://www.abisource.com/LICENSE/ 
## 
## Software distributed under the License is distributed on an
## "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
## implied. See the License for the specific language governing
## rights and limitations under the License. 
## 
## The Original Code is AbiSource Utilities.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

##################################################################
##################################################################
## abi_rules.mk --  Makefile definitions for building AbiSource software.
## This is a makefile include.  It should be included before any
## other rules are defined.
##
## The general structure of an AbiSource Makefile should be:
##
##        #! gmake
##        ABI_DEPTH=<your depth in source tree from abi/src>
##        include $(ABI_DEPTH)/config/abi_defs.mk
##        <local declarations>
##        include $(ABI_DEPTH)/config/abi_rules.mk
##        <local rules>
##
##################################################################
##################################################################


##################################################################
##################################################################
## Interlude into NSPR makefile system.
## 
## Map ABI_DEPTH (set in makefile that included us) into NSPR's
## MOD_DEPTH, include their rules.mk, and fix up any paths as
## necessary.

MOD_DEPTH=$(ABI_DEPTH)/other/nsprpub
include $(MOD_DEPTH)/config/rules.mk

##################################################################
##################################################################

