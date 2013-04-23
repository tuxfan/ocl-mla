################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

#------------------------------------------------------------------------------#
# Recurse subdirs
#------------------------------------------------------------------------------#

SUBDIRS = lib utils test examples ${DEVDIRS}

#------------------------------------------------------------------------------#
# Utils file
#------------------------------------------------------------------------------#

EXTRA_DIST = utils/stringify_kernel_files utils/make_devices

#------------------------------------------------------------------------------#
# Create links
#------------------------------------------------------------------------------#

links:
	@(cd local; \
		for file in `find ${abs_top_srcdir}/src -type f -regex '.*\.h*'` ; do \
			link=`echo $$file`; \
			ln -s $$link .; \
		done)

#------------------------------------------------------------------------------#
# Create devices file
#------------------------------------------------------------------------------#

devices:
	@(${top_srcdir}/utils/make_devices "${OCL_DEVICES}" "${top_builddir}/local")

#------------------------------------------------------------------------------#
# Configure message
#------------------------------------------------------------------------------#

reminder: FORCE
	@(echo ; \
		echo "/*----------------------------------------------------------------------------*\\"; \
		echo " * Configuration Complete!"; \
		echo "\*----------------------------------------------------------------------------*/")

#------------------------------------------------------------------------------#
# Build documentation
#------------------------------------------------------------------------------#

doc: FORCE
	@([ -d @DOC@ ] && cd @DOC@ && make doc > doxygen.log 2>&1)

#------------------------------------------------------------------------------#
# Extra install exec
#------------------------------------------------------------------------------#

install-exec-hook:
	@(cp ${top_builddir}/lib/*.mod ${libdir}/; \
		cp ${top_builddir}/local/ocl_config.h ${includedir}; \
		cp ${top_builddir}/local/ocl_local.h ${includedir}; \
		cp ${top_builddir}/local/ocl_device_config.h ${includedir})
	@([ -d ${bindir}/kernels ] || mkdir ${bindir}/kernels && \
		cp ${top_srcdir}/examples/kernels/*.cl ${bindir}/kernels)
	@([ -d ${bindir}/data ] || mkdir ${bindir}/data && \
		cp ${top_srcdir}/examples/direct/data/* ${bindir}/data)

#------------------------------------------------------------------------------#
# Extra install data
#------------------------------------------------------------------------------#

install-data-hook:
	@([ -d ${top_builddir}/@DOC@ ] && \
		([ -d ${datarootdir} ] || mkdir -p ${datarootdir}) && \
		([ -d ${mandir} ] || mkdir -p ${mandir}) && \
		([ -d ${datarootdir}/html ] || mkdir -p ${datarootdir}/html) && \
		([ -d ${top_builddir}/doc/man ] && \
			cp -r ${top_builddir}/doc/man/* ${mandir}) && \
		([ -d ${top_builddir}/doc/html ] && \
			cp -r ${top_builddir}/doc/html/* ${datarootdir}/html))

#------------------------------------------------------------------------------#
# Extra clean
#------------------------------------------------------------------------------#

clean-local:
	@(echo "rm -f *.mod"; rm -f ${top_builddir}/lib/*.mod)
	@(cd test; ${MAKE} clean)

#------------------------------------------------------------------------------#
# Extra distribution
#------------------------------------------------------------------------------#

dist-hook:
	@(mkdir -p ${distdir}/machine/templates && \
		cp ${top_srcdir}/machine/README ${distdir}/machine && \
		cp ${top_srcdir}/machine/templates/* ${distdir}/machine/templates)
	@(mkdir ${distdir}/examples/kernels && \
		cp ${top_srcdir}/examples/kernels/*.cl ${distdir}/examples/kernels)
	@(mkdir ${distdir}/examples/direct/kernels && \
		cp ${top_srcdir}/examples/direct/kernels/*.cl \
		${distdir}/examples/direct/kernels)
	@(mkdir ${distdir}/examples/direct/data && \
		cp ${top_srcdir}/examples/direct/data/* \
		${distdir}/examples/direct/data)
	@(mkdir ${distdir}/doc && \
		cp ${top_srcdir}/doc/Makefile.* ${distdir}/doc && \
		cp ${top_srcdir}/doc/doxygen.conf.in ${distdir}/doc)

#------------------------------------------------------------------------------#
# Dummy rule
#------------------------------------------------------------------------------#

FORCE:
