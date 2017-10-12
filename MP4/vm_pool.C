/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    // assert(false);
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    

    // Put the list of information of regions in the first page of the VMPool
    // region_descrpitors_list = (Region_Descriptors*)(frame_pool->get_frames(1) * PageTable::PAGE_SIZE);
    region_descrpitors_list = (Region_Descriptors*)base_address;
    region_descrpitors_list[0].region_address = base_address;
    region_descrpitors_list[0].length = 4096;
    total_regions = 1;
    total_regions_size = 4096;

    // Register this virtual memory pool for the page table
    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    assert(false);

    // If size is zero, do not allocate virtual memory pool
    if (_size == 0) {
    	return 0;
    }

    if (total_regions >= MAX_REGIONS) {
    	Console::puts("Reach Maximum Number of Regions. Cannot Allocate Any More!!!\n");
    	for(;;);
    }

	if ((total_regions_size + _size) > size) {
		Console::puts("No More Space in Virtual Memory Pool. Cannot Allocate Any More!!!\n");
    	for(;;);
	}

    // Address of region
    unsigned long region_address;

    // If this is the first region, first region address is the base address
    if (total_regions == 0) {
    	region_address = base_address;
    }

    else {
    	unsigned long last_index = total_regions - 1;
    	region_address = region_descrpitors_list[last_index].region_address + region_descrpitors_list[last_index].length/4;
    }

    unsigned long current_index = total_regions;

    region_descrpitors_list[current_index].region_address = region_address;
    region_descrpitors_list[current_index].length = _size;

    total_regions++;
    total_regions_size += _size;

    return base_address;
    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) {
    assert(false);
    unsigned long region_index;

    for (unsigned long i; i < total_regions; ++i) {
    	if (region_descrpitors_list[i].region_address == _start_address) {
    		region_index = i;
    		break;
    	}
    }

    unsigned long end_address = _start_address + region_descrpitors_list[region_index].length/4;
    unsigned long release_address = _start_address;
    while (release_address < end_address) {
    	page_table->free_page(release_address);
    	release_address += PageTable::PAGE_SIZE;
    }

    total_regions_size -= region_descrpitors_list[region_index].length;

    for (unsigned long i = region_index; i < total_regions - 1; ++i) {
    	region_descrpitors_list[i].region_address = region_descrpitors_list[i+1].region_address;
    	region_descrpitors_list[i].length = region_descrpitors_list[i+1].length;
    }

    region_descrpitors_list[total_regions - 1].region_address = 0;
    region_descrpitors_list[total_regions - 1].length = 0;
    total_regions--;


    page_table->load();
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    // assert(false);
    if ((_address > (base_address +size)) || (_address < base_address)) {
    	return false;
    }
    return true;
    Console::puts("Checked whether address is part of an allocated region.\n");
}

