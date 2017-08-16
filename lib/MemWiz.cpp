

#include "MemWiz.h"

using namespace MemoryControl;
static size_t reference_amount = 512;
//static size_t reference_size = sizeof(_memory_interface::ref);
static size_t unit_memory = 128; //� ��

namespace MemoryControl {
	_memory_interface mem_wiz = _memory_interface();


	template<class T>
	bool _memory_interface::_smart_ptr<T>::check_if_can_continue(size_t _new_size_of_arr)
	{
		size_t = sizeof(this->at(0));
		ref* next_ref = s_ref + 1;
		if (next_ref < mem_wiz.mem_pool) {
			if (!next_ref->mem_ref || !next_ref->ref_count && !(next_ref + 1)->mem_ref) {
				next_ref->mem_ref = (int8_t)s_ref->mem_ref + _new_size_of_arr;
				return true;
			}
			if ((int8_t)s_ref->mem_ref + _new_size_of_arr < next_ref->mem_ref) return true;
		}
		else if (next_ref == mem_wiz.mem_pool) {
			if ((int8_t)s_ref->mem_ref + _new_size_of_arr < (int8_t)mem_wiz.block + unit_memory*1024) return true;
		}
		return false;
	}
	
	template<class T>
	_memory_interface::_smart_ptr<T>::_smart_ptr(T _obj)
	{
		s_ref = mem_wiz.allocate_mem(sizeof(_obj));
		T *temp_ptr = (T*)s_ref->mem_ref;
		*temp_ptr = _obj;
	}
	template<class T>
	_memory_interface::_smart_ptr<T>::_smart_ptr(T _obj, size_t _size_of_array)
	{
		s_ref = mem_wiz.allocate_mem(sizeof(_obj), _size_of_array);
	}	

	template<class T>
	T * _memory_interface::_smart_ptr<T>::get_ptr_unsafe()
	{
		return (T*) s_ref->mem_ref;
	}

	template<class T>
	T _memory_interface::_smart_ptr<T>::at(size_t _place)
	{
		return this[_place];
	}
	//����� ������������� ��������� ������ ��� ���������� ������������������ � ������� �������� ������������.
	template<class T>	
	void _memory_interface::_smart_ptr<T>::alloc(T _obj, size_t _size_of_arr)
	{
		if (s_ref) return;
						  //���� ������ ��� �������� - ������ ������ �� ����.
		s_ref = mem_wiz.allocate_mem(sizeof(_obj), _size_of_arr);

	}

	//�� ��������������� �������� ������������� ������ ��� ������ ������� ����. ��� �� ������ ����� �� ������������ ������������������ ������� ������������� ����������.
	template<class T>
	void _memory_interface::_smart_ptr<T>::realloc(size_t _new_size_of_arr)
	{
		if (!s_ref) return;
		if (check_if_can_continue(_new_size_of_arr)) {//���� ����� ��� ������ �������� ������ - ������ �� ��� ������
			return;
		}
		else{//���� ��� - �������� ������ � ����� ����� � �������� ���� ������ ������. ��� ���� ��������� ��������� ������ ����� ���������. 
			//���� ���� ������ ��������� �� �������������� ������� ������ ��� �������� ��������� ���������, �� ����� ����������� ������ ������ �����������.
			ref* temp = mem_wiz.allocate_mem(sizeof(this->at(0)), _new_size_of_arr);
			size_t num_of_bytes = 0;
			if ((s_ref+1)!= mem_wiz.mem_pool){
				num_of_bytes = (int8_t*)((s_ref + 1)->mem_ref) - (int8_t*)s_ref->mem_ref;
			}
			else{
				num_of_bytes = ((int8_t*)block + unit_memory*1024) - (int8_t*)s_ref->mem_ref;
			}
			memcpy(temp->mem_ref, s_ref->mem_ref, num_of_bytes);
			s_ref->ref_count--;
			s_ref = temp;
		}
	}

	template<class T>
	T * _memory_interface::_smart_ptr<T>::operator->()
	{
		if (!s_ref || !s_ref->mem_ref) return &T();//���� ����������� ������ ������������ ������ �� ������ ��������� T ��� ������� ����������
		//TODO: �������� ����� ����� ��� �� ��������� ����� ������ ��������� �������� �� ������ ���� ������ (� �� �� ��� �������)
		return (T*)s_ref->mem_ref; 
	}

	template<class T>
	T & _memory_interface::_smart_ptr<T>::operator*()
	{
		if (!s_ref || !s_ref->mem_ref) return T();
		return *((T*)s_ref->mem_ref);
	}

