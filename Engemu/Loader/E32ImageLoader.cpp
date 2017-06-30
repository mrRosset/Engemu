#include <iostream>

#include "E32ImageLoader.h"
#include "Utils.h"
#include "..\E32Image.h"

bool E32ImageLoader::load(std::string path, E32Image& image)
{
	auto err = utils::loadData(path, image.data);
	if (!err) {
		return false;
	}
	parseHeader(image);
	checkHeaderValidity(image);

	if (!image.valid_uid_checksum) {
		std::cerr << "Wrong uid checksum, not a valid E32Image.";
		return false;
	}
	if (!image.valid_signature) {
		//std::cerr << "Not an E32Image. TRomImage are not supported";
		return false;
	}
	if (image.header->flags.header_format > 1) {
		std::cerr << "Not a basic header or J-header. V-headers are not supported";
		return false;
	}
	if (image.header->flags.import_format != 0 && image.header->flags.import_format != 2) {
		std::cerr << "ELF import section not supported";
		return false;
	}

	parseIAT(image);
	parseExportDir(image);
	parseImportSection(image);
	parseRelocSections(image);
	checkImportValidity(image);

	if (!image.valid_imports) {
		std::cerr << "Incoherence between the Import Address Table and the Import section" << std::endl;
		return false;
	}

	return true;
}

void E32ImageLoader::checkHeaderValidity(E32Image& image) {
	std::unique_ptr<E32ImageHeader>& header = image.header;

	// Check the signature
	image.valid_signature = header->signature == 'COPE'; // 'EPOC' backwards for little-endian

														 // Check the UID checksum validity
	u8 uids[12] = {};
	utils::u8_from_32(uids, image.header->uid1);
	utils::u8_from_32(uids + 4, image.header->uid2);
	utils::u8_from_32(uids + 8, image.header->uid3);

	u8 even_bytes[] = { uids[0], uids[2], uids[4], uids[6], uids[8], uids[10] };
	u8 odd_bytes[] = { uids[1], uids[3], uids[5], uids[7], uids[9], uids[11] };

	u32 uid_crc = ((u32)utils::crc16_ccitt(odd_bytes) << 16) | utils::crc16_ccitt(even_bytes);

	image.valid_uid_checksum = uid_crc == image.header->uid_checksum;
}

void E32ImageLoader::parseHeader(E32Image& image)
{
	u32* data32 = reinterpret_cast<u32*>(image.data.data());

	//Check what header format we are using
	u32 flags_raw = data32[11];
	u8 header_format = (flags_raw >> 24) & 0xF;

	switch (header_format) {
	case 0: image.header = std::make_unique<E32ImageHeader>(); //Basic format
		break;
	case 1: image.header = std::make_unique<E32ImageHeaderJ>(); //J-format
		break;
	case 2: image.header = std::make_unique<E32ImageHeader>(); //V-format
		break;
	default: image.header = std::make_unique<E32ImageHeader>();
		break;
	}

	std::unique_ptr<E32ImageHeader>& header = image.header;

	header->uid1 = data32[0];
	header->uid2 = data32[1];
	header->uid3 = data32[2];
	header->uid_checksum = data32[3];
	header->signature = data32[4];
	header->cpu = static_cast<CPUType>(data32[5]);
	header->code_checksum = data32[6];
	header->data_checksum = data32[7];
	header->major = data32[8] & 0xFF;
	header->minor = data32[8] & 0xFF00;
	header->build = data32[8] >> 16;
	header->timestamp = ((u64)data32[9] << 32) | data32[10];
	header->flags_raw = data32[11];
	header->code_size = data32[12];
	header->data_size = data32[13];
	header->heap_minimum_size = data32[14];
	header->heap_maximum_size = data32[15];
	header->stack_size = data32[16];
	header->BSS_size = data32[17];
	header->entry_point_offset = data32[18];
	header->code_base_address = data32[19];
	header->data_base_address = data32[20];
	header->dll_count = data32[21];
	header->export_offset = data32[22];
	header->export_count = data32[23];
	header->text_size = data32[24];
	header->code_offset = data32[25];
	header->data_offset = data32[26];
	header->import_offset = data32[27];
	header->code_relocation_offset = data32[28];
	header->data_relocation_offset = data32[29];
	header->priority = static_cast<ProcessPriority>(data32[30]);

	// Get the flags
	header->flags.executable_type = utils::getBit(header->flags_raw, 0);
	header->flags.call_entry_point = utils::getBit(header->flags_raw, 1);
	header->flags.fixed_address = utils::getBit(header->flags_raw, 2);
	header->flags.abi = (header->flags_raw >> 3) & 0x3;
	header->flags.entry_point_type = (header->flags_raw >> 5) & 7;
	header->flags.header_format = (header->flags_raw >> 24) & 0xF;
	header->flags.import_format = (header->flags_raw >> 28) & 0xF;

	if (header->flags.header_format == 1) {
		//Is is possible to get a uniq_ptr to E32ImageHeaderJ ? Didn't found how
		E32ImageHeaderJ* headerJ = static_cast<E32ImageHeaderJ*>(image.header.get());
		headerJ->compression_type = data32[7];
		if (headerJ->compression_type != 0) {
			headerJ->uncompressed_size = data32[31];
		}
		else {
			/// default value to avoid non - initialized value.Not in any standard.
			headerJ->uncompressed_size = 0;
		}
	}

	// Don't include the header in the data
	//std::vector<decltype(image.data)::value_type>(image.data.begin() + image.header->code_offset, image.data.end()).swap(image.data);
}

