/*
 * dyn_block_management.c
 *
 *  Created on: Sep 21, 2022
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
uint32 svaLastAlloc=0; //For Next Fit alloc_Block_NF()
//===========================
// PRINT MEM BLOCK LISTS:
//===========================

void print_mem_block_lists()
{
	cprintf("\n=========================================\n");
	struct MemBlock* blk ;
	struct MemBlock* lastBlk = NULL ;
	cprintf("\nFreeMemBlocksList:\n");
	uint8 sorted = 1 ;
	LIST_FOREACH(blk, &FreeMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nFreeMemBlocksList is NOT SORTED!!\n") ;

	lastBlk = NULL ;
	cprintf("\nAllocMemBlocksList:\n");
	sorted = 1 ;
	LIST_FOREACH(blk, &AllocMemBlocksList)
	{
		if (lastBlk && blk->sva < lastBlk->sva + lastBlk->size)
			sorted = 0 ;
		cprintf("[%x, %x)-->", blk->sva, blk->sva + blk->size) ;
		lastBlk = blk;
	}
	if (!sorted)	cprintf("\nAllocMemBlocksList is NOT SORTED!!\n") ;
	cprintf("\n=========================================\n");

}

//********************************************************************************//
//********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//===============================
// [1] INITIALIZE AVAILABLE LIST:
//===============================
void initialize_MemBlocksList(uint32 numOfBlocks)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] initialize_MemBlocksList
	// Write your code here, remove the panic and write your code

    LIST_INIT(&AvailableMemBlocksList);
    for(int i = 0 ; i < numOfBlocks; i++){
    	LIST_INSERT_HEAD(&AvailableMemBlocksList, &(MemBlockNodes[i]));
    }

}

//===============================
// [2] FIND BLOCK:
//===============================
struct MemBlock *find_block(struct MemBlock_List *blockList, uint32 va)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] find_block
	// Write your code here, remove the panic and write your code
	struct MemBlock *blk_itr;
	struct MemBlock *found_block = NULL;

	LIST_FOREACH(blk_itr, blockList){

		if(blk_itr->sva == va)
			found_block = blk_itr;
	}

	return found_block;

}

//=========================================
// [3] INSERT BLOCK IN ALLOC LIST [SORTED]:
//=========================================
void insert_sorted_allocList(struct MemBlock *blockToInsert)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_allocList
	// Write your code here, remove the panic and write your code

	struct MemBlock *blk_itr;

	struct MemBlock *next;


	if(LIST_EMPTY(&(AllocMemBlocksList))) // Case 1 : IF List Is Empty
	{

		LIST_INSERT_HEAD(&(AllocMemBlocksList),  blockToInsert);
	}

	else
	{
		if(blockToInsert->sva  < LIST_FIRST(&(AllocMemBlocksList))->sva)   // Case 2 : IF blk_itr Less than all
		{
			LIST_INSERT_HEAD(&(AllocMemBlocksList),  blockToInsert);
		}
		else if(blockToInsert->sva  > LIST_LAST(&(AllocMemBlocksList))->sva) // Case 3 : IF blk_itr Greater than all
		{
			LIST_INSERT_TAIL(&(AllocMemBlocksList),  blockToInsert);
		}
		else
		{
			LIST_FOREACH(blk_itr, &(AllocMemBlocksList))
					{
				     next=LIST_NEXT(blk_itr);


					if(blockToInsert->sva >= blk_itr->sva + blk_itr->size   && (blockToInsert->sva) < next->sva)
						{
							LIST_INSERT_AFTER(&(AllocMemBlocksList), blk_itr, blockToInsert);
			                break;
						}
					}
		}


	}

}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
struct MemBlock *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_FF
	// Write your code here, remove the panic and write your code
	    struct MemBlock *blk_itr;
		struct MemBlock *new;
		char flag =0 ;
		struct MemBlock* element =LIST_FIRST(&(AvailableMemBlocksList));
		LIST_FOREACH(blk_itr, &(FreeMemBlocksList))
			{
			   if(blk_itr->size==size){
				   new=blk_itr;
				   LIST_REMOVE(&FreeMemBlocksList, blk_itr);

				   flag=1;
				   break;
			   }
			   if(blk_itr->size >size){

				   LIST_REMOVE(&AvailableMemBlocksList, element);
				   element->sva=blk_itr->sva;
				   element->size=size;
				   blk_itr->sva=blk_itr->sva + size;
				   blk_itr->size=blk_itr->size - size;
	                new=element;
	              flag=1;
	                break;

			   }

			}

		if(flag==0){

			new=NULL;
		}


