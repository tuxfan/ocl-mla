#! /usr/bin/env python
################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
#
# $Revision: 43 $
# $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
# $Author: bergen $
################################################################################

'''
Tools for creating build files for test directories
'''

import os, string, sys, shutil
import build_utils

def write_testsuite_at(testdir,dir,bld):
	# write testsuite.at
	at = open('%s/%s/testsuite.at' % (testdir, dir), 'w')
	at.write('AT_INIT([%s])\n\n' % dir)
	at.write('AT_BANNER([%s])\n\n' % dir)
	at.write('m4_include([local.at])\n\n')
	if bld._have_test:
		for test in bld._tests:
			# write the macro
			at.write('CONFIG_AT_RUN_PROGRAM([')
			# write the test
			at.write('%s]' % test.exe)
			at.write(')\n')
	at.close()

def write_atlocal_in(testdir,dir):
	atl = open('%s/%s/atlocal.in' % (testdir,dir), 'w')
	atl.write(70 * '#' + '\n')
	atl.write('## atlocal.in automatically generated by %s\n' % sys.argv[0])
	atl.write(70 * '#' + '\n')
	atl.write("ccs_srcdir='@top_srcdir@'\n")
	atl.write("ccs_host_builddir='@top_builddir@'\n")
	atl.close()

build_file = 'Build.py'
topdir = os.getcwd()
testdir = '%s/test' % topdir

# list subdirectories in test
subdirs = os.listdir(testdir)

dirs_with_tests = []

# loop through the subdirs and look for a Build.py
for dir in subdirs:
	f = '%s/%s/%s' % (testdir,dir,build_file)
        if os.path.exists(f):
		# record that this dir has a test so we can write a Makefile.am
		# in the test dir
		dirs_with_tests.append(dir)

		# create the header of the Makefile.am
		mf = '%s/%s/Makefile.am' % (testdir,dir)
		m = open(mf,'w')
		m.write(70 * '#' + '\n')
		m.write('# Makefile.am for %s/%s\n' % (testdir,dir))
		m.write('# Automatically generated by %s\n' % sys.argv[0])
		m.write(70 * '#' + '\n\n')
		# ... copy these files from the config to build directory
		for lat in ['local.at']:
		    m.write('%s : %s/config/%s\n' % (lat, topdir, lat))
		    m.write('\tcp $< $@\n')

		m.write('\nAM_FCFLAGS = @FC_MODULE_PATH_FLAG@${top_builddir}/lib\n')

		m.write('\nbin_PROGRAMS =\n')

		# get the tests from the build file
		bld = build_utils.get_bld(f,build_utils.Test_Builder())

		# regular tests
		if bld._have_test:
			for test in bld._tests:
				m.write('\nbin_PROGRAMS +=')
				m.write(' \\\n\t%s' % test.exe)
				m.write('\n')
				m.write('\n%s_SOURCES =' % test.exe)
				for src in test.srcs:
					m.write(' \\\n\t%s' % src)
				m.write('\n')
				m.write('\n%s_CPPFLAGS = \\\n' % (test.exe))
				m.write('\t-I${top_srcdir}/src/include \\\n')
				m.write('\t-I${top_builddir}/local \\\n')
				m.write('\t@EXTRA_CPPFLAGS@\n')
				m.write('\n%s_LDFLAGS = @EXTRA_LDFLAGS@\n' % (test.exe))
				m.write('\n%s_LDADD = ${top_builddir}/lib/libocl.la @EXTRA_LIBS@\n' % (test.exe))

		# now do the autotest stuff
		m.write('\n\n## autotest makefile.am magic\n')
		m.write('TESTSUITE = testsuite\n')
		m.write('EXTRA_DIST = $(TESTSUITE) atlocal.in testsuite.at\n')

		m.write('check-local : atconfig atlocal $(TESTSUITE)\n')
		m.write("\t$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)\n\n")
		m.write('installcheck-local : atconfig atlocal $(TESTSUITE)\n')
		m.write("\t$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' $(TESTSUITEFLAS)\n\n")
		m.write('clean-local :\n')
		m.write("\ttest ! -f '$(TESTSUITE)' || $(SHELL) '$(TESTSUITE)' --clean\n\n")
		m.write('AUTOM4TE = `which autom4te`\n')
		m.write('AUTOTEST = $(AUTOM4TE) --language=autotest\n\n')
		m.write('$(TESTSUITE) : tsdeps\n')
		m.write("\t$(AUTOTEST) -I '%s/%s' -o \$@.tmp \$@.at\n" % (testdir,dir))
		m.write('\tmv \$@.tmp \$@\n\n')
		m.write('## tsdeps are testsuite dependencies.\n')
		m.write('tsdeps : atfiles package.m4\n')
		# Write out TESTSUITEFLAGS
		tsflags=''
                # ... if there are test files, add them to the TESTSUITEFLAGS
		if len(bld._test_input_files) > 0:
			tsflags += "INPUTFILES='%s/%s/%s" % (testdir,dir,
							     bld._test_input_files[0])
			for f in bld._test_input_files[1:]:
				tsflags += ' %s/%s/%s' % (testdir,dir,f)
			tsflags += "'"
#SAVE THIS FOR WHEN START USING INPUT DIRS
#                # ... if there are test dirs, add them to the TESTSUITEFLAGS
#                if len(bld._test_input_dirs) > 0:
#                    tsflags += " INPUTDIRS='%s/test/%s" % (libname,
#                                                           bld._test_input_dirs[0])
#                    for f in bld._test_input_dirs[1:]:
#                        tsflags += ' %s/test/%s' % (libname,f)
#                    tsflags += "'"
#                
		if len(tsflags) > 1:
			m.write('\n## at least one of the tests depends on an input file\n')
			m.write("TESTSUITEFLAGS=%s\n" % tsflags)

		# Create dependencies on "at" files.
		m.write('\natfiles : local.at \\\n')
		m.write('\t%s/%s/testsuite.at\n' % (testdir,dir))

		# create rule to write package.m4
		m.write('\n# write the package.m4\n')
		m.write("# The ':;' works around a Bash 3.2 bug when the output is not writeable.\n")
		m.write("package.m4:\n")
		m.write("\t@:;{ \\\n")
		m.write("\techo '# Signature of the current package.' ; \\\n")
		m.write("\techo 'm4_define([AT_PACKAGE_NAME], [%s])' && \\\n" % testdir)
		m.write("\techo 'm4_define([AT_PACKAGE_STRING], [%s])' ; \\\n" % testdir)
		m.write("\techo 'm4_define([AT_PACKAGE_BUGREPORT], [wohlbier@lanl.gov])'; \\\n")
		m.write("\t} > package.m4\n")
		m.close()

		# write testsuite.at
		write_testsuite_at(testdir,dir,bld)
		# Write atlocal.in
		write_atlocal_in(testdir,dir)

# write the Makefile.am for the test dir
m = open('%s/Makefile.am' % testdir, 'w')
m.write(70 * '#' + '\n')
m.write('## Makefile.am for %s\n' % (testdir))
m.write('## Automatically generated by %s\n' % sys.argv[0])
m.write(70 * '#' + '\n\n')
m.write('SUBDIRS =')
for dir in dirs_with_tests:
	m.write(' \\\n\t%s' % dir)
