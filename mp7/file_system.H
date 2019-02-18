/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*-----------------------------------------------------*/

#define MB * (0x1 << 20)
#define SYSTEM_DISK_SIZE (10 MB)
#define BLOCK_SIZE 512
#define HEADER_SIZE 12
#define USED (-1)
#define FREE 0
#define N_BLOCKS SYSTEM_DISK_SIZE/BLOCK_SIZE
#define BLOCK_ARR_SIZE 125

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

class Block {
     public:
        unsigned int size, id, availability, data[BLOCK_ARR_SIZE];
        Block (unsigned int id,unsigned int availability,unsigned int size) {
           id = id;
           availability = availability;
           size = size;
          }
        Block () {}
};
/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

class File;

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

static unsigned char disk_buffer[BLOCK_SIZE];
static Block * block = (Block*) disk_buffer;
//extern FileSystem * FILE_SYSTEM;

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
     SimpleDisk * disk;
     unsigned int size;
     
     File * files;
     unsigned int block_no, n_files;
     void add_file(File * new_file);
     unsigned int modify_disk(unsigned int block_number, int change);

public:

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */
    bool LookupFile(unsigned int _file_id, File * _file);
    
    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */
   
};
#endif
