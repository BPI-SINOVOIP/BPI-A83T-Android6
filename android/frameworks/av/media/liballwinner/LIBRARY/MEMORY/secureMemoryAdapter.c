/*
 * secureos_adapter.c
 *
 *  Created on: 2014-8-18
 *      Author: wei
 */

#include "config.h"
#if (CONFIG_OS == OPTION_OS_ANDROID)
//#define LOG_NDEBUG 0
#define LOG_TAG "secureMemoryAdapter"
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "secureMemoryAdapter.h"


#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION

extern int  ion_alloc_open();
extern int  ion_alloc_close();
extern unsigned long  ion_alloc_alloc(int size);
extern unsigned long  ion_alloc_alloc_drm(int size);
extern void ion_alloc_free(void * pbuf);
extern unsigned long  ion_alloc_vir2phy(void * pbuf);
extern unsigned long  ion_alloc_phy2vir(void * pbuf);
extern void ion_flush_cache(void* startAddr, int size);

#else

#warning "invalid configuration of memory driver."

#endif

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif

#define SE_ASSERT(expr) \
	do {\
		if(!(expr)) {\
			ALOGE("%s:%d, assert \"%s\" failed", __FILE__, __LINE__, #expr); \
			abort();\
		}\
	} while(0)

#ifdef SECUREOS_ENABLED

#if(SECURE_OS_OPTEE == 1)
	#include <tee_client_api.h>
#else
	#include <sunxi_tee_api.h>
#endif

enum {
	/* keep these values (started with MSG_ADAPTER_) in sync
	 * with definitions in vdecoder in secureos*/
	MSG_ADPATER_MIN = 0,
	MSG_ADPATER_INIT,
	MSG_ADPATER_MEM_ALLOC,
	MSG_ADPATER_MEM_FREE,
	MSG_ADPATER_MEM_COPY,
	MSG_ADPATER_MEM_SET,
	MSG_ADPATER_MEM_FLUSH_CACHE,
	MSG_ADPATER_MEM_GET_PHYSICAL_ADDRESS,
	MSG_ADPATER_MEM_GET_VIRTUAL_ADDRESS,
	MSG_ADPATER_MEM_READ,
	MSG_ADPATER_MEM_WRITE,
	MSG_ADPATER_MEM_READ_INT,
	MSG_ADPATER_MEM_WRITE_INT,
	MSG_ADPATER_MEM_DUMP,
	MSG_ADPATER_MEM_DEBUG,
	MSG_ADPATER_EXIT,
	MSG_ADPATER_SETUP_VE,
	MSG_ADPATER_SHUTDOWN_VE,
	MSG_ADPATER_MAX,
};

//decoder singleton to ensure there is only one decoder created.

typedef struct {
	TEEC_Context tee_context;
	TEEC_Session tee_session;
	int ref_count;
}AdapterContext;

static AdapterContext *gAdapterContext = NULL;
static pthread_mutex_t gAdapterMutex = PTHREAD_MUTEX_INITIALIZER;

static const uint8_t adapter_UUID[16] = {
	0xEA, 0xD2, 0x78, 0x4D, 0x31, 0xA6,	0xFB, 0x70,
	0xAA, 0xA7,	0x87, 0xC2, 0xB5, 0x77, 0x30, 0x52
};

void *SecureMemAdapterAlloc(size_t size)
{
	ALOGV("SecureMemAdapterAlloc, size %d", size);
	if(size <= 0) {
		return NULL;
	}
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_alloc_drm((int)size);
#else
    return NULL;
#endif
}

int SecureMemAdapterFree(void *ptr)
{
	ALOGV("SecureMemAdapterFree, ptr %p", ptr);
	if(ptr == NULL) {
		return -1;
	}

#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    ion_alloc_free(ptr);
#else
#endif
    return 0;
}

int SecureMemAdapterCopy(void *dest, void *src, size_t n)
{
	ALOGV("SecureMemAdapterCopy, dest %p, src %p", dest, src);

	if(dest == NULL || src == NULL) {
		return -1;
	}

	SE_ASSERT(gAdapterContext != NULL);

    TEEC_Operation operation;

    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
    		TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
    operation.started = 1;

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    operation.params[0].value.a = (uintptr_t)dest;
	operation.params[1].value.a = (uintptr_t)src;
	#else
    operation.params[0].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(dest);
    operation.params[1].value.a = (uintptr_t)(SecureMemAdapterGetPhysicAddressCpu)(src);
	#endif

    operation.params[2].value.a = (uint32_t)n;
    if((operation.params[0].value.a == 0) || (operation.params[1].value.a == 0)) {
    	ALOGE("copy with invalid address, 0x%x vs 0x%x", operation.params[0].value.a,
    			operation.params[1].value.a);
    	return -1;
    }
    TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
    		MSG_ADPATER_MEM_COPY, &operation, NULL);
    return err;
}

