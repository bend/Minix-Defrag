#include "fs.h"
#include "buf.h"
#include "inode.h"
#include "super.h"
#include <minix/vfsif.h>
#include <stdio.h>


/*===========================================================================*
 *				nb_frags		              	     *
 *===========================================================================*/
PUBLIC int nb_frags(rip)
register struct inode *rip;     /* inode pointer */
{
  int nfrags;                   /* number of fragments of the file*/
  off_t pos;                    /* position in the file (for iteration)*/
  block_t block_number,         /* iteration variables*/
  previous_block_number;

  nfrags = 0;
  /* count number of fragments */
  for (pos=0; pos<rip->i_size; pos+=rip->i_sp->s_block_size){                               
  	block_number = read_map(rip,pos);
    	if (block_number!=previous_block_number){
      		if (block_number-previous_block_number>1 || block_number-previous_block_number<0) {
			nfrags++; /* if the current block is distant of 2 to the previous, we have a fragment */
		}
    	}	
    	previous_block_number=block_number;
  }
  return(nfrags);
}
