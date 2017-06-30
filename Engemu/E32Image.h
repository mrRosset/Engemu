#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "E32Std.h"

// Sources:
// For a short overview: https://web.archive.org/web/20120813204404/http://www.antonypranata.com/node/10
// For a more detailed explanation, see "Symbian OS Internals" chapter 10 and Appendix 2 and 3
// For factual informations:
//	s60 SDK v3: f32image.h


struct E32CodeSection
{
	//Text section
	//Should the data be copied here ? Or just let the offset from the header.

	std::vector<uint32_t> import_address_table;
	std::vector<uint32_t> export_directory;
};

struct E32ImportBlock
{
	//This is only for EKA1 targeted import block from PE
	uint32_t dll_name_offset; //relative to the import section
	int32_t number_of_imports; // number of imports from this dll
	std::vector<uint32_t> ordinals;	// TUint32 iImport[iNumberOfImports];
};

struct E32ImportSection
{
	int32_t size; // size of this section
	std::vector<std::unique_ptr<E32ImportBlock>> imports; // E32ImportBlock[iDllRefTableCount];
};

//See page 390 of Symbian Internals
//For a good schema on the PE version of this: http://stackoverflow.com/questions/17436668/how-are-pe-base-relocations-build-up
struct E32RelocationBlock
{
	uint32_t offset;
	uint32_t block_size;
	//2 byte sub-block entry.
	//The top 4 bits specify the type of relocation :
	//	0 – Not a valid relocation.
	//	1 – Relocate relative to code section.
	//	2 – Relocate relative to data section.
	//	3 – Try to work it out at load time(legacy algorithm).
	//	The bottom 12 bits specify the offset within the 4 K page of the item
	//	to be relocated.
	std::vector<uint16_t> sub_block_entry;
};

struct E32RelocSection
{
	int32_t size;
	int32_t number_of_relocs;
};


//Structure created to contains the flags in more easily understandble way
//Not representative of what is in the on-disk file.
struct E32Flags
{
	bool executable_type;  // false = executable, true = DLL
	bool call_entry_point; // false = call, true = don't call
	bool fixed_address;    // 0 = not fixed address, 1 = fixed address
	uint8_t abi;                // 0 = GCC98r2, 1 = EABI
	uint8_t entry_point_type;   // 0 = EKA1, 1 = EKA2
	uint8_t header_format;      // 0 = Basic, 1 = J-format, 2 = V-format
	uint8_t import_format;      // 0 = Standard PE format, 1 = ELF format, 2 = PE format without redundancy in the import section
};

struct E32ImageHeader {
	uint32_t uid1;
	uint32_t uid2;
	uint32_t uid3;
	uint32_t uid_checksum;
	uint32_t signature;
	CPUType cpu;
	uint32_t code_checksum;
	uint32_t data_checksum;
	uint8_t major;
	uint8_t minor;
	uint16_t build;
	int64_t timestamp;
	uint32_t flags_raw;
	E32Flags flags;
	int32_t code_size;
	int32_t data_size;
	int32_t heap_minimum_size;
	int32_t heap_maximum_size;
	int32_t stack_size;
	int32_t BSS_size;
	uint32_t entry_point_offset;
	uint32_t code_base_address;
	uint32_t data_base_address;
	int32_t dll_count;
	uint32_t export_offset;
	int32_t export_count;
	int32_t text_size;
	uint32_t code_offset;
	uint32_t data_offset;
	uint32_t import_offset;
	uint32_t code_relocation_offset;
	uint32_t data_relocation_offset;
	ProcessPriority priority;
};

struct E32ImageHeaderJ : public E32ImageHeader {
	//Elements are not placed in the same order as they are in the file
	
	/*
	Should there be a version for this executable or it's still the code and data checksum ?
	The description from Symbian Internals is not very clear:
		"Version number of this executable – a 16-bit major and a 16-bit
		minor version number. This is used in link resolution (V-format only).
		In original format, this contained a checksum of the code, but this
		was never used."
	*/

	uint32_t compression_type; // 0 = no compression
	uint32_t uncompressed_size;	// Only if compression_type != 0
								// Comment from f32image.h in SDKv3:
								// "Uncompressed size of file
								// For J format this is file size - sizeof(E32ImageHeader)
								// and this is included as part of the compressed data :-(
								// For other formats this is file size - total header size"
};

struct E32Image {
	std::vector<uint8_t> data;
	bool valid_uid_checksum = false;
	bool valid_signature = false;
	bool valid_imports = false;

	std::unique_ptr<E32ImageHeader> header;
	E32CodeSection code_section; //Contains text section, import address table and export directory
	//BSS section <- usually empty
	//Data section  <- usually empty
	E32ImportSection import_section;
	E32RelocSection code_reloc_section;
	E32RelocSection data_reloc_section;
};