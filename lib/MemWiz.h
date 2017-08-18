#pragma once
#ifndef MEMWIZ_H
#define MEMWIZ_H
#define _TEST
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
#ifdef _TEST
	int test();	
#endif


	template<class T>
	class _smart_ptr {
	protected:
		_memory_interface::ref *s_ref;
		bool check_if_can_continue(size_t);
	public:
		_smart_ptr();
		_smart_ptr(T _obj);
		_smart_ptr(T _obj, size_t _size_of_array);//так же объ€вление массива. возможно имеет смысл сделать отдельный класс указателей только под массивы.
		~_smart_ptr() {
#ifdef _TEST
			std::cout << "dest smart\n";
			std::cout << "memory adress: " << s_ref->mem_ref;
#endif
			if (s_ref) {
				if (s_ref->ref_count > 0) {
					s_ref->ref_count--;
#ifdef _TEST
					std::cout << ", links left: " << s_ref->ref_count << "\n";
#endif
				}
				else {//возможен какой-либо кос€к с учЄтом всех ссылок.

				}
			}
			else
#ifdef _TEST
				std::cout << ", links' not initialized\n";
#endif
			s_ref = nullptr;
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



#endif //MEMWIZ_H