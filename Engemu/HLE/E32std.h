#pragma once

#include "..\Common.h"

/*
TODO: Get a better grip of how vptr are layed down in memory
Documentation:
	https://stackoverflow.com/questions/1632600/memory-layout-c-objects#answer-27682344
	https://www.openrce.org/articles/full_view/23
*/

//sizeof(RHeap) = 116
#pragma pack(1)
struct RHeap {

	//RHeapBase
	/*00*/ u32 iMinLength;
	/*04*/ u32 iMaxLength;
	/*08*/ u32 iOffset;
	/*12*/ u32 iGrowBy;
	/*16*/ u32 iAccessCount;
	/*20*/ u32 iType;
	/*24*/ u32 iChunk;
	/*28*/ u64 iLock;
	/*36*/ u32 iBase;
	/*40*/ u32 iTop;
	/*44*/ u32 iFree_len;
	/*48*/ u32 iFree_next;

	/*52*/ u32 unknow; // <- ? vptr ? part of base or not ?

	/*56*/ u32 iTestCodeRunning; //it's supposed to be a bool. does it take the whole 4 bytes or only 1 ?
	/*60*/ s32 iTestNestingLevel;
	/*64*/ s32 iTestNumAllocCells;
	/*68*/ u32 iTestAddress;
	/*72*/ u32 iTestSize;
	/*76*/ u32 iTestAllocCount;
	/*80*/ u32 iNestingLevel;
	/*84*/ u32 iAllocCount;
	/*88*/ u32 iLevelNumAllocCells;
	/*92*/ u32 iPtrDebugCell;
	/*96*/ u32 iFailType;
	/*100*/ u32 iFailRate;
	/*104*/ u32 iFailed;
	/*108*/ u32 iFailAllocCount;
	/*112*/ u32 iRand;
};
#pragma pack(pop)