int SecureMemAdapterSet(void *s, int c, size_t n)
{
	ALOGV("SecureMemAdapterSet, s %p", s);
	if(s == NULL || n <= 0) {
		return -1;
	}
	SE_ASSERT(gAdapterContext != NULL);

    TEEC_Operation operation;

    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
    		TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
    operation.started = 1;

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    operation.params[0].value.a = (uintptr_t)(s);
	#else	
	operation.params[0].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(s);
	#endif

    operation.params[1].value.a = (uint32_t)c;
    operation.params[2].value.a = (uint32_t)n;
    if(operation.params[0].value.a == 0) {
    	ALOGE("set with invalid address");
    	return -1;
    }
    TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
    		MSG_ADPATER_MEM_SET, &operation, NULL);
    return err;
}

int SecureMemAdapterFlushCache(void *ptr, size_t size)
{
	ALOGV("SecureMemAdapterFlushCache, %p ", ptr);
	if(ptr == NULL || size <= 0) {
		return -1;
	}
	SE_ASSERT(gAdapterContext != NULL);

    TEEC_Operation operation;

    memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
    		TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
    operation.started = 1;

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    operation.params[0].value.a = (uintptr_t)(ptr);
	#else
	operation.params[0].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(ptr);
	#endif
	
    operation.params[1].value.a = (uint32_t)size;
    if(operation.params[0].value.a == 0) {
    	ALOGE("flush with invalid address");
    	return -1;
    }
    TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
    		MSG_ADPATER_MEM_FLUSH_CACHE, &operation, NULL);
    return err;
}

int SecureMemAdapterRead(void *src, void *dest, size_t n)
{
	ALOGV("SecureMemAdapterRead");
	if(src == NULL || dest == 0 || n <= 0) {
		return 0;
	}
	SE_ASSERT(gAdapterContext != NULL);

    TEEC_Operation operation;

	memset(&operation, 0, sizeof(TEEC_Operation));
    operation.started = 1;

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    operation.params[0].value.a = (uintptr_t)src;
	#else
	operation.params[0].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(src);
	#endif

    operation.params[2].value.a = (uint32_t)n;
    if(operation.params[0].value.a == 0) {
    	ALOGE("read with invalid address");
    	return -1;
    }
    TEEC_Result err;
	if (n > 4) {
		TEEC_SharedMemory share_mem;
		share_mem.size = n;
		share_mem.flags = TEEC_MEM_OUTPUT ;
		if(TEEC_AllocateSharedMemory(&gAdapterContext->tee_context, &share_mem) != TEEC_SUCCESS) {
			ALOGE("allocate share memory fail");
			return 0;
		}
	    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
	    		TEEC_MEMREF_WHOLE, TEEC_VALUE_INPUT, TEEC_NONE);

	    operation.params[1].memref.parent = &share_mem;
		operation.params[1].memref.offset = 0;
		operation.params[1].memref.size   = 0;

	    err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
	    		MSG_ADPATER_MEM_READ, &operation, NULL);

    	memcpy(dest, share_mem.buffer, n);
    	TEEC_ReleaseSharedMemory(&share_mem);
	} else {
	    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
	    		TEEC_VALUE_OUTPUT, TEEC_VALUE_INPUT, TEEC_NONE);
	    err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
	    		MSG_ADPATER_MEM_READ_INT, &operation, NULL);

    	memcpy(dest, (unsigned char*)(&operation.params[1].value.a), n);
	}

    return err;
}

