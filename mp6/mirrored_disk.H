/*
     File        : mirrored_disk.H

     Author      : Garuda Suma

     Date        : 
     Description : 

*/

#ifndef _MIRRORED_DISK_H_
#define _MIRRORED_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "blocking_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class mirrored_disk {

protected:

   DISK_ID disk_id_master;
   DISK_ID disk_id_slave;
   unsigned int disk_size;

   void issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID disk_id);
   virtual void wait_until_ready();
   bool is_ready();
	
public:

    mirrored_disk(DISK_ID _disk_id_master,DISK_ID _disk_id_slave, unsigned int _size); 
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */
    void read(unsigned long _block_no, unsigned char * _buf);
     void write(unsigned long _block_no, unsigned char * _buf);
};

#endif
