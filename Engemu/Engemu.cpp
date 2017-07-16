#include <iostream>
#include <chrono>
#include <thread>
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

int main(int argc, char* argv[])
{
	if (argc < 4) {
		std::cout << "Error missing E32Image or library folder path or rom file" << std::endl;
		return -1;
	}
	E32Image image;
	E32ImageLoader::parse(std::string(argv[1]), image);
	
	CPU cpu;
	cpu.mem.loadRom(std::string(argv[3]));
	E32ImageLoader::load(image, cpu.mem, std::string(argv[2]));

	cpu.gprs[Regs::PC] = image.header->code_base_address + image.header->entry_point_offset; // 0x50392D54 <- entry of Euser.dll;
	//TODO: find the correct place where the SP is initialized
	cpu.gprs[Regs::SP] = 0x7FFF'FFFF; //start of the ram section
	
	Gui* gui = new GuiMain(cpu, extract_filename(std::string(argv[1])));

	cpu.swi_callback = [&](u32 number) {Kernel::Executive_Call(number, cpu, gui); };

	const int FRAMES_PER_SECOND = 25;
	const uint64_t SKIP_TICKS = 1000 / FRAMES_PER_SECOND;
	auto clock = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto next_game_tick = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
	uint64_t sleep_time = 0;
	bool running = true;

	while (running) {
		running = gui->render();
		
		if(cpu.step) {
			cpu.Step();
			cpu.step = false;
		}

		next_game_tick += SKIP_TICKS;

		clock = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto tickCount = std::chrono::duration_cast<std::chrono::milliseconds>(clock).count();
		sleep_time = next_game_tick - tickCount;

		if (sleep_time > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		}
	}


	/*while (true) {
		for (int i = 0; i < 5; i++) {
			if (cpu.cpsr.flag_T) {
				u16 instr = cpu.mem.read16(cpu.gprs[Regs::PC] + i * 2);
				IR_Thumb ir;
				ir.instr = TInstructions::SWI;
				try {
					Decoder::Decode(ir, instr);
					std::string text = Disassembler::Disassemble(ir);
					std::cout << std::hex << cpu.gprs[Regs::PC] + i * 2 << " - " << instr << std::dec << ": " << std::hex << Disassembler::Disassemble(ir) << std::endl;
				}
				catch (...) {
					std::cout << std::hex << cpu.gprs[Regs::PC] + i * 2 << " - " << instr << std::dec << ": " << "Unkown instruction" << std::endl;
				}
			}
			else {
				u32 instr = cpu.mem.read32(cpu.gprs[Regs::PC] + i * 4);
				IR_ARM ir;
				try {
					Decoder::Decode(ir, instr);
					std::cout << std::hex << cpu.gprs[Regs::PC] + i * 4 << " - " << instr << std::dec << ": " << std::hex << Disassembler::Disassemble(ir) << std::endl;
				}
				catch (...) {
					std::cout << std::hex << cpu.gprs[Regs::PC] + i * 4 << " - " << instr << std::dec << ": " << "Unkown instruction" << std::endl;
				}
			}
		}

		std::cout << "\n\n";

		for (int i = 0; i <= 15; i++) {
			std::cout << Disassembler::Disassemble_Reg(i) << ": " << std::hex << cpu.gprs[i] << std::dec << std::endl;
		}

		std::cout << "N:" << cpu.cpsr.flag_N << " Z:" << cpu.cpsr.flag_Z << " C:" << cpu.cpsr.flag_C << " V:" << cpu.cpsr.flag_V << std::endl;

		std::cin.get();
		system("cls");
		cpu.Step();
	}*/
	
	return 0;
}

