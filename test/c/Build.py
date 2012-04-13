################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

bld.test('file_kernel', 'file_kernel.c')
bld.test('inline_kernel', 'inline_kernel.c')
bld.test('host_timer', 'host_timer.c')
bld.test('device_timer', 'device_timer.c')
bld.test('compare_timer', 'compare_timer.c')
bld.test('serial_reduction', 'serial_reduction.c')

bld.test_input_files('T_RDP.cl T_RS.cl T_T.cl T_U.cl')
