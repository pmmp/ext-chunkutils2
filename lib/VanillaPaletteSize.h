#ifndef HAVE_VANILLA_PALETTE_SIZES_H
#define HAVE_VANILLA_PALETTE_SIZES_H

#include <cstdint>

#define MACRO_PER_BITS_PER_BLOCK \
	BPB_MACRO(0) \
	BPB_MACRO(1) \
	BPB_MACRO(2) \
	BPB_MACRO(3) \
	BPB_MACRO(4) \
	BPB_MACRO(5) \
	BPB_MACRO(6) \
	BPB_MACRO(8) \
	BPB_MACRO(16)

enum class VanillaPaletteSize : uint8_t {
#define BPB_MACRO(i) BPB_##i = i,
	MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO
};

#endif
