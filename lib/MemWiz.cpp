

#include "MemWiz.h"

using namespace MemoryControl;


namespace MemoryControl {
	_memory_interface mem_wiz = _memory_interface();

	ref_iterator MemoryControl::_memory_interface::allocate_mem(size_t _size_of_type, size_t _size_of_arr)
	{
		ref_iterator temp_ref = ALLOCATOR.reference_table.before_begin();
		ref_iterator next_ref;
		//void *end = (int8_t*)mem_wiz.block + unit_memory * 1024 - 8;
		size_t size = _size_of_type*_size_of_arr;
		while (temp_ref !=  ALLOCATOR.reference_table.end()) {
			if (ALLOCATOR.reference_table.empty()) {
				temp_ref = ALLOCATOR.reference_table.emplace_after(temp_ref, ref());
				next_ref = ALLOCATOR.reference_table.emplace_after(temp_ref, ref());
			}
			else {
				temp_ref++;
				next_ref = temp_ref; 
				next_ref++;				
			}
			if (!temp_ref->ref_count) {//������� ������ ����� ����				
				if (temp_ref->shift) {//���� ����� ������ ���� ������������/��������������� ��������� �� ����� ����������� �����					
					if (next_ref == ALLOCATOR.reference_table.end()) {//������ ���������

						if ((int8_t*)ALLOCATOR.block + (temp_ref->shift - 1 + size) <= ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							next_ref = ALLOCATOR.reference_table.emplace_after(temp_ref, ref());
							next_ref->shift = temp_ref->shift + size;
							return temp_ref;
						}
						else {//TODO: ��������� ��� ����� ������.

						}
					}
					else if (!next_ref->shift) {
						if ((int8_t*)ALLOCATOR.block + (temp_ref->shift - 1 + size) <= ALLOCATOR.block_end) {
							temp_ref->ref_count += 1;
							next_ref->shift = temp_ref->shift + size; //������ ����� ����� ������/������ ����������.
							return temp_ref;
						}
						else {// ��������� ��� ����� ������.

						}
					}
					else {//next_ref->shift != 0
						if (temp_ref->shift + size <= next_ref->shift) {//���������� �� �����
							temp_ref->ref_count += 1;
							if (!next_ref->ref_count) {//���� �� ��������� ���� ������ ������ �� ��������� - ����������� ��������� ������� (���� ����� ������� ��������)
								next_ref->shift = temp_ref->shift + size;
							}
							return temp_ref;
						}
					}
				}
				else{//������ ������ � �������
					temp_ref->shift = 1;
					temp_ref->ref_count += 1;
					next_ref->shift = temp_ref->shift + size;
					return temp_ref;
				}
			}			
		}
		return ALLOCATOR.reference_table.end();//�� ������ ������.
	}

	MemoryControl::_memory_interface::_memory_interface()
	{
		//reference_table; //��� ��������������� ������������� �� ���������
		block = nullptr;
		block_end = nullptr;
		uint32_t try_count = 0;

		//WriteProcessMemory(hDF, (void*)Inject_point, modsrc, 13, NULL);
		//ReadProcessMemory(hDF, (void*)PauseStateAddr, &PauseState, 1, NULL);
		do {
			block = VirtualAllocEx(GetCurrentProcess(), NULL, unit_memory * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			try_count++;
		} while (!block&&try_count < 5);
		/*if (!block) { //err
			GetLastError();
		}*/
		block_end = (void*)((int8_t*)block + unit_memory * 1024);
#ifdef _TEST
		std::cout << "_memory_interface() constructed.\n Pool from: " << block << " to " << block_end <<" All memory: " << unit_memory <<" Kb\n";
#endif

	}


	MemoryControl::_memory_interface::~_memory_interface()
	{
		//TODO:�������� �������� ������. ��� ���������� ������ ��������� � �������� �� ���������. �� �������� ����������� ����������������� ���� ������ � ������� ������, �������� ���� ����������� ����������/�������� ��������
		ref_iterator iter = this->reference_table.begin();
#ifdef _TEST
		std::cout << "_memory_interface destructed.\n number of links left: 0";
		while (iter !=  this->reference_table.end()) {
			if (iter->ref_count) {
				std::cout << ", " << iter->ref_count ;
			}
			iter++;
		}	
#endif

		VirtualFreeEx(GetCurrentProcess(),this->block,unit_memory*1024, MEM_RELEASE);		

		this->reference_table.~forward_list();
	}	

#ifdef _TEST
	int test()
	{
		return 0;		
	}
#endif	
	

}


