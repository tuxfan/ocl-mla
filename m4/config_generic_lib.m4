dnl ------------------------------------------------------------------------ dnl
dnl Copyright (c) 2012 Los Alamos National Security, LLC
dnl All rights reserved.
dnl ------------------------------------------------------------------------ dnl

dnl ------------------------------------------------------------------------ dnl
dnl Generic rule for auto-detection of unix-style libraries
dnl
dnl CONFIG_GENERIC_LIB($1 = string name,
dnl                 $2 = macro name,
dnl                 $3 = library symbol,
dnl                 $4 = language,
dnl                 $5 = dependencies)
dnl ------------------------------------------------------------------------ dnl

AC_DEFUN([CONFIG_GENERIC_LIB], [
	AC_ARG_VAR([$2], [User specified path to $1 top-level directory])
	AC_ARG_VAR([$2_CPPFLAGS], [User specified path to $1 includes])
	AC_ARG_VAR([$2_LDFLAGS], [User specified path to $1 libraries])
	AC_ARG_VAR([$2_CHECKHEADER], [User specified $1 check header])
	AC_ARG_VAR([$2_CHECKLIB], [User specified $1 check library])
	AC_ARG_VAR([$2_LIBS], [User specified $1 link library names])
	AC_ARG_VAR([$2_FRAMEWORK], [User specified apple $1 framework])

	AC_ARG_WITH([$1], [AC_HELP_STRING([--with-$1],
		[User specified parth to $1 top-level directory])],
		[
			if test -n "$2" ; then

				if test -z "$$2_CPPFLAGS" ; then
					lib_CPPFLAGS="-I$2/include"
				else
					lib_CPPFLAGS="$$2_CPPFLAGS"
				fi

				if test -z "$$2_LDFLAGS" ; then
					lib_LDFLAGS="-L$2/lib"
				else
					lib_LDFLAGS="$$2_LDFLAGS"
				fi

			elif test "$withval" != no ; then

				if test -z "$$2_CPPFLAGS" ; then
					lib_CPPFLAGS="-I$withval/include"
				else
					lib_CPPFLAGS="$$2_CPPFLAGS"
				fi

				if test -z "$$2_LDFLAGS" ; then
					lib_LDFLAGS="-L$withval/lib"
				else
					lib_LDFLAGS="$$2_LDFLAGS"
				fi

			fi
		],
		[
			if test -n "$2" ; then

				if test -z "$$2_CPPFLAGS" ; then
					lib_CPPFLAGS="-I$2/include"
				else
					lib_CPPFLAGS="$$2_CPPFLAGS"
				fi

				if test -z "$$2_LDFLAGS" ; then
					lib_LDFLAGS="-L$2/lib"
				else
					lib_LDFLAGS="$$2_LDFLAGS"
				fi

			else
				lib_CPPFLAGS=""
				lib_LDFLAGS=""
			fi
		])

		if test -z "$$2_CHECKHEADER" ; then
			check_header="$1.h"
		else
			check_header="$$2_CHECKHEADER"
		fi

		if test -z "$$2_CHECKLIB" ; then
			check_lib="$1"
		else
			check_lib="$$2_CHECKLIB"
		fi

		old_CPPFLAGS=$CPPFLAGS
		old_LDFLAGS=$LDFLAGS

		if test -z "$$2_FRAMEWORK" ; then
			CPPFLAGS="$CPPFLAGS $lib_CPPFLAGS"
			LDFLAGS="$LDFLAGS $lib_LDFLAGS"
		else
			CPPFLAGS="$CPPFLAGS $$2_FRAMEWORK"
			LDFLAGS="$LDFLAGS"
			check_lib="c"
		fi

		case "$4" in
			C++)
				AC_LANG_PUSH(C++)
			;;
			*)
				AC_LANG_PUSH(C)
			;;
		esac

		AC_CHECK_HEADER($check_header, [lib_h=yes], [lib_h=no], /* check */)

		old_LIBS="$LIBS"
		LIBS="$$2_LIBS $5 $LIBS"
		AC_MSG_CHECKING(linkability of \"$3\" with $$2_FRAMEWORK $$2_LIBS)
		AC_LINK_IFELSE([AC_LANG_SOURCE([[
#include <$check_header>
int main () { $3; }
		]])],
			[lib_a=yes],[lib_a=no])
		AC_MSG_RESULT($lib_a)
		LIBS="$old_LIBS"

		AC_LANG_POP()

		CPPFLAGS="$old_CPPFLAGS"
		LDFLAGS="$old_LDFLAGS"

		if test "$lib_h" = yes && test "$lib_a" = yes ; then
			AC_SUBST($2_CPPFLAGS, [$lib_CPPFLAGS])
			AC_SUBST($2_HEADER, $check_header)

			if test -z "$$2_FRAMEWORK" ; then
				AC_SUBST($2_LDFLAGS, [$lib_LDFLAGS])
				AC_SUBST($2_LIBS, [$$2_LIBS])
			else
				AC_SUBST($2_LIBS, [$$2_FRAMEWORK])
			fi

			AC_DEFINE_UNQUOTED(HAVE_$2, 1, [Specify value for $2])
		else
			AC_DEFINE_UNQUOTED(HAVE_$2, 0, [Specify value for $2])
		fi
])
