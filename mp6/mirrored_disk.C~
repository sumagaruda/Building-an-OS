/*
     File        : simple_disk.c

     Author      : Riccardo Bettati
     Modified    : 10/04/01

     Description : Block-level READ/WRITE operations on a simple LBA28 disk 
                   using Programmed I/O.
                   
                   The disk must be MASTER or SLAVE on the PRIMARY IDE controller.

                   The code is derived from the "LBA HDD Access via PIO" 
                   tutorial by Dragoniz3r. (google it for details.)
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
#include "simple_disk.H"
#include "machine.H"
#include "mirrored_disk.H"
#include "thread.H"
#include "scheduler.H"
//#include "blocking_disk.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

Scheduler * Scheduler::SYSTEM_SCHEDULER = NULL;

mirrored_disk::mirrored_disk(DISK_ID _disk_id_master,DISK_ID _disk_id_slave, unsigned int _size) {
   disk_id_master = _disk_id_master;
   disk_id_slave = _disk_id_slave;
   disk_size = _size;
}

void mirrored_disk::issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id) {

  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
                         /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_id << 4));
                         /* send drive indicator, some bits, 
                            highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == READ) ? 0x20 : 0x30);

}

bool mirrored_disk::is_ready() {
   return ((Machine::inportb(0x1F7) & 0x08) != 0);
}

void mirrored_disk::wait_until_ready() 
{
  while (!is_ready()) {
/* Is called after each read/write operation to check whether the disk is ready to start transfering the data from/to the disk. */
/* In SimpleDisk, this function simply loops until is_ready() returns TRUE. In more sophisticated disk implementations, the thread may give up the CPU and return to check later. */
	Scheduler::SYSTEM_SCHEDULER -> add(Thread::CurrentThread());
	Scheduler::SYSTEM_SCHEDULER -> yield();
  }
}

void mirrored_disk::read(unsigned long _block_no, unsigned char * _buf) {
/* Reads 512 Bytes in the given block of the given disk drive and copies them 
   to the given buffer. No error check! */

  //master disk read
  issue_operation(READ, _block_no, disk_id_master);
  wait_until_ready();

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }

  //slave disk read
  issue_operation(READ, _block_no, disk_id_slave);
  wait_until_ready();

  /* read data from port */
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
}

void mirrored_disk::write(unsigned long _block_no, unsigned char * _buf) {
/* Writes 512 Bytes from the buffer to the given block on the given disk drive. */

  //master disk write
  issue_operation(WRITE, _block_no, disk_id_master);
  wait_until_ready();

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }

  //slave disk write
  issue_operation(WRITE, _block_no, disk_id_slave);
  wait_until_ready();

  /* write data to port */
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }

}
