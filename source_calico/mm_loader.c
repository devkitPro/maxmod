#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <maxmod9.h>
#include <mm_msl.h>
#include <mm_mas.h>
#include "mm_pxi.h"

#define MM_SAMP_REF (1U<<24)

static mm_callback s_mmLoader;

static struct {
	int fd;
	msl_head hdr;
} s_mmFile;

extern mm_ds_system s_mmState;

static mm_word _mmFileLoadCallback(mm_word msg, mm_word param);

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

void mmInitDefault(const char* filename)
{
	// Open soundbank file
	mmSoundBankInFiles(filename);

	// Allocate memory for memory bank
	unsigned num_files = s_mmFile.hdr.sampleCount + s_mmFile.hdr.moduleCount;
	void** mem_bank = (void**)calloc(num_files, sizeof(void*));

	// Initialize maxmod
	mm_ds_system sys = {
		.mod_count    = s_mmFile.hdr.moduleCount,
		.samp_count   = s_mmFile.hdr.sampleCount,
		.mem_bank     = mem_bank,
		.fifo_channel = 0, // Unused
	};
	mmInit(&sys);
}

void mmSoundBankInMemory(const void* address)
{
	s_mmLoader = NULL;
}

void mmSoundBankInFiles(const char* filename)
{
	// Open soundbank file and read header
	s_mmFile.fd = open(filename, O_RDONLY);
	read(s_mmFile.fd, &s_mmFile.hdr, sizeof(s_mmFile.hdr));

	// Register handler
	mmSetCustomSoundBankHandler(_mmFileLoadCallback);
}

void mmSetCustomSoundBankHandler(mm_callback p_loader)
{
	s_mmLoader = p_loader;
}

static void _mmLoadSample(mm_word sample_ID)
{
	mm_word* bank = (mm_word*)s_mmState.mem_bank + s_mmState.mod_count;

	if (bank[sample_ID] >= MM_SAMP_REF) {
		bank[sample_ID] += MM_SAMP_REF; // addref
		return;
	}

	mm_mas_prefix* prefix = (mm_mas_prefix*)s_mmLoader(MMCB_SAMPREQUEST, sample_ID);
	if (!prefix) {
		return;
	}

	armDCacheFlush(prefix, sizeof(*prefix) + prefix->size);
	bank[sample_ID] = MM_SAMP_REF | ((uptr)prefix - MM_MAINRAM);
}

static void _mmUnloadSample(mm_word sample_ID)
{
	mm_word* bank = (mm_word*)s_mmState.mem_bank + s_mmState.mod_count;

	mm_word entry = bank[sample_ID];
	if (entry < MM_SAMP_REF) {
		return;
	}

	entry -= MM_SAMP_REF;
	if (entry < MM_SAMP_REF) {
		s_mmLoader(MMCB_DELETESAMPLE, MM_MAINRAM + entry);
		entry = 0;
	}

	bank[sample_ID] = entry;
}

static void _mmFlushBank(void)
{
	armDCacheFlush(s_mmState.mem_bank, sizeof(void*) * (s_mmState.mod_count + s_mmState.samp_count));
}

void mmLoad(mm_word module_ID)
{
	if (!s_mmLoader) {
		return;
	}

	void** bank = (void**)s_mmState.mem_bank;
	if (bank[module_ID]) {
		return;
	}

	mm_mas_prefix* prefix = (mm_mas_prefix*)s_mmLoader(MMCB_SONGREQUEST, module_ID);
	if (!prefix) {
		return;
	}

	armDCacheFlush(prefix, sizeof(*prefix) + prefix->size);
	bank[module_ID] = prefix;

	mm_mas_head* mas = (mm_mas_head*)&prefix[1];
	mm_word* sampl_off_table = &mas->tables[mas->instr_count];

	for (unsigned i = 0; i < mas->sampl_count; i ++) {
		mm_mas_sample_info* sampl = (mm_mas_sample_info*)((u8*)mas + sampl_off_table[i]);
		_mmLoadSample(sampl->msl_id);
	}

	_mmFlushBank();
}

void mmUnload(mm_word module_ID)
{
	if (!s_mmLoader) {
		return;
	}

	void** bank = (void**)s_mmState.mem_bank;
	mm_mas_prefix* prefix = (mm_mas_prefix*)bank[module_ID];
	if (!prefix) {
		return;
	}

	mm_mas_head* mas = (mm_mas_head*)&prefix[1];
	mm_word* sampl_off_table = &mas->tables[mas->instr_count];

	for (unsigned i = 0; i < mas->sampl_count; i ++) {
		mm_mas_sample_info* sampl = (mm_mas_sample_info*)((u8*)mas + sampl_off_table[i]);
		_mmUnloadSample(sampl->msl_id);
	}

	s_mmLoader(MMCB_DELETESONG, (mm_word)prefix);
	bank[module_ID] = NULL;

	_mmFlushBank();
}

void mmLoadEffect(mm_word sample_ID)
{
	if (!s_mmLoader) {
		return;
	}

	_mmLoadSample(sample_ID);
	_mmFlushBank();
}

void mmUnloadEffect(mm_word sample_ID)
{
	if (!s_mmLoader) {
		return;
	}

	_mmUnloadSample(sample_ID);
	_mmFlushBank();
}

mm_word _mmFileLoadCallback(mm_word msg, mm_word param)
{
	switch (msg) {
		default:
			return 0;

		case MMCB_DELETESONG:
		case MMCB_DELETESAMPLE:
			free((void*)param);
			return 0;

		case MMCB_SONGREQUEST:
			param += s_mmState.samp_count;
			// fallthrough

		case MMCB_SAMPREQUEST: {
			if (lseek(s_mmFile.fd, offsetof(msl_head, sampleModuleOffsetTable[param]), SEEK_SET) < 0) {
				return 0;
			}

			mm_word offset;
			if (read(s_mmFile.fd, &offset, sizeof(offset)) != sizeof(offset)) {
				return 0;
			}

			if (lseek(s_mmFile.fd, offset, SEEK_SET) < 0) {
				return 0;
			}

			mm_mas_prefix prefix;
			if (read(s_mmFile.fd, &prefix, sizeof(prefix)) != sizeof(prefix)) {
				return 0;
			}

			void* data = malloc(sizeof(prefix) + prefix.size);
			if (!data) {
				return 0;
			}

			memcpy(data, &prefix, sizeof(prefix));
			if (read(s_mmFile.fd, (u8*)data + sizeof(prefix), prefix.size) != prefix.size) {
				free(data);
				return 0;
			}

			return (mm_word)data;
		}
	}
}
