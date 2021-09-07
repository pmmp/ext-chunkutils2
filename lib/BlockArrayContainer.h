#pragma once
#ifndef HAVE_BLOCK_ARRAY_CONTAINER_H
#define HAVE_BLOCK_ARRAY_CONTAINER_H

#include <exception>
#include <string>
#include <vector>
#include <gsl/span>

#include "VanillaPaletteSize.h"
#include "PalettedBlockArray.h"

template<typename Block>
class BlockArrayContainer {
private:
	typedef IPalettedBlockArray<Block> BlockArray;

	static BlockArray *blockArrayFromData(uint8_t bitsPerBlock, gsl::span<uint8_t> &wordArray, std::vector<Block> &paletteEntries) {
		switch (bitsPerBlock) {
#define BPB_MACRO(i) case i: return new PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>(wordArray, paletteEntries);
			MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO
		default:
			throw std::runtime_error("invalid bits-per-block: " + std::to_string(bitsPerBlock));
		}
	}

	static BlockArray *blockArrayFromCapacity(Block initializer, unsigned short capacity) {
#define BPB_MACRO(i) \
		if (capacity <= PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>::MAX_PALETTE_SIZE) { \
			return new PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>(initializer); \
		}

		MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO

		throw std::invalid_argument("invalid capacity specified: " + std::to_string(capacity));
	}

	static VanillaPaletteSize getOptimalBitsPerBlock(unsigned short capacity) {
#define BPB_MACRO(i) \
		if (capacity <= PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>::MAX_PALETTE_SIZE) { \
			return VanillaPaletteSize::BPB_##i; \
		}

		MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO

		throw std::invalid_argument("invalid capacity specified: " + std::to_string(capacity));
	}

	BlockArray *blockArray = nullptr;

public:
	static unsigned int getExpectedPayloadSize(uint8_t bitsPerBlock) {
		switch (bitsPerBlock) {
#define BPB_MACRO(i) case i: return PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>::PAYLOAD_SIZE;
			MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO
		default:
			throw std::invalid_argument("invalid bits-per-block: " + std::to_string(bitsPerBlock));
		}
	}

	BlockArrayContainer(Block block, unsigned short capacity) {
		blockArray = blockArrayFromCapacity(block, capacity);
	}

	BlockArrayContainer(uint8_t bitsPerBlock, gsl::span<uint8_t> &wordArray, std::vector<Block> &paletteEntries) {
		blockArray = blockArrayFromData(bitsPerBlock, wordArray, paletteEntries);
	}

	BlockArrayContainer(const BlockArrayContainer &other) {
		blockArray = other.blockArray->clone();
	}

	BlockArrayContainer& operator=(const BlockArrayContainer &other) {
		if (&other != this) {
			delete blockArray;
			blockArray = other.blockArray->clone();
		}
		return *this;
	}

	~BlockArrayContainer() {
		delete blockArray;
	}

	// A godawful hack to avoid thousands of virtual calls during conversion and other things.
	template<typename ReturnType, typename Visited>
	ReturnType specializeForArraySize(Visited& v) {
#define BPB_MACRO(size) \
		if (auto casted = dynamic_cast<PalettedBlockArray<VanillaPaletteSize::BPB_##size, Block>*>(blockArray)){ \
			return v.template visit<VanillaPaletteSize::BPB_##size>(*casted); \
		}

		MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO

		v.visit(*blockArray);
	}

	const gsl::span<const char> getWordArray() const {
		return blockArray->getWordArray();
	}

	const gsl::span<const Block> getPalette() const {
		return blockArray->getPalette();
	}

	unsigned short getMaxPaletteSize() const {
		return blockArray->getMaxPaletteSize();
	}

	VanillaPaletteSize getBitsPerBlock() const {
		return blockArray->getBitsPerBlock();
	}

	Block get(unsigned char x, unsigned char y, unsigned char z) const {
		return blockArray->get(x, y, z);
	}

	void set(unsigned char x, unsigned char y, unsigned char z, Block val) {
		if (!blockArray->set(x, y, z, val)) {
			unsigned short count = blockArray->getPaletteSize();
			if (count < BlockArray::ARRAY_CAPACITY) {
				//palette is full, resize to larger one

				BlockArray *newArray = blockArrayFromCapacity(val, count + 1);

				newArray->fastUpsize(*blockArray);
				newArray->set(x, y, z, val);
				delete blockArray;
				blockArray = newArray;
			} else {
				//largest palette size, try and ditch some entries from palette
				collectGarbage(false, 1);
				//after GC, one of two things should happen
				//1) there should be a free slot in the palette for the new entry
				//2) palette remains full (indicating every block has its own palette slot), so we overwrite the existing entry for x,y,z
				//therefore this should never return false, since false would indicate resize/GC is needed, and we just did that.
				auto result = blockArray->set(x, y, z, val);
				assert(result);
			}
		}
	}

	void replaceAll(Block oldVal, Block newVal) {
		blockArray->replaceAll(oldVal, newVal);
	}

	// Repacks the block array to the smallest format possible with the number of unique blocks found
	void collectGarbage(bool forceCollect, unsigned short reserved = 0) {
		if (forceCollect || blockArray->needsGarbageCollection()) {
			auto unique = blockArray->countUniqueBlocks();

			if (getOptimalBitsPerBlock(unique) != blockArray->getBitsPerBlock()) {
				BlockArray *newArray = blockArrayFromCapacity(blockArray->get(0, 0, 0), unique + reserved);
				assert(newArray != nullptr);
				newArray->convertFrom(*blockArray);
				delete blockArray;
				blockArray = newArray;
			}
			blockArray->setGarbageCollected();
		}
	}
};

#endif
