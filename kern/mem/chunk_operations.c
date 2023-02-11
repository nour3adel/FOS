/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include "kheap.h"
#include "memory_manager.h"


/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	//panic("cut_paste_pages() is not implemented yet...!!");
	uint32  sva = ROUNDDOWN(source_va, PAGE_SIZE);
    uint32  dva = ROUNDDOWN(dest_va, PAGE_SIZE);

	uint32 size=( PAGE_SIZE * num_of_pages );




	uint32 start_dest = dva;
	uint32 end_dest = ROUNDUP(dva + size,PAGE_SIZE);
	for(uint32 i = start_dest ; i < end_dest ; i += PAGE_SIZE)
	{
		uint32 *ptr_table = NULL;
		struct FrameInfo *frame_ptr =NULL;

		frame_ptr = get_frame_info(page_directory,i,&ptr_table);
		if(frame_ptr != NULL){
			return -1;
		}

	}


	uint32 start_src = sva;
	uint32 end_src= ROUNDUP(sva + size,PAGE_SIZE);
	for(int i= start_src ; i < end_src ; i +=PAGE_SIZE )
	{
		    uint32 *src_ptr_table = NULL;
		    get_page_table(page_directory, i, &src_ptr_table);
			uint32 *dest_ptr_table = NULL;
			int ret = get_page_table(page_directory, dva, &dest_ptr_table);

			if(ret == TABLE_NOT_EXIST )
			{
			create_page_table(page_directory, dva);
			}

			struct FrameInfo* ptr1;
			struct FrameInfo* ptr2;
			ptr1 = get_frame_info(page_directory,i,&src_ptr_table);
			ptr2 = ptr1;
			int tableIndex = PTX(i);
			int TableEntry = src_ptr_table[tableIndex];
			uint32 src_perm =  TableEntry & 0x00000FFF;
			map_frame(page_directory, ptr2, dva, src_perm);
            unmap_frame(page_directory,i);


		dva += PAGE_SIZE;

	}

  return 0;
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va
//if the page table at any destination page in the range is not exist, it should create it
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the panic and write your code
	//panic("copy_paste_chunk() is not implemented yet...!!");

		 uint32 start_dest = dest_va;
		 uint32 end_dest =dest_va + size;
	    for(uint32 i = start_dest ; i < end_dest; i+=PAGE_SIZE)
	     {

			uint32 *table_ptr =NULL;
			struct FrameInfo *frame_ptr =NULL;
			frame_ptr = get_frame_info(page_directory,i,&table_ptr);
			int tableIndex = PTX(i);
			int TableEntry = table_ptr[tableIndex];

			if((TableEntry & PERM_WRITEABLE) == 0 && frame_ptr != 0 ){
					return -1;
			}

	   }


	     uint32 start_src =source_va ;
		 uint32 end_src =source_va + size;
		for(int i = start_src ; i < end_src ; i++ )
		{

		  uint32 *src_ptr_table = NULL;
		  get_page_table(page_directory, i, &src_ptr_table);
		  uint32 *dest_ptr_table = NULL;
		  get_page_table(page_directory, dest_va, &dest_ptr_table);

		  int tableIndex = PTX(i);
		  int TableEntry = src_ptr_table[tableIndex];

          int src_perm = TableEntry & PERM_USER;

      	if(dest_ptr_table == NULL ){

      		create_page_table(page_directory, dest_va);

      	}

            struct FrameInfo* ptr2=  get_frame_info(page_directory,dest_va,&dest_ptr_table);

            if(ptr2 == NULL)
            {
	            int alloc_fr = allocate_frame(&ptr2);
	             if(alloc_fr != E_NO_MEM)
	               {
		            int map_fr = map_frame(page_directory, ptr2, dest_va, src_perm|PERM_WRITEABLE);

	                 if(map_fr == E_NO_MEM)
	                    {
                          free_frame(ptr2);
                           return -1;
		                }
	               }
	               else
	               {
		           return -1;
	              }
            }
            uint8 *ptr_addr1 = (uint8 *)i;
            uint8 *ptr_addr2 = (uint8 *)dest_va;
          	*ptr_addr2 = *ptr_addr1;
       	    ptr_addr1++;
       	    ptr_addr2++;
		    dest_va ++;

		}


		return 0;
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from dest_va with the source_va
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the panic and write your code
	//panic("share_chunk() is not implemented yet...!!");

 uint32 sva =ROUNDDOWN(source_va,PAGE_SIZE);
 uint32 dva =ROUNDDOWN(dest_va,PAGE_SIZE);

 uint32 start_dest = dva;
 uint32 end_dest = ROUNDUP(dva + size,PAGE_SIZE);
 for(uint32 i = start_dest ; i < end_dest ; i+=PAGE_SIZE)
  {
 	 uint32 *ptr_table =NULL;
 	 struct FrameInfo *frame_ptr = get_frame_info(page_directory, i , &ptr_table);
 	 if(frame_ptr != NULL)
 	 {
 	 	return -1;
 	 }

  }

     uint32 start_src =sva;
	 uint32 end_src =ROUNDUP((source_va+size),PAGE_SIZE);
     for(int i= start_src ;i < end_src ; i += PAGE_SIZE){

	        uint32 *src_ptr_table = NULL;
			get_page_table(page_directory, i, &src_ptr_table);

			uint32 *dest_ptr_table = NULL;
			int dest_ret = get_page_table(page_directory, dva, &dest_ptr_table);

			if( dest_ret == TABLE_NOT_EXIST )
			{

			create_page_table(page_directory, dva);

			}

			struct FrameInfo* src_ptr;
			struct FrameInfo* dest_ptr;
			src_ptr = get_frame_info(page_directory,i,&src_ptr_table);
			dest_ptr = src_ptr;

			map_frame(page_directory, dest_ptr, dva, perms);
			dva += PAGE_SIZE;
}


	return 0;
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate in RAM the given range [va, va+size)
//Hint: use ROUNDDOWN/ROUNDUP macros to align the addresses
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//TODO: [PROJECT MS2] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the panic and write your code
	//panic("allocate_chunk() is not implemented yet...!!");
	 uint32 startadd =ROUNDDOWN(va,PAGE_SIZE);
	 uint32 finaladd =ROUNDUP((va+size),PAGE_SIZE);

	 for(int i=startadd; i < finaladd; i+=PAGE_SIZE)
	 {

		 uint32* alloc_ptr_table = NULL;
		 struct FrameInfo * frame_ptr = get_frame_info(page_directory, i, &alloc_ptr_table);
		 if(frame_ptr != NULL){
			 return -1;

		 }

		  struct FrameInfo * ptr_info;
		  int alloc_ret = allocate_frame(&ptr_info);
		  if(alloc_ret == E_NO_MEM){

			  return -1;

		  }
		  alloc_ret = map_frame(page_directory, ptr_info,i , perms);
		  if(alloc_ret==E_NO_MEM)
		  {
		     free_frame(ptr_info);
		     return -1;
		  }
		  ptr_info->va = i;

	 }

 return 0;
}

