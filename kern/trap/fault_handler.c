/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}


//Handle the page fault

// IMPLEMENTATION OF CLOCK

void clock(struct Env * curenv, uint32 fault_va)
{

    uint32 victim = 0;
    uint32 counter =curenv->page_last_WS_index;
    uint32 index_ws = counter;
    uint32 ws_maxSize = curenv->page_WS_max_size;
    uint32 Used_page_perm;

    uint32 i = 0 ;
    while(i < ws_maxSize)
    {
    	victim = env_page_ws_get_virtual_address(curenv,counter);
    	uint32 Used_page_perm = pt_get_page_permissions(curenv->env_page_directory,victim);
    	if ( (Used_page_perm &(PERM_USED) ) == 0 )
    	{
    	  index_ws = counter;
    	  break;
    	}
    	else
    	{
    	   pt_set_page_permissions( curenv->env_page_directory,victim , 0 , PERM_USED );
    	}

    	counter ++;

    	if ( counter == ws_maxSize )
    	{
    		counter = 0 ;
    	}


    	i++;
    }


    victim = env_page_ws_get_virtual_address(curenv,index_ws);
    Used_page_perm=pt_get_page_permissions(curenv->env_page_directory,victim);
    if ((Used_page_perm &(PERM_MODIFIED))==0)
    {
    	unmap_frame(curenv->env_page_directory,victim);
    }
    else
    {
    	uint32 *ptr_table;
        get_page_table(curenv->env_page_directory,victim,&ptr_table);

        struct FrameInfo *modframe_info= get_frame_info(curenv->env_page_directory,victim,&ptr_table);

        int updated=pf_update_env_page(curenv,victim,modframe_info);

        if (updated==E_PAGE_NOT_EXIST_IN_PF)
        {
         return;
        }

        unmap_frame(curenv->env_page_directory,victim);

    }
    env_page_ws_set_entry(curenv,index_ws,fault_va );
    curenv->page_last_WS_index=index_ws+1;
    if ( ws_maxSize == curenv->page_last_WS_index  )
    	curenv->page_last_WS_index = 0;

}





void page_fault_handler(struct Env * curenv, uint32 fault_va)
{

	//TODO: [PROJECT MS3] [FAULT HANDLER] page_fault_handler
	// Write your code here, remove the panic and write your code
	//panic("page_fault_handler() is not implemented yet...!!");

	// [1] allocated frame for faulted page
	struct FrameInfo *faultedPageFrameInfo = NULL;
	int alloc_frame = allocate_frame(&faultedPageFrameInfo);

	if (alloc_frame != E_NO_MEM)
	{
	uint32 perm = PERM_PRESENT | PERM_USER | PERM_WRITEABLE;
    map_frame(curenv->env_page_directory, faultedPageFrameInfo , fault_va, perm);

	//[2] Read an environment page from the page file to the main memory
	int read_env = pf_read_env_page(curenv, (void*)fault_va);
	if (read_env == E_PAGE_NOT_EXIST_IN_PF)
	{
		if ((fault_va > USTACKTOP || fault_va < USTACKBOTTOM ) && (fault_va > USER_HEAP_MAX || fault_va < USER_HEAP_START ))
		{
			unmap_frame(curenv->env_page_directory,fault_va);
			panic("  ILLEGAL MEMORY ACCESS  ");
		}
	}

    uint32 Ws_size = env_page_ws_get_size(curenv);
	uint32 ws_maxSize = curenv->page_WS_max_size;
	if(Ws_size < ws_maxSize)
    {
	  uint32 count;

		if(env_page_ws_is_entry_empty(curenv,curenv->page_last_WS_index))
		 {
	     	count = curenv->page_last_WS_index;
		 }
		else
		 {
			for (int i = 0; i <  curenv->page_WS_max_size; i++)
			 {
				if (env_page_ws_is_entry_empty(curenv,i))
				 {
				   curenv->page_last_WS_index = i;
			   	   count = i;
				   break;
				 }
		     }
		}
	env_page_ws_set_entry(curenv, count, fault_va);
	curenv->page_last_WS_index = count + 1;

	}
	else
	{
		if(isPageReplacmentAlgorithmCLOCK())
		{
		  clock(curenv,fault_va);
		}

	}
	if(curenv->page_last_WS_index == ws_maxSize)
	 {
		curenv->page_last_WS_index=0;
	 }

	}
}



void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	// Write your code here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");


}
