#pragma once
#ifndef MEMWIZ_H
#define MEMWIZ_H

#include <cstdint>
#include <cstdlib>
#include <iostream>
#define ALLOCATOR MemoryControl::mem_wiz
//#define NEW (auto o) MemoryControl::mem_wiz.get_smart(o) //TODO:попробовать задефайнить свой NEW как макрос.

namespace MemoryControl {	
	
	class _memory_interface {
	private:
		template<class T>
		friend class _smart_ptr;
		struct ref {
			uint32_t ref_count;
			void * mem_ref;
		};
		
		ref *reference_table;	//таблица адресации хранитс€ в виде массива. ¬озможно сделать еЄ в виде списка std::list, чтобы не заботитс€ о выходе за пределы адресного пол€	
		void *block;		
		void *block_end;

		ref* allocate_mem(size_t _size_of_type, size_t _size_of_arr = 1);

	public:
		_memory_interface();
		~_memory_interface();
		
	};
	extern _memory_interface mem_wiz;
	int test();	



	template<class T>
	class _smart_ptr {
	private:
		_memory_interface::ref *s_ref;
		bool check_if_can_continue(size_t);
	public:
		_smart_ptr();
		_smart_ptr(T _obj);
		_smart_ptr(T _obj, size_t _size_of_array);//так же объ€вление массива. возможно имеет смысл сделать отдельный класс указателей только под массивы.
		~_smart_ptr() {
			std::cout << "dest smart\n";
			std::cout << "memory adress: " << s_ref;
			if (s_ref) {
				if (s_ref->ref_count > 0) {
					s_ref->ref_count--;
					std::cout << ", links left: " << s_ref->ref_count << "\n";
				}
				else {//возможен какой-либо кос€к с учЄтом всех ссылок.

				}
			}
			else
				std::cout << ", links' not initialized\n";
			//s_ref = nullptr;
		}

		//Ќебезопасно! Ќельз€ удал€ть ("забывать") сам smart_pointer! Ќельз€ использовать delete!
		T* get_ptr_unsafe();

		T at(size_t);

		void alloc(T _obj, size_t _size_of_arr = 1);
		void realloc(size_t);

		T* operator->();
		T& operator* ();

		T& operator[](size_t);

		_smart_ptr<T>& operator=(_smart_ptr<T>& _left);

		void get_smart(T &_obj);

	};



}

//не придумал ничего умнее.
//signed
typedef MemoryControl::_smart_ptr<int8_t> int8__sptr;
typedef MemoryControl::_smart_ptr<int16_t> int16__sptr;
typedef MemoryControl::_smart_ptr<int32_t> int32__sptr;
typedef MemoryControl::_smart_ptr<int64_t> int64_sptr;
//unsigned
typedef MemoryControl::_smart_ptr<uint8_t> uint8__sptr;
typedef MemoryControl::_smart_ptr<uint16_t> uint16__sptr;
typedef MemoryControl::_smart_ptr<uint32_t> uint32__sptr;
typedef MemoryControl::_smart_ptr<uint64_t> uint64_sptr;
//floating
typedef MemoryControl::_smart_ptr<float> float_sptr;
typedef MemoryControl::_smart_ptr<double> double_sptr;
typedef MemoryControl::_smart_ptr<long double> ldouble_sptr;

#define MY_DOUBLE MemoryControl::_smart_ptr<long double> 

#endif //MEMWIZ_H