// SafeMemory.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "../lib/MemWiz.h"


using namespace MemoryControl;


int main()
{
#ifdef _TEST
	MemoryControl::test();
#endif

	
	/*void *ExtCode = VirtualAllocEx(GetCurrentProcess(), NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	uint64_t addr = (uint64_t)ExtCode;
	int* p = (int*)addr;
	*p = 5;*/
	uint64_t* adr;
	int a = 314;
	
	_smart_ptr<int> ptr1 = _smart_ptr<int>(a,10);
	for (int i = 0; i < 10; i++) {
		ptr1[i] = i*i;
		
		std::cout << ptr1[i] << ",   " << &ptr1[i] << "\n";
	}
	adr=(uint64_t*)ptr1[0];
	_smart_ptr<double> ptr2 = _smart_ptr<double>((double)a, 10);
	for (int i = 0; i < 10; i++) {
		ptr2[i] = i*i;

		std::cout << ptr2[i] << ",   " << &ptr2[i] << "\n";
	}
	ptr2.~_smart_ptr();
	
	_smart_ptr<int> ptr3 = _smart_ptr<int>(a, 20);
	for (int i = 0; i < 20; i++) {
		ptr3[i] = i*i;

		std::cout << ptr3[i] << ",   " << &ptr3[i] << "\n";
	}
	_smart_ptr<int> ptr4;
	ptr4 = ptr3;
	ptr1.~_smart_ptr();
	ptr3.~_smart_ptr();
	ptr4.~_smart_ptr();
	MemoryControl::mem_wiz.~_memory_interface();

    return 0;
}

