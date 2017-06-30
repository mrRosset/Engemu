#include "Decoder.h"

void Decoder::Decode(IR_Thumb& ir, u16 instr){
	switch ((instr >> 13) & 0b111) {
	case 0b000:
		switch ((instr >> 10) & 0b111) {
		default: break; //Shift by immediate
		case 110: break; // Add/subtract register 
		case 111: break; // Add/subtract immediate
		}
		break;
	case 0b001: break; // Add/subtract/compare/move immediate
	}
	switch ((instr >> 8) & 0b11'1111) {
	case 0b010000: break;
	case 0b010001: break;
	case 0b01010: 
	case 0b01011: break;
	}

}