	template<class T>
	T & _memory_interface::_smart_ptr<T>::operator[](size_t _place)
	{
		//�� ������ �������� ��������� � �.�. � �� ��� ��������. ��� ����� �������� �� ���� �������� _place �� ������ ��������� �������� ������������, �� ������.
		if (!s_ref || !s_ref->mem_ref) return T();//������� ���������
		if ((int8_t*)s_ref->mem_ref + _place > (int8_t*)mem_wiz.block + unit_memory * 1024) return T(); //����� �� ������� ���������� ������
		if (!(s_ref + 1)->mem_ref) return *((T*)s_ref->mem_ref + _place);
		if ((int8_t*)s_ref->mem_ref + _place >(s_ref + 1)->mem_ref) return T();
		return *((T*)s_ref->mem_ref + _place);		
	}

	template<class T>
	_memory_interface::_smart_ptr<T>& _memory_interface::_smart_ptr<T>::operator=(_memory_interface::_smart_ptr<T>& _left)
	{
		this->s_ref = _left.s_ref;
		this->s_ref->ref_count++;
		return *this;
	}



	_memory_interface::ref * MemoryControl::_memory_interface::allocate_mem(size_t _size_of_type, size_t _size_of_arr)
	{
		ref* temp_ref = mem_wiz.reference;
		ref* next_ref = nullptr;
		void *end = (int8_t*)mem_wiz.block + unit_memory * 1024 - 8;
		size_t size = _size_of_type*_size_of_arr;
		while (temp_ref < mem_wiz.mem_pool) {
			if (!temp_ref->ref_count) {//������� ������ ����� ����
				next_ref = temp_ref + 1;
				if (temp_ref->mem_ref) {//���� ����� ������ ���� ������������/��������������� ��������� �� ����� ����������� �����					
					if (next_ref == (ref*)mem_wiz.mem_pool) {//������ ���������( ���� ���-�� ������
						if ((int8_t*)temp_ref->mem_ref + size < end) {
							temp_ref->ref_count += 1;
							return temp_ref;
						}
						else {//TODO: ��������� ��� ����� ������.

						}
					}
					else if (!next_ref->mem_ref) {
						if ((int8_t*)temp_ref->mem_ref + size < end) {
							temp_ref->ref_count += 1;
							next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;//������ ����� ����� ������/������ ����������.
							return temp_ref;
						}
						else {// ��������� ��� ����� ������.

						}
					}
					else {//next_ref->mem_ref != nullptr
						if ((int8_t*)temp_ref->mem_ref + size < next_ref->mem_ref) {//���������� �� �����
							temp_ref->ref_count += 1;
							if (!next_ref->ref_count) {//���� �� ��������� ���� ������ ������ �� ��������� - ����������� ��������� ������� (���� ����� ������� ��������)
								next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;
							}
							return temp_ref;
						}
					}
				}
				else{//������ ������ � �������
					temp_ref->mem_ref = mem_pool;
					temp_ref->ref_count += 1;
					next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;
					return temp_ref;
				}
			}
			temp_ref++;
		}
		return nullptr;
	}

	


	MemoryControl::_memory_interface::_memory_interface()
	{
		reference = nullptr;
		mem_pool = nullptr;
		block = nullptr;
		uint32_t try_count = 0;
		do {
			block = calloc(unit_memory, 1024);//� ��
			try_count++;
		} while (try_count < 5 && (!block));//���������� �������� ������ ��� ���������� ���� 
		reference = (ref*)block;
		ref *ref_temp = reference + reference_amount;
		mem_pool = (void*)ref_temp;
	}


	MemoryControl::_memory_interface::~_memory_interface()
	{
		//TODO:�������� �������� ������. ��� ���������� ������ ��������� � �������� �� ���������. �� �������� ����������� ����������������� ���� ������ � ������� ������, �������� ���� ����������� ����������/�������� ��������
		free(block);
	}

	template<class T>
	_memory_interface::_smart_ptr<T>  _memory_interface::get_smart(T _obj, size_t _size_of_array)
	{
		if (_size_of_array = 1) {
			return _smart_ptr<T>(_obj);
		}
		return _smart_ptr<T>(_obj, _size_of_array);
	}



	int test()
	{
		int a = 7;
		double b = 15.2;
		_memory_interface::_smart_ptr<double> ptr;
		ptr = mem_wiz.get_smart(b);
		double *c = ptr.get_ptr_unsafe();
		double d = *c;
		_memory_interface::_smart_ptr<int> ptr1;
		ptr1 = mem_wiz.get_smart(a);
		int *k = ptr1.get_ptr_unsafe();
		int n = *k;
		_memory_interface::_smart_ptr<char> ptr3;
		

		return 0;
	}


	

}