#include<iostream>
#include<cstring>
#include"memory.h"
using namespace std;
pagetable_item page_table[PAGE_TABLE_SIZE];
page_bit v_page[V_PAGE_USE_SIZE];
page_bit p_page[P_PAGE_USE_SIZE];
memory_use mem_use[USE_RECORD_SIZE];
atom_data memory[MEMORY_SIZE + SWAP_SIZE];//交换表
atom_data disk[DISK_SIZE];
void init() {
	//初始化页表
	for (p_address addr = 0; addr < PAGE_TABLE_SIZE; addr++) {
		pagetable_item page_item = page_table[addr];
		page_item.v_id = addr;
		page_item.p_id = FULL;
		page_item.if_use = 0;
		page_item.if_in_memory = 0;
		page_table[addr] = page_item;
	}
	//初始化内存
	memset(memory, 0, sizeof(memory));
	//初始化位表
	memset(v_page, 0, sizeof(v_page));
	memset(p_page, 0, sizeof(p_page));
	//初始化内存表
	for (p_address addr = 0; addr < USE_RECORD_SIZE; addr++) {
		mem_use[addr].pid = 1001;
		mem_use[addr].address = 0;
		mem_use[addr].size = 0;
	}
	//初始化磁盘
	memset(disk, 0, sizeof(disk));
}
void disk_load(p_address mem_now, p_address disk_now, m_size size) {
	if (mem_now + size > MEMORY_SIZE || disk_now + size > DISK_SIZE) {
		exit(-1);//中断
	}
	else {
		for (m_size i = 0; i < size; ++i) {
			memory[mem_now + i] = disk[disk_now + i];
		}
	}
}
void disk_save(p_address mem_now, p_address disk_now, m_size size) {
	if (mem_now + size > MEMORY_SIZE || disk_now + size > DISK_SIZE) {
		exit(-1);//中断
	}
	else {
		for (m_size i = 0; i < size; ++i) {
			disk[disk_now + i] = memory[mem_now + i];
		}
	}
}
int read(atom_data* data, v_address addr, m_pid pid) {
	if (addr > DISK_SIZE) {
		return -1;
	}
	int flag = 0;
	for (p_address p = 0; p < USE_RECORD_SIZE; p++) {
		memory_use used = mem_use[p];
		if (used.pid >= 1000)break;
		if (used.pid == pid && used.size + used.address > addr && used.address <= addr) {
			flag = 1;
			break;
		}
	}
	if (!flag) {
		return -1;
	}
	pagetable_item* item = &page_table[addr / PAGE_SIZE];
	if (!item->if_in_memory) {
		for (p_address p = 0; p < PAGE_TABLE_SIZE; p++) {
			if (page_table[p].if_in_memory) {
				if (page_table[p].if_use) {
					page_table[p].if_use = 0;
				}
				else {
					for (p_address offset = 0; offset < PAGE_SIZE; offset++) {
						memory[SWAP_START + offset] = memory[page_table[p].p_id * PAGE_SIZE + offset];
					}
					disk_load(page_table[p].p_id * PAGE_SIZE, item->v_id * PAGE_SIZE, PAGE_SIZE);
					disk_save(SWAP_START, item->v_id * PAGE_SIZE, PAGE_SIZE);
					item->p_id = page_table[p].p_id;
					item->if_in_memory = 1; item->if_use = 1;
					page_table[p].if_use = 0; page_table[p].if_in_memory = 0;
					break;
				}
			}
		}
	}
	else {
		item->if_use = 1;
	}
	*data = memory[item->p_id * PAGE_SIZE + addr % PAGE_SIZE];
	return 0;
}
int write(atom_data data, v_address addr, m_pid pid) {
	//cout << addr << " " << DISK_SIZE;
	if (addr > DISK_SIZE) {
		return -1;
	}
	int flag = 0;
	for (p_address p = 0; p < USE_RECORD_SIZE; p++) {
		memory_use used = mem_use[p];
		if (used.pid >= 1000)break;
		if (used.pid == pid && used.size + used.address > addr && used.address <= addr) {
			flag = 1;
			break;
		}
	}
	if (!flag) {
		return -1;
	}
	pagetable_item* item = &page_table[addr / PAGE_SIZE];
	if (!item->if_in_memory) {
		for (p_address p = 0; p < PAGE_TABLE_SIZE; p++) {
			if (page_table[p].if_in_memory) {
				if (page_table[p].if_use) {
					page_table[p].if_use = 0;
				}
				else {
					for (p_address offset = 0; offset < PAGE_SIZE; offset++) {
						memory[SWAP_START + offset] = memory[page_table[p].p_id * PAGE_SIZE + offset];
					}
					disk_load(page_table[p].p_id * PAGE_SIZE, item->v_id * PAGE_SIZE, PAGE_SIZE);
					disk_save(SWAP_START, item->v_id * PAGE_SIZE, PAGE_SIZE);
					item->p_id = page_table[p].p_id;
					item->if_in_memory = 1; item->if_use = 1;
					page_table[p].if_use = 0; page_table[p].if_in_memory = 0;
					break;
				}
			}
		}
	}
	else {
		item->if_use = 1;
	}
	memory[item->p_id * PAGE_SIZE + addr % PAGE_SIZE] = data;
	return 0;
}

int alloc(v_address* addr, m_size size, m_pid pid) {
	if (size > DISK_SIZE) {

		return -1;
	}
	unsigned int page_num = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	unsigned int found_page = 0;
	unsigned int start_v_page = 0;
	for (unsigned int i = 0; i < V_PAGE_USE_SIZE; i++) {
		found_page++;
		if (v_page[i] == 1) {
			start_v_page += found_page;
			found_page = 0;
		}
		else {
			if (found_page == page_num) {
				break;
			}
		}
	}
	//cout <<"found_page: "<<found_page << endl;
	if (found_page < page_num) {
		cout << 12;
		return -1;
	}
	*addr = start_v_page * PAGE_SIZE;
	for (unsigned int i = start_v_page; i < start_v_page + page_num; i++) {
		v_page[i] = 1;
	}
	for (unsigned int i = 0; i < P_PAGE_USE_SIZE; i++) {
		if (p_page[i] == 0) {
			p_page[i] += 1;
			page_table[start_v_page].p_id = i;
			page_table[start_v_page].if_in_memory = 1;
			page_table[start_v_page++].if_use = 0;
			found_page--;
			if (found_page == 0) {
				break;
			}
		}
	}
	for (unsigned int i = 0; i < USE_RECORD_SIZE; i++) {
		if (mem_use[i].pid >= 1001) {
			mem_use[i].pid = pid;
			mem_use[i].address = *addr;
			mem_use[i].size = size;
			break;
		}
	}
	return 0;
}
int free(v_address addr, m_pid pid) {
	int flag = 0;
	p_address p = 0;
	for (p = 0; p < USE_RECORD_SIZE; p++) {
		if (mem_use[p].pid >= 1001) {
			break;
		}
		if (mem_use[p].pid == pid && mem_use[p].address == addr) {
			flag = 1;
			break;
		}
	}
	if (!flag) {
		return -1;
	}
	for (; p < USE_RECORD_SIZE - 1; p++) {
		mem_use[p] = mem_use[p + 1];
	}
	mem_use[p].pid = 1001;
	unsigned int start_page_id = mem_use[p].address / PAGE_SIZE;
	unsigned int use_page_number = (mem_use[p].size + PAGE_SIZE - 1) / PAGE_SIZE;
	for (unsigned int i = start_page_id; i < start_page_id + use_page_number; i++) {
		v_page[i] = 0;
		page_table[i].p_id = 0;
	}
	return 0;
}