################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

bin_PROGRAMS = \
	direct

AM_CPPFLAGS = -I${top_srcdir}/src/include \
	-I${top_builddir}/local \
	@EXTRA_CPPFLAGS@ -DENABLE_OCL

BUILT_SOURCES = \
	${top_builddir}/local/direct_strings.c \
	${top_builddir}/local/direct_strings.h

${top_builddir}/local/direct_strings.c : \
	${top_srcdir}/examples/direct/kernels/*.cl
	${top_srcdir}/utils/stringify_kernel_files --fortran \
		${top_srcdir}/examples/direct/kernels \
		${top_builddir}/local/direct_strings

direct_SOURCES = \
	${top_builddir}/local/direct_strings.c \
	${top_builddir}/local/direct_strings.h \
	${top_srcdir}/examples/direct/ewald.h \
	${top_srcdir}/examples/direct/grav.h \
	${top_srcdir}/examples/direct/kd.h \
	${top_srcdir}/examples/direct/spline.h \
	${top_srcdir}/examples/direct/tipsydefs.h \
	${top_srcdir}/examples/direct/ewald.c \
	${top_srcdir}/examples/direct/grav.c \
	${top_srcdir}/examples/direct/kd.c \
	${top_srcdir}/examples/direct/main.c
direct_LDFLAGS = @EXTRA_LDFLAGS@
direct_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la -lm