int SecureMemAdapterWrite(void *src, void *dest, size_t n)
{
	ALOGV("SecureMemAdapterWrite");
	if(src == NULL || dest == 0 || n <= 0) {
		return 0;
	}
	SE_ASSERT(gAdapterContext != NULL);

	TEEC_Operation operation;

	memset(&operation, 0, sizeof(TEEC_Operation));

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
	operation.params[1].value.a = (uintptr_t)dest;
	#else
	operation.params[1].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(dest);
	#endif

	operation.params[2].value.a = (uint32_t)n;

    if(operation.params[1].value.a == 0) {
    	ALOGE("write with invalid address");
    	return -1;
    }
	TEEC_Result err;
	if (n > 4) {
		TEEC_SharedMemory share_mem;
		share_mem.size = n;
		share_mem.flags = TEEC_MEM_INPUT;
		if(TEEC_AllocateSharedMemory(&gAdapterContext->tee_context, &share_mem) != TEEC_SUCCESS) {
			ALOGE("allocate share memory fail");
			return 0;
		}
		memcpy(share_mem.buffer, src, n);

		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
				TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);
		operation.started = 1;

		operation.params[0].memref.parent = &share_mem;
		operation.params[0].memref.offset = 0;
		operation.params[0].memref.size   = 0;

		err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
				MSG_ADPATER_MEM_WRITE, &operation, NULL);
		TEEC_ReleaseSharedMemory(&share_mem);
	} else {
	    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
	    		TEEC_VALUE_INPUT, TEEC_VALUE_INPUT, TEEC_NONE);

		operation.params[0].value.a = *((uint32_t*)dest);
	    err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
	    		MSG_ADPATER_MEM_WRITE_INT, &operation, NULL);
	}

    return err;
}

int SecureMemAdapterDump(void *ptr, size_t size)
{
	ALOGV("SecureMemAdapterDump");
    TEEC_Operation operation;

    SE_ASSERT(gAdapterContext != NULL);

	memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
    		TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE);
    operation.started = 1;

	#if(ADJUST_ADDRESS_FOR_SECURE_OS_OPTEE)
    operation.params[0].value.a = (uintptr_t)ptr;
	#else
	operation.params[0].value.a = (uintptr_t)SecureMemAdapterGetPhysicAddressCpu(ptr);
	#endif
	
    operation.params[0].value.b = (uint32_t)size;

    if(operation.params[0].value.a == 0) {
    	ALOGE("dump with invalid address");
     	return -1;
     }
    TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
    		MSG_ADPATER_MEM_DUMP, &operation, NULL);
    return err;
}

int SecureMemAdapterDebug(int s)
{
	ALOGV("SecureMemAdapterDebug");
    TEEC_Operation operation;

    SE_ASSERT(gAdapterContext != NULL);

	memset(&operation, 0, sizeof(TEEC_Operation));
    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
    		TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.started = 1;
    operation.params[0].value.a = (uint32_t)s;
    ALOGV("operation.params[0].value.a %d", operation.params[0].value.a);
    TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
    		MSG_ADPATER_MEM_DEBUG, &operation, NULL);
    return 0;
}

void* SecureMemAdapterGetPhysicAddress(void *virt)
{
	ALOGV("SecureMemAdapterGetPhysicAddress");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)(ion_alloc_vir2phy(virt) - PHY_OFFSET);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetVirtualAddress(void *phy)
{
	ALOGV("SecureMemAdapterGetVirtualAddress");
    uintptr_t nPhysicAddressForCpu = (uintptr_t)phy + PHY_OFFSET;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetPhysicAddressCpu(void *virt)
{
	ALOGV("SecureMemAdapterGetPhysicAddress");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_vir2phy(virt);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetVirtualAddressCpu(void *phy)
{
	ALOGV("SecureMemAdapterGetVirtualAddress");
    uintptr_t nPhysicAddressForCpu = (uintptr_t)phy;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

int SecureMemAdapterOpen()
{
	ALOGV("SecureMemAdapterOpen, pid %d", getpid());
	pthread_mutex_lock(&gAdapterMutex);
	TEEC_Result err = TEEC_SUCCESS;
    if(gAdapterContext == NULL) {
    	ion_alloc_open();

		gAdapterContext = (AdapterContext *)malloc(sizeof(AdapterContext));
		SE_ASSERT(gAdapterContext != NULL);
		memset(gAdapterContext, 0, sizeof(AdapterContext));
		err = TEEC_InitializeContext(NULL, &gAdapterContext->tee_context);
		if(err == TEEC_SUCCESS) {
			err = TEEC_OpenSession(&gAdapterContext->tee_context, &gAdapterContext->tee_session,
					(const TEEC_UUID *)&adapter_UUID[0], TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);

			if(err == TEEC_SUCCESS) {
				//session opened successfully, init adapter.
				TEEC_Operation operation;

					memset(&operation, 0, sizeof(TEEC_Operation));
					operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
							TEEC_NONE, TEEC_NONE);

					operation.started = 1;
					err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
							MSG_ADPATER_INIT, &operation, NULL);
					ALOGI("secure memory adapter has been created.");
			}
		}
    }
    if(err == 0)
    	gAdapterContext->ref_count ++;
    else {
		ALOGE("initialize context failed, %d", err);
		free(gAdapterContext);
		gAdapterContext = NULL;
    }
	pthread_mutex_unlock(&gAdapterMutex);
	return err;
}


int SecureMemAdapterClose()
{
	ALOGV("SecureMemAdapterClose, pid %d", getpid());
	pthread_mutex_lock(&gAdapterMutex);
    if(!gAdapterContext) {
    	ALOGE("secure adapter has not been initialized");
    	pthread_mutex_unlock(&gAdapterMutex);
    	return -1;
    }
    TEEC_Result err = 0;
    if(--gAdapterContext->ref_count == 0) {
		TEEC_Operation operation;

		memset(&operation, 0, sizeof(TEEC_Operation));
		operation.started = 1;
		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
		err  = TEEC_InvokeCommand(&gAdapterContext->tee_session,
				MSG_ADPATER_EXIT, &operation, NULL);
		if (err != TEEC_SUCCESS) {
			ALOGE("call invoke command error");
		}
		TEEC_CloseSession(&gAdapterContext->tee_session);
		TEEC_FinalizeContext(&gAdapterContext->tee_context);
		gAdapterContext = NULL;
		ion_alloc_close();
		ALOGI("secure memory adapter has been destroyed.");
    }
	pthread_mutex_unlock(&gAdapterMutex);
	return err;
}

int SecureAdapterSetupHW()
{
	ALOGV("SecureAdapterSetupHW");
	pthread_mutex_lock(&gAdapterMutex);
	if(gAdapterContext == NULL) {
		pthread_mutex_unlock(&gAdapterMutex);
		return -1;
	}
	TEEC_Operation operation;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
			TEEC_NONE, TEEC_NONE);

	operation.started = 1;
	TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
			MSG_ADPATER_SETUP_VE, &operation, NULL);
	pthread_mutex_unlock(&gAdapterMutex);
	return err;
}

