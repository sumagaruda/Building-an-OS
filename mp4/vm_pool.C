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
               PageTable     *_page_table) 
 {
    base_address = _base_address;
    Console::puti(base_address);
    Console::puts("\n");
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    max_address = _base_address + _size;
    
    block = 0;
    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    
    block_info* block_base = (block_info*) base_address;
    //Console::puti(sizeof(block_info));

    if (block == 0) {
      Console::puts("We're creating a new VM block\n");
      block++;
      //Console::puti(size);
      //Console::puts("\n");
      //Console::puti(sizeof(block_info));
      block_base->block_size = size - sizeof(block_info);
      //Console::puts("Are we reaching here"); 
      block_base->available = 1; //true 
    }

    unsigned int allocate = 0; //false
    unsigned int i;
   //finding an available block to allocate
    for (i=0; i< block; i++)
     {
       if ((block_base->block_size) >= _size && (block_base->available == 1))
        {
          allocate = 1;
          Console::puts("Found area to allocate\n");
          break;
        }
      else 
       {
         allocate = 0;
         Console::puts("did not find a block of _size bytes to allocate\n");
       return 0;
     }
      //allocated
      block_base = (block_info*)((unsigned int)block_base + block_base->block_size + sizeof(block_info));
    }
    //Virtual address of the start of the alloacted region of memory
     unsigned long block_address = (unsigned long)(block_base) + sizeof(block_info);
   Console::puts("Allocated region of memory.\n");
   
    //if we did not find a block of _size bytes to allocate, return 0
    if (allocate == 0)
     {
       Console::puts("did not find a block of _size bytes to allocate\n");
       return 0;
     }
   
     block_base -> available = 0;
      //new block is created when size of block > size of block to be allocated - we're using the first fit approach
     if ((block_base->block_size - _size) > sizeof(block_info))
      {
        block_info* new_block_base = (block_info*)((unsigned long)(block_address + _size));
        new_block_base->block_size = block_base->block_size - _size - sizeof(block_info);
        new_block_base->available = 1;
        block_base->block_size = _size;
        block++;    
       Console::puts("new block is created with the remaining memory of the block\n");
      }

//Contiguously allocated memory
block_base = (block_info*) base_address;
for (unsigned int i =0; i < block; i++)
  {
   //Console::puts("Are we ever coming into this loop; we should");
    block_base = (block_info*)((unsigned long)block_base + block_base->block_size + sizeof(block_info));
  }
  //Find out why the loop stops after i = 10
     return block_address;
}

void VMPool::release(unsigned long _start_address) 
{
    block_info* block_base = (block_info*)base_address;

    unsigned int i;
    for (i=0; i< block; i++)
     {
      unsigned long block_address = (unsigned long) block_base + sizeof(block_info);

      if (block_address > _start_address)
       {
         //check next block
         break;
       }
   
      if (block_address != _start_address)
       {
         block_base = (block_info*)((unsigned long)(block_address + block_base->block_size));
         continue;
       }
      
      block_base -> available = true;
      //release block and merge blocks of memory
      block_info* new_block_base = (block_info*)(block_address + block_base->block_size);
      
      if ((new_block_base->available == true ) && (i < block + 1))
       {
        block_base -> block_size = block_base -> block_size +
        (new_block_base -> block_size + sizeof(block_info));
         block --;
        Console::puts("Released block and merged blocks of memory\n");
       }
    //Console::puts("Released region of memory.\n");
    return;
}
    Console::puts("There was some problem releasing memory");
}

bool VMPool::is_legitimate(unsigned long _address) {
    if (_address > base_address && _address < max_address)
     {
       return true;
     }
    else 
     {
       return false;
     }
    Console::puts("Checked whether address is part of an allocated region.\n");
}

ContFramePool * VMPool::get_frame_pool()
  {
    return frame_pool;
  }


