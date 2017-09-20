/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/
ContFramePool** ContFramePool::pool_list;
unsigned int ContFramePool::number_of_pool = 0;


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // Bitmap must fit in a single frame!
    assert(_n_frames <= FRAME_SIZE * 4);
    
    base_frame_no = _base_frame_no;	// Where does the frame pool start in phys mem?
    n_frames = _n_frames;				// Size of the frame pool
    nFreeFrames = _n_frames;	
    info_frame_no = _info_frame_no;	// Where do we store the management information?
    n_info_frames = _n_info_frames;	// number of consecutive frames store the management information

    ContFramePool::pool_list[ContFramePool::number_of_pool]=this;
    ContFramePool::number_of_pool ++;

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);       
        nFreeFrames -= 1;
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }
    
    // Number of frames must be "fill" the bitmap!
    assert ((n_frames % 4 ) == 0);
    
    
    // Everything ok. Proceed to mark all bits in the bitmap to FREE(00)
    for(int i=0; i*4 < _n_frames; i++) {
        bitmap[i] = 0x00;
    }

    // Mark the first frame as HEAD_OF_SEQUENCE(01)
    if (info_frame_no == 0){
        unsigned char mask1 = 0x40;	//HEAD_OF_SEQUENCE(01)
        bitmap[info_frame_no / 4] = bitmap[info_frame_no / 4] | mask1;
    }
    
    Console::puts("Contiguous Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // Any frames left to allocate?
    assert(_n_frames > 0);

    // If not enough space, return 0
    if (nFreeFrames < _n_frames) 
        return 0;
    
    unsigned int first_frame_no = base_frame_no;
    unsigned int virtual_frame_no = 0;
    unsigned char check_mask = 0xC0;
    unsigned int i = 0;
    unsigned int offset = 0;
    unsigned int available_frames = 0;  // Number of consecutive available frame

    //Find available FREE frames
    while (available_frames < _n_frames){ 

        //If there is a free frame, continue to check next frames                      
    	if ((bitmap[i] & check_mask) == 0x0){	

    		available_frames = 1;
    		unsigned int frame_count = 0;

            // Check if the next frames are free
    		for (int j = 0; j < _n_frames; j++){ 
                unsigned char check_mask2 = check_mask >> 2;
    		    if ( check_mask2 == 0x0){
    		        check_mask2 = 0xC0;
    		        frame_count ++;
    		    }
    		    if ((bitmap[i + ((offset + frame_count) / 4)] & check_mask2) == 0x0){
    		        check_mask2 = check_mask2 >> 2;
    		        available_frames++;
    		    } else {
    		        frame_count = 0;
    		        available_frames = 0;
    		        break;
    		    }
    		}
    	}
        if (available_frames < _n_frames){
            check_mask = check_mask >> 2;
            offset ++;
            if (check_mask == 0x0){
                check_mask = 0xC0;
                offset = 0;
                i++;
            }
        }
    }    

    virtual_frame_no = i * 4 + offset;
    first_frame_no += virtual_frame_no;
    
    unsigned char mask1 = 0x40 >> 2 * offset;	//HEAD_OF_SEQUENCE(01)
    unsigned char mask2 = 0xC0 >> 2 * offset;   //ALLOCATED(11)

    // Mark the first frame as HEAD_OF_SEQUENCE(01)
    bitmap[i] = bitmap[i] | mask1;

    //Set the following frames to ALLOCATED(11)    
    for (int j = virtual_frame_no+1; j < virtual_frame_no + _n_frames; j++) {        
        mask2 = mask2 >> 2;
    	if (mask2 == 0x0){
    		mask2 = 0xC0;
    	}
        bitmap[j / 4] = bitmap[j / 4] | mask2;
    }

    nFreeFrames -= _n_frames;
    
    return (first_frame_no);
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // Mark the first frame as HEAD_OF_SEQUENCE(01)
    unsigned char mask1 =  0x40 >> 2*(_base_frame_no % 4);
    bitmap[(_base_frame_no - base_frame_no)/4] |= mask1;

    //Set the following frames to ALLOCATED(11) 
    unsigned char mask2 = 0xC0 >> 2*(_base_frame_no % 4);
    for(int j = _base_frame_no + 1; j < _base_frame_no + _n_frames; j++){
        // Let's first do a range check.
    	assert ((j >= base_frame_no) && (j < base_frame_no + n_frames));
    
    	mask2 = mask2 >> 2;
        if (mask2 == 0x0){
            mask2 = 0xC0;
        }
        bitmap[(j - base_frame_no)/ 4] = bitmap[(j - base_frame_no) / 4] | mask2;
    }
    nFreeFrames -= _n_frames;
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    unsigned int pool_index = 0;
    // If first_frame is greater than kernel mem pool limit, it belong to process mem pool
    if (_first_frame_no >= (pool_list[0]->n_frames + pool_list[0]->base_frame_no)){
        pool_index = 1;
    }
    
    unsigned int bitmap_index = (_first_frame_no - pool_list[pool_index]->base_frame_no) / 4;
    unsigned char mask = 0xC0 >> 2*(_first_frame_no % 4);
    unsigned char head_bitmap = pool_list[pool_index]->bitmap[bitmap_index];
    head_bitmap &= mask;
    head_bitmap = head_bitmap << 2*(_first_frame_no % 4);

    if(head_bitmap != 0x40) {
        Console::puts("Error, Frame being released is not HEAD_OF_SEQUENCE\n");
        assert(false);
    }

    // Release first frame
    pool_list[pool_index]->bitmap[bitmap_index] &= ~mask;    
    pool_list[pool_index]->nFreeFrames++;

    // Release next frames in the queue
    do {
        mask = mask >>2;
        if (mask == 0x0){
            mask = 0xC0;
            bitmap_index ++;
        }        
        // Check if the frame is ALLOCATED
        if ((~pool_list[pool_index]->bitmap[bitmap_index] & mask) == 0x00){
            pool_list[pool_index]->bitmap[bitmap_index] &= ~mask;
            pool_list[pool_index]->nFreeFrames++;

        } else {break;}
    } while(true);

}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return _n_frames / (4*FRAME_SIZE) + (_n_frames % (4*FRAME_SIZE) > 0 ? 1 : 0); //Round up
}
