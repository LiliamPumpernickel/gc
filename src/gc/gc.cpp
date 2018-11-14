#include <stdlib.h>
#include <stdio.h>

#include "gc.h"
#include "gc-private.h"

GC_info s_gc_info;

extern "C"
{
	void gc_init()
	{
		// mark where main thread stack starts
		void* p = NULL;
		s_gc_info.stack_start = &p;
		s_gc_info.total_heap = 1 * 1024 * 1024; // 1MB
		s_gc_info.free_heap = s_gc_info.total_heap;
	}
	
	void* gc_alloc(unsigned sz)
	{
		// if there is not enough free space, try to collect
		if (s_gc_info.free_heap < sz)
			gc_collect();
		
		// if still there is not enough space, just give up
		if (s_gc_info.free_heap < sz)
			return NULL;
		
		void *p = calloc(1, sz);
		
		if (p != NULL)
		{
			s_gc_info.allocated[p] = sz;
			s_gc_info.free_heap -= sz;
		}
		
		return p;
	}
	
	void gc_collect()
	{
		// TODO: port to windows asm
#if __APPLE__
		// Trick compiler to push registers to stack
		asm volatile ("nop"
					  : /* no outputs */
					  : /* no inputs */
					  : "rax", "rcx", "rdx", "rbx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
#endif
		
		void *stack_end = NULL;
		
		clock_t st = clock();
		
		// Scan stack
		// Note: Intel & AMD architecture stack grows down, so start > end
		std::set<void*> roots, reachable;
		gc_scan_range_internal(&stack_end, s_gc_info.stack_start, roots, reachable);
		
		// Mark
		unsigned allocated_size = gc_mark_internal(roots, reachable);
		
		// Sweep
		gc_sweep_internal(reachable);
		
		// Update free space
		s_gc_info.free_heap = s_gc_info.total_heap - allocated_size;
		
		printf("GC collection took %gs of CPU time\n", gc_debug_get_duration(st));
	}
	
	void gc_shutdown()
	{
		clock_t st = clock();
		
		// Time to release all the remaining memory
		gc_batch_free_internal(s_gc_info.allocated);
		
		s_gc_info.free_heap = 0;
		s_gc_info.total_heap = 0;
		
		printf("GC shutdown took %gs of CPU time\n", gc_debug_get_duration(st));
	}
	
	unsigned gc_get_total_heap()
	{
		return s_gc_info.total_heap;
	}
	
	unsigned gc_get_free_heap()
	{
		return s_gc_info.free_heap;
	}
	
	double gc_debug_get_duration(clock_t st)
	{
		clock_t end = clock();
		return (double)(end - st) / CLOCKS_PER_SEC;
	}
	
	unsigned gc_debug_get_allocation_size(void* p)
	{
		if (!gc_is_allocated_pointer(p, &s_gc_info))
			return 0;
		
		return s_gc_info.allocated[p];
	}
	
	// Not implemented, left for homework
	void gc_register_finalizer(void* p, finalizer_func func)
	{
		if (p != NULL)
		{
			s_gc_info.finalizered[p] = func;
		}
		//TODO: implement
	}
	
	// Not implemented, left for homework
	void gc_pin_pointer(void* p)
	{
		if (p != NULL)
		{	
			
			s_gc_info.pinned[p] = true;
			//printf("%s \n", s_gc_info.pinned[p] ? "true" : "false");
		}
		//TODO: implement
	}
	
	// Not implemented, left for homework
	void gc_unpin_pointer(void* p)
	{
		if (p != NULL)
		{
			s_gc_info.pinned[p] = (bool)false;
		}
		//TODO: implement
	}
}

unsigned gc_mark_internal(std::set<void*>& roots, std::set<void*>& reachable)
{
	unsigned allocated_size = 0;
	
	while (!roots.empty())
	{
		std::set<void*>::iterator it = roots.begin();
		reachable.insert(*it);
		
		unsigned block_size = gc_debug_get_allocation_size(*it);
		allocated_size += block_size;
		gc_scan_range_internal(*it, (char*)*it + block_size, roots, reachable);
		
		roots.erase(it);
	}
	
	return allocated_size;
}

void gc_scan_range_internal(void* st, void* end, std::set<void*>& candidates, std::set<void*>& reachable)
{
	for (void** it = (void**)st; it < (void**)end; it++)
	{
		if (gc_is_allocated_pointer(*it, &s_gc_info) && reachable.find(*it) == reachable.end())
		{
			candidates.insert(*it);
		}
	}
}

void gc_batch_free_internal(std::set<void*>& allocations_to_cleanup, allocations_t& all_allocations)
{
	for (std::set<void*>::iterator it = allocations_to_cleanup.begin(); it != allocations_to_cleanup.end(); it++)
	{

		finalizer_func f = s_gc_info.finalizered[*it];
		if (f != NULL)
			f(*it);

		// remove allocation from all allocations
		all_allocations.erase(*it);
		
		// *it is the pointer stored in set
		free(*it);
	}
	
	allocations_to_cleanup.clear();
}

void gc_batch_free_internal(allocations_t& allocations)
{
	for (allocations_t::iterator it = allocations.begin(); it != allocations.end(); it++)
	{	
		//printf("finalizing obj : %p", s_gc_info.finalizered.find(it->first));

		finalizer_func f = s_gc_info.finalizered[it->first];
		if (f != NULL)
			f(it->first);
		// it->first is the pointer stored as map key
		free(it->first);
	}
	
	allocations.clear();
}

void gc_sweep_internal(std::set<void*>& reachable)
{
	std::set<void*> cleanup_list;
	
	allocations_t::iterator alloc_it = s_gc_info.allocated.begin();
	allocations_t::iterator alloc_end = s_gc_info.allocated.end();
	
	std::set<void*>::iterator reach_it = reachable.begin();
	std::set<void*>::iterator reach_end = reachable.end();
	
	// Two set substraction
	while (alloc_it != alloc_end)
	{
		// Second set is already empty, just copy remainder of the first set
		if (reach_it == reach_end)
		{
			while (alloc_it != alloc_end)
			{

					if (!s_gc_info.pinned[alloc_it->first]) {
						cleanup_list.insert(alloc_it->first);
						
						
					}

				alloc_it++;
				
			}
			break;
		}
		
		// Current element of the first set is not part of second set, include into result
		if (alloc_it->first < *reach_it)
		{
	
				if (!s_gc_info.pinned[alloc_it->first]) {
					cleanup_list.insert(alloc_it->first);
				}

			alloc_it++;
		}
		else
		{
			// Element skipped, because it appears in both sets
			if (alloc_it->first == *reach_it)
			{
				alloc_it++;
			}
			reach_it++;
		}
	}
	
	gc_batch_free_internal(cleanup_list, s_gc_info.allocated);
}