int SecureAdapterShutdownHW()
{
	ALOGV("SecureAdapterShutdownHW");
	pthread_mutex_lock(&gAdapterMutex);
	//do not abort here, so codec can shutdown drm protection
	//without caring whether secure os is initialized or not.
	if(gAdapterContext == NULL) {
		pthread_mutex_unlock(&gAdapterMutex);
		return -1;
	}
	TEEC_Operation operation;

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
			TEEC_NONE, TEEC_NONE);

	operation.started = 1;
	TEEC_Result err = TEEC_InvokeCommand(&gAdapterContext->tee_session,
			MSG_ADPATER_SHUTDOWN_VE, &operation, NULL);
	pthread_mutex_unlock(&gAdapterMutex);
	return err;
}

#else //SECUREOS_ENABLED
#ifdef ATTRIBUTE_UNUSED
#undef ATTRIBUTE_UNUSED
#endif
#define ATTRIBUTE_UNUSED __attribute__ ((__unused__))

void *SecureMemAdapterAlloc(size_t size)
{
	ALOGV("SecureMemAdapterAlloc");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_alloc(size);
#else
    return NULL;
#endif
}

int SecureMemAdapterFree(void *ptr)
{
	ALOGV("SecureMemAdapterFree");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    ion_alloc_free(ptr);
#else
#endif
	return 0;
}

int SecureMemAdapterCopy(void *dest, void *src, size_t n)
{
	ALOGV("SecureMemAdapterCopy");
	memcpy(dest, src, n);
	return 0;
}
int SecureMemAdapterFlushCache(void *ptr, size_t size)
{
	ALOGV("SecureMemAdapterFlushCache");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    ion_flush_cache(ptr, size);
#else
#endif
    return 0;
}

int SecureMemAdapterRead(void *src, void *dest, size_t n)
{
	ALOGV("SecureMemAdapterRead");
	memcpy(dest, src, n);
	return n;
}

int SecureMemAdapterWrite(void *src, void *dest, size_t n)
{
	ALOGV("SecureMemAdapterWrite");
	memcpy(dest, src, n);
	return n;
}

int SecureMemAdapterSet(void *s, int c, size_t n)
{
	ALOGV("SecureMemAdapterSet");
	memset(s, c, n);
	return 0;
}

void * SecureMemAdapterGetPhysicAddress(void *virt)
{
	ALOGV("SecureMemAdapterGetPhysicAddress");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)(ion_alloc_vir2phy(virt) - PHY_OFFSET);
#else
    return NULL;
#endif
}

