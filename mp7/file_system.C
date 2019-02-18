/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
   Console::puts("In file system constructor.\n");
    block_no = 0;
    n_files = 0;
    files = NULL;
    memset(disk_buffer, 0, BLOCK_SIZE);
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) 
{
    Console::puts("associating file system with a disk\n");
    disk = _disk; 
    n_files = block->size;
    for (unsigned int i=0; i<n_files; i++)
       {
        //we make buffer point to the root node of the system
           disk->read(0, disk_buffer); 
	//file inode is put in buffer
           disk->read(block->data[i], disk_buffer);
        //create a new file
	   File * new_file= new File();

	new_file->file_size = block->size;
	new_file->file_id = block->id;
        add_file(new_file);
       }  
        return true;
}

void FileSystem::add_file(File * new_file)
{
   Console::puts("Adding a file\n");
   if (files == NULL)
      {  files = new_file; return;}
   else
    {
   File * new_array = (File *)new File[n_files + 1];
   //setting the new index to the new block no
   new_array[n_files + 1] = *new_file;
 
   for (unsigned int i = 0; i< n_files; i++)
       {
	//here we will just copy the old array
	new_array[i] = files[i];
       }

   delete files; //deleting the old array
   files = new_array; //setting the pointer to a new array
   n_files++; //increasing the file size by 1
        }
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) 
{
    Console::puts("formatting disk\n");
    FILE_SYSTEM ->disk = _disk;
    //to wipe any file system from disk, we set the entire disk to 0
    memset(disk_buffer, 0, BLOCK_SIZE);

    for (unsigned int i=0; i < N_BLOCKS; i++)
	{
            _disk->write(i, disk_buffer);
        }

    block -> availability = USED;
    block-> size = 0;
    //initializes the master block
    _disk->write(0, disk_buffer);

return true;
}

File * FileSystem::LookupFile(int _file_id) 
{
    Console::puts("looking up file\n");
    for (unsigned int i= 0; i< n_files + 1; i++)
	{
	   if (files[i].file_id == _file_id || files[i].file_id+1 == _file_id)
	      {
		 Console::puts("found\n");
	         return &files[i];
	      }
	}
     return NULL;
}

bool FileSystem::LookupFile(unsigned int _file_id, File * _file)
{
   for (unsigned int i=0; i<n_files+1; i++)
     {
	if(files[i].file_id == _file_id)
	   {
		* _file = files[i];
		return true;
	   }
   return false;
     }
}

unsigned int FileSystem::modify_disk(unsigned int block_number, int change)
{
  if (change == 0) //allocate
    {
      if (block_number == 0)
        { unsigned int i;
	  for (i=0; i< N_BLOCKS; i++)
            {
              disk->read(i, disk_buffer);
              if (block->availability != USED)
                { break; }
            }
              if (i == N_BLOCKS)
                { return 0; }
              disk->read(i,disk_buffer);
              block->availability = USED;
              disk->write(i, disk_buffer);
              
              block_number = i;
              return i;
        }
       else
         {
           disk->read(block_number, disk_buffer);
           block->availability = USED;
           disk->write(block_number, disk_buffer);
           return block_number;
         }
      }
       else 
         {
	   disk->read(block_number, disk_buffer);
	   block->availability= FREE;
	   disk->write(block_number, disk_buffer);
         }
}
	 
bool FileSystem::CreateFile(int _file_id) 
{
    Console::puts("creating file\n");
    File * new_file = new File();

   //check if file exists, if it does, returns false
   if (LookupFile(_file_id, new_file))
      { return false; }
   //if it doesn't we have to create new file

   new_file->file_id = _file_id;
   new_file->file_size = 0;
   new_file->availability = FREE;
   new_file->Rewrite(); //clears all data and sets the fields to 0

   new_file->inode_block_no = modify_disk(0,0);
   //2nd argument is for allocation (0)/deallocation (1)
   //modify_disk is a new function to handle allocating data

   disk->read(new_file->inode_block_no, disk_buffer);
   //this loads the inode block in buffer

   block->availability = USED; //set block to used
   block->size = 0; //set size to 0
   block->id = _file_id;
   
   disk->write(new_file->inode_block_no, disk_buffer);
   //write the inode block to disk

   add_file(new_file);
   Console::puts("File created \n");
   return true;
}

bool FileSystem::DeleteFile(int _file_id) 
{
    Console::puts("deleting file\n");  
    if (LookupFile(_file_id) == NULL) 
      { return false; }

    File * new_array = new File[n_files];
    bool found = false;
    for (unsigned int i=0; i< n_files; i++)
      {
	if (files[i].file_id == _file_id)
	  {
	    found = true;
            files[i].Rewrite(); //clears all data and sets all fields to 0
	    modify_disk(files[i].inode_block_no, 1);
            //deallocate inode of files
	   }
	if (found == false)
	  {
	    new_array[i] = files[i];
	    n_files--;
	  }
        else
	  { new_array[i] = files[i+1]; }
	delete files;
	files = new_array;
	return true;
      }
}