void E32ImageLoader::parseIAT(E32Image& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());
	u32 iat_offset = image.header->code_offset + image.header->text_size;
	u32 i = 0;

	u32 line = data32[iat_offset / 4]; //divided by 4 we because we use uin32t (= 4 bytes)

	while (line != 0) {
		image.code_section.import_address_table.push_back(line);
		i++;
		line = data32[(iat_offset / 4) + i];
	}
}

void E32ImageLoader::parseExportDir(E32Image& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());

	for (s32 i = 0; i < image.header->export_count; i++) {
		image.code_section.export_directory.push_back(data32[(image.header->export_offset / 4) + i]);
	}
}

void E32ImageLoader::parseImportSection(E32Image& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());

	u32 offset = image.header->import_offset / 4;
	image.import_section.size = data32[offset++];

	for (s32 i = 0; i < image.header->dll_count; i++) {
		std::unique_ptr<E32ImportBlock> block(new E32ImportBlock);

		block->dll_name_offset = data32[offset++];
		block->number_of_imports = data32[offset++];

		//only standard PE import format has redundancy
		if (image.header->flags.import_format == 0) {
			for (s32 j = 0; j < block->number_of_imports; j++) {
				block->ordinals.push_back(data32[offset++]);
			}
		}

		image.import_section.imports.push_back(std::move(block));
	}
}

void E32ImageLoader::parseRelocSections(E32Image& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());

	//we divide by 4 since we are using a uint32 (= 4 bytes) array, so we need to adapt the offset
	if (image.header->code_relocation_offset) {
		image.code_reloc_section.size = data32[image.header->code_relocation_offset / 4];
		image.code_reloc_section.number_of_relocs = data32[(image.header->code_relocation_offset / 4) + 1];
	}
	else {
		image.code_reloc_section.size = 0;
		image.code_reloc_section.number_of_relocs = 0;
	}
	if (image.header->data_relocation_offset) {
		image.data_reloc_section.size = data32[image.header->data_relocation_offset / 4];
		image.data_reloc_section.number_of_relocs = data32[(image.header->data_relocation_offset / 4) + 1];
	}
	else {
		image.data_reloc_section.size = 0;
		image.data_reloc_section.number_of_relocs = 0;
	}
}

void E32ImageLoader::checkImportValidity(E32Image& image) {
	//import validity only exist when there is redundancy between iat and import section
	//(so only when import format is standard PE)
	if (image.header->flags.import_format != 0) {
		image.valid_imports = true;
		return;
	}

	//Assumption: Both list should be in the same order.
	//Since we have redundancy, I'm not sure which one the software use.
	//And there is no garuantee someone didn't mess with the one not used
	image.valid_imports = true;
	int iat_index = 0;

	for (std::unique_ptr<E32ImportBlock>& block : image.import_section.imports) {
		for (u32 ordinal : block->ordinals) {
			if (image.code_section.import_address_table[iat_index++] != ordinal) {
				image.valid_imports = false;
			}
		}
	}
}
