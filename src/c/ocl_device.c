/******************************************************************************\
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved.
\******************************************************************************/

#define _ocl_source

#include <string.h>

#include "ocl_device.h"
#include "ocl_utils.h"

extern ocl_data_t ocl;

/*------------------------------------------------------------------------------
 * Known platforms with defines
 *----------------------------------------------------------------------------*/

#define OCL_KNOWN_PLATFORMS 4
const char * platform_defines[OCL_KNOWN_PLATFORMS][2] = {
	{ "Apple", "-D__OCL_APPLE__" },
	{ "AMD Accelerated Parallel Processing", "-D__OCL_AMD__" },
	{ "Intel(R) OpenCL", "-D__OCL_INTEL__" },
	{ "NVIDIA CUDA", "-D__OCL_NVIDIA__" }
};

/*------------------------------------------------------------------------------
 * This function initializes a generic CPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_cpu(ocl_device_instance_t * instance,
	const char * platform, size_t thread) {
	return ocl_init_generic_device(instance, platform, CL_DEVICE_TYPE_CPU,
		thread);
} // ocl_init_generic_cpu

/*------------------------------------------------------------------------------
 * This function initializes a generic GPU device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_gpu(ocl_device_instance_t * instance,
	const char * platform, size_t thread) {
	return ocl_init_generic_device(instance, platform, CL_DEVICE_TYPE_GPU,
		thread);
} // ocl_init_generic_gpu

/*------------------------------------------------------------------------------
 * This function initializes a generic MIC device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_mic(ocl_device_instance_t * instance,
	const char * platform, size_t thread) {
	return ocl_init_generic_device(instance, platform,
		CL_DEVICE_TYPE_ACCELERATOR, thread);
} // ocl_init_generic_gpu

/*------------------------------------------------------------------------------
 * This function initializes a generic device.
 *----------------------------------------------------------------------------*/

