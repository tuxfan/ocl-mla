################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

bld.test('file_kernel', 'file_kernel.f90')
bld.test_input_files('test.cl')
