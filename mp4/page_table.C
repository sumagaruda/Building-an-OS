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
VMPool* PageTable::registered_pools[];
unsigned int PageTable::vmpool_no;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   vmpool_no = 0;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   //Give page directory a frame to store entries
   page_directory = (unsigned long *)(process_mem_pool -> get_frames(1)*PAGE_SIZE);
   if (page_directory == 0)
   {
     Console::puts("Frame not available in kernel pool - page directory request");
   }

  //frame for first page table mapping 0-4MB
   unsigned long *page_table = (unsigned long *)(process_mem_pool -> get_frames(1)*PAGE_SIZE);

   unsigned int i;
   unsigned long address = 0;
   vmpool_no = 0;
   
   //initialize the page table
   //ENTRIES_PER_PAGE = 1024
   //Implementation same as K.J.'s tutorial
   for (i=0; i<ENTRIES_PER_PAGE; i++) 
   {
   page_table[i] = address|3; //setting attributes as supervisor level, read/write and present
   address = address + PAGE_SIZE;
   }
   
   //now we have to fill the page directory entries
   //fill the first entry of the page directory with the page table
   page_directory[0] = ((unsigned long)page_table)|3; //setting attributes as supervisor level, read/write and present
    
    //There are no page tables for the remaining 1023 entries
    for (i=1; i< ENTRIES_PER_PAGE-1; i++)
    {
    page_directory[i] = 0 | 7; 
    } 
  
  //for recursive lookup
  page_directory[ENTRIES_PER_PAGE - 1] = ((unsigned long) page_directory)|3;
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   //put the address of the page directory into CR3
   write_cr3((unsigned long)current_page_table -> page_directory);
    //write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //set the paging bit i.e. CR0 to 1
   write_cr0(read_cr0()|0x80000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   ContFramePool *current_mem_pool;
   //the address that caused the page fault is stored in CR2
  unsigned long pagefault_address = read_cr2();
  unsigned int error_code = (_r -> err_code & 7); //we need the last 3 bits  
  if (error_code == 1)
  {
    Console::puts("Protection fault \n");
  }
 
  //Part II-3
  //adding support for region check in page fault handler
  int i=0;
   for (i=0; i<current_page_table->vmpool_no;i++)
   {
    if (current_page_table->registered_pools[i]->is_legitimate(pagefault_address))
     {
       current_mem_pool = current_page_table->registered_pools[i]->get_frame_pool();
       break;
     }
   }

  if (i == current_page_table->vmpool_no) 
   {
    Console::puts("pagefault_address was not found in any memory pool");
    while(1);
    }

  unsigned long *pagedir_entry = NULL;
  unsigned int pagedir_index = (pagefault_address/(ENTRIES_PER_PAGE * PAGE_SIZE)) ; //first 10 bits - >> 22
  unsigned int pagetable_index = ((pagefault_address/PAGE_SIZE)
 & 0X3FF);  // next 10 bits - >>12
  unsigned long *pagedir = (unsigned long*)(current_page_table -> page_directory);
  unsigned long *page_table = (unsigned long *)((0x3FF<<22) + (pagedir_index<<12));
  
  if ((error_code & 1) == 0) //page is not present in memory
  {  
  //Page Directory entry is empty
  if ((pagedir[pagedir_index] & 1) == 0)
  {
  pagedir[pagedir_index] = (unsigned long)(process_mem_pool -> get_frames(1)*PAGE_SIZE)|7;  //replaced 3 with 7
    //this operation ensures that the page table starts at 4Kb and the pointer points to the page table
  page_table = (unsigned long *) ((0x3FF<<22) + (pagedir_index<<12)); //edited here

  for (unsigned int i=0; i< ENTRIES_PER_PAGE-1; i++)
  {
    //page_table[i] = 0 | 2; //initializing all page table entries to empty
  //loads page into page table
  page_table[i] = (unsigned long)(process_mem_pool -> get_frames(1) * PAGE_SIZE) | 7;
}
}

 else if ((pagedir[pagedir_index] & 1) == 1) //page is in memory
   {
     // gets the page table
    page_table = (unsigned long *)((0X3FF<<22)+(pagedir[pagedir_index] <<12));
    page_table[pagetable_index] = (process_mem_pool -> get_frames(1) * PAGE_SIZE) | 7;
}

 else {
  switch(_r -> err_code) {
  case 1:
         Console::puts("Protection fault caused due to supervisory process trying to read a page");
         break;
   case 3:
         Console::puts("Protection fault caused due to supervisory process trying to write a page");
         break;
   case 5:
         Console::puts("Protection fault caused due to user process trying to read a page");
         break;
   case 7:
         Console::puts("Protection fault caused due to user process trying to write a page");
         break;
   default:
         break;
   
  Console::puts("handled page fault\n");
}
} 
}
}
void PageTable::register_pool(VMPool * _vm_pool)
{
    //Part II - 1
    //maintains a list of registered pools
    registered_pools[vmpool_no] = _vm_pool;
    vmpool_no++;
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    //Part II-2
    //requesting the release of previously allocated page
  unsigned long pagedir_index = _page_no/ENTRIES_PER_PAGE; // >>10
  unsigned long pagetable_index = _page_no%ENTRIES_PER_PAGE;
  unsigned long *page_table = (unsigned long *)((0x3FF<<22) + (pagedir_index<<12));

    unsigned long frame_no;
    //page no is now 20 bits
    frame_no = (page_table[pagetable_index] & 0xFFFFF000)/PAGE_SIZE;

    if ((page_table[pagetable_index] & 1) == 0)
     {
       Console::puts("Page to be released is invalid");
       assert(false);
     }
    //else page is valid
    ContFramePool::release_frames(frame_no);
    //mark the last bit of the page table entry as invalid
    page_table[pagetable_index] = page_table[pagetable_index] & 0xFFFFFFFE;
    
   //reloading CR3 register with current value to flush the TLB 
    write_cr3(read_cr3());
    Console::puts("freed page\n");
}
