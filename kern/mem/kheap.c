#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//==================================================================//
//==================================================================//
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)//
//==================================================================//
//==================================================================//


void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]

	 LIST_INIT(&AllocMemBlocksList);
     LIST_INIT(&FreeMemBlocksList);

#if STATIC_MEMBLOCK_ALLOC
	//DO NOTHING
#else


    MemBlockNodes = (void *)KERNEL_HEAP_START;

    MAX_MEM_BLOCK_CNT = NUM_OF_KHEAP_PAGES;

    int size_of_struct = sizeof( struct MemBlock);
    int size_of_MemBlock_array = size_of_struct * MAX_MEM_BLOCK_CNT;
    size_of_MemBlock_array = ROUNDUP(size_of_MemBlock_array, PAGE_SIZE) ;

    allocate_chunk(ptr_page_directory,KERNEL_HEAP_START,size_of_MemBlock_array,PERM_WRITEABLE);

#endif
	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes

    initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);

	//[4] Insert a new MemBlock with the remaining heap size into the FreeMemBlocksList
    struct MemBlock * first_Avablock = LIST_FIRST(&(AvailableMemBlocksList));

    first_Avablock->sva = (KERNEL_HEAP_START + size_of_MemBlock_array);
    uint32 Memblock_End = first_Avablock->sva;
    first_Avablock->size = ( KERNEL_HEAP_MAX - Memblock_End );
    LIST_REMOVE(&AvailableMemBlocksList, first_Avablock);
    LIST_INSERT_HEAD(&(FreeMemBlocksList),  first_Avablock);



}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kmalloc
	// your code is here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer


	     uint32 return_of_kmalloc = 0 ;
         size = ROUNDUP(size ,PAGE_SIZE);
		if(isKHeapPlacementStrategyFIRSTFIT()){

		  struct MemBlock * ff_block=alloc_block_FF(size);
		  if(ff_block != NULL){

			uint32 start_ff = ff_block->sva;
			uint32 end_ff =  ff_block->size;
			int alloc_ff_ret= allocate_chunk(ptr_page_directory, start_ff , end_ff ,PERM_WRITEABLE);
			if( alloc_ff_ret == 0)
			{

				insert_sorted_allocList(ff_block);
				return_of_kmalloc = start_ff;
			}
			else
		    {
				insert_sorted_with_merge_freeList(ff_block);
			}

		  }

		}

		if(isKHeapPlacementStrategyBESTFIT()){

			struct MemBlock * bf_block=alloc_block_BF(size);
			  if(bf_block!=NULL){
				  uint32 start_bf = bf_block->sva;
				  uint32 end_bf= bf_block->size;

				int alloc_BF_ret= allocate_chunk(ptr_page_directory,start_bf,end_bf,PERM_WRITEABLE);

				if(alloc_BF_ret==0){

					insert_sorted_allocList(bf_block);
					return_of_kmalloc = start_bf;

				}
				else
				{
					insert_sorted_with_merge_freeList(bf_block);

				}

			  }

			}

		if(isKHeapPlacementStrategyNEXTFIT()){
					struct MemBlock * NF_block=alloc_block_NF(size);
					  if(NF_block != NULL)
					  {
						  uint32 start_NF = NF_block->sva;
						  uint32 end_NF = NF_block->size;
						int alloc_NF_ret= allocate_chunk(ptr_page_directory,start_NF,end_NF,PERM_WRITEABLE);
						if(alloc_NF_ret==0){

							insert_sorted_allocList(NF_block);
							return_of_kmalloc = start_NF;

						}
						else
						{
							insert_sorted_with_merge_freeList(NF_block);

						}

					  }

					}

return (void *) return_of_kmalloc;

}
void kfree(void* virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	struct MemBlock * kfree_block= find_block(&AllocMemBlocksList,(uint32 )virtual_address);
	if(kfree_block != NULL){

		LIST_REMOVE(&AllocMemBlocksList,kfree_block);

		uint32 start_kf = ROUNDDOWN(kfree_block->sva,PAGE_SIZE);

		uint32 kfree_blk_end = kfree_block->sva + kfree_block->size;

		uint32 end_kf= ROUNDUP(kfree_blk_end,PAGE_SIZE);

		for(uint32 i = start_kf ; i < end_kf ; i+=PAGE_SIZE ){

			unmap_frame(ptr_page_directory,i);

		}

		insert_sorted_with_merge_freeList(kfree_block);
	}

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

    struct FrameInfo * ptr_frame_info ;
    ptr_frame_info = to_frame_info(physical_address);

    uint32 virtual_adress = ptr_frame_info->va;
    return virtual_adress;

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT MS2] [KERNEL HEAP] kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	int Physical_adress =virtual_to_physical(ptr_page_directory,virtual_address);
	 return Physical_adress;


}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT MS2 - BONUS] [KERNEL HEAP] krealloc
	// Write your code here, remove the panic and write your code
	//panic("krealloc() is not implemented yet...!!");
	void * Kreal_ret = NULL;
	kfree(virtual_address);
	Kreal_ret = kmalloc(new_size);

	return Kreal_ret;
}
