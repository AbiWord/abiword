import regtest
import os
import os.path

def do_diff(file1, file2):
	import filecmp
	return filecmp.cmp(file1, file2, shallow=False)

def testSimple2HTML():
	"convert a simple document to html"
	simple_doc = os.path.abspath("simple.abw")
	regtest.run_abiword_converter("html", simple_doc)

	result_file = os.path.abspath("simple.html")
	goal_file = os.path.abspath("simple.html.goal")


	# diff the files
	# remember, 0 is pass
	result = not(do_diff(result_file, goal_file))

	# cleanup
	os.remove(os.path.abspath("simple.html"))

	return result

def testSimple2RTF():
	"convert a simple document to rtf"
	simple_doc = os.path.abspath("simple.abw")
	regtest.run_abiword_converter("rtf", simple_doc)

	result_file = os.path.abspath("simple.rtf")
	goal_file = os.path.abspath("simple.rtf.goal")


	# diff the files
	# remember, 0 is pass
	result = not(do_diff(result_file, goal_file))

	# cleanup
	os.remove(result_file)


# this list of tests, starting with None
test_list = [None,
			 testSimple2HTML,
			 testSimple2RTF]


if __name__ == '__main__':
  regtest.run_tests(test_list)
  # NOTREACHED