return new;

}

//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
struct MemBlock *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] alloc_block_BF
	// Write your code here, remove the panic and write your code
	    struct MemBlock *blk_itr;
		struct MemBlock *BF_blk;
		struct MemBlock *temp ;
		struct MemBlock* element =LIST_FIRST(&(AvailableMemBlocksList));
		char flag =0 ;
		int index = 1;

		LIST_FOREACH(blk_itr, &(FreeMemBlocksList))
			{
			   if(blk_itr->size==size)
			   {
				   BF_blk=blk_itr;
				   LIST_REMOVE(&FreeMemBlocksList, blk_itr);

				   flag=1;
				   break;
			   }
			   if(blk_itr->size > size)
			   {
				   if(index == 1){
					   temp = blk_itr;
				   }

				   if(blk_itr->size < temp->size && temp->size > size){
					   temp = blk_itr;
				   }

				   flag=2;
				   index=2;

			   }


			}


		if(flag==2){

			 LIST_REMOVE(&AvailableMemBlocksList, element);
			          element->sva=temp->sva;
			 	      element->size=size;
			 		  temp->sva=temp->sva + size;
			 		  temp->size=temp->size - size;
			 		 BF_blk=element;
				}



		if(flag==0){

			BF_blk=NULL;
		}


return BF_blk;
}

uint32 LastAlloc=0;
//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
struct MemBlock *alloc_block_NF(uint32 size)
{
	//TODO: [PROJECT MS1 - BONUS] [DYNAMIC ALLOCATOR] alloc_block_NF
	// Write y our code here,remove the panic and write your code
	//panic("alloc_block_NF() is not implemented yet...!!");
	bool found=0;
	struct MemBlock*it;
	struct MemBlock*it2;
	struct MemBlock*tmp=LIST_FIRST(&AvailableMemBlocksList);

	LIST_FOREACH(it,&((FreeMemBlocksList)))
	{

		if (LastAlloc == 0)
							{

			if(it->size == size)
			{


				found=1;
                it2 = it;
                LastAlloc = it->sva;
				LIST_REMOVE(&FreeMemBlocksList,it);
				break;
			}
			else if(it->size > size)
			{
				found=1;

               LIST_REMOVE(&AvailableMemBlocksList,tmp);
                LastAlloc=it->sva;
				tmp->sva=it->sva;
				tmp->size=size;
				it->size-=size;
				it->sva+=size;
				it2 = tmp;
				break;
			}
}

else{


		if(it->sva > LastAlloc)
		{

		if(it->size == size)
		{
		    found=1;
		    it2 = it;

			LastAlloc=it->sva;
			LIST_REMOVE(&FreeMemBlocksList,it);
			break;
		}
		else if(it->size > size)
		{
            LIST_REMOVE(&AvailableMemBlocksList,tmp);
			found=1;
            LastAlloc=it->sva;
			tmp->sva=it->sva;
			tmp->size=size;
			it->size-=size;
			it->sva+=size;
			it2 = tmp;

                break;

		}

	}
}
	}
	if (LastAlloc != 0)
								{
if(found == 0){
it = NULL;
	LIST_FOREACH(it,&((FreeMemBlocksList)))
	{
			if(it->sva < LastAlloc)
			{

			if(it->size == size)
			{
			    found=1;
			    it2 = it;
LastAlloc=it->sva;
				LIST_REMOVE(&FreeMemBlocksList,it);


				break;
			}
			else if(it->size > size)
			{
				found=1;
	LIST_REMOVE(&AvailableMemBlocksList,tmp);
LastAlloc=it->sva;


				tmp->sva=it->sva;
				tmp->size=size;
				it->size-=size;
				it->sva+=size;
				it2=tmp;

				break;
			}
		}
           else{
            break;
               }

	}
 }
}

if(found == 0){
	it2= NULL;
}

