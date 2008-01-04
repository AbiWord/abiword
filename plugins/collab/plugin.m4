
collab_req="libxml-2.0 >= 2.4.0"
collab_xmpp_req="loudmouth-1.0 >= 1.0.1"
# TODO explicitely check for dbus-1 >= 1.0.1 ? dbus-glib depends on it anyways.
collab_sugar_req="dbus-glib-1 >= 0.70"

COLLAB_CFLAGS=
COLLAB_LIBS=

if test "$enable_collab" == "yes"; then


# check for various boost libs, needs to be done before
AX_BOOST_BASE([1.33.1])


AC_ARG_ENABLE([collab-backend-fake], 
    [AS_HELP_STRING([--enable-collab-backend-fake], [Fake backend for debugging purposes only (default: off)])], 
[
	enable_collab_backend_fake=$enableval
], [
	enable_collab_backend_fake="no"
])
AM_CONDITIONAL([COLLAB_BACKEND_FAKE], [test $enable_collab_backend_fake == "yes"])


AC_ARG_ENABLE([collab-backend-xmpp], 
    [AS_HELP_STRING([--enable-collab-backend-xmpp], [Jabber backend (default: auto)])], 
[
	enable_collab_backend_xmpp=$enableval
], [
	enable_collab_backend_xmpp="auto"
])
if test $enable_collab_backend_xmpp == "yes"; then
	PKG_CHECK_EXISTS([ $collab_xmpp_req ],
	[
		collab_req="$collab_req $collab_xmpp_req"
	], [
		AC_MSG_ERROR([$collab_xmpp_req is required for the collab plugin TCP backend])
	])
elif test $enable_collab_backend_xmpp == "auto"; then
	PKG_CHECK_EXISTS([ $collab_xmpp_req ],
	[
		enable_collab_backend_xmpp="yes"
		collab_req="$collab_req $collab_xmpp_req"
	], [
		enable_collab_backend_xmpp="no"
	])
fi
AM_CONDITIONAL([COLLAB_BACKEND_XMPP], [test $enable_collab_backend_xmpp == "yes"])


AC_ARG_ENABLE([collab-backend-tcp], 
    [AS_HELP_STRING([--enable-collab-backend-tcp], [TCP backend (default: auto)])], 
[
	enable_collab_backend_tcp=$enableval
], [
	enable_collab_backend_tcp="auto"
])
if test $enable_collab_backend_tcp == "yes"; then
	AC_LANG_PUSH(C++)
	AX_BOOST_THREAD
	if test $ax_cv_boost_thread == "no"; then
		AC_MSG_ERROR([Boost::Thread is required for the collab plugin TCP backend])		
	fi
	AC_CHECK_HEADERS([asio.hpp], [], 
	[
		AC_MSG_ERROR([Boost.Asio is required for the collab plugin TCP backend])
	])
	AC_LANG_POP
elif test $enable_collab_backend_tcp == "auto"; then
	AX_BOOST_THREAD
	if test $ax_cv_boost_thread == "yes"; then
		AC_LANG_PUSH(C++)
		AC_CHECK_HEADERS([asio.hpp], 
		[
			enable_collab_backend_tcp="yes"
		], [
			enable_collab_backend_tcp="no"
			AC_MSG_WARN([Boost.Asio is required for the TCP backend])
		])
		AC_LANG_POP
	else
		enable_collab_backend_tcp="no"
		AC_MSG_WARN([Boost::Thread is required for the TCP backend])		
	fi
fi
AM_CONDITIONAL([COLLAB_BACKEND_TCP], [test $enable_collab_backend_tcp == "yes"])


