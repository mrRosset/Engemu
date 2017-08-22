#include <iostream>
#include <chrono>
#include <thread>
#include <experimental/filesystem>
#include <spdlog/spdlog.h>
#include "Common.h"
#include "CPU/CPU.h"
#include "E32Image.h"
#include "Loader/E32ImageLoader.h"
#include "CPU/Decoder/Decoder.h"
#include "CPU/Disassembler/Disassembler.h"
#include "Gui/Gui.h"
#include "Gui/GuiMain.h"
#include "Gui/GuiMemory.h"
#include "HLE/Kernel.h"
#include "Symbols/SymbolsManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

std::string extract_filename(const std::string& filepath)
{
	auto pos = filepath.rfind("\\");
	if (pos == std::string::npos)
		pos = -1;
	return std::string(filepath.begin() + pos + 1, filepath.end());
}

void emulate(std::string& app_path, std::string& lib_folder_path, std::string& rom_path, std::string& symbols_folder_path) {
	auto logger = spdlog::get("console");
	
	Memory mem;
	CPU cpu(mem);
	
	E32Image image;
	E32ImageLoader::parse(app_path, image);

	std::string file_name = extract_filename(app_path);
	GuiMain* guimain = new GuiMain(&cpu, extract_filename(app_path));
	//ImGuiContext* guimainContext = ImGui::GetCurrentContext();

	//ImGuiContext* guiMemoryContext = ImGui::CreateContext(malloc, free);
	//ImGui::SetCurrentContext(guiMemoryContext);
	//GuiMemory* guiMemory = new GuiMemory(cpu.mem, std::string("Memory Editor"));

	cpu.mem.loadRom(rom_path);
	E32ImageLoader::load(image, file_name, cpu.mem, lib_folder_path);

	//Load Symbols if exists
	logger->info("Loading Symbols");
	Symbols::load(symbols_folder_path);

	cpu.gprs[Regs::PC] = image.header->code_base_address + image.header->entry_point_offset; // 0x50392D54 <- entry of Euser.dll;
	//cpu.gprs[Regs::PC] = image.header->code_base_address + image.code_section.export_directory[0];
	//cpu.gprs[Regs::PC] = 0x5063D444; //Main of AppRun
	//cpu.cpsr.flag_T = true;
	
	//TODO: find the correct place where the SP is initialized
	//cpu.gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section
	cpu.gprs[Regs::SP] = 0x7FFFFFFC; //start of the ram section aligned with last 2 bit 0


	cpu.swi_callback = [&](u32 number) {logger->info("SWI {:x}", number); Kernel::Executive_Call(number, cpu, guimain); };
	std::vector<u32> breakpoints = { 0x503aa384 };


	//emulation loop

	const int FRAMES_PER_SECOND = 25;
	const uint64_t SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	auto clock = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto next_game_tick = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
	uint64_t sleep_time = 0;
	bool running = true;

	while (running) {
		//ImGui::SetCurrentContext(guimainContext);
		running = guimain->render();
		//ImGui::SetCurrentContext(guiMemoryContext);
		//running = guiMemory->render();

		//Breakpoints
		if (std::find(breakpoints.begin(), breakpoints.end(), cpu.gprs.RealPC()) != breakpoints.end() && cpu.state == CPU::State::Running) {
			cpu.state = CPU::State::Stopped;
		}


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


	//setup logging system.
	try
	{
		auto console = spdlog::stdout_color_st("console");
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

