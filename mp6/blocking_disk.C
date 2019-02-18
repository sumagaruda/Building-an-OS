/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"

Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::wait_until_ready() 
{
  while (!is_ready()) {
/* Is called after each read/write operation to check whether the disk is ready to start transfering the data from/to the disk. */
/* In SimpleDisk, this function simply loops until is_ready() returns TRUE. In more sophisticated disk implementations, the thread may give up the CPU and return to check later. */
	SYSTEM_SCHEDULER -> add(Thread::CurrentThread());
	SYSTEM_SCHEDULER -> yield();
  }
}


void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	
	wait_until_ready();
/* Reads 512 Bytes in the given block of the given disk drive and copies them to the given buffer. No error check! */
	SimpleDisk::read(_block_no, _buf);
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) 
{

	wait_until_ready();
/* Writes 512 Bytes from the buffer to the given block on the given disk drive. */

  SimpleDisk::write(_block_no, _buf);
}
