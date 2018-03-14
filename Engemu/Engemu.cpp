#include <iostream>
#include <chrono>
#include <thread>
#include <experimental/filesystem>
#include <spdlog/spdlog.h>
#include "Common.h"
#include "E32Image.h"
#include "Emulator.h"
#include "Loader/E32ImageLoader.h"
#include "CPU/CPU_Interface.h"
#include "CPU/Tharm/CPU.h"
#include "CPU/Tharm/Decoder/Decoder.h"
#include "CPU/Tharm/Disassembler/Disassembler.h"
#include "CPU/Unicorn/CPUnicorn.h"
#include "Memory/GageMemory.h"
#include "Memory/BootMemory.h"
#include "Gui/Gui.h"
#include "Gui/GuiMain.h"
#include "Gui/GuiMemory.h"
#include "HLE/Kernel.h"
#include "Symbols/SymbolsManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#define CATCH_CONFIG_RUNNER
#include <catch\catch.hpp>

std::string extract_filename(const std::string& filepath) {
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

void emulate(std::string& app_path, std::string& lib_folder_path, std::string& rom_path, std::string& symbols_folder_path) {
	auto logger = spdlog::get("console");
	
	BootMemory mem;
	CPU_Interface& cpu = CPUnicorn(mem);
	Emulator emu(mem, cpu, app_path, lib_folder_path, rom_path);

	std::string file_name = extract_filename(app_path);
	GuiMain* guimain = new GuiMain(emu, extract_filename(app_path));
	
	emu.getCPU().swi_callback = [&](u32 number) {
		auto logger = spdlog::get("console");
		logger->info("SWI {:x}", number);
		Kernel::Executive_Call(number, emu, guimain);
	};


	//Load Symbols if exists
	logger->info("Loading Symbols");
	Symbols::load(symbols_folder_path);

	std::vector<u32> breakpoints = { /*0x503A76DC*/ };

	//emulation loop

	const int FRAMES_PER_SECOND = 30;
	const uint64_t SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	auto clock = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto next_game_tick = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
	uint64_t sleep_time = 0;
	bool running = true;

	while (running) {
		//Rendering
		running = guimain->render();

		//Breakpoints
		if (std::find(breakpoints.begin(), breakpoints.end(), cpu.GetPC()) != breakpoints.end() && emu.state == CPUState::Running) {
			emu.state = CPUState::Stopped;
		}

		//Stepping
		emu.Step();
		
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
		int result = Catch::Session().run(argc, argv);
		std::cin.get();
		return (result < 0xff ? result : 0xff);
	}

	//setup logging system.
	try
	{
		auto console = spdlog::stdout_color_mt("console");
		// set the log pattern to [HH:MM:SS.nano]
		spdlog::set_pattern("[%T] [%l] %v");
		console->info("Start of the emulator");

		
		emulate(std::string(argv[1]), std::string(argv[2]), std::string(argv[3]), std::string(argv[4]));

		// Release and close all loggers
		spdlog::drop_all();
	}
	// Exceptions will only be thrown upon failed logger or sink construction (not during logging)
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
		std::cin.get();
		return 1;
	}
	catch (std::exception const& ex) {
		std::cout << "Uncaught exception:\n" << ex.what() << std::endl;
		std::cin.get();
	}
	catch (std::string& error_message) {
		std::cout << "Uncaught exception:\n" << error_message << std::endl;
		std::cin.get();
	}
	catch (const char* error_message) {
		std::cout << "Uncaught exception:\n" << error_message << std::endl;
		std::cin.get();
	}




	return 0;
}

