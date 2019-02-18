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

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/* METHODS FOR CLASS   C o n t F r a m e P o o l */

    ContFramePool * ContFramePool::head = NULL;
     ContFramePool* ContFramePool::tail = NULL;

    //#define MB * (0x1 << 20)
    //#define KB * (0x1 << 10)

    //#define KERNEL_POOL_START_FRAME ((2 MB)/(4 KB))
    //#define KERNEL_POOL_SIZE ((2 MB)/(4 KB))
    //#define PROCESS_POOL_START_FRAME ((4 MB)/(4 KB))
    //#define PROCESS_POOL_SIZE ((28 MB)/(4 KB))
    
    // bitmap allocations
    // first bit is to indicate whether it is head of sequence
    // second bit to indicate if it is used or free
    // 00 - free
    // 01 - allocated
    // 11 - head of sequence and allocated

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{  
   void mark_inaccessible(unsigned long _frame_no);
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    //Console::puti(nframes);
    nFreeframes = _n_frames;
    n_info_frames = _n_info_frames;
    
    ContFramePool* next = NULL;
    ContFramePool* prev = NULL;
    // Assign bitmap to base_frame_no if _info_frame_no = 0
    // else assign it to the frame mentioned in _info_frame_no
    if(_info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }
    
    //initialize all disk blocks as available
    //we don't multiply coz our implementation is 1 bitmap each frame
    for(int i=0 ; i < _n_frames; i++) {
        bitmap[i] = 0x00;
    }
   
    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        bitmap[0] = 0xFF;
        nFreeframes--;
    }
  
    //Console::puts("Are we reaching here?");
    //initialize the frame pool linked list
  
   if (ContFramePool::head == NULL) {
     ContFramePool::head = this;
     ContFramePool::tail = this;

     next = NULL;
     prev = NULL;
   }
   else {
    ContFramePool::tail->next = this;
    this -> prev = tail;
    ContFramePool::tail = this;
    }
//Console::puti(nFreeframes);
Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{   
    //Console::puti(nFreeframes);
    //Console::puti(_n_frames);
    // Any frames left to allocate?
    assert(nFreeframes > _n_frames);
    //nframes = _n_frames;
    Console::puti(nframes);
    // Find a frame that is not being used and return its frame index.
    // Mark that frame as being used in the bitmap.
    unsigned int frame_no = base_frame_no;
    unsigned int frame_count=_n_frames;
    unsigned int head =0,frames_acquired=0,i =0; 
    do{
        //Traverse bitmap to find first free entry
         while (bitmap[i] == 0xFF || bitmap[i]== 0x55|| bitmap[i]== 0xaa) {
             i++;
         }
         head=i;
         //check for n subsequent free blocks
         while (frame_count > 0) {
             if(bitmap[i] == 0xFF || bitmap[i]== 0x55 || bitmap[i]== 0xaa){
                 i++;
                 break; 
             }
             else{
                 if( bitmap[i]== 0x00){
                     i++;
                     frame_count--;
                     frames_acquired+=1;
                 }
             }
         }
    // If the acquired size is less than the _n_frames; traverse bitmap again
    }while (frames_acquired < _n_frames); 
    
    //bitmap index to frame conversion
    frame_no += head;
    //Mark bitmap entry as head of sequence 
    bitmap[head]=0xFF;
    //Mark subsequent _n_frames-1 entries as busy
    while (frame_count < _n_frames-1)
    {
        frame_count++;
        bitmap[head+frame_count]=0x55;
    }
   
   //reduce the number of free frames available
   int j;
   for(j=0;j<_n_frames;j++){
        nFreeframes--;
   }
    // output frame no
    //Console::puts("frame number output:");Console::puti(frame_no);Console::puts("\n"); 
     //Console::puts("get frame successful");
return frame_no;
    
} 

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    unsigned long j =0;
    Console::puts("We are in mark_inaccessible \n");
    for(j = _base_frame_no; j<_base_frame_no + _n_frames; j++) {
       mark_inaccessible(j);
    }
    nFreeframes -= _n_frames;
}

void ContFramePool::mark_inaccessible (unsigned long _frame_no)
   {
    //range check
    //assert((_frame_no >= base_frame_no) && (_frame_no < base_frame_no + nframes));

   unsigned int bitmap_index = (_frame_no - base_frame_no);
   
   // Is the frame being used already?
   assert((bitmap[bitmap_index] !=0x55) || (bitmap[bitmap_index] !=0xFF));
   
   //update bitmap as frames being allocated
   bitmap[bitmap_index] = 0x55;
   nFreeframes--;
   }

