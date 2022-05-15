typedef unsigned int m_pid;
typedef unsigned int m_size;
typedef unsigned char atom_data;
typedef unsigned int v_address;
typedef unsigned int p_address;
typedef unsigned int page;
#define PAGE_TABLE_SIZE 1024*128
#define V_PAGE_USE_SIZE 128
#define PAGE_SIZE 4 * 1024
#define P_PAGE_USE_SIZE 32
#define USE_RECORD_SIZE 1024*16
#define FULL (1<<24)-1
#define SWAP_SIZE 4 * 1024
#define SWAP_START 1024 * 1024 * 128
#define MEMORY_SIZE 1024 * 1024 * 128
#define page_bit unsigned char
#define DISK_SIZE 1024 * 1024 * 512
struct pagetable_item {
	unsigned int v_id;
	unsigned int p_id;
	char if_in_memory;
	char if_use;
	pagetable_item() :v_id(0), p_id(FULL), if_in_memory(0), if_use(0) {}
	pagetable_item(unsigned int addr) :v_id(addr), p_id(FULL), if_in_memory(0), if_use(0) {}
};

struct memory_use {
	m_pid pid;
	v_address address;
	m_size size;
};

atom_data read_memory(p_address);
void write_memory(atom_data, p_address);