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
    region_descrpitors_list = (Region_Descriptors*)base_address;
    region_descrpitors_list[0].region_address = base_address;
    region_descrpitors_list[0].length = 4096;

    // Increase the total number of regions and total region size
    total_regions = 1;
    total_regions_size = 4096;

    // Register this virtual memory pool for the page table
    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}


unsigned long VMPool::allocate(unsigned long _size) {
    // assert(false);

    // If size is zero, do not allocate virtual memory pool
    if (_size == 0) {
    	return 0;
    }

	// If there is no more region to allocate, do not allocate
    if (total_regions >= MAX_REGIONS) {
    	Console::puts("Reach Maximum Number of Regions. Cannot Allocate Any More!!!\n");
    	for(;;);
    }
	
    // It there is no more space, don't allocate memory
	if ((total_regions_size + _size) > size) {
		Console::puts("No More Space in Virtual Memory Pool. Cannot Allocate Any More!!!\n");
    	for(;;);
	}

    // Address of region
    unsigned long current_region_address;

    // If this is the first region, first region address is the base address
    if (total_regions == 0) {
    	current_region_address = base_address;
    }
    // If not the first region, get the current region address from the last region information
    else {
    	unsigned long last_index = total_regions - 1;
    	current_region_address = region_descrpitors_list[last_index].region_address + region_descrpitors_list[last_index].length;
    }

    unsigned long current_index = total_regions;

    // Update descrptor information for current region
    region_descrpitors_list[current_index].region_address = current_region_address;
    region_descrpitors_list[current_index].length = _size;

    // Increase the total number of regions and total region size
    total_regions++;
    total_regions_size += _size;

    Console::puts("Allocated region of memory.\n");
    return current_region_address;
}


void VMPool::release(unsigned long _start_address) {
    // assert(false);
    unsigned long region_index;

    // Find the region descriptor information for region at _start_address
    for (unsigned long i; i < total_regions; ++i) {
    	if (region_descrpitors_list[i].region_address == _start_address) {
    		region_index = i;
    		break;
    	}
    }

    // Release all the pages that the region located in
    unsigned long end_address = _start_address + region_descrpitors_list[region_index].length;
    unsigned long release_address = _start_address;
    while (release_address < end_address) {
    	page_table->free_page(release_address);
    	release_address += PageTable::PAGE_SIZE;
    }

    // Decrease the total region size
    total_regions_size -= region_descrpitors_list[region_index].length;

    // Shift region descriptor to the left to fill the empty descriptor
    for (unsigned long i = region_index; i < total_regions - 1; ++i) {
    	region_descrpitors_list[i].region_address = region_descrpitors_list[i+1].region_address;
    	region_descrpitors_list[i].length = region_descrpitors_list[i+1].length;
    }

    // Clear last region in the list and decrease the total number or regions
    region_descrpitors_list[total_regions - 1].region_address = 0;
    region_descrpitors_list[total_regions - 1].length = 0;
    total_regions--;


    page_table->load();
    Console::puts("Released region of memory.\n");
}


bool VMPool::is_legitimate(unsigned long _address) {
    // assert(false);

    unsigned long min_address, max_address;

    // If an address is in the range of min and max addressed of a region, it is legitimate
    for (unsigned long i = 0; i < total_regions; i++) {
    	min_address = region_descrpitors_list[i].region_address;
    	max_address = min_address + region_descrpitors_list[i].length;
	    if ((_address >= (min_address)) && (_address <= max_address)) {
	    	return true;
	    }
	}
    return false;
    Console::puts("Checked whether address is part of an allocated region.\n");
}

