/*
 * paging_helpers.c
 *
 *  Created on: Sep 30, 2022
 *      Author: HP
 */
#include "memory_manager.h"

/*[2.1] PAGE TABLE ENTRIES MANIPULATION */
inline void pt_set_page_permissions(uint32* page_directory, uint32 virtual_address, uint32 permissions_to_set, uint32 permissions_to_clear)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_set_page_permissions
	// Write your code here, remove the panic and write your code
	//panic("pt_set_page_permissions() is not implemented yet...!!");


			uint32 *ptr_table = NULL ;
			get_page_table(page_directory, virtual_address,  &ptr_table); // Get page Table
			if (ptr_table != NULL)
			{
				int tableIndex = PTX(virtual_address);

					ptr_table[tableIndex] &= ~permissions_to_clear;

					ptr_table[tableIndex] |= permissions_to_set ;
					tlb_invalidate((void *)NULL, (void *)virtual_address);
			}

			else{
				panic("Invalid virtual_address");
			}




}

inline int pt_get_page_permissions(uint32* page_directory, uint32 virtual_address )
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_get_page_permissions
	// Write your code here, remove the panic and write your code
	//panic("pt_get_page_permissions() is not implemented yet...!!");
	int page_perm ;
	uint32 *ptr_table = NULL;

			get_page_table(page_directory, virtual_address,  &ptr_table);

			if (ptr_table != NULL)
			{

				page_perm=  ptr_table[PTX(virtual_address)] & 0x00000FFF;
			}
			else
			{
				page_perm = -1;
			}

return page_perm;


}

inline void pt_clear_page_table_entry(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] pt_clear_page_table_entry
	// Write your code here, remove the panic and write your code
	//panic("pt_clear_page_table_entry() is not implemented yet...!!");

			int tableIndex = PTX(virtual_address);

			uint32 *ptr_table = NULL ;
			get_page_table(page_directory, virtual_address,  &ptr_table);
			if (ptr_table != NULL)
			{


				  ptr_table[tableIndex] = 0;
				  tlb_invalidate((void *)NULL, (void *)virtual_address);
			}
			if (ptr_table == NULL ){
				panic("Invalid virtual_address");
			}




}

/***********************************************************************************************/

/*[2.2] ADDRESS CONVERTION*/
inline int virtual_to_physical(uint32* page_directory, uint32 virtual_address)
{
	//TODO: [PROJECT MS2] [PAGING HELPERS] virtual_to_physical
	// Write your code here, remove the panic and write your code
	//panic("virtual_to_physical() is not implemented yet...!!");

	            int Physical_adress;
				uint32 *v2p_ptr_table = NULL;
				get_page_table(page_directory, virtual_address,  &v2p_ptr_table);
				if (v2p_ptr_table != NULL)
				{
					int tableIndex = PTX(virtual_address);
					int TableEntry = v2p_ptr_table[tableIndex];

				    int frame_num = TableEntry >> 12;
				    int pagePA = frame_num * PAGE_SIZE;
				    Physical_adress=pagePA;

				}

				else
				{
					Physical_adress = -1;
				}

				return	Physical_adress ;

}

/***********************************************************************************************/

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

///============================================================================================
/// Dealing with page directory entry flags

inline uint32 pd_is_table_used(uint32* page_directory, uint32 virtual_address)
{
	return ( (page_directory[PDX(virtual_address)] & PERM_USED) == PERM_USED ? 1 : 0);
}

inline void pd_set_table_unused(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] &= (~PERM_USED);
	tlb_invalidate((void *)NULL, (void *)virtual_address);
}

inline void pd_clear_page_dir_entry(uint32* page_directory, uint32 virtual_address)
{
	page_directory[PDX(virtual_address)] = 0 ;
	tlbflush();
}
