/* AbiSource Program Utilities
 * Some parts copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// This is the FSF's GetOpt class from libg++-2.8.1.1a.  libg++ is
// not maintained anymore, the important parts of which have
// migrated to libstdc++.  This class, however, is really handy
// since we can't expect all platforms (Unix, Windows, BeOS) to
// have getopt installed.  Changes by AbiSource are marked and are
// released under the LGPL for compatibility with the original code.

// Class names have been changed from GetOpt:: to UT_getOpt:: to be
// consistent with other AbiSource classes.  A naming convention
// like "thisClass" has been used in favor of "ThisClass".

/* Getopt for GNU. 
   Copyright (C) 1987, 1989, 1992 Free Software Foundation, Inc.
   (Modified by Douglas C. Schmidt for use with GNU G++.)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* This version of `getopt' appears to the caller like standard Unix `getopt'
   but it behaves differently for the user, since it allows the user
   to intersperse the options with the other arguments.

   As `getopt' works, it permutes the elements of `argv' so that,
   when it is done, all the options precede everything else.  Thus
   all application programs are extended to handle flexible argument order.

   Setting the environment variable _POSIX_OPTION_ORDER disables permutation.
   Then the behavior is completely standard.

   GNU application programs can use a third alternative mode in which
   they can distinguish the relative order of options and other arguments.  */

#ifndef UT_GETOPT_H
#define UT_GETOPT_H

// #include <std.h>
#include <stdio.h>

class UT_getOpt
{
private:
  /* The next char to be scanned in the option-element
     in which the last option character we returned was found.
     This allows us to pick up the scan where we left off.
        
     If this is zero, or a null string, it means resume the scan
     by advancing to the next ARGV-element.  */
  
  static char *nextchar;
  
  
  /* Describe how to deal with options that follow non-option ARGV-elements.
    
    UNSPECIFIED means the caller did not specify anything;
    the default is then REQUIRE_ORDER if the environment variable
    _OPTIONS_FIRST is defined, PERMUTE otherwise.
      
    REQUIRE_ORDER means don't recognize them as options.
    Stop option processing when the first non-option is seen.
    This is what Unix does.
            
    PERMUTE is the default.  We permute the contents of `argv' as we scan,
    so that eventually all the options are at the end.  This allows options
    to be given in any order, even with programs that were not written to
    expect this.
        
    RETURN_IN_ORDER is an option available to programs that were written
    to expect options and other ARGV-elements in any order and that care about
    the ordering of the two.  We describe each non-option ARGV-element
    as if it were the argument of an option with character code zero.
    Using `-' as the first character of the list of option characters
    requests this mode of operation.
                    
    The special argument `--' forces an end of option-scanning regardless
    of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
    `--' can cause `getopt' to return EOF with `optind' != ARGC.  */
  
   enum OrderingEnum { REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER };
   OrderingEnum ordering;

  /* Handle permutation of arguments.  */
  
  /* Describe the part of ARGV that contains non-options that have
     been skipped.  `first_nonopt' is the index in ARGV of the first of them;
     `last_nonopt' is the index after the last of them.  */
  
  static int first_nonopt;
  static int last_nonopt;
  
  void exchange (char **argv);
public:
  /* For communication from `getopt' to the caller.
     When `getopt' finds an option that takes an argument,
     the argument value is returned here.
     Also, when `ordering' is RETURN_IN_ORDER,
     each non-option ARGV-element is returned here.  */
  
  char *optarg;
  
  /* Index in ARGV of the next element to be scanned.
     This is used for communication to and from the caller
     and for communication between successive calls to `getopt'.
     On entry to `getopt', zero means this is the first call; initialize.
          
     When `getopt' returns EOF, this is the index of the first of the
     non-option elements that the caller should itself scan.
              
     Otherwise, `optind' communicates from one call to the next
     how much of ARGV has been scanned so far.  */
  
  int optind;

  /* Callers store zero here to inhibit the error message
     for unrecognized options.  */
  
  int opterr;
  
  int    nargc;
  char **nargv;
  const char  *noptstring;
  
  UT_getOpt (int argc, char **argv, const char *optstring);
  int operator () (void);
};

#endif /* UT_GETOPT_H */
