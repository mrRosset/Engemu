#include <iostream>
#include <chrono>
#include <thread>
#include <experimental/filesystem>
#include "Common.h"
#include "CPU/CPU.h"
#include "E32Image.h"
#include "Loader/E32ImageLoader.h"
#include "CPU/Decoder/Decoder.h"
#include "CPU/Disassembler/Disassembler.h"
#include "Gui/Gui.h"
#include "Gui/GuiMain.h"
#include "HLE/Kernel.h"

std::string extract_filename(const std::string& filepath)
{
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

void emulate(std::string& app_path, std::string& lib_folder_path, std::string& rom_path, std::string& symbols_folder_path) {
	CPU cpu;
	
	E32Image image;
	E32ImageLoader::parse(app_path, image);

	std::string file_name = extract_filename(app_path);
	GuiMain* gui = new GuiMain(cpu, extract_filename(app_path));

	cpu.mem.loadRom(rom_path);
	E32ImageLoader::load(image, file_name, cpu.mem, lib_folder_path);

	//Load Symbols if exists
	for (auto & p : std::experimental::filesystem::directory_iterator(symbols_folder_path)) {
		gui->loadSymbols(p.path().string());
	}


	//TODO: find the correct place where the SP is initialized
	cpu.gprs[Regs::PC] = image.header->code_base_address + image.header->entry_point_offset; // 0x50392D54 <- entry of Euser.dll;
	cpu.gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section

	

	cpu.swi_callback = [&](u32 number) {Kernel::Executive_Call(number, cpu, gui); };
	
	//emulation loop

	const int FRAMES_PER_SECOND = 25;
	const uint64_t SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	auto clock = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto next_game_tick = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
	uint64_t sleep_time = 0;
	bool running = true;

	while (running) {
		running = gui->render();

		switch (cpu.state) {

		case CPU::State::Step:
			cpu.Step();
			cpu.state = CPU::State::Stopped;
			break;

		case CPU::State::Running:
			cpu.Step();
			break;
		}

		next_game_tick += SKIP_TICKS;

		clock = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto tickCount = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
		sleep_time = next_game_tick - tickCount;

		if (sleep_time > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		}
	}

}

int main(int argc, char* argv[])
{
	if (argc < 5) {
		std::cout << "Error missing E32Image or library folder path or rom file or symbols folder" << std::endl;
		return -1;
	}

	try {
		emulate(std::string(argv[1]), std::string(argv[2]), std::string(argv[3]), std::string(argv[4]));
	}
	catch (std::string error_message){
		std::cout << "Uncaught exception:\n" << error_message << std::endl;
		std::cin.get();
	}

	return 0;
}

