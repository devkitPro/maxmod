#include <stdlib.h>
#include <maxmod9.h>
#include <mm_msl.h>
#include "mm_pxi.h"

static mm_callback s_mmLoader;
extern mm_ds_system s_mmState;

void mmInitDefaultMem(const void* soundbank)
{
	// Parse soundbank header and allocate memory for the bank table
	const msl_head* head = (const msl_head*)soundbank;
	unsigned num_files = head->sampleCount + head->moduleCount;
	void** mem_bank = (void**)malloc(sizeof(void*) * num_files);
	mm_ds_system sys = {
		.mod_count    = head->moduleCount,
		.samp_count   = head->sampleCount,
		.mem_bank     = mem_bank,
		.fifo_channel = 0, // Unused
	};

	// Resolve pointers to module files
	for (unsigned i = 0; i < sys.mod_count; i ++) {
		mem_bank[i] = (u8*)soundbank + head->sampleModuleOffsetTable[sys.samp_count+i];
	}

	// Resolve pointers to sample files
	for (unsigned i = 0; i < sys.samp_count; i ++) {
		mem_bank[sys.mod_count+i] = (u8*)soundbank + head->sampleModuleOffsetTable[i];
	}

	// Flush the bank table
	armDCacheFlush(mem_bank, sizeof(void*) * num_files);

	// Initialize maxmod
	mmInit(&sys);
	mmSoundBankInMemory(soundbank);
}

void mmSoundBankInMemory(const void* address)
{
	s_mmLoader = NULL;
}

void mmSetCustomSoundBankHandler(mm_callback p_loader)
{
	s_mmLoader = p_loader;
}

void mmLoad(mm_word module_ID)
{
	if (!s_mmLoader) {
		return;
	}
}

void mmUnload(mm_word module_ID)
{
	if (!s_mmLoader) {
		return;
	}
}

void mmLoadEffect(mm_word sample_ID)
{
	if (!s_mmLoader) {
		return;
	}
}

void mmUnloadEffect(mm_word sample_ID)
{
	if (!s_mmLoader) {
		return;
	}
}
