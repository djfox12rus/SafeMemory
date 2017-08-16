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
		friend class _smart_ptr;
		struct ref {
			uint32_t ref_count;
			void * mem_ref;
		};
		
		ref *reference;	//пока таблица адресации блока пам€ти находитс€ непосредственно в блоке пам€ти. ќднако поскольку внешние "умные" указатели на сам пул пам€ти никогда напр€мую не ссылаютс€,
						//а ссылаютс€ на нужную запись в таблице, имеет смысл под адреса выдел€ть пам€ть отдельно. “огда дл€ основного пула пам€ти можно будет использовать сишный realloc 
						//или calloc + memmove (занул€€ новый кусок пам€ти), а ссылки востанавливать с помощью арифметики указателей. “аким образом возможно сам пул сделать полностью динамическим.
		void *mem_pool;
		void *block;

		ref* allocate_mem(size_t _size_of_type, size_t _size_of_arr = 1);

	public:
		
		

		template<class T>
		class _smart_ptr {
		private:
			ref *s_ref;
			bool check_if_can_continue(size_t);
		public:
			_smart_ptr() : s_ref(nullptr){}
			_smart_ptr(T _obj);
			_smart_ptr(T _obj, size_t _size_of_array);//так же объ€вление массива. возможно имеет смысл сделать отдельный класс указателей только под массивы.
			~_smart_ptr() {
				std::cout << "dest smart\n";
				std::cout << "memory adress: "<< s_ref;
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

			_smart_ptr<T> operator=(_smart_ptr<T>& _left);
			
		};

		_memory_interface();
		~_memory_interface();

		template<class T>
		_smart_ptr get_smart(T _obj, size_t _size_of_array = 1);
		
	};
	extern _memory_interface mem_wiz;
	int test();		

}

//не придумал ничего умнее.
typedef MemoryControl::_memory_interface::_smart_ptr<int8_t> sp_int8_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int16_t> sp_int16_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int32_t> sp_int32_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int64_t> sp_int64_t;
typedef MemoryControl::_memory_interface::_smart_ptr<double> sp_double;


#endif //MEMWIZ_H