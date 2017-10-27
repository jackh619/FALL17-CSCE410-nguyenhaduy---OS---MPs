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
    // assert(false);
    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    // assert(false);
    //Only initialize if there is no page directory
    if (page_directory == NULL){
      //Put the page_directory in kernel_mem_pool
      // page_directory = ( unsigned long*)(kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
      page_directory = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE); 
      

      //Direct mapped page_table
      //Get frame for kernel page table (first 4MB)
      unsigned long *page_table = (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE); 
      unsigned long  memory_addr = 0;

      for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
        //Set the page table to 'kernel', 'r&w', 'present' -> 011
        page_table[i] = memory_addr | 0x3;
        memory_addr += PAGE_SIZE;
      }

      //Set the first entry of page directory to 'user', 'r&w', 'present' -> 011
      page_directory[0] = (unsigned long) page_table | 0x7;
    }

    // Set all page table entries in the directory to 'kernel', 'r&w', ' not present' -> 010
    for(unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
      page_directory[i] = 0 | 0x2; 

  	// Set last entry of page diretory point to itseft to enable page recursion
  	page_directory[ENTRIES_PER_PAGE - 1] = (unsigned long)page_directory | 0x3;

  	// Set the set of virtual memory pools to NULL
  	for (unsigned int i = 0; i < 16; ++i) {
  		VM_Pools[i] = NULL;
  	}

  	paging_enabled = false;
    write_cr0(read_cr0() & 0x7FFFFFFF);
  
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    // assert(false);
    current_page_table = this;
    write_cr3((unsigned long) this->page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // assert(false);
    paging_enabled = 1;
    //Set the most significant bit of register CR0 to 1 to enable paging
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    // assert(false);
    //Get the error code
    unsigned long error_code = _r->err_code;

    //Check the lowest bit for page fault error
    if((error_code & 0x1) == 1)  // protection fault
      Console::puts("Protection Fault!\n");
      
    else { // If age not present

      //Read page directory from register CR3
      // unsigned long *page_dir = (unsigned long *) read_cr3();      

      // If the page directory table is from the process_mem_pool
      unsigned long *page_dir = (unsigned long *) 0xFFFFF000; // logical address to access page directory table


      //Read the fault page address from register CR2
      unsigned long memory_addr = read_cr2();

      unsigned long *page_table;

      //Calulating indexces
      unsigned long page_tab_index = (memory_addr / PAGE_SIZE) & 0x3FF;
      unsigned long page_dir_index = memory_addr / (ENTRIES_PER_PAGE*PAGE_SIZE);

      VMPool** VMPool_Array = current_page_table->VM_Pools;
      bool legitimated = false;
      for (unsigned int i = 0; i < 16; ++i) {
      	if ((VMPool_Array[i]!=NULL) && (VMPool_Array[i]->is_legitimate(memory_addr))) {
      		legitimated = true;
      		break;
      	}
      }

      // If the address is not legitimate in any pools
      if (legitimated) {
      	Console::puts("Address is not legitimated!!\n");
      	// return;
      }

	    //If the 2nd level page_table is in memory
	  if(page_dir[page_dir_index] & 0x1 == 0x1) {  
	    // Get page table from page directory
	    // page_table = (unsigned long *) ((page_dir[page_dir_index]) & 0xFFFFF000);
	    page_table = (unsigned long *) ((page_dir_index*PAGE_SIZE) | 0xFFC00000);
	  } 
	  //If the 2nd level page_table is not loaded in memory
	  else {  
	    // Get address for new page table      
	    page_dir[page_dir_index] = (unsigned long)(process_mem_pool->get_frames(1)*PAGE_SIZE) | 0x7;
	    // Get the page table logical address
	    // page_table = (unsigned long *) ((page_dir[page_dir_index]) & 0xFFFFF000);
	    page_table = (unsigned long *) ((page_dir_index*PAGE_SIZE) | 0xFFC00000);

	    //Initialize all entry of the page table to empty
	    for(unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
	        page_table[i] = 0x6;
	    }
	  }

	      //Load page and set to 'user', 'r&w', 'present' -> 111
	      page_table[page_tab_index] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 0x7;

	      Console::puts("Handled page fault\n");
	  
	}	    
}

void PageTable::free_page (unsigned long _page_no) {
    // assert(false);
	// _page_no is a logical address (virtual address)
	// Get frame number from a logical address
	unsigned long page_dir_index = _page_no / (ENTRIES_PER_PAGE*PAGE_SIZE);
	unsigned long page_tab_index = (_page_no / PAGE_SIZE) & 0x3FF;
	// unsigned long* page_table = (unsigned long*) (page_directory[page_dir_index] * PAGE_SIZE);
	unsigned long* page_table = (unsigned long*)((page_dir_index * PAGE_SIZE) | 0xFFC00000);
	unsigned long frame_number = page_table[page_tab_index];

	// Release the frame
	process_mem_pool->release_frames(frame_number);

	// Mark the page invalid
	page_table[page_tab_index] = 0x0;
	write_cr3(read_cr3());
}

void PageTable::register_pool (VMPool *_pool) {
    // assert(false);
	int VM_Pools_Index = -1;
	for (unsigned int i = 0; i < 16; ++i) {
		if (VM_Pools[i] == NULL) {
			VM_Pools_Index = i;
		}
	}
	if (VM_Pools_Index >= 0) {
		VM_Pools[VM_Pools_Index] = _pool;
		Console::puts("Register new Virtual Memory Pool successfull! \n");
	} else {
		Console::puts("ERROR: Failed to register new Virtual Memory Pool. Virtual Memory Pool array is full! \n");
	}
	
}