int32_t ocl_init_generic_device(ocl_device_instance_t * instance,
	const char * platform, cl_uint device_type, size_t thread) {
	CALLER_SELF
	cl_uint num_platforms;
	cl_platform_id platforms[OCL_MAX_PLATFORMS];
	int32_t matched = 0;
	size_t pi, i;
	cl_int err;

	// get number of platforms for this system
	CL_CHECKerr(clGetPlatformIDs, 0, NULL, &num_platforms);

	// get platform information
	CL_CHECKerr(clGetPlatformIDs, num_platforms, platforms, &num_platforms);

	// read device information for each platform
	for(pi=0; pi<num_platforms; ++pi) {
		cl_uint num_devices;
		cl_device_id devices[OCL_MAX_PLATFORM_DEVICES];
		char platform_name[256];
		char platform_version[256];
		size_t platform_info_size;

		CL_CHECKerr(clGetPlatformInfo, platforms[pi], CL_PLATFORM_NAME,
			sizeof(platform_name), platform_name, &platform_info_size);

		if(platform_info_size >= sizeof(platform_name)) {
			CL_ABORTsanity("Platform name is too long\n");
		} // if

		// check platform
		if((strcmp(platform, "default") != 0) &&
			(strcmp(platform_name, platform) != 0)) {
			continue;
		} // if

		matched = 1;

		// set platform preprocessor information
		strcpy(instance->info.platform_defines, "");
		for(i=0; i<OCL_KNOWN_PLATFORMS; ++i) {
			if(strcmp(platform, platform_defines[i][0]) == 0) {
				strcpy(instance->info.platform_defines, platform_defines[i][1]);
				break;
			} // if
		} // for

		CL_CHECKerr(clGetPlatformInfo, platforms[pi], CL_PLATFORM_VERSION,
			sizeof(platform_version), platform_version, &platform_info_size);

		if(platform_info_size >= sizeof(platform_name)) {
			CL_ABORTsanity("Platform version is too long\n");
		} // if

		err = clGetDeviceIDs(platforms[pi], device_type, 0, NULL, &num_devices);
		
		if(err != 0) {
			if(err != CL_DEVICE_NOT_FOUND) {
				CL_ABORTerr("clGetDeviceIds", err);
			}
			else {
				num_devices = 0;
			} // if
		} // if

		if(num_devices > 0) {
			CL_CHECKerr(clGetDeviceIDs, platforms[pi], device_type,
				num_devices, devices, &num_devices);

			if(thread+1 > num_devices) {
				warning("Requested thread id exceeds the number of actual "
					"hardware devices: using round robin...\n");
			} // if

			size_t device_type_id = device_type == CL_DEVICE_TYPE_CPU ? OCL_CPU :
				device_type == CL_DEVICE_TYPE_GPU ? OCL_GPU :
				device_type == CL_DEVICE_TYPE_ACCELERATOR ? OCL_ACCELERATOR :
				OCL_CUSTOM;

			size_t device_id = thread%num_devices +
				ocl.initialized_devices[device_type_id];
			device_id = device_id >= num_devices ? 0 : device_id;

			// increment how many devices of this type have been initialized
			instance->id = devices[device_id];

			ocl.initialized_devices[device_type_id]++;

			// get device information
			CL_CHECKerr(clGetDeviceInfo, devices[device_id], CL_DEVICE_NAME,
				sizeof(instance->info.name), instance->info.name, NULL);

			// save platform name
			strcpy(instance->info.platform_name, platform_name);

			// get platform version
			char * token = strtok(platform_version, " "); // Platform
			token = strtok(NULL, " "); // Version:
			sscanf(token, "%u.%u", &instance->info.version_major,
				&instance->info.version_minor);

			instance->info.type = device_type;
			
			CL_CHECKerr(clGetDeviceInfo, devices[device_id], CL_DEVICE_VENDOR_ID,
				sizeof(instance->info.vendor_id), &instance->info.vendor_id, NULL); 

			CL_CHECKerr(clGetDeviceInfo, devices[device_id],
				CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
				&instance->info.max_compute_units, NULL);
			
			CL_CHECKerr(clGetDeviceInfo,
				devices[device_id], CL_DEVICE_MAX_CLOCK_FREQUENCY,
				sizeof(cl_uint), &instance->info.max_clock_frequency, NULL);

			CL_CHECKerr(clGetDeviceInfo,
				devices[device_id], CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(size_t), &instance->info.max_work_group_size, NULL);

			CL_CHECKerr(clGetDeviceInfo,
				devices[device_id], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
				sizeof(cl_uint), &instance->info.max_work_item_dimensions, NULL);

			CL_CHECKerr(clGetDeviceInfo,
				devices[device_id], CL_DEVICE_MAX_WORK_ITEM_SIZES,
				instance->info.max_work_item_dimensions*sizeof(size_t),
				instance->info.max_work_item_sizes, NULL);

			CL_CHECKerr(clGetDeviceInfo,
				devices[device_id], CL_DEVICE_LOCAL_MEM_SIZE,
				sizeof(cl_ulong), &instance->info.local_mem_size, NULL);
		}
		else {
			CL_ABORTsanity("No devices found");
		} // if
	} // for

	if(!matched) {
		CL_ABORTsanity("Match criteria failed\n");
	} // if

	instance->context = clCreateContext(NULL, 1, &instance->id,
		NULL, NULL, &err);

	if(err != CL_SUCCESS) {
		CL_ABORTerr(clCreateContext, err);
	} // if

#if defined(ENABLE_OPENCL_PROFILING)
	cl_int queue_creation_flags = CL_QUEUE_PROFILING_ENABLE;
#else
	cl_int queue_creation_flags = 0;
#endif // ENABLE_OPENCL_PROFILING

	instance->queue = clCreateCommandQueue(instance->context, instance->id,
		queue_creation_flags, &err);

	if(err != CL_SUCCESS) {
		CL_ABORTerr(clCreateCommandQueue, err);
	} // if

	return !matched;
} // ocl_init_generic_cpu

int32_t ocl_finalize_device(ocl_device_instance_t * instance) {
	CALLER_SELF
	cl_int err = 0;

	CL_CHECKerr(clReleaseCommandQueue, instance->queue);
	CL_CHECKerr(clReleaseContext, instance->context);

	return err;
} // ocl_finalize_device
