/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "ocl.h"

int main(int argc, char ** argv) {

	ocl_init();

	ocl_host_initialize_timer("test");

	ocl_host_start_timer("test");
	sleep(1);
	ocl_host_stop_timer("test");
	ocl_host_report_timer("test");

	ocl_host_start_timer("test");
	sleep(2);
	ocl_host_stop_timer("test");
	ocl_host_report_timer("test");

	ocl_host_clear_timer("test");

	ocl_host_start_timer("test");
	sleep(3);
	ocl_host_stop_timer("test");
	ocl_host_report_timer("test");

	ocl_finalize();

	return 0;
} // main
