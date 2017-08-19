

#include "MemWiz.h"

using namespace MemoryControl;


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
						if ((int8_t*)temp_ref->mem_ref + size <= ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							return temp_ref;
						}
						else {//TODO: выделение доп блока памяти.

						}
					}
					else if (!next_ref->mem_ref) {
						if ((int8_t*)temp_ref->mem_ref + size <= ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							next_ref->mem_ref = (int8_t*)temp_ref->mem_ref + size;//задать конец куска памяти/начало следующего.
							return temp_ref;
						}
						else {// выделение доп блока памяти.

						}
					}
					else {//next_ref->mem_ref != nullptr
						if ((int8_t*)temp_ref->mem_ref + size <= next_ref->mem_ref) {//достаточно ли места
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
		std::list<ref> l;
		do {

			reference_table = (ref*)calloc(sizeof(ref), reference_amount);
			try_count++;
		} while (try_count < 5 && (!reference_table)); //void *ExtCode = VirtualAllocEx(hDF, NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		//WriteProcessMemory(hDF, (void*)Inject_point, modsrc, 13, NULL);
		//ReadProcessMemory(hDF, (void*)PauseStateAddr, &PauseState, 1, NULL);
		try_count = 0;		
		block = VirtualAllocEx(GetCurrentProcess(), NULL, unit_memory * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);			 		
		block_end = (void*)((int8_t*)block + unit_memory * 1024);
#ifdef _TEST
		std::cout << "_memory_interface() constructed.\n Pool from: " << block << " to " << block_end <<" All memory: " << unit_memory <<" Kb\n";
#endif

	}


	MemoryControl::_memory_interface::~_memory_interface()
	{
		//TODO:проверка счётчика ссылок. При завершении работы программы в принципе не требуется. но возможно потребуется переинициализация пула памяти в течение работы, например если реализовать сохранение/загрузку программ
		ref* iter = this->reference_table;
#ifdef _TEST
		std::cout << "_memory_interface destructed.\n number of links left: 0\n";
		while (iter < reference_table + 512) {
			if (iter->ref_count) {
				std::cout << iter->ref_count << "\n";
			}
			iter++;
		}	
#endif
		VirtualFreeEx(GetCurrentProcess(),this->block,unit_memory*1024, MEM_RELEASE);
		free(this->reference_table);
	}	

#ifdef _TEST
	int test()
	{
		return 0;
		
	}
#endif	
	
	
	

}


