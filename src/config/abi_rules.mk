#! gmake

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

