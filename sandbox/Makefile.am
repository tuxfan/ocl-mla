################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

bin_PROGRAMS = \
	reduce \
	tridiag \
	tricycl_example \
	size

INCLUDES = -I${top_srcdir}/src/include \
	-I${top_builddir}/local \
	@EXTRA_CPPFLAGS@

AM_FCFLAGS = -I${top_builddir}/local @FC_MODULE_PATH_FLAG@${top_builddir}/lib

BUILT_SOURCES = ${top_builddir}/local/sandbox_strings.c

${top_builddir}/local/sandbox_strings.c : ${top_srcdir}/sandbox/kernels/*.cl
	${top_srcdir}/utils/stringify_kernel_files --fortran \
		${top_srcdir}/sandbox/kernels ${top_builddir}/local/sandbox_strings

reduce_SOURCES = \
	@top_builddir@/local/sandbox_strings.c \
	@top_srcdir@/sandbox/reduce.c
reduce_LDFLAGS = @EXTRA_LDFLAGS@
reduce_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

tridiag_SOURCES = \
	@top_builddir@/local/sandbox_strings.c \
	@top_srcdir@/sandbox/tridiag.c
tridiag_LDFLAGS = @EXTRA_LDFLAGS@
tridiag_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

tricycl_example_SOURCES = \
	@top_builddir@/local/sandbox_strings.c \
	@top_srcdir@/sandbox/tricycl.c \
	@top_srcdir@/sandbox/tricycl_example.c
tricycl_example_LDFLAGS = @EXTRA_LDFLAGS@
tricycl_example_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

size_SOURCES = \
	@top_builddir@/local/sandbox_strings.c \
	@top_srcdir@/sandbox/size.c
size_LDFLAGS = @EXTRA_LDFLAGS@
size_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la