	return it2;
}
//===================================================
// [8] INSERT BLOCK (SORTED WITH MERGE) IN FREE LIST:
//===================================================

void insert_sorted_with_merge_freeList(struct MemBlock *blockToInsert)
{
//cprintf("BEFORE INSERT with MERGE: insert [%x, %x)\n=====================\n", blockToInsert->sva, blockToInsert->sva + blockToInsert->size);
//print_mem_block_lists() ;

	//TODO: [PROJECT MS1] [DYNAMIC ALLOCATOR] insert_sorted_with_merge_freeList
	// Write your code here, remove the panic and write your code
	struct MemBlock * blk_itr;
	struct MemBlock * next;
	struct MemBlock * first = LIST_FIRST(&(FreeMemBlocksList));
	struct MemBlock * last = LIST_LAST(&(FreeMemBlocksList));

	if(LIST_EMPTY(&(FreeMemBlocksList)))
				{

					LIST_INSERT_HEAD(&(FreeMemBlocksList),  blockToInsert);

				}
	else
	{
		if( blockToInsert->sva  < first->sva )
		{
	                if( blockToInsert->sva + blockToInsert->size == first->sva)
	                {

	                	first->sva = blockToInsert->sva;
	                	first->size = first->size + blockToInsert->size;
	                	blockToInsert->size = blockToInsert->sva = 0;
	                	LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert );
	                }

	                else
	                {
	                	LIST_INSERT_HEAD(&(FreeMemBlocksList),  blockToInsert);
	                }
		}
	    else if(blockToInsert->sva > last->sva)
     	{

					if(last->sva + last->size == blockToInsert->sva)
					{

						last->size = last->size + blockToInsert->size;
						blockToInsert->size = blockToInsert->sva = 0;
						LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert );

					}

					else
					{
						LIST_INSERT_TAIL(&(FreeMemBlocksList),  blockToInsert);
					}


		}

	    else{

					LIST_FOREACH(blk_itr, &(FreeMemBlocksList))
					{
					     next=LIST_NEXT(blk_itr);


						if( (blockToInsert->sva > blk_itr->sva) )
							{
								if(((blk_itr->sva) + (blk_itr->size) < blockToInsert->sva && (blockToInsert->sva) + (blockToInsert->size) < next->sva))
								{
									LIST_INSERT_AFTER(&(FreeMemBlocksList), blk_itr, blockToInsert);
									break;

								}
								else if((blockToInsert->sva + blockToInsert->size) < next->sva  && blockToInsert->sva == (blk_itr->sva + blk_itr->size))
								{

									blk_itr->size=blk_itr->size+blockToInsert->size;
									 blockToInsert->size=blockToInsert->sva=0;
									 LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert );
									 break;

								}
								else if( (blockToInsert->sva + blockToInsert->size) == next->sva && blockToInsert->sva > (blk_itr->sva + blk_itr->size))
								{
									next->size = next->size + blockToInsert->size;
									next->sva = blockToInsert->sva;

									blockToInsert->size = blockToInsert->sva = 0;
									LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert );
									break;
								}
								else if ((blockToInsert->sva + blockToInsert->size) == next->sva && blockToInsert->sva == (blk_itr->sva + blk_itr->size))
								{
									blk_itr->size = blk_itr->size + blockToInsert->size + next->size;
									blockToInsert->size = blockToInsert->sva=0;
									next->size = next->sva = 0;
									LIST_REMOVE(&FreeMemBlocksList, next);
									LIST_INSERT_HEAD(&AvailableMemBlocksList, next );
								    LIST_INSERT_HEAD(&AvailableMemBlocksList, blockToInsert );
									break;
								}

							}
					}
	        }

   }

	    //cprintf("\nAFTER INSERT with MERGE:\n=====================\n");
		//print_mem_block_lists();

}
