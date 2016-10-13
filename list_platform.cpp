#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
int main() 
    {
    int i, j, k;
    char* info;
    size_t infoSize;
    cl_uint platformCount; // get the # of platforms available
    cl_platform_id *platforms;
    const char* attributeNames[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    // A platform only gets these 5 types of attributes, and we have to follow the naming rule
    const cl_platform_info attributeTypes[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };
    const int attributeCount = sizeof(attributeNames) / sizeof(char*);
 
    // get platform count
    clGetPlatformIDs(5, NULL, &platformCount);
    // get all platforms
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);
 
    // for each platform print all attributes
    for (i = 0; i < platformCount; i++) 
        {
        printf("\n %d. Platform \n", i+1);
 
        for (j = 0; j < attributeCount; j++) 
            {
            // attributeCount == 5 here
            // get platform attribute value size
            clGetPlatformInfo(platforms[i], attributeTypes[j], 0, NULL, &infoSize);
            info = (char*) malloc(infoSize);
 
            // get platform attribute value
            clGetPlatformInfo(platforms[i], attributeTypes[j], infoSize, info, NULL);
 
            printf("  %d.%d %-11s: %s\n", i+1, j+1, attributeNames[j], info);
            free(info);
            }
        printf("\n");
        }
    ////
    char* value;
    size_t valueSize;
    cl_uint deviceCount;
    cl_device_id* devices;
    
    const char* device_attributeNames[5] = { "Device", "Vendor", "Hardware version", "Software version", "OpenCL C version"};
    const cl_device_info device_attributeTypes[5] = { CL_DEVICE_NAME, CL_DEVICE_VENDOR, CL_DEVICE_VERSION, CL_DRIVER_VERSION, CL_DEVICE_OPENCL_C_VERSION};
    const int device_attributeCount = sizeof(device_attributeNames) / sizeof(char*);

    cl_uint clAttrUnits;
    const char* device_attributeNames_2[3] = { "Global Memory Cacheline Size", "Max Clock Freq", "Max Compute Units"};
    const cl_device_info device_attributeTypes_2[3] = { 
        CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, 
        CL_DEVICE_MAX_CLOCK_FREQUENCY, 
        CL_DEVICE_MAX_COMPUTE_UNITS};
    const int device_attributeCount_2 = sizeof(device_attributeNames_2) / sizeof(char*);

    const char* device_attributeNames_3[4] = 
        { 
        "Global device memory size (bytes)",
        "Global memory cache size (bytes)",
        "Global memory cache line size (bytes)",
        "Local memory arena size (bytes)"
        };
    const cl_device_info device_attributeTypes_3[4] = 
        { 
        CL_DEVICE_GLOBAL_MEM_SIZE,
        CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, 
        CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, 
        CL_DEVICE_LOCAL_MEM_SIZE
        };
    const int device_attributeCount_3 = 4;
 
    for (i = 0; i < platformCount; i++) 
        {
        // for each platform, get all devices on the platform
        // last 3 parameters are like the parameters in the clGetPlatformIDs
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
 
        // for each device print critical attributes
        for (j = 0; j < deviceCount; j++) 
            {
            /*
            // print device name
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
            printf("%d. Device: %sn", j+1, value);
            free(value);
 
            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            printf(" %d.%d Hardware version: %sn", j+1, 1, value);
            free(value);
 
            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            printf(" %d.%d Software version: %sn", j+1, 2, value);
            free(value);
 
            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            printf(" %d.%d OpenCL C version: %sn", j+1, 3, value);
            free(value);
 
            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            printf(" %d.%d Parallel compute units: %dn", j+1, 4, maxComputeUnits);
            */
            for (k = 0; k < device_attributeCount; k++)
                {
                clGetDeviceInfo(devices[j], device_attributeTypes[k], 0, NULL, &valueSize);
                value = (char*) malloc(valueSize);
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
                printf("%d.%d %s: %s\n", j+1, k, device_attributeNames[k], value);
                free(value);
                }
            for (k = 0; k < device_attributeCount_2; k++)
                {
                clGetDeviceInfo(devices[j], device_attributeTypes_2[k], sizeof(clAttrUnits), &clAttrUnits, NULL);
                printf("%d.%d %s: %d\n", j+1, k+device_attributeCount, device_attributeNames_2[k], clAttrUnits);
                }
            for (k = 0; k < device_attributeCount_3; k++)
                {
                clGetDeviceInfo(devices[j], device_attributeTypes_3[k], sizeof(clAttrUnits), &deviceMemInfo, NULL);
                printf("%d.%d %s: %d\n", j+1, k+device_attributeCount+device_attributeCount_2, device_attributeNames_3[k], deviceMemInfo);
                }
            }
        free(devices);
        }

    ////
    free(platforms);
    return 0;
    }