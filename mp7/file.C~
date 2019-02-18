/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
   //Console::puts("In file constructor\n");
   file_id = 0;
   file_size = 0;
   cur_blk = 0;
   cur_pos = 0;   
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) 
{
    Console::puts("reading from file\n");
    unsigned int i;
    for (i=_n; i> 0; i--)
      {
    
	FILE_SYSTEM->disk->read(blocks[cur_blk], disk_buffer);
        for (cur_pos; cur_pos < BLOCK_SIZE - HEADER_SIZE; cur_pos++)
	  { 
            if (i == 1) { break; }
            memcpy(_buf, disk_buffer + HEADER_SIZE, 1);
	    _buf ++;
	    i--;
            Console::puti(i);
	  }
	if (cur_pos == (BLOCK_SIZE - HEADER_SIZE))
	  {
	     cur_pos = 0;
	     cur_blk++;
          }
        
	if (EoF())
	   { return 0;  }

        Console::puti(_n-i);
	return (_n-i);
   }
}

void File::Write(unsigned int _n, const char * _buf) 
{
    Console::puts("writing to file\n");
    for (unsigned int i = _n; i> BLOCK_SIZE - HEADER_SIZE; i = i - (BLOCK_SIZE - HEADER_SIZE))
      {
	 if (EoF())
	   {
	     unsigned int new_block_no = FILE_SYSTEM -> modify_disk(0,0);
	     //allocate blocks
	     unsigned int * new_block_ar = (unsigned int *) new unsigned int[file_size + 1];
	     for (int j=0; j< file_size; j++) //copy the old list
		{  new_block_ar[j] = blocks[j];}
	     if (blocks != NULL)
	   	{  new_block_ar[file_size] = new_block_no;}
             else
                { new_block_ar[0] = new_block_no;}
	     file_size++; 
	     delete blocks;
	     blocks = new_block_ar;
           }
	memcpy((void*) (disk_buffer + HEADER_SIZE), _buf, (BLOCK_SIZE - HEADER_SIZE));
	FILE_SYSTEM->disk->write(blocks[cur_blk], disk_buffer);
     }
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    cur_blk = 0;
    cur_pos = 0;
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    for (cur_blk = 0; cur_blk < file_size; cur_blk++)
      {
	FILE_SYSTEM->modify_disk(blocks[cur_blk],1); //deallocate blocks
      }
     cur_blk = 0;
     cur_pos = 0;
     file_size = 0;
}

bool File::EoF() 
{
    Console::puts("testing end-of-file condition\n");
    if (blocks == NULL)
	{  
	  Console::puts ("blocks == NULL\n");
          return true; 
	}
    if (cur_blk +1 == file_size)
	{ Console::puts("cur_blk +1 == file_size\n");
	  return true;  }
    if (cur_pos +1 == BLOCK_SIZE - HEADER_SIZE)
        { return true;  }
    else
	{ return false; }
}
