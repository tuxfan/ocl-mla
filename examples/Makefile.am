################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

SUBDIRS = direct

bin_PROGRAMS = \
	c_example \
	f90_example

AM_CPPFLAGS = \
	-I${top_builddir}/local \
	@EXTRA_CPPFLAGS@

AM_FCFLAGS = -I${top_builddir}/local @FC_MODULE_PATH_FLAG@${top_builddir}/lib

BUILT_SOURCES = \
	${top_builddir}/local/example_strings.c \
	${top_builddir}/local/example_strings.h \
	${top_builddir}/local/example_strings_f.f90

${top_builddir}/local/example_strings.c : \
	${top_srcdir}/examples/kernels/*.cl \
	${top_srcdir}/examples/direct/kernels/*.cl
	${top_srcdir}/utils/stringify_kernel_files --fortran \
		${top_srcdir}/examples ${top_builddir}/local/example_strings

c_example_SOURCES = \
	${top_builddir}/local/example_strings.c \
	${top_builddir}/local/example_strings.h \
	${top_srcdir}/examples/c_example.c
c_example_LDFLAGS = @EXTRA_LDFLAGS@
c_example_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

f90_example_SOURCES = \
	${top_builddir}/local/example_strings.c \
	${top_builddir}/local/example_strings_f.f90 \
	${top_srcdir}/examples/f90_example.f90
f90_example_LDFLAGS = @EXTRA_LDFLAGS@
f90_example_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la
