#pragma once
#ifndef MEMWIZ_H
#define MEMWIZ_H
#define _TEST
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <Windows.h>
#include <forward_list>
#define ALLOCATOR MemoryControl::mem_wiz


static size_t reference_amount = 512;

static size_t unit_memory = 32; //в Кб

namespace MemoryControl {	

	struct ref {
		uint32_t ref_count;
		uint32_t shift;//сдвиг относительно указателя block в байтах + 1.
	};

	typedef  std::forward_list<MemoryControl::ref>::iterator ref_iterator;

	class _memory_interface {
	private:
		template<class T>
		friend class _smart_ptr;
		
		
		std::forward_list<ref> reference_table;
		void *block;		
		void *block_end;

		ref_iterator allocate_mem(size_t _size_of_type, size_t _size_of_arr = 1);

	public:
		_memory_interface();
		~_memory_interface();
		
	};
	extern _memory_interface mem_wiz;



	template<class T>
	class _smart_ptr {
	protected:
		ref_iterator s_ref;

		bool check_if_can_continue(size_t _new_size_of_arr) {
			size_t size_of_type = sizeof(T);
			ref_iterator next_ref = s_ref;			
			next_ref++;
			ref_iterator next_next_ref = next_ref;
			next_next_ref++;
			if (!next_ref->ref_count) {//далее записывать имеет смысл только если на следующую запись ничего не указывает
				if (next_next_ref != ALLOCATOR.reference_table.end()) {
					if (s_ref->shift + _new_size_of_arr*size_of_type < next_next_ref->shift) {
						next_ref->shift = s_ref->shift + _new_size_of_arr*size_of_type;
						return true;
					}
					if (s_ref->shift + _new_size_of_arr*size_of_type == next_next_ref->shift) {
						next_ref = ALLOCATOR.reference_table.erase_after(s_ref);
						return true;
					}
				}
				else {//next_next_ref == ALLOCATOR.reference_table.end()
					if ((int8_t*)ALLOCATOR.block + (s_ref->shift - 1 + _new_size_of_arr*size_of_type) < ALLOCATOR.block_end) {//есть место в блоке памяти
						next_ref->shift = s_ref->shift + _new_size_of_arr*size_of_type;
						return true;
					}
					else {//TODO: выделение доп блока памяти.

					}
				}
			}
			return false;
		}
	public:
		_smart_ptr() {
			s_ref = ALLOCATOR.reference_table.before_begin();
		}


		_smart_ptr(T _obj) {
			s_ref = ALLOCATOR.allocate_mem(sizeof(T));
			T *temp_ptr = (T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1));
			*temp_ptr = _obj;
#ifdef _TEST
			std::cout << "smart_ptr(_obj) constructed. Links: " << s_ref->ref_count << ", adress: " << (void*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1)) << "\n";
#endif
		}


		_smart_ptr(T _obj, size_t _size_of_array) {
			s_ref = ALLOCATOR.allocate_mem(sizeof(T), _size_of_array);
			T *temp_ptr = (T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1));
			*temp_ptr = _obj;
#ifdef _TEST
			std::cout << "smart_ptr(_obj, _size_of_array) constructed. Links: " << s_ref->ref_count << ", adress: " << (void*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1)) << "\n";
#endif
		}
		~_smart_ptr() {
#ifdef _TEST
			std::cout << "dest smart\n";
			
#endif
			if (s_ref!= ALLOCATOR.reference_table.before_begin()) {
#ifdef _TEST
				std::cout << "memory adress: " << (void*)((int8_t*)ALLOCATOR.block + (s_ref->shift-1));
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
		}

		//Небезопасно! Нельзя удалять ("забывать") сам smart_pointer! Нельзя использовать delete!
		T* get_ptr_unsafe() {
			if (s_ref == ALLOCATOR.reference_table.before_begin()) return nullptr;
			return (T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1));
		}

		T at(size_t _place) {
			return this[_place];
		}

		void alloc(T _obj, size_t _size_of_arr = 1) {
			if (s_ref != ALLOCATOR.reference_table.before_begin()) return;
			//если память уже выделена - делать ничего не надо.
			s_ref = ALLOCATOR.allocate_mem(sizeof(_obj), _size_of_arr);

		}
		void realloc(size_t _new_size_of_arr) {
			if (s_ref == ALLOCATOR.reference_table.before_begin()) return;
			if (check_if_can_continue(_new_size_of_arr)) {//если можно доп память выделить дальше - значит всё уже готово
				return;
			}
			else {//если нет - выделяем память в новом месте и копируем туда старые данные. При этом изменения коснуться только ЭТОГО указателя. 
				  //Если были другие указатели на первоначальный участок памяти они окажутся полностью валидными, но будут располагать только старой информацией.
				ref_iterator temp = ALLOCATOR.allocate_mem(sizeof(T), _new_size_of_arr);
				size_t num_of_bytes = 0;
				ref_iterator next_ref = s_ref;
				next_ref++;
				num_of_bytes = next_ref->shift - s_ref->shift;
				void *source = ((int8_t*)ALLOCATOR.begin + (s_ref->shift - 1));
				void *dest = ((int8_t*)ALLOCATOR.begin + (temp->shift - 1));
				memcpy(source, dest, num_of_bytes);
				s_ref->ref_count--;
				s_ref = temp;
			}
		}

		T* operator->() {
			T zero = T();
			if (s_ref == ALLOCATOR.reference_table.before_begin() || !s_ref->shift) return &zero;//ради безопасного вызова возвращается ссылка на пустой экземпляр T при нулевых указателях
													   //TODO: Возможно имеет смысл так же проверять чтобы нужный указатель ссылался во внутрь пула памяти (а не за его пределы)
			return (T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1));
		}
		T& operator* () {
			T zero = T();
			if (!s_ref || !s_ref->mem_ref) return zero;
			return *((T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1)));
		}

		T& operator[](size_t _place) {
			//НО данный оператор сработает в т.ч. и не для массивов. При любом отличном от нуля значении _place он выдаст результат нулевого конструктора, но выдаст.

			T zero = T();
			T* out;
			if (s_ref == ALLOCATOR.reference_table.before_begin() || !s_ref->shift) return zero;//нулевые указатели
			if ((void*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1 + _place * sizeof(T))) >= ALLOCATOR.block_end) return zero; //выход за пределы глобальной памяти
			ref_iterator next_ref = s_ref;
			next_ref++;
			
			if (s_ref->shift + _place*sizeof(T) >= next_ref->shift) return zero;

			out = (T*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1 + _place * sizeof(T)));
			return *out;
		}

		_smart_ptr<T>& operator=(_smart_ptr<T>& _left) {
			this->s_ref = _left.s_ref;
			this->s_ref->ref_count++;
#ifdef _TEST
			std::cout << "smart_ptr (memory adress: "<< (void*)((int8_t*)ALLOCATOR.block + (s_ref->shift - 1))<<") copied. Links: "<< this->s_ref->ref_count<<"\n";
#endif	
			return *this;
		}
	};	

#ifdef _TEST
	int test();
#endif

}





#endif //MEMWIZ_H