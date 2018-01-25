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
#include "Gui/Gui.h"
#include "Gui/GuiMain.h"
#include "Gui/GuiMemory.h"
#include "HLE/Kernel.h"
#include "Symbols/SymbolsManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#define CATCH_CONFIG_RUNNER
#include <catch\catch.hpp>

std::string extract_filename(const std::string& filepath)
{
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

void emulate(std::string& app_path, std::string& lib_folder_path, std::string& rom_path, std::string& symbols_folder_path) {
	auto logger = spdlog::get("console");
	
	GageMemory mem;
	CPU_Interface& cpu = CPUnicorn(mem);
	Emulator emu(mem, cpu, app_path, lib_folder_path, rom_path);

	GageMemory mem2;
	CPU_Interface& cpu2 = CPU(mem2);
	Emulator emu2(mem2, cpu2, app_path, lib_folder_path, rom_path);

	std::string file_name = extract_filename(app_path);
	GuiMain* guimain = new GuiMain(&cpu, extract_filename(app_path));
	
	emu.cpu.swi_callback = [&](u32 number) {
		auto logger = spdlog::get("console");
		logger->info("SWI1 {:x}", number);
		Kernel::Executive_Call(number, emu.cpu, guimain);
	};

	emu2.cpu.swi_callback = [&](u32 number) {
		auto logger = spdlog::get("console");
		logger->info("SWI2 {:x}", number);
		Kernel::Executive_Call(number, emu2.cpu, nullptr);
	};


	//Load Symbols if exists
	logger->info("Loading Symbols");
	Symbols::load(symbols_folder_path);

	std::vector<u32> breakpoints = { 1348724304 };

	//emulation loop

	const int FRAMES_PER_SECOND = 25;
	const uint64_t SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	auto clock = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto next_game_tick = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
	uint64_t sleep_time = 0;
	bool running = true;

	while (running) {
		//Rendering
		running = guimain->render();

		//Breakpoints
		if (std::find(breakpoints.begin(), breakpoints.end(), cpu.GetPC()) != breakpoints.end() && emu.cpu.state == CPUState::Running) {
			emu.cpu.state = CPUState::Stopped;
		}

		emu2.cpu.state = emu.cpu.state;
		
		/*if (emu.cpu.state == CPUState::Step) {
			std::cout << emu.cpu.GetPC() << " " << emu2.cpu.GetPC() << "\n";
		}*/
		
		//Stepping

		//Note: for thumb bl instructions are separated into 2 parts. Tharm execute one after the other while
		//unicorn execute both at once so we need to step a second time to avoid a desynchronization
		if (emu.cpu.GetCPSR().flag_T && (emu.cpu.state == CPUState::Step || emu.cpu.state == CPUState::Step)) {
			IR_Thumb ir;
			Decoder::Decode(ir, emu.mem.read16(emu.cpu.GetPC()));
			if (ir.instr == TInstructions::BL_high) {
				emu2.Step();
				emu2.cpu.state = CPUState::Step;
			}
		}
		
		emu.Step();
		emu2.Step();

		if (!emu.Equals(emu2)) {
			std::cout << "------ CPU 1 --------\n";
			emu.Print();
			std::cout << "------ CPU 2 --------\n";
			emu2.Print();


			throw std::string("Emu not equals");
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
		return 1;
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

