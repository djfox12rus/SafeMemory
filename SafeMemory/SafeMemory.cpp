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

	double a = 45.14;
	_smart_ptr<double> d_arr = _smart_ptr<double>(a, 10);
	double *unsafe = d_arr.get_ptr_unsafe();
	std::cout << *unsafe << ",   " << unsafe << "\n";
	d_arr.~_smart_ptr();
	/*
	ldouble_sptr d_arr = ldouble_sptr(double(), 50);
	std::cout << "d_arr: ";
	for (int i = 0; i < 50; i++) {
		d_arr[i] = pow((double)i / 50, i);
		std::cout << d_arr[i] << ", ";
	}
	std::cout << "\n";
	ldouble_sptr d_arr1 = ldouble_sptr(double(), 50);
	std::cout << "d_arr1: ";
	for (int i = 0; i < 50; i++) {
		d_arr1[i] = i*d_arr[i];
		std::cout << d_arr1[i] << ", ";
	}
	std::cout << "\n";
	ldouble_sptr d_arr2;
	d_arr2 = d_arr1;
	std::cout << "d_arr2: ";
	for (int i = 0; i < 50; i++) {
		std::cout << d_arr2[i] << ", ";
	}
	std::cout << "\n";


	d_arr = d_arr2;
	std::cout << "d_arr: ";
	for (int i = 0; i < 50; i++) {
		std::cout << d_arr[i] << ", ";
	}
	std::cout << "\n";*/


	MemoryControl::mem_wiz.~_memory_interface();
    return 0;
}

