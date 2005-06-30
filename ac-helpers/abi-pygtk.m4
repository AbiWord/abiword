# 
# start: abi/ac-helpers/abi-pygtk.m4
# 
AC_DEFUN([ABI_PYGTK], [

AC_ARG_ENABLE(python,
			[AC_HELP_STRING([--enable-python], [Compile with python bindings])],enable_python="$enableval",enable_python=no)

if test "x$enable_python" = "xyes"; then
	AC_PATH_PROG(PYTHON, python, no)
	if test x$PYTHON = xno; then
		AC_MSG_ERROR(Please install python)
	fi
	AC_MSG_CHECKING(Python compile flags)
	changequote(<<, >>)dnl
	PY_VER=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("VERSION")[0];'`
	PY_LIB=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_python_lib(standard_lib=1);'`
	PY_INC=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_config_vars("INCLUDEPY")[0];'`
	PY_PREFIX=`$PYTHON -c 'import sys; print sys.prefix'`
	PY_EXEC_PREFIX=`$PYTHON -c 'import sys; print sys.exec_prefix'`
	changequote([, ])dnl
	if test -f $PY_INC/Python.h; then
		PYTHON_LIBS="-L$PY_LIB/config -lpython$PY_VER -lpthread -lutil"
		PYTHON_CFLAGS="-I$PY_INC"
		AC_MSG_RESULT(ok)
	else
		AC_MSG_ERROR([Can't find Python.h])
	fi
	PKG_CHECK_MODULES(PYGTK, pygtk-2.0)
	PYGTK_CODEGENDIR="`$PKG_CONFIG --variable=codegendir pygtk-2.0`"
	PYGTK_DEFSDIR="`$PKG_CONFIG --variable=defsdir pygtk-2.0`"
	AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
	if test x$PYGTK_CODEGEN = xno; then
		AC_MSG_ERROR(Please install the application pygtk-codegen-2.0)
	fi
	AC_MSG_CHECKING(for pygtk codegendir)
	AC_MSG_RESULT($PYGTK_CODEGENDIR)
	AC_MSG_CHECKING(for pygtk defsdir)
	AC_MSG_RESULT($PYGTK_DEFSDIR)

	AC_DEFINE([ENABLE_PYTHON], [1], [Enable python bindings ***DOES NOT WORK YET!])
else
	PY_VER=""
	PYTHON_CFLAGS=""
	PYTHON_LIBS=""
	PYGTK_CFLAGS=""
	PYGTK_LIBS=""
	PYGTK_CODEGENDIR=""
	PYGTK_CODEGEN=""
	PYGTK_DEFSDIR=""
fi
AC_SUBST(PY_VER)
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(PYGTK_CFLAGS)
AC_SUBST(PYGTK_LIBS)
AC_SUBST(PYGTK_CODEGENDIR)
AC_SUBST(PYGTK_DEFSDIR)
AM_CONDITIONAL(ENABLE_PYTHON, test x$enable_python = xyes)

])
# 
# end: abi/ac-helpers/abi-pygtk.m4
# 
