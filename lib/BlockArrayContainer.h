#pragma once
#ifndef HAVE_BLOCK_ARRAY_CONTAINER_H
#define HAVE_BLOCK_ARRAY_CONTAINER_H

#include <exception>
#include <string>
#include <vector>

#include "PalettedBlockArray.h"

template<typename Block>
class BlockArrayContainer {
private:
	typedef IPalettedBlockArray<Block> BlockArray;

	static BlockArray *blockArrayFromData(uint8_t bitsPerBlock, std::vector<char> &wordArray, std::vector<Block> &paletteEntries) {
#define SWITCH_CASE(i) case i: return new PalettedBlockArray<i, Block>(wordArray, paletteEntries);
		switch (bitsPerBlock) {
			SWITCH_CASE(1)
			SWITCH_CASE(2)
			SWITCH_CASE(3)
			SWITCH_CASE(4)
			SWITCH_CASE(5)
			SWITCH_CASE(6)
			SWITCH_CASE(8)
			SWITCH_CASE(16)
		default:
			throw std::runtime_error("invalid bits-per-block: " + std::to_string(bitsPerBlock));
		}

#undef SWITCH_CASE
	}

	static BlockArray *blockArrayFromCapacity(unsigned short capacity) {
#define CONDITION(i) \
		if (capacity <= PalettedBlockArray<i, Block>::MAX_PALETTE_SIZE) { \
			return new PalettedBlockArray<i, Block>; \
		}

		CONDITION(1)
		CONDITION(2)
		CONDITION(3)
		CONDITION(4)
		CONDITION(5)
		CONDITION(6)
		CONDITION(8)
		CONDITION(16)

		throw std::invalid_argument("invalid capacity specified: " + std::to_string(capacity));

#undef CONDITION
	}

	static uint8_t getMinBitsPerBlock(unsigned short capacity) {
#define CONDITION(i) \
		if (capacity <= PalettedBlockArray<i, Block>::MAX_PALETTE_SIZE) { \
			return i; \
		}

		CONDITION(1)
		CONDITION(2)
		CONDITION(3)
		CONDITION(4)
		CONDITION(5)
		CONDITION(6)
		CONDITION(8)
		CONDITION(16)

#undef CONDITION

		throw std::invalid_argument("invalid capacity specified: " + std::to_string(capacity));
	}

	BlockArray *blockArray = nullptr;

public:
	BlockArrayContainer(Block block) {
		blockArray = new PalettedBlockArray<1, Block>(block);
	}

	BlockArrayContainer(unsigned short capacity) {
		blockArray = blockArrayFromCapacity(capacity);
	}

	BlockArrayContainer(uint8_t bitsPerBlock, std::vector<char> &wordArray, std::vector<Block> &paletteEntries) {
		blockArray = blockArrayFromData(bitsPerBlock, wordArray, paletteEntries);
	}

	~BlockArrayContainer() {
		delete blockArray;
	}

	const char *getWordArray(unsigned int &length) const {
		return blockArray->getWordArray(length);
	}

	const Block *getPalette(unsigned short &paletteSize) const {
		return blockArray->getPalette(paletteSize);
	}

	unsigned short getMaxPaletteSize() const {
		return blockArray->getMaxPaletteSize();
	}

	uint8_t getBitsPerBlock() const {
		return blockArray->getBitsPerBlock();
	}

	Block get(unsigned char x, unsigned char y, unsigned char z) const {
		return blockArray->get(x, y, z);
	}

	void set(unsigned char x, unsigned char y, unsigned char z, Block val) {
		if (!blockArray->set(x, y, z, val)) {
			unsigned short count = blockArray->countUniqueBlocks();
			if (count < BlockArray::ARRAY_CAPACITY) {
				//reached max capacity and less than ARRAY_CAPACITY unique blocks in array
				//this also automatically handles GC on chunks when there are unused palette entries in an array

				BlockArray *newArray = blockArrayFromCapacity(count + 1);

				newArray->convertFrom(*blockArray);
				newArray->set(x, y, z, val);
				delete blockArray;
				blockArray = newArray;
			}
			else {
				//array already has ARRAY_CAPACITY unique blocks - force overwrite of existing blockstate
				//this is a rather expensive way to do it... TODO: allow writes to arbitrary palette offsets
				blockArray->replaceAll(blockArray->get(x, y, z), val);
			}
		}
	}

	void replace(unsigned short offset, Block val) {
		blockArray->replace(offset, val);
	}

	void replaceAll(Block oldVal, Block newVal) {
		blockArray->replaceAll(oldVal, newVal);
	}

	// Repacks the block array to the smallest format possible with the number of unique blocks found
	void collectGarbage(bool forceCollect) {
		if (forceCollect || blockArray->needsGarbageCollection()) {
			auto unique = blockArray->countUniqueBlocks();

			if (getMinBitsPerBlock(unique) != blockArray->getBitsPerBlock()) {
				BlockArray *newArray = blockArrayFromCapacity(unique);
				assert(newArray != nullptr);
				newArray->convertFrom(*blockArray);
				delete blockArray;
				blockArray = newArray;
			}
			else {
				//this only needs to be set if the array was not reallocated
				blockArray->setGarbageCollected();
			}
		}
	}
};

#endif
