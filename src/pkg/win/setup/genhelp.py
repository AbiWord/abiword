
import string
import os
import os.path

def do_dir(s, below):
    items = os.listdir(s)
    for it in items:
        if string.strip(string.lower(it)) == 'cvs':
            continue

        full = string.replace(os.path.join(s, it), '\\', '/')
        partial = string.replace(os.path.join(below, it), '\\', '/')

        if os.path.isfile(full):
            print '<datafile name="__CANONDIR__/AbiWord/help/%s" relpath="AbiWord/help/%s" />' % (partial, below)
        if os.path.isdir(full):
            do_dir(full, partial)

os.chdir('../../../../../')

do_dir('abi/user/wp/help', '')

os.chdir('abi/src/pkg/win/setup')


