#pragma once
#ifndef MEMWIZ_H
#define MEMWIZ_H

#include <cstdint>
#include <cstdlib>
#include <iostream>
#define ALLOCATOR MemoryControl::mem_wiz
//#define NEW (auto o) MemoryControl::mem_wiz.get_smart(o) //TODO:����������� ����������� ���� NEW ��� ������.

namespace MemoryControl {	
	
	class _memory_interface {
	private:
		friend class _smart_ptr;
		struct ref {
			uint32_t ref_count;
			void * mem_ref;
		};
		
		ref *reference;	//���� ������� ��������� ����� ������ ��������� ��������������� � ����� ������. ������ ��������� ������� "�����" ��������� �� ��� ��� ������ ������� �������� �� ���������,
						//� ��������� �� ������ ������ � �������, ����� ����� ��� ������ �������� ������ ��������. ����� ��� ��������� ���� ������ ����� ����� ������������ ������ realloc 
						//��� calloc + memmove (������� ����� ����� ������), � ������ �������������� � ������� ���������� ����������. ����� ������� �������� ��� ��� ������� ��������� ������������.
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
			_smart_ptr(T _obj, size_t _size_of_array);//��� �� ���������� �������. �������� ����� ����� ������� ��������� ����� ���������� ������ ��� �������.
			~_smart_ptr() {
				std::cout << "dest smart\n";
				std::cout << "memory adress: "<< s_ref;
				if (s_ref) {
					if (s_ref->ref_count > 0) {
						s_ref->ref_count--;
						std::cout << ", links left: " << s_ref->ref_count << "\n";
					}
					else {//�������� �����-���� ����� � ������ ���� ������.

					}
				}
				else
					std::cout << ", links' not initialized\n";
				//s_ref = nullptr;
			}

			//�����������! ������ ������� ("��������") ��� smart_pointer! ������ ������������ delete!
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

//�� �������� ������ �����.
typedef MemoryControl::_memory_interface::_smart_ptr<int8_t> sp_int8_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int16_t> sp_int16_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int32_t> sp_int32_t;
typedef MemoryControl::_memory_interface::_smart_ptr<int64_t> sp_int64_t;
typedef MemoryControl::_memory_interface::_smart_ptr<double> sp_double;


#endif //MEMWIZ_H