#include <iostream>

#include "TRomImageLoader.h"
#include "Utils.h"
#include "..\TRomImage.h"


bool TRomImageLoader::load(std::string path, TRomImage& image) {
	auto err = utils::loadData(path, image.data);
	if (!err) {
		return false;
	}

	parseHeader(image);
	checkHeaderValidity(image);

	if (!image.valid_uid_checksum) {
		std::cerr << "Wrong uid checksum, not a valid TRomImage.";
		return false;
	}

	return true;
}

void TRomImageLoader::checkHeaderValidity(TRomImage& image) {
	TRomImageHeader& header = image.header;
	uint8_t uids[12] = {};
	utils::u8_from_32(uids, image.header.uid1);
	utils::u8_from_32(uids + 4, image.header.uid2);
	utils::u8_from_32(uids + 8, image.header.uid3);

	uint8_t even_bytes[] = { uids[0], uids[2], uids[4], uids[6], uids[8], uids[10] };
	uint8_t odd_bytes[] = { uids[1], uids[3], uids[5], uids[7], uids[9], uids[11] };

	uint32_t uid_crc = ((uint32_t)utils::crc16_ccitt(odd_bytes) << 16) | utils::crc16_ccitt(even_bytes);

	image.valid_uid_checksum = uid_crc == image.header.uid_checksum;
}

void TRomImageLoader::parseHeader(TRomImage& image) {
	uint32_t* data32 = reinterpret_cast<uint32_t*>(image.data.data());
	TRomImageHeader& header = image.header;

	header.uid1 = data32[0]; //uint32_t
	header.uid2 = data32[1]; //uint32_t
	header.uid3 = data32[2]; //uint32_t
	header.uid_checksum = data32[3]; //uint32_t
	header.entry_point = data32[4]; //uint32_t
	header.code_address = data32[5]; //uint32_t
	header.data_address = data32[6]; //uint32_t
	header.code_size = data32[7]; //int32_t
	header.text_size = data32[8]; //int32_t
	header.data_size = data32[9]; //int32_t
	header.bss_size = data32[10]; //int32_t
	header.heap_minimum_size = data32[11]; //int32_t
	header.heap_maximum_size = data32[12]; //int32_t
	header.stack_size = data32[13]; //int32_t
	header.dll_ref_table_address = data32[14]; //uint32_t
	header.export_dir_count = data32[15]; //int32_t
	header.export_dir_address = data32[16]; //uint32_t
	header.code_checksum = data32[17]; //uint32_t
	header.data_checksum = data32[18]; //uint32_t
	header.major = data32[19]; //uint8_t
	header.minor = data32[20]; //uint8_t
	header.build = data32[21]; //uint16_t
	header.flags_raw = data32[22]; //uint32_t
	header.priority = static_cast<ProcessPriority>(data32[23]); //ProcessPriority
	header.data_bss_linear_base_address = data32[24]; //uint32_t
	header.next_extension_linear_address = data32[25]; //uint32_t
	header.harware_variant = data32[26]; //uint32_t

	//parse flags
	header.flags.executable_type = utils::getBit(header.flags_raw, 0);
	header.flags.fixed_address = utils::getBit(header.flags_raw, 2);
	header.flags.abi = (header.flags_raw >> 3) & 0b11;
	header.flags.entry_point_type = (header.flags_raw >> 5) & 0b111;
	header.flags.exe_in_tree_flag = utils::getBit(header.flags_raw, 23);
	header.flags.data_present_flag = utils::getBit(header.flags_raw, 24);
	header.flags.data_init_flag = utils::getBit(header.flags_raw, 25);
	header.flags.data_flag = utils::getBit(header.flags_raw, 26);
	header.flags.secondary_flag = utils::getBit(header.flags_raw, 27);
	header.flags.device_flag = utils::getBit(header.flags_raw, 28);
	header.flags.extension_flag = utils::getBit(header.flags_raw, 29);
	header.flags.variant_flag = utils::getBit(header.flags_raw, 30);
	header.flags.primary_flag = utils::getBit(header.flags_raw, 31);

}