void * SecureMemAdapterGetVirtualAddress(void *phy)
{
	ALOGV("SecureMemAdapterGetVirtualAddress");
    long nPhysicAddressForCpu = (long)phy + PHY_OFFSET;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetPhysicAddressCpu(void *virt)
{
	ALOGV("SecureMemAdapterGetPhysicAddress");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_vir2phy(virt);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetVirtualAddressCpu(void *phy)
{
	ALOGV("SecureMemAdapterGetVirtualAddress");
    long nPhysicAddressForCpu = (long)phy;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

int SecureMemAdapterDump(void *ptr ATTRIBUTE_UNUSED, size_t size ATTRIBUTE_UNUSED)
{
	ALOGV("SecureMemAdapterDump");
	return 0;
}

int SecureMemAdapterDebug(int s ATTRIBUTE_UNUSED)
{
    return 0;
}

int SecureMemAdapterOpen()
{
	ALOGV("SecureMemAdapterOpen");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
	ion_alloc_open();
#else
    return -1;
#endif
	return 0;
}

int SecureMemAdapterClose()
{
	ALOGV("SecureMemAdapterClose");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
	ion_alloc_close();
#else
#endif
	return 0;
}

int SecureAdapterSetupHW()
{
	return 0;
}

int SecureAdapterShutdownHW()
{
	return 0;
}
#endif//SECUREOS_ENABLED

#else//CONFIG_OS == OPTION_OS_ANDROID

#include "log.h"

#if CONFIG_CHIP == OPTION_CHIP_1639
#define PHY_OFFSET 0x20000000
#else
#define PHY_OFFSET 0x40000000
#endif

#ifdef ATTRIBUTE_UNUSED
#undef ATTRIBUTE_UNUSED
#endif
#define ATTRIBUTE_UNUSED __attribute__ ((__unused__))

void *SecureMemAdapterAlloc(size_t size)
{
	logv("SecureMemAdapterAlloc");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_alloc(size);
#else
    return NULL;
#endif
}

int SecureMemAdapterFree(void *ptr)
{
	logv("SecureMemAdapterFree");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    ion_alloc_free(ptr);
#else
#endif
	return 0;
}

int SecureMemAdapterCopy(void *dest, void *src, size_t n)
{
	logv("SecureMemAdapterCopy");
	memcpy(dest, src, n);
	return 0;
}
int SecureMemAdapterFlushCache(void *ptr, size_t size)
{
	logv("SecureMemAdapterFlushCache");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    ion_flush_cache(ptr, size);
#else
#endif
    return 0;
}

int SecureMemAdapterRead(void *src, void *dest, size_t n)
{
	logv("SecureMemAdapterRead");
	memcpy(dest, src, n);
	return n;
}

int SecureMemAdapterWrite(void *src, void *dest, size_t n)
{
	logv("SecureMemAdapterWrite");
	memcpy(dest, src, n);
	return n;
}

int SecureMemAdapterSet(void *s, int c, size_t n)
{
	logv("SecureMemAdapterSet");
	memset(s, c, n);
	return 0;
}

void * SecureMemAdapterGetPhysicAddress(void *virt)
{
	logv("SecureMemAdapterGetPhysicAddress");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)(ion_alloc_vir2phy(virt) - PHY_OFFSET);
#else
    return NULL;
#endif
}

void * SecureMemAdapterGetVirtualAddress(void *phy)
{
	logv("SecureMemAdapterGetVirtualAddress");
    long nPhysicAddressForCpu = (long)phy + PHY_OFFSET;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetPhysicAddressCpu(void *virt)
{
	logv("SecureMemAdapterGetPhysicAddressCpu");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_vir2phy(virt);
#else
    return NULL;
#endif
}

void* SecureMemAdapterGetVirtualAddressCpu(void *phy)
{
    logv("SecureMemAdapterGetVirtualAddressCpu");
    long nPhysicAddressForCpu = (long)phy;
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
    return (void*)ion_alloc_phy2vir((void*)nPhysicAddressForCpu);
#else
    return NULL;
#endif
}

int SecureMemAdapterDump(void *ptr ATTRIBUTE_UNUSED, size_t size ATTRIBUTE_UNUSED)
{
	logv("SecureMemAdapterDump");
	return 0;
}

int SecureMemAdapterDebug(int s ATTRIBUTE_UNUSED)
{
    return 0;
}

int SecureMemAdapterOpen()
{
	logv("SecureMemAdapterOpen");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
	ion_alloc_open();
#else
    return -1;
#endif
	return 0;
}

int SecureMemAdapterClose()
{
	logv("SecureMemAdapterClose");
#if CONFIG_MEMORY_DRIVER == OPTION_MEMORY_DRIVER_ION
	ion_alloc_close();
#else
#endif
	return 0;
}

int SecureAdapterSetupHW()
{
	return 0;
}

int SecureAdapterShutdownHW()
{
	return 0;
}

#endif//CONFIG_OS == OPTION_OS_ANDROID
