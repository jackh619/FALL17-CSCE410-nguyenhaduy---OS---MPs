#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    //Only initialize if there is no page directory
    if (page_directory == NULL){
      //Put the page_directory in kernel_mem_pool
      page_directory = ( unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
      

      //Direct mapped page_table
      //Get frame for kernel page table
      unsigned long *kernel_page_table = (unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
      unsigned long  memory_addr = 0;

      for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
        //Set the page table to 'kernel', 'r&w', 'present' -> 011
        kernel_page_table[i] = memory_addr | 0x3;
        memory_addr += PAGE_SIZE;
      }

      //Set the first entry of page directory to 'kernel', 'r&w', 'present' -> 011
      page_directory[0] = (unsigned long) kernel_page_table | 0x3;
    }

    // Set all page tables in the directory to 'kernel', 'r&w', ' not present' -> 010
    for(unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
      page_directory[i] = 0 | 0x2; 
  
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long) this->page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
    //Set the most significant bit of register CR0 to 1 to enable paging
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    //Get the error code
    unsigned long error_code = _r->err_code;

    //Check the lowest bit for page fault error
    if((error_code & 0x1) == 1)  // protection fault
      Console::puts("Protection Fault!\n");
      
    else { // If age not present

      //Read page directory from register CR3
      unsigned long *page_dir = (unsigned long *)read_cr3();      

      //Read the fault page address from register CR2
      unsigned long memory_addr = read_cr2();

      unsigned long *page_table;

      //Calulating indexces
      unsigned long page_tab_index = (memory_addr / PAGE_SIZE) & 0x3FF;
      unsigned long page_dir_index = memory_addr / (ENTRIES_PER_PAGE*PAGE_SIZE);

      VMPool** VMPool_Array = current_page_table->vmpool_array;
      unsigned int VM_Index = 0;

      for (unsigned int i = 0; i < VM_ARRAY_SIZE; ++i) {
      	if (VMPool_Array[i]!=NULL) && (VMPool_Array[i]->islegitimate(memory_addr)) {
      		VM_Index = i;
      		break;
      	}
      }

      if (VM_Index == 0)
      	Console::puts("ADDRESS INVALID!!!\n");

      //If the 2nd level page_table is in memory
      if(page_dir[page_dir_index] & 0x1 == 0x1) {  
        //Get page table from page directory
        page_table = (unsigned long *) ((page_dir[page_dir_index]) & 0xFFFFF000);
      } 
      //If the 2nd level page_table is not loaded in memory
      else {  
        //Load page_table into memory
        page_dir[page_dir_index] = (unsigned long)((kernel_mem_pool->get_frames(1)*PAGE_SIZE) | 0x3);
        //Get address of new page table      
        page_table = (unsigned long *)((page_dir[page_dir_index]) & 0xFFFFF000);

        //Initialize all entry of the page table to empty
        for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
            page_table[i] = 0;
        }
      }

      //Load page and set to 'kernel', 'r&w', 'present' -> 011
      page_table[page_tab_index] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 0x3;      
    }

    Console::puts("handled page fault\n");
}

void PageTable::free_page(unsigned long _page_no) {
	unsigned long directory_index = _page_no / (ENTRIES_PER_PAGE*PAGE_SIZE);
	unsigned long page_index = (_page_no / PAGE_SIZE) & 0x3FF;
	unsigned long* page_table = (unsigned long*)(0xFFC00000 | (directory_index * PAGE_SIZE));
	unsigned long frame_number = page_table[page_index];
	process_mem_pool->release_frame(frame_number);
}

void PageTable::register_vmpool (VMPool *_pool) {
	int index = -1;
	for (unsigned int i = 0, v < VM_ARRAY_SIZE; ++i) {
		if (vmpool_array[i] == NULL) {
			index = i;
		}
	}
	if (index >= 0) {
		vmpool_array[index] = _pool;
		Console::puts("Register Virtual Memory Pool successfull! \n");
	} else {
		Console::puts("ERROR: Failed to register Virtual Memory Pool. Array is full! \n);
	}
	
}