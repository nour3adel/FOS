#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//2017

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] Create "shares" array:
//===========================
//Dynamically allocate the array of shared objects
//initialize the array of shared objects by 0's and empty = 1
void create_shares_array(uint32 numOfElements)
{
#if USE_KHEAP
	MAX_SHARES  = numOfElements ;
	shares = kmalloc(numOfElements*sizeof(struct Share));
	if (shares == NULL)
	{
		panic("Kernel runs out of memory\nCan't create the array of shared objects.");
	}
#endif
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		memset(&(shares[i]), 0, sizeof(struct Share));
		shares[i].empty = 1;
	}
}

//===========================
// [2] Allocate Share Object:
//===========================
//Allocates a new (empty) shared object from the "shares" array
//It dynamically creates the "framesStorage"
//Return:
//	a) if succeed:
//		1. allocatedObject (pointer to struct Share) passed by reference
//		2. sharedObjectID (its index in the array) as a return parameter
//	b) E_NO_SHARE if the the array of shares is full (i.e. reaches "MAX_SHARES")
int allocate_share_object(struct Share **allocatedObject)
{
	int32 sharedObjectID = -1 ;
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		if (shares[i].empty)
		{
			sharedObjectID = i;
			break;
		}
	}

	if (sharedObjectID == -1)
	{
		return E_NO_SHARE ;
/*		//try to increase double the size of the "shares" array
#if USE_KHEAP
		{
			shares = krealloc(shares, 2*MAX_SHARES);
			if (shares == NULL)
			{
				*allocatedObject = NULL;
				return E_NO_SHARE;
			}
			else
			{
				sharedObjectID = MAX_SHARES;
				MAX_SHARES *= 2;
			}
		}
#else
		{
			panic("Attempt to dynamically allocate space inside kernel while kheap is disabled .. ");
			*allocatedObject = NULL;
			return E_NO_SHARE;
		}
#endif
*/
	}

	*allocatedObject = &(shares[sharedObjectID]);
	shares[sharedObjectID].empty = 0;

#if USE_KHEAP
	{
		shares[sharedObjectID].framesStorage = create_frames_storage();
	}
#endif
	memset(shares[sharedObjectID].framesStorage, 0, PAGE_SIZE);

	return sharedObjectID;
}

//=========================
// [3] Get Share Object ID:
//=========================
//Search for the given shared object in the "shares" array
//Return:
//	a) if found: SharedObjectID (index of the shared object in the array)
//	b) else: E_SHARED_MEM_NOT_EXISTS
int get_share_object_ID(int32 ownerID, char* name)
{
	int i=0;

	for(; i< MAX_SHARES; ++i)
	{
		if (shares[i].empty)
			continue;

		//cprintf("shared var name = %s compared with %s\n", name, shares[i].name);
		if(shares[i].ownerID == ownerID && strcmp(name, shares[i].name)==0)
		{
			//cprintf("%s found\n", name);
			return i;
		}
	}
	return E_SHARED_MEM_NOT_EXISTS;
}

//=========================
// [4] Delete Share Object:
//=========================
//delete the given sharedObjectID from the "shares" array
//Return:
//	a) 0 if succeed
//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists
int free_share_object(uint32 sharedObjectID)
{
	if (sharedObjectID >= MAX_SHARES)
		return E_SHARED_MEM_NOT_EXISTS;

	//panic("deleteSharedObject: not implemented yet");
	clear_frames_storage(shares[sharedObjectID].framesStorage);
#if USE_KHEAP
	kfree(shares[sharedObjectID].framesStorage);
#endif
	memset(&(shares[sharedObjectID]), 0, sizeof(struct Share));
	shares[sharedObjectID].empty = 1;

	return 0;
}

