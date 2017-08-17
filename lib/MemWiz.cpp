

#include "MemWiz.h"

using namespace MemoryControl;
static size_t reference_amount = 512;
//static size_t reference_size = sizeof(_memory_interface::ref);
static size_t unit_memory = 128; //в Кб

namespace MemoryControl {
	_memory_interface mem_wiz = _memory_interface();

	_memory_interface::ref * MemoryControl::_memory_interface::allocate_mem(size_t _size_of_type, size_t _size_of_arr)
	{
		ref* temp_ref = ALLOCATOR.reference_table;
		ref* next_ref = nullptr;
		//void *end = (int8_t*)mem_wiz.block + unit_memory * 1024 - 8;
		size_t size = _size_of_type*_size_of_arr;
		while (temp_ref <  ALLOCATOR.reference_table + 512) {
			if (!temp_ref->ref_count) {//счётчик ссылок равен нулю
				next_ref = temp_ref + 1;
				if (temp_ref->mem_ref) {//если ранее память была использована/инициализирован указатель на конец предыдущего блока					
					if (next_ref == ALLOCATOR.reference_table + 512) {//адреса кончились( надо что-то делать
						if ((int8_t*)temp_ref->mem_ref + size < ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							return temp_ref;
						}
						else {//TODO: выделение доп блока памяти.

						}
					}
					else if (!next_ref->mem_ref) {
						if ((int8_t*)temp_ref->mem_ref + size < ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;//задать конец куска памяти/начало следующего.
							return temp_ref;
						}
						else {// выделение доп блока памяти.

						}
					}
					else {//next_ref->mem_ref != nullptr
						if ((int8_t*)temp_ref->mem_ref + size < next_ref->mem_ref) {//достаточно ли места
							temp_ref->ref_count += 1;
							if (!next_ref->ref_count) {//если на следующий блок памяти ничего не указывает - передвинуть указатель поближе (ради более плотной упаковки)
								next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;
							}
							return temp_ref;
						}
					}
				}
				else{//первая запись в таблице
					temp_ref->mem_ref = ALLOCATOR.block;
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
		reference_table = nullptr;		
		block = nullptr;
		block_end = nullptr;
		uint32_t try_count = 0;
		do {
			reference_table = (ref*)calloc(sizeof(ref), reference_amount);
			try_count++;
		} while (try_count < 5 && (!reference_table));
		try_count = 0;
		do {
			block = calloc(unit_memory, 1024);//в Кб
			try_count++;
		} while (try_count < 5 && (!block));//попытаться выделить память для начального пула 		
		block_end = (int8_t*)block + unit_memory * 1024;
	}


	MemoryControl::_memory_interface::~_memory_interface()
	{
		//TODO:проверка счётчика ссылок. При завершении работы программы в принципе не требуется. но возможно потребуется переинициализация пула памяти в течение работы, например если реализовать сохранение/загрузку программ
		ref* iter = this->reference_table;
		std::cout << "_memory_interface dest. number of links left: \n";
		while (iter < reference_table + 512) {
			if (iter->ref_count) {
				std::cout << iter->ref_count << "\n";
			}
		}		
		free(this->block);
		free(this->reference_table);
	}

	template<class T>
	void _smart_ptr<T>::get_smart(T &_obj)
	{
		*((T*)this->s_ref->mem_ref) = _obj;		
	}

	int test()
	{
		int a = 7;
		double b = 15.2;
		double_sptr ptr = double_sptr(a);
		
		double *c = ptr.get_ptr_unsafe();
		double d = *c;

		int32__sptr ptr1 = int32__sptr(int(),5);
		for (int i = 0; i < 5; i++) {
			a = i *(2 + i);
			ptr1[i] = a;
			std::cout << a << "\n";
		}
		for (int i = 0; i < 5; i++) {
			std::cout << ptr1[i] << "\n";
		}
		
		
		
		double_sptr ptr4;
		double_sptr ptr5;
		ptr5 = ptr4 = ptr;

		c = ptr4.get_ptr_unsafe();
		d = *c;
		d = (*ptr + *ptr4)/(*ptr5);

		return 0;
	}



	template<class T>
	bool _smart_ptr<T>::check_if_can_continue(size_t _new_size_of_arr)
	{
		size_t = sizeof(T);
		_memory_interface::ref* next_ref = s_ref + 1;
		if (next_ref < ALLOCATOR.reference_table+512) {
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

	template<class T>
	_smart_ptr<T>::_smart_ptr()
	{
		s_ref = ALLOCATOR.allocate_mem(sizeof(T));
	}

	template<class T>
	_smart_ptr<T>::_smart_ptr(T _obj)
	{
		s_ref = ALLOCATOR.allocate_mem(sizeof(T));
		T *temp_ptr = (T*)s_ref->mem_ref;
		*temp_ptr = _obj;
	}
	template<class T>
	_smart_ptr<T>::_smart_ptr(T _obj, size_t _size_of_array)
	{
		s_ref = ALLOCATOR.allocate_mem(sizeof(T), _size_of_array);
		T *temp_ptr = (T*)s_ref->mem_ref;
		*temp_ptr = _obj;
	}

	template<class T>
	T * _smart_ptr<T>::get_ptr_unsafe()
	{
		return (T*)s_ref->mem_ref;
	}

	template<class T>
	T _smart_ptr<T>::at(size_t _place)
	{
		return this[_place];
	}
	//метод подразумевает выделение памяти для указателей инициализированных с помощью нулевого конструктора.
	template<class T>
	void _smart_ptr<T>::alloc(T _obj, size_t _size_of_arr)
	{
		if (s_ref) return;
		//если память уже выделена - делать ничего не надо.
		s_ref = ALLOCATOR.allocate_mem(sizeof(_obj), _size_of_arr);

	}

	//не рассматривается ситуация перевыделения памяти под объект другого типа. Так же данный метод не предназначен инициализированных нулевым конструктором указателей.
	template<class T>
	void _smart_ptr<T>::realloc(size_t _new_size_of_arr)
	{
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

	template<class T>
	T * _smart_ptr<T>::operator->()
	{
		if (!s_ref || !s_ref->mem_ref) return &T();//ради безопасного вызова возвращается ссылка на пустой экземпляр T при нулевых указателях
												   //TODO: Возможно имеет смысл так же проверять чтобы нужный указатель ссылался во внутрь пула памяти (а не за его пределы)
		return (T*)s_ref->mem_ref;
	}

	template<class T>
	T & _smart_ptr<T>::operator*()
	{
		T zero = T();
		if (!s_ref || !s_ref->mem_ref) return zero;
		return *((T*)s_ref->mem_ref);
	}

	template<class T>
	T & _smart_ptr<T>::operator[](size_t _place)
	{
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

	template<class T>
	_smart_ptr<T>& _smart_ptr<T>::operator=(_smart_ptr<T>& _left)
	{
		this->s_ref = _left.s_ref;
		this->s_ref->ref_count++;
		return *this;
	}	

}