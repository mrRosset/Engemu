#pragma once
#include <vector>
#include "E32Std.h"

//-----------Sources-----------//
// - E32rom.h from S60 SDK v1.2 (there are some changes in more recent versions but they also have more explanations
// - Symbian OS Internals, Chapter 10 and Appendix 3
// - Symbian ^3 Documentation: http://www.symlab.org/main/documentation/reference/s3/pdk/GUID-88C84EC0-B63E-3848-82BF-E3A81E2F7E9A.html

//TODO move some from E32Image.h struct/class to a common file
//TODO figure out what exactly is TRomHeader and how it's related to TRomImageHeader

struct TDllRefTable
{
	u16 flags;
	u16 number_of_entries;// iNumberOfEntries;
	//probably has a vector of ref block, but it's never noted, I need to check
	//TRomImageHeader *	iEntry <- it's an array
};

//Structure created to contains the flags in more easily understandble way
//Not representative of what is in the on-disk file.
struct TRomImageFlags
{
	bool executable_type; // false = executable, true = DLL
	bool fixed_address; // 0 = not fixed address, 1 = fixed address
	u8 abi; // 0 = GCC98r2, 1 = EABI
	u8 entry_point_type; // 0 = EKA1, 1 = EKA2
	bool exe_in_tree_flag; // 1 = links directly/indirectly to an exe (if yes it's first id dll ref table)
	bool data_present_flag; // 1 = it/it's dependencies (with linked dll) has .bss/.data 
	bool data_init_flag; // 1 = it/it's dependencies (not linked dll) has .bss/.data 
	bool data_flag; // 1 = it has a .data/.bss and is not a variant/extension
	bool secondary_flag;// 1 = it's a file server
	bool device_flag; // 1 = it's a device drivers
	bool extension_flag; // 1 = it's a kernel extension
	bool variant_flag; // 1 = it's a variant DLL image
	bool primary_flag; // 1 = it's a kernel image
};


struct TRomImageHeader {

	u32 uid1;
	u32 uid2;
	u32 uid3;
	u32 uid_checksum;
	u32 entry_point;
	u32 code_address;
	u32 data_address;
	s32 code_size;
	s32 text_size;
	s32 data_size;
	s32 bss_size;
	s32 heap_minimum_size;
	s32 heap_maximum_size;
	s32 stack_size;
	u32 dll_ref_table_address;
	s32 export_dir_count;
	u32 export_dir_address;
	u32 code_checksum;
	u32 data_checksum;
	u8 major;
	u8 minor;
	u16 build;
	u32 flags_raw;
	TRomImageFlags flags;
	ProcessPriority priority;
	u32 data_bss_linear_base_address;
	u32 next_extension_linear_address;
	u32 harware_variant; //I have no idea what this is exactly

	//Note, EKA2+ variants have additionnal fields after harware variants.
	//Since they are not in the e32rom.h from s60 SDK v1.2, I'm assuming they
	//are not used here

};

/*
From Symbian ^3 documentation on TRomImageHeader

ROM file images have the following sections:

The header
.text  - code
.rdata - constant (read-only) data
.edata - the export directory
.data  - initialised data that is copied to RAM when the executable runs.
The DLL reference table (a TDllRefTable structure), which is a list of DLLs
used by the executable.
*/

struct TRomImage {
	std::vector<u8> data;
	bool valid_uid_checksum = false;

	TRomImageHeader header;

};