// 2014 - edited in 2017
//===========================
// [5] Create frames_storage:
//===========================
// if KHEAP = 1: Create the frames_storage by allocating a PAGE for its directory
inline uint32* create_frames_storage()
{
	uint32* frames_storage = kmalloc(PAGE_SIZE);
	if(frames_storage == NULL)
	{
		panic("NOT ENOUGH KERNEL HEAP SPACE");
	}
	return frames_storage;
}
//===========================
// [6] Add frame to storage:
//===========================
// Add a frame info to the storage of frames at the given index
inline void add_frame_to_storage(uint32* frames_storage, struct FrameInfo* ptr_frame_info, uint32 index)
{
	uint32 va = index * PAGE_SIZE ;
	uint32 *ptr_page_table;
	int r = get_page_table(frames_storage,  va, &ptr_page_table);
	if(r == TABLE_NOT_EXIST)
	{
#if USE_KHEAP
		{
			ptr_page_table = create_page_table(frames_storage, (uint32)va);
		}
#else
		{
			__static_cpt(frames_storage, (uint32)va, &ptr_page_table);

		}
#endif
	}
	ptr_page_table[PTX(va)] = CONSTRUCT_ENTRY(to_physical_address(ptr_frame_info), 0 | PERM_PRESENT);
}

//===========================
// [7] Get frame from storage:
//===========================
// Get a frame info from the storage of frames at the given index
inline struct FrameInfo* get_frame_from_storage(uint32* frames_storage, uint32 index)
{
	struct FrameInfo* ptr_frame_info;
	uint32 *ptr_page_table ;
	uint32 va = index * PAGE_SIZE ;
	ptr_frame_info = get_frame_info(frames_storage,  va, &ptr_page_table);
	return ptr_frame_info;
}

//===========================
// [8] Clear the frames_storage:
//===========================
inline void clear_frames_storage(uint32* frames_storage)
{
	int fourMega = 1024 * PAGE_SIZE ;
	int i ;
	for (i = 0 ; i < 1024 ; i++)
	{
		if (frames_storage[i] != 0)
		{
#if USE_KHEAP
			{
				kfree((void*)kheap_virtual_address(EXTRACT_ADDRESS(frames_storage[i])));
			}
#else
			{
				free_frame(to_frame_info(EXTRACT_ADDRESS(frames_storage[i])));
			}
#endif
			frames_storage[i] = 0;
		}
	}
}


//==============================
// [9] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	// your code is here, remove the panic and write your code
	//panic("getSizeOfSharedObject() is not implemented yet...!!");

	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//

	int shareObjectID = get_share_object_ID(ownerID, shareName);
	if (shareObjectID == E_SHARED_MEM_NOT_EXISTS)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return shares[shareObjectID].size;

	return 0;
}

//********************************************************************************//

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=========================
// [1] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT MS3] [SHARING - KERNEL SIDE] createSharedObject()
	// your code is here, remove the panic and write your code
	//panic("createSharedObject() is not implemented yet...!!");

	struct Env* myenv = curenv; //The calling environment
	uint32 *share_dir = myenv->env_page_directory;

     int get_ret = get_share_object_ID(ownerID, shareName);
	 if ( get_ret != E_SHARED_MEM_NOT_EXISTS)
	 {
		 return E_SHARED_MEM_EXISTS;
	 }

	struct Share *sh_info = NULL;
	int sh_info_ID = allocate_share_object(&sh_info);

	  if (sh_info_ID == E_NO_SHARE)
	   {
		return E_NO_SHARE;
       }
	  else
	  {
		 if (virtual_address != NULL)
		  	{
		  		int index  = 0;
		  		uint32 start = (uint32) virtual_address;
		  		uint32 end = size + start;
		  		uint32 final_size = (end-start)/PAGE_SIZE;

		        while(start < end)
		        {
		  	        struct FrameInfo* frameInfo = NULL;
		  			int alloc_ret = allocate_frame(&frameInfo);
		  			if (alloc_ret != E_NO_MEM)
		  				{

		  					  int object_permission = PERM_PRESENT | PERM_USER | PERM_WRITEABLE;
		  					  map_frame(share_dir, frameInfo , start , object_permission);
		  					  add_frame_to_storage(sh_info->framesStorage, frameInfo, index);
		  				}

		  			index++;
		  			start += PAGE_SIZE;
		        }


		  		sh_info->ownerID = ownerID;
		  		sh_info->isWritable = isWritable;
		  		sh_info->references = 1;
		  		sh_info->size = size;
		  		strcpy(sh_info->name, shareName);

		  		return sh_info_ID;
		     }

	  }

		return 0;
}