bool ContFramePool::outofbounds (unsigned long _first_frame_no)
 {
   //Console::puts("We're in outofbounds");
    //Console::puti(_first_frame_no);
    //Console::puti(base_frame_no);
    //Console::puti(nframes);
   //Console::puts("\n");
   if ((_first_frame_no >= base_frame_no) && (_first_frame_no <= nframes +  base_frame_no))
    {
     //Console::puts("bound condition satisfied is satisfied");
     return true;
    }
   return false;
 }

void ContFramePool::release_frame(unsigned long _first_frame_no)
 {
   unsigned long l = _first_frame_no - base_frame_no;
   int released_frame = 0;     
 //now check if the requested frame is head of sequence
       if (bitmap[l] == 0xFF) {
          //mark it as free
          bitmap[l] = 0x00;
          l++;
          released_frame++;
          nFreeframes++;
       }
       else {
       //if the frame is not head of sequence, something went wrong
             return;
       }
       
       // if the frames after _first_frame_no are allocated, 
       // we need to release them until we get free or head of
       // sequence frames (releasing the frame pool)
       while (bitmap[l] == 0x55) {
              //mark the frame as free
              bitmap[l] = 0x00;
              l++;
              released_frame++;
              nFreeframes++;
       }     
    //Console::puts("released_frame: "); 
    //Console::puti(released_frame);
    }

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    //assert(false);
    //first we need to identify if the frame belongs to the  
    //process or kernel frame pool
    
   /* // check process frame space defined in kernel.C
    unsigned long frameno_processpool;
    unsigned long frameno_kernelpool;
  
    
if ((_first_frame_no >= PROCESS_POOL_START_FRAME) && (_first_frame_no < (PROCESS_POOL_START_FRAME + PROCESS_POOL_SIZE))) {
       
       frameno_processpool = (_first_frame_no - PROCESS_POOL_START_FRAME);
       
       //tried calling a non static function to access bitmap
       //did not work 
       //release_frame(frameno_processpool);
       unsigned long l = frameno_processpool;
        
 //now check if the requested frame is head of sequence
       if (bitmap_1[l] == 0xFF) {
          //mark it as free
          bitmap_1[l] = 0x00;
          l++;
       }
       else {
       //if the frame is not head of sequence, something went wrong
             return;
       }
       
       // if the frames after _first_frame_no are allocated, 
       // we need to release them until we get free or head of
       // sequence frames (releasing the frame pool)
       while (bitmap[l] == 0x55) {
              //mark the frame as free
              bitmap[l] = 0x00;
              l++;
       }      
    }
// check kernel frame space defined in kernel.C
    else if ((_first_frame_no >= KERNEL_POOL_START_FRAME) && (_first_frame_no < (KERNEL_POOL_START_FRAME + KERNEL_POOL_SIZE))) {
       
       frameno_kernelpool = (_first_frame_no - KERNEL_POOL_START_FRAME);
       
       //release_frame(frameno_kernelpool);
       unsigned long l = frameno_kernelpool;
        
 //now check if the requested frame is head of sequence
       if (bitmap[l] == 0xFF) {
          //mark it as free
          bitmap[l] = 0x00;
          l++;
       }
       else {
       //if the frame is not head of sequence, something went wrong
             return;
       }
       
       // if the frames after _first_frame_no are allocated, 
       // we need to release them until we get free or head of
       // sequence frames (releasing the frame pool)
       while (bitmap[l] == 0x55) {
              //mark the frame as free
              bitmap[l] = 0x00;
              l++;
       }      
    } */ 

   //let's try to use the linkedlist approach so that we can access bitmap
     
     //Console::puts("We are in release frames \n");
     ContFramePool * pool_pointer = head;
     //Console::puts("\n");
     //Console::puti (_first_frame_no);
    //Console::puts("\n");
    //Console::puti(pool_pointer -> base_frame_no);
    //Console::puts("\n");
    //Console::puti(pool_pointer -> nframes);

      while (pool_pointer != NULL) {
         //Console::puts ("Are we even entering the while loop");
         if (pool_pointer -> outofbounds(_first_frame_no) == true)
           {
             //Console::puts("We are in the if statement");
               pool_pointer -> release_frame(_first_frame_no);
               //Console::puts("successfully released frame pool \n");
               pool_pointer = pool_pointer -> next; 
             }
         else {
                pool_pointer = pool_pointer -> next;
            if (pool_pointer == NULL)
            { break; }
             Console::puts("Couldn't find the frame in either of the pools \n");
     }
  }
}     

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    //since we use a char to represent the state of each frame
    //we need one info frame for each FRAME_SIZE frames

    unsigned long mgmt_frames = (_n_frames/4096) + (_n_frames % 4096 >0 ? 1:0);
    return(mgmt_frames);

}
