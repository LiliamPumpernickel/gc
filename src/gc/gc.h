#pragma once

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
	
	typedef int (*finalizer_func)(void* p);
	
	// inits GC, must be first call in main(), before any stack variable allocations
	void gc_init(void);
	
	void* gc_alloc(unsigned sz);
	void gc_collect(void);
	void gc_shutdown(void);
	
	unsigned gc_get_total_heap(void);
	unsigned gc_get_free_heap(void);
	
	double gc_debug_get_duration(clock_t);
	unsigned gc_debug_get_allocation_size(void* p);
	
	// Not implemented, left for homework
	void gc_register_finalizer(void* p, finalizer_func func);
	void gc_pin_pointer(void* p);
	void gc_unpin_pointer(void* p);

#ifdef __cplusplus
}
#endif // __cplusplus
