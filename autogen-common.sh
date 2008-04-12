#!/bin/sh

# find plugins Makefile templates
find plugins -name Makefile.am | sed  's|.am$||g' > plugin-makefiles.m4

# create plugin list
(cd plugins && find . -maxdepth 1 -type d | grep -v '^\.$' | grep -v '\./\.' | sed 's|\./||g' | xargs echo) > plugin-list.m4

# create conditionals for builtin plugins
(for plugin in `cat plugin-list.m4`; do
	u=`echo $plugin | tr '[:lower:]' '[:upper:]'`
	echo 'AM_CONDITIONAL(['$u'_BUILTIN], test "$enable_'$plugin'_builtin" == "yes")'
done) > plugin-builtin.m4

# create plugin configuration
find plugins -name plugin.m4 | xargs cat > plugin-configure.m4

# find extra m4 files provided by plugins and symlink them
for f in ` find ./plugins -name '*.m4' | grep -v 'plugin\.m4'`; do
    ln -sf $f
done