/*BONUS*/
//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	//panic("calculate_allocated_space() is not implemented yet...!!");


	 uint32 temp_no_table = 0;
	 char flag =0 ;

	 uint32 start = ROUNDDOWN(sva, PAGE_SIZE);
	 uint32 final = ROUNDUP(eva, PAGE_SIZE);

	 for(int i = start; i < eva ; i +=PAGE_SIZE){

		 uint32* pages_ptr=NULL;
		 struct FrameInfo * pages_ret =get_frame_info(page_directory,i,&pages_ptr);

		 if(pages_ret != NULL && pages_ptr != NULL){

			 (*num_pages) ++;
		 }

	 }

	 for(int i = start; i <eva ; i+=PAGE_SIZE){

			  uint32* table_ptr=NULL;
			  get_page_table(page_directory, i,&table_ptr);
			  if(flag == 0 && table_ptr!=NULL)
			  {
				  temp_no_table=PDX(i);
				  (*num_tables) ++;
				  flag=1;
			  }

		     if( table_ptr != NULL && flag == 1 && temp_no_table != PDX(i) )
			 {
					    	  (*num_tables)++;
					    	  temp_no_table=PDX(i);
			 }
	 }

}

/*BONUS*/
//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
// calculate_required_frames:
// calculates the new allocation size required for given address+size,
// we are not interested in knowing if pages or tables actually exist in memory or the page file,
// we are interested in knowing whether they are allocated or not.
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//TODO: [PROJECT MS2 - BONUS] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	//panic("calculate_required_frames() is not implemented yet...!!");

         uint32 page_counter = 0 ;
         uint32 temp_no_table = 0 ;
         char flag =0;

         uint32 startadd =ROUNDDOWN(sva,PAGE_SIZE);
         uint32 finaladd =ROUNDUP((sva+size),PAGE_SIZE);
		 for(int i = startadd; i < finaladd; i +=PAGE_SIZE)
		 {
			 uint32* pages_ptr = NULL;
			 struct FrameInfo * pages_frame_info = get_frame_info(page_directory, i, &pages_ptr);
			 if(pages_frame_info == NULL)
			 {
				 page_counter++;
			 }

			 if(flag == 0 && pages_ptr == NULL)
			 {
				 temp_no_table = PDX(i);
				 page_counter++;
				 flag = 1;
			 }
			 if(temp_no_table != PDX(i) && pages_ptr == NULL)
			 {
				 temp_no_table= PDX(i);
			 	 page_counter++;
			 }
		 }

		 return page_counter;
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	// Write your code here, remove the panic and write your code
	panic("allocate_user_mem() is not implemented yet...!!");
}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3] [USER HEAP - KERNEL SIDE] free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");
	//This function should:

        size = ROUNDUP(size,PAGE_SIZE);
		uint32 start =virtual_address;
		uint32 end =virtual_address + size;

		for(uint32 i= start ; i< end;i += PAGE_SIZE)
		{
			pf_remove_env_page(e,i);

		}

		//2. Free ONLY pages that are resident in the working set from the memory

        uint32 WS_maxsize= e->page_WS_max_size;
		for(int i=  0; i < WS_maxsize; i++ )
		{

			uint32 Ws_virt_addr = env_page_ws_get_virtual_address(e,i);

			if( Ws_virt_addr >= start )
			{
				if ( Ws_virt_addr < end )
				{

				  unmap_frame(e->env_page_directory,Ws_virt_addr);
				  env_page_ws_clear_entry(e,i);

				}

			}
		}

		//3. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)

	  for(uint32 i = start; i < end; i += PAGE_SIZE)
		{
			char flag = 1;
			uint32* ptr_page_table=NULL;
			get_page_table(e->env_page_directory,i,&ptr_page_table);

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
					pd_clear_page_dir_entry(e->env_page_directory,(uint32)i);
				}
		}

	}
}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");

	//This function should:
	//1. Free ALL pages of the given range from the Page File
	//2. Free ONLY pages that are resident in the working set from the memory
	//3. Free any BUFFERED pages in the given range
	//4. Removes ONLY the empty page tables (i.e. not used) (no pages are mapped in the table)
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//TODO: [PROJECT MS3 - BONUS] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");

	// This function should move all pages from "src_virtual_address" to "dst_virtual_address"
	// with the given size
	// After finished, the src_virtual_address must no longer be accessed/exist in either page file
	// or main memory

	/**/
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