AC_ARG_ENABLE([collab-backend-sugar], 
    [AS_HELP_STRING([--enable-collab-backend-sugar], [Sugar/OLPC backend (default: auto)])], 
[
	enable_collab_backend_sugar=$enableval
], [
	enable_collab_backend_sugar="auto"
])
if test $enable_collab_backend_sugar == "yes"; then
	PKG_CHECK_EXISTS([ $collab_sugar_req ],
	[
		collab_req="$collab_req $collab_sugar_req"
	], [
		AC_MSG_ERROR([$collab_sugar_req is required for the collab plugin Sugar/OLPC backend])
	])
elif test $enable_collab_backend_sugar == "auto"; then
	PKG_CHECK_EXISTS([ $collab_sugar_req ],
	[
		enable_collab_backend_sugar="yes"
		collab_req="$collab_req $collab_sugar_req"
	], [
		enable_collab_backend_sugar="no"
	])
fi
AM_CONDITIONAL([COLLAB_BACKEND_SUGAR], [test $enable_collab_backend_sugar == "yes"])


AC_ARG_ENABLE([collab-backend-service], 
    [AS_HELP_STRING([--enable-collab-backend-service], [collaborate.abisource.com backend (default: off)])], 
[
	enable_collab_backend_service=$enableval
], [
	enable_collab_backend_service="no"
])
if test $enable_collab_backend_service == "yes"; then
	AX_BOOST_THREAD
	if test $ax_cv_boost_thread == "no"; then
		AC_MSG_ERROR([Boost::Thread is required for the collab plugin \`collaborate.abisource.com' backend])		
	fi
	AC_LANG_PUSH(C++)
	AC_CHECK_HEADERS([asio.hpp], [], 
	[
		AC_MSG_ERROR([Boost.Asio is required for the \`collaborate.abisource.com' backend])
	])
	AC_LANG_POP
elif test $enable_collab_backend_service == "auto"; then
	AX_BOOST_THREAD
	if test $ax_cv_boost_thread == "yes"; then
		AC_LANG_PUSH(C++)
		AC_CHECK_HEADERS([asio.hpp], 
		[
			enable_collab_backend_service="yes"
		], [
			enable_collab_backend_service="no"
		])
		AC_LANG_POP
	else
		enable_collab_backend_service="no"
		AC_MSG_WARN([Boost::Thread is required for the \`collaborate.abisource.com' backend])		
	fi
fi
AM_CONDITIONAL([COLLAB_BACKEND_SERVICE], [test $enable_collab_backend_service == "yes"])


AC_ARG_ENABLE([collab-record-always], 
    [AS_HELP_STRING([--enable-collab-record-always], [Always record AbiCollab sessions (default: off)])], 
[
	enable_collab_record_always=$enableval
], [
	enable_collab_record_always="no"
])
AM_CONDITIONAL([COLLAB_RECORD_ALWAYS], [test $enable_collab_record_always == "yes"])


PKG_CHECK_MODULES(COLLAB,[ $collab_req ])

if test $enable_collab_backend_fake == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_HANDLER_FAKE"
fi
if test $enable_collab_backend_xmpp == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_HANDLER_XMPP"
fi
if test $enable_collab_backend_tcp == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_HANDLER_TCP"
fi
if test $enable_collab_backend_sugar == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_HANDLER_SUGAR"
fi
if test $enable_collab_backend_service == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_HANDLER_SERVICE"
fi
if test $enable_collab_record_always == "yes"; then
	COLLAB_CFLAGS="$COLLAB_CFLAGS -DABICOLLAB_RECORD_ALWAYS"
fi

if test $enable_collab_backend_tcp == "yes" || \
   test $enable_collab_backend_service == "yes"; then
	COLLAB_LIBS="$COLLAB_LIBS $BOOST_THREAD_LIB"
fi

COLLAB_CFLAGS="$COLLAB_CFLAGS "'${WP_CPPFLAGS}'
COLLAB_LIBS="$COLLAB_LIBS "'${PLUGIN_LIBS}'

fi # plugin conditional

AC_SUBST([COLLAB_CFLAGS])
AC_SUBST([COLLAB_LIBS])

