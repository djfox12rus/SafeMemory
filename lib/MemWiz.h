#pragma once
#ifndef MEMWIZ_H
#define MEMWIZ_H
#define _TEST
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <Windows.h>
#include <list>
#define ALLOCATOR MemoryControl::mem_wiz
//#define NEW (auto o) MemoryControl::mem_wiz.get_smart(o) //TODO:попробовать задефайнить свой NEW как макрос.

static size_t reference_amount = 512;

static size_t unit_memory = 32; //в Кб

namespace MemoryControl {	
	
	class _memory_interface {
	private:
		template<class T>
		friend class _smart_ptr;
		struct ref {
			uint32_t ref_count;
			void * mem_ref;
			uint32_t slide;
		};
		
		ref *reference_table;	//таблица адресации хранится в виде массива. Возможно сделать её в виде списка std::list, чтобы не заботится о выходе за пределы адресного поля	
		void *block;		
		void *block_end;

		ref* allocate_mem(size_t _size_of_type, size_t _size_of_arr = 1);

	public:
		_memory_interface();
		~_memory_interface();
		
	};
	extern _memory_interface mem_wiz;



	template<class T>
	class _smart_ptr {
	protected:
		_memory_interface::ref *s_ref;
		bool check_if_can_continue(size_t _new_size_of_arr) {
			size_t size_of type = sizeof(T);
			_memory_interface::ref* next_ref = s_ref + 1;
			if (next_ref < ALLOCATOR.reference_table + 512) {
				if (!next_ref->mem_ref || !next_ref->ref_count && !(next_ref + 1)->mem_ref) {
					next_ref->mem_ref = (int8_t)s_ref->mem_ref + _new_size_of_arr;
					return true;
			}
				if ((int8_t)s_ref->mem_ref + _new_size_of_arr < next_ref->mem_ref) return true;
		}
			else if (next_ref == ALLOCATOR.mem_pool) {//TODO:выделение доп памяти
				if ((int8_t)s_ref->mem_ref + _new_size_of_arr < (int8_t)ALLOCATOR.block + unit_memory * 1024) return true;
				//TODO:выделение доп памяти
			}
			return false;
	}
	public:
		_smart_ptr() {
			
		}


		_smart_ptr(T _obj) {
			s_ref = ALLOCATOR.allocate_mem(sizeof(T));
			T *temp_ptr = (T*)s_ref->mem_ref;
			*temp_ptr = _obj;
#ifdef _TEST
			std::cout << "smart_ptr(_obj) constructed. Links: " << s_ref->ref_count << ", adress: " << s_ref->mem_ref << "\n";
#endif
		}


		_smart_ptr(T _obj, size_t _size_of_array) {
			s_ref = ALLOCATOR.allocate_mem(sizeof(T), _size_of_array);
			T *temp_ptr = (T*)s_ref->mem_ref;
			*temp_ptr = _obj;
#ifdef _TEST
			std::cout << "smart_ptr(_obj, _size_of_array) constructed. Links: " << s_ref->ref_count << ", adress: " << s_ref->mem_ref << "\n";
#endif
		}
		~_smart_ptr() {
#ifdef _TEST
			std::cout << "dest smart\n";
			
#endif
			if (s_ref) {
#ifdef _TEST
				std::cout << "memory adress: " << s_ref->mem_ref;
#endif
				if (s_ref->ref_count > 0) {
					s_ref->ref_count--;
#ifdef _TEST
					std::cout << ", links left: " << s_ref->ref_count << "\n";
#endif
				}
				else {//возможен какой-либо косяк с учётом всех ссылок.

				}
			}
			else
#ifdef _TEST
				std::cout << ", links' not initialized\n";
#endif
			s_ref = nullptr;
		}

		//Небезопасно! Нельзя удалять ("забывать") сам smart_pointer! Нельзя использовать delete!
		T* get_ptr_unsafe() {
			return (T*)s_ref->mem_ref;
		}

		T at(size_t _place) {
			return this[_place];
		}

		void alloc(T _obj, size_t _size_of_arr = 1) {
			if (s_ref) return;
			//если память уже выделена - делать ничего не надо.
			s_ref = ALLOCATOR.allocate_mem(sizeof(_obj), _size_of_arr);

		}
		void realloc(size_t _new_size_of_arr) {
			if (!s_ref) return;
			if (check_if_can_continue(_new_size_of_arr)) {//если можно доп память выделить дальше - значит всё уже готово
				return;
			}
			else {//если нет - выделяем память в новом месте и копируем туда старые данные. При этом изменения коснуться только ЭТОГО указателя. 
				  //Если были другие указатели на первоначальный участок памяти они окажутся полностью валидными, но будут располагать только старой информацией.
				_memory_interface::ref* temp = ALLOCATOR.allocate_mem(sizeof(this->at(0)), _new_size_of_arr);
				size_t num_of_bytes = 0;
				if ((s_ref + 1) != ALLOCATOR.mem_pool) {
					num_of_bytes = (int8_t*)((s_ref + 1)->mem_ref) - (int8_t*)s_ref->mem_ref;
				}
				else {
					num_of_bytes = ((int8_t*)block + unit_memory * 1024) - (int8_t*)s_ref->mem_ref;
				}
				memcpy(temp->mem_ref, s_ref->mem_ref, num_of_bytes);
				s_ref->ref_count--;
				s_ref = temp;
			}
		}

		T* operator->() {
			if (!s_ref || !s_ref->mem_ref) return &T();//ради безопасного вызова возвращается ссылка на пустой экземпляр T при нулевых указателях
													   //TODO: Возможно имеет смысл так же проверять чтобы нужный указатель ссылался во внутрь пула памяти (а не за его пределы)
			return (T*)s_ref->mem_ref;
		}
		T& operator* () {
			T zero = T();
			if (!s_ref || !s_ref->mem_ref) return zero;
			return *((T*)s_ref->mem_ref);
		}

		T& operator[](size_t _place) {
			//НО данный оператор сработает в т.ч. и не для массивов. При любом отличном от нуля значении _place он выдаст результат нулевого конструктора, но выдаст.

			T zero = T();
			T* out;
			if (!s_ref || !s_ref->mem_ref) return zero;//нулевые указатели
			if ((void*)((T*)s_ref->mem_ref + _place) >= (void*)((int8_t*)ALLOCATOR.block + unit_memory * 1024)) return zero; //выход за пределы глобальной памяти
			_memory_interface::ref* next_ref = s_ref + 1;
			if (!next_ref->mem_ref) {
				out = (T*)s_ref->mem_ref + _place;
				return *out;
			}
			if ((void*)((T*)s_ref->mem_ref + _place) >= next_ref->mem_ref) return zero;

			out = (T*)s_ref->mem_ref + _place;
			return *out;
		}

		_smart_ptr<T>& operator=(_smart_ptr<T>& _left) {
			this->s_ref = _left.s_ref;
			this->s_ref->ref_count++;
			return *this;
		}

		void get_smart(T &_obj) {
			*((T*)this->s_ref->mem_ref) = _obj;
		}

	};	

#ifdef _TEST
	int test();
#endif

}


#endif //MEMWIZ_H