#pragma once

#include "..\Common.h"


//sizeof(RHeap) = 116
struct RHeap {
	/*00*/ u32 unknow_0;
	/*01*/ u32 unknow_1;
	/*02*/ u32 unknow_2;
	/*03*/ u32 unknow_3;
	/*04*/ u32 unknow_4;
	/*05*/ u32 unknow_5;
	/*06*/ u32 unknow_6;
	/*07*/ u32 unknow_7;
	/*08*/ u32 unknow_8;
	/*09*/ u32 unknow_9;
	/*10*/ u32 unknow_10;
	/*11*/ u32 unknow_11;
	/*12*/ u32 unknow_12;
	/*13*/ u32 unknow_13;
	/*14*/ u32 iTestCodeRunning; //it's supposed to be a bool. does it take the whole 4 bytes or only 1 ?
	/*15*/ s32 iTestNestingLevel;
	/*16*/ s32 iTestNumAllocCells;
	/*17*/ u32 iTestAddress;
	/*18*/ u32 iTestSize;
	/*19*/ u32 iTestAllocCount;
	/*20*/ u32 unknow_20; //iNestingLevel
	/*21*/ u32 unknow_21; //iAllocCount
	/*22*/ u32 unknow_22; //iLevelNumAllocCells
	/*23*/ u32 unknow_23; //iPtrDebugCell
	/*24*/ u32 unknow_24; //iFailType
	/*25*/ u32 unknow_25; //iFailRate
	/*26*/ u32 unknow_26; //iFailed
	/*27*/ u32 unknow_27; //iFailAllocCount
	/*28*/ u32 unknow_28; //iRand
};