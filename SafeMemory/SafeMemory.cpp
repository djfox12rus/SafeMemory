// SafeMemory.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "../lib/MemWiz.h"
//extern MemoryControl::_memory_interface mem_wiz;

int main()
{
	
	MemoryControl::test();
	

	MemoryControl::mem_wiz.~_memory_interface();
    return 0;
}