//======================
// [2] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT MS3] [SHARING - KERNEL SIDE] getSharedObject()
	// your code is here, remove the panic and write your code
	//panic("getSharedObject() is not implemented yet...!!");


	struct Env* myenv = curenv; //The calling environment
	uint32 *share_dir = myenv->env_page_directory;

	//Get SHARED ID
	int sh_info_Idx = get_share_object_ID(ownerID, shareName);
	if (sh_info_Idx != E_SHARED_MEM_NOT_EXISTS)
	  {
		  struct Share *sh_info = &shares[sh_info_Idx];

		  uint32 start = (uint32) virtual_address;
		  uint32 end =  ROUNDUP(sh_info->size,PAGE_SIZE) + start;
		  uint32 final_size = (end-start)/PAGE_SIZE;

		  int index  = 0;
		  while(start < end)
		  {

			 struct FrameInfo* frame = get_frame_from_storage( sh_info->framesStorage, index);

			 //PERMISSION

			 int perm = PERM_PRESENT | PERM_USER ;
			 if (sh_info->isWritable == 1)
			 {
			   perm |= PERM_WRITEABLE;
			 }

            map_frame(share_dir, frame,start, perm);
			index++;
			start += PAGE_SIZE;
		  }

		sh_info->references++;
		return sh_info_Idx;
	  }
	  else
	  {
		return E_SHARED_MEM_NOT_EXISTS;

	  }

  return 0;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===================
// Free Share Object:
//===================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT MS3 - BONUS] [SHARING - KERNEL SIDE] freeSharedObject()
	// your code is here, remove the panic and write your code
	//panic("freeSharedObject() is not implemented yet...!!");

	struct Env* myenv = curenv; //The calling environment

    uint32  sva = (uint32 )startVA;
	struct FrameInfo * ptr_frame_info ;
	int shared_Index = -1;

	for(int i =0 ; i < MAX_SHARES ; i++)
	{
	      if(shares[i].empty)
	      {
	    	  continue;
	      }
		  ptr_frame_info = get_frame_from_storage(shares[i].framesStorage, 0);
		  uint32 *frame_ptr=NULL;
		  struct FrameInfo * ptr_frame_info1=get_frame_info(curenv->env_page_directory,sva,&frame_ptr);
		  if(ptr_frame_info == ptr_frame_info1)
			{
				shared_Index=i ;
				break ;
		    }
	}

	if(shared_Index==-1)
	{
	   return E_SHARED_MEM_NOT_EXISTS;
	}


	 uint32 size = shares[shared_Index].size;
	 uint32 start =(uint32 )startVA;
	 uint32 end =sva + size;


	 for(uint32 i= start ; i< end;i += PAGE_SIZE)
	 {
		 unmap_frame(myenv->env_page_directory,i);
	 }


	 for(uint32 i = start; i < end; i += PAGE_SIZE)
	 {
		char flag = 1;
		uint32* ptr_page_table=NULL;
		get_page_table(myenv->env_page_directory,i,&ptr_page_table);

		if(ptr_page_table != NULL)
		{
          int page = 0;
		  while(page < 1024)
		  {
			if(ptr_page_table[page] != 0)
			  {
				flag = 0;
				break;
			  }
		   page++;
		  }

		  if(flag)
			{
				kfree((void*)ptr_page_table);
				pd_clear_page_dir_entry(myenv->env_page_directory,(uint32)i);
			}
		}

	}

	shares[shared_Index].references--;
	if(shares[shared_Index].references <= 0)
		{
			free_share_object(shared_Index);
		}
	tlbflush();
  //change this "return" according to your answer
  return 0;
}
