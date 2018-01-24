#include <iostream>
#include <spdlog/spdlog.h>

#include "TRomImageLoader.h"
#include "Utils.h"
#include "../TRomImage.h"

void TRomImageLoader::load(TRomImage& image, Memory& mem, std::string& lib_folder_path) {
	//What is there to do here ?
}


bool TRomImageLoader::parse(std::string& path, TRomImage& image) {
	auto err = utils::loadData(path, image.data);
	if (!err) {
		return false;
	}

	parseHeader(image);
	parseExportDir(image);
	checkHeaderValidity(image);

	if (!image.valid_uid_checksum) {
		std::cerr << "Wrong uid checksum, not a valid TRomImage.";
		return false;
	}

	return true;
}

void TRomImageLoader::checkHeaderValidity(TRomImage& image) {
	u8 uids[12] = {};
	utils::u8_from_32(uids, image.header.uid1);
	utils::u8_from_32(uids + 4, image.header.uid2);
	utils::u8_from_32(uids + 8, image.header.uid3);

	u8 even_bytes[] = { uids[0], uids[2], uids[4], uids[6], uids[8], uids[10] };
	u8 odd_bytes[] = { uids[1], uids[3], uids[5], uids[7], uids[9], uids[11] };

	u32 uid_crc = ((u32)utils::crc16_ccitt(odd_bytes) << 16) | utils::crc16_ccitt(even_bytes);

	image.valid_uid_checksum = uid_crc == image.header.uid_checksum;
}

void TRomImageLoader::parseHeader(TRomImage& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());
	TRomImageHeader& header = image.header;

	header.uid1 = data32[0]; //u32
	header.uid2 = data32[1]; //u32
	header.uid3 = data32[2]; //u32
	header.uid_checksum = data32[3]; //u32
	header.entry_point = data32[4]; //u32
	header.code_address = data32[5]; //u32
	header.data_address = data32[6]; //u32
	header.code_size = data32[7]; //s32
	header.text_size = data32[8]; //s32
	header.data_size = data32[9]; //s32
	header.bss_size = data32[10]; //s32
	header.heap_minimum_size = data32[11]; //s32
	header.heap_maximum_size = data32[12]; //s32
	header.stack_size = data32[13]; //s32
	header.dll_ref_table_address = data32[14]; //u32
	header.export_dir_count = data32[15]; //s32
	header.export_dir_address = data32[16]; //u32
	header.code_checksum = data32[17]; //u32
	header.data_checksum = data32[18]; //u32
	header.major = data32[19]; //u8
	header.minor = data32[20]; //u8
	header.build = data32[21]; //u16
	header.flags_raw = data32[22]; //u32
	header.priority = static_cast<ProcessPriority>(data32[23]); //ProcessPriority
	header.data_bss_linear_base_address = data32[24]; //u32
	header.next_extension_linear_address = data32[25]; //u32
	header.harware_variant = data32[26]; //u32

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

void TRomImageLoader::parseExportDir(TRomImage& image) {
	u32* data32 = reinterpret_cast<u32*>(image.data.data());

	u32 header_size = 0x50392D54 - 0x50392CF0; //Code addr - File add for Euser.dll
	u32 export_offset = image.header.export_dir_address - image.header.code_address + header_size;

	for (s32 i = 0; i < image.header.export_dir_count; i++) {
		image.export_directory.push_back(data32[(export_offset / 4) + i]);
	}
}