#!/usr/bin/env python

import os
import os.path
import sys
import testcase

if (sys.platform == 'win32'):
	windows = 1
	ext = '.exe'
else:
	ext = '-2.0'

binary_prefix = '../../build'
#binary_prefix = '../'

abiword_binary = os.path.abspath(binary_prefix + '/src/wp/main/unix/AbiWord' + ext)
print abiword_binary

# This code from Subversion, copyright Collab.net

# Run any binary, logging the command line (TODO: and return code)
def _run_command(command, error_expected, *varargs):
  """Run COMMAND with VARARGS; return stdout, stderr as lists of lines.
  If ERROR_EXPECTED is None, any stderr also will be printed."""

  args = ''
  for arg in varargs:					# build the command string
	args += ' "' + arg + '"'
	
  # Log the command line
  print 'CMD:',  os.path.basename(command) + args
																							

  infile, outfile, errfile = os.popen3(command + args)
  stderr_lines = errfile.readlines()
  
  outfile.close()
  infile.close()
  errfile.close()

																							
def run_abiword(*varargs):
  """Run AbiWord with VARARGS; return stdout, stderr as lists of lines."""
  return _run_command(abiword_binary, None, *varargs)

def run_abiword_converter(format, file, *varargs):
  """Run AbiWord with to convert FILE to FORMAT, with additional
  arguments in VARARGS.	 Return stdout, stderr as lists of lines.."""
#  print "running converter"
  return run_abiword('--to=' + format, file, *varargs)



######################################################################
# Main testing functions

# These two functions each take a TEST_LIST as input.  The TEST_LIST
# should be a list of test functions; each test function should take
# no arguments and return a 0 on success, non-zero on failure.
# Ideally, each test should also have a short, one-line docstring (so
# it can be displayed by the 'list' command.)

# Func to run one test in the list.
def run_one_test(n, test_list):
  "Run the Nth client test in TEST_LIST, return the result."

  if (n < 1) or (n > len(test_list) - 1):
    print "There is no test", `n` + ".\n"
    return 1

  tc = testcase.TestCase(test_list[n], n)
  # Run the test.
  args = []
  return tc.run(args)

def _internal_run_tests(test_list, testnum=None):
  exit_code = 0

  if testnum is None:
    for n in range(1, len(test_list)):
      if run_one_test(n, test_list):
        exit_code = 1
  else:
    exit_code = run_one_test(testnum, test_list)

  return exit_code



def run_tests(test_list):
  """Main routine to run all tests in TEST_LIST.

  NOTE: this function does not return. It does a sys.exit() with the
        appropriate exit code.
  """

  for arg in sys.argv:

    if arg == "list":
      print "Test #  Mode   Test Description"
      print "------  -----  ----------------"
      n = 1
      for x in test_list[1:]:
        testcase.TestCase(x, n).list()
        n = n+1

      # done. just exit with success.
      sys.exit(0)

  exit_code = _internal_run_tests(test_list)

  # return the appropriate exit code from the tests.
  sys.exit(exit_code)
