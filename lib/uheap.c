#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
	//panic("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);
	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)

    MemBlockNodes = (void *)USER_DYN_BLKS_ARRAY;

    MAX_MEM_BLOCK_CNT = NUM_OF_UHEAP_PAGES;

    int size_of_struct = sizeof( struct MemBlock);
    int size_of_MemBlock_array = size_of_struct * MAX_MEM_BLOCK_CNT;
    size_of_MemBlock_array = ROUNDUP(size_of_MemBlock_array, PAGE_SIZE) ;

	sys_allocate_chunk(USER_DYN_BLKS_ARRAY,size_of_MemBlock_array,PERM_WRITEABLE | PERM_USER);

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes

    initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);

	//[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList

    struct MemBlock * first_Avablock = LIST_FIRST(&(AvailableMemBlocksList));

    first_Avablock->sva = (USER_HEAP_START);
    first_Avablock->size = ( USER_HEAP_MAX -  first_Avablock->sva );
    LIST_REMOVE(&AvailableMemBlocksList, first_Avablock);
    LIST_INSERT_HEAD(&(FreeMemBlocksList),  first_Avablock);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================


struct MemBlock * first_fit_strategy(uint32 size)
{
	size = ROUNDUP(size ,PAGE_SIZE);
	int no_of_pages=NUM_OF_UHEAP_PAGES;
	int counter = 0;
	struct MemBlock *blk_itr;
	struct MemBlock *new;
	char flag =0 ;
	struct MemBlock* element =LIST_FIRST(&(AvailableMemBlocksList));
	LIST_FOREACH(blk_itr, &(FreeMemBlocksList))
		{
			if(blk_itr->size==size)
			{
			new=blk_itr;
			LIST_REMOVE(&FreeMemBlocksList, blk_itr);

				 flag=1;
				 break;
		   }
		   if(blk_itr->size >size)
		   {

			 LIST_REMOVE(&AvailableMemBlocksList, element);
			 element->sva=blk_itr->sva;
			 element->size=size;
			 blk_itr->sva=blk_itr->sva + size;
			 blk_itr->size=blk_itr->size - size;
			 new=element;
			 flag=1;
			  break;

		  }
		   counter++;
		   if(counter == no_of_pages)
		   {
			   break;
		   }

		}

		if(flag==0)
		{

		  new=NULL;
		}


		return new;
}

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================

	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] malloc
	// your code is here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FF strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	uint32 return_of_malloc = 0 ;

	//Use sys_isUHeapPlacementStrategyFIRSTFIT()... to check the current strategy
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{

			  struct MemBlock * ff_block=first_fit_strategy(size);
			  if(ff_block != NULL)
			  {

				    uint32 start_ff = ff_block->sva;
					insert_sorted_allocList(ff_block);
					return_of_malloc = start_ff;

              }
			  else
			  {
				  return_of_malloc = 0;
			  }



	}
	//	2) if no suitable space found, return NULL
	// 	3) Return pointer containing the virtual address of allocated space,
	return (void *) return_of_malloc;


}



//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.


void free(void* virtual_address)
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] free
	// your code is here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	struct MemBlock * ufree_block= find_block(&AllocMemBlocksList,(uint32 )virtual_address);
	if(ufree_block != NULL)
	  {

		LIST_REMOVE(&AllocMemBlocksList,ufree_block);
		uint32 start_uf = (uint32) virtual_address;
		uint32 end_uf= ufree_block->size;
		sys_free_user_mem(start_uf,end_uf);
		insert_sorted_with_merge_freeList(ufree_block);

	  }
	//you should get the size of the given allocation using its address
	//you need to call sys_free_user_mem()
	//refer to the project presentation and documentation for details
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	uint32 return_of_smalloc = 0 ;
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
       struct MemBlock * smalloc_block = first_fit_strategy(size);
	   if(smalloc_block != NULL)
	   {
		 int smalloc_ID = sys_createSharedObject(sharedVarName,size,isWritable,(void *)smalloc_block->sva);
		 if(smalloc_ID >= 0)
		 {
		 insert_sorted_allocList(smalloc_block);
		 return_of_smalloc =  smalloc_block->sva;
		 }
		 else
		 {
			 return_of_smalloc = 0;
		 }

	   }
	   else
	   {
		   return_of_smalloc = 0;
	   }

	}



	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy
	return (void *) return_of_smalloc;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	uint32 return_of_sget = 0 ;

	uint32 sget_size = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(sget_size == E_SHARED_MEM_NOT_EXISTS)
	{
		return_of_sget = 0 ;
	}

	if(sys_isUHeapPlacementStrategyFIRSTFIT())
		{
		  struct MemBlock * sget_block=first_fit_strategy(sget_size);
		  if(sget_block != NULL)
		   {
				int Sget_ID = sys_getSharedObject(ownerEnvID,sharedVarName,(void *)sget_block->sva);
				if(Sget_ID >= 0)
				 {
					  return_of_sget = sget_block->sva;
				 }
				else
				{
					return_of_sget = 0;
				}


			}
		  else
		  {
			  return_of_sget = 0;
		  }

		}


	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
				//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
    //	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return (void *)return_of_sget;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{

	//TODO: [PROJECT MS3 - BONUS] [SHARING - USER SIDE] sfree()

	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	 sys_freeSharedObject( 0,virtual_address);

}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}
