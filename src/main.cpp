#include <stdio.h>

#include "gc/gc.h"
#include "test.h"

int main(int argc, const char * argv[])
{
	printf("Proof of Concept GC...\n");
	
	gc_init();
	
	run_tests();
	
	gc_shutdown();
	
	return 0;
}
