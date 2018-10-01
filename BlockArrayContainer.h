#pragma once
#ifndef HAVE_BLOCK_ARRAY_CONTAINER_H
#define HAVE_BLOCK_ARRAY_CONTAINER_H

#include <exception>
#include <string>
#include <vector>

#include "PalettedBlockArray.h"

template<typename Block = unsigned int, typename Word = uint32_t>
class BlockArrayContainer {
private:
	typedef IPalettedBlockArray<Block, Word> BlockArray;

	static BlockArray *createBlockArray(uint8_t bitsPerBlock) {
#define SWITCH_CASE(i) case i: return new PalettedBlockArray<i, Block, Word>;
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
			return nullptr;
		}

#undef SWITCH_CASE
	}

	static BlockArray *createBlockArray(uint8_t bitsPerBlock, std::vector<Word> &wordArray, std::vector<Block> &paletteEntries) {
#define SWITCH_CASE(i) case i: return new PalettedBlockArray<i, Block, Word>(wordArray, paletteEntries);
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
			return nullptr;
		}

#undef SWITCH_CASE
	}

	static uint8_t getMinBitsPerBlock(unsigned short nBlocks) {
#define CONDITION(i) \
		if (nBlocks <= PalettedBlockArray<i, Block, Word>::MAX_PALETTE_SIZE) { \
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

		throw std::invalid_argument("invalid capacity " + std::to_string(nBlocks));
	}

	static BlockArray *createBlockArrayCapacity(unsigned short capacity) {
#define CONDITION(i) \
		if (capacity <= PalettedBlockArray<i, Block, Word>::MAX_PALETTE_SIZE) { \
			return new PalettedBlockArray<i, Block, Word>; \
		}

		CONDITION(1)
		CONDITION(2)
		CONDITION(3)
		CONDITION(4)
		CONDITION(5)
		CONDITION(6)
		CONDITION(8)
		CONDITION(16)

		return nullptr;

#undef CONDITION
	}

	BlockArray *blockArray = nullptr;

public:
	BlockArrayContainer(uint8_t bitsPerBlock) {
		blockArray = createBlockArray(bitsPerBlock);
		if (blockArray == nullptr) {
			throw std::invalid_argument("invalid bits-per-block specified: " + std::to_string(bitsPerBlock));
		}
	}

	BlockArrayContainer() {
		BlockArrayContainer(1);
	}

	BlockArrayContainer(uint8_t bitsPerBlock, std::vector<Word> &wordArray, std::vector<Block> &paletteEntries) {
		blockArray = createBlockArray(bitsPerBlock, wordArray, paletteEntries);
		if (blockArray == nullptr) {
			throw std::invalid_argument("invalid bits-per-block specified: " + std::to_string(bitsPerBlock));
		}
	}

	~BlockArrayContainer() {
		delete blockArray;
	}

	const Word *getWordArray(unsigned short &arraySize) const {
		return blockArray->getWordArray(arraySize);
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

				BlockArray *newArray = createBlockArrayCapacity(count + 1);

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
			auto newBitsPerBlock = getMinBitsPerBlock(blockArray->countUniqueBlocks());

			//don't realloc unless the number of unique blocks is different to the palette size
			if (newBitsPerBlock != blockArray->getBitsPerBlock()) {
				BlockArray *newArray = createBlockArray(newBitsPerBlock);
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
