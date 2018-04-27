#pragma once

#include <assert.h>
#include <stdint.h>

#include <array>
#include <bitset>
#include <climits>
#include <exception>
#include <string>
#include <vector>

template<typename Block, typename Word>
class IPalettedBlockArray {
protected:
	// Whether this block array may have unused entries in its palette. Indicates that garbage collection could reduce the size of the array
	bool mayNeedGC = false;
public:
	static const unsigned char COORD_BIT_SIZE = 4; //number of coordinate bits per dimension
	static const unsigned char ARRAY_DIM = 1 << COORD_BIT_SIZE; //length of one side of chunk
	static const unsigned short ARRAY_CAPACITY = ARRAY_DIM * ARRAY_DIM * ARRAY_DIM; //number of blocks in chunk

	bool needsGarbageCollection() const {
		return mayNeedGC;
	}

	void setGarbageCollected() {
		mayNeedGC = false;
	}

	virtual const Word *getWordArray(unsigned short &arraySize) const = 0;

	virtual const Block *getPalette(unsigned short &paletteSize) const = 0;
	virtual unsigned short getPaletteSize() const = 0;
	virtual unsigned short getMaxPaletteSize() const = 0;

	virtual unsigned short countUniqueBlocks() const = 0;

	virtual uint8_t getBitsPerBlock() const = 0;
	virtual Block get(unsigned char x, unsigned char y, unsigned char z) const = 0;
	virtual bool set(unsigned char x, unsigned char y, unsigned char z, Block val) = 0;

	virtual bool replaceAll(Block from, Block to) = 0;

	virtual bool convertFrom(IPalettedBlockArray &otherArray) = 0;
};

template <uint8_t BITS_PER_BLOCK, typename Block, typename Word>
class PalettedBlockArray : public IPalettedBlockArray<Block, Word> {
public:
	static const unsigned char BLOCKS_PER_WORD = sizeof(Word) * CHAR_BIT / BITS_PER_BLOCK;
	static const Word BLOCK_MASK = (1 << BITS_PER_BLOCK) - 1;
	static const unsigned short WORD_COUNT = ARRAY_CAPACITY / BLOCKS_PER_WORD + (ARRAY_CAPACITY % BLOCKS_PER_WORD ? 1 : 0);
	static const unsigned short MAX_PALETTE_SIZE = (1 << BITS_PER_BLOCK) < ARRAY_CAPACITY ? (1 << BITS_PER_BLOCK) : ARRAY_CAPACITY;

private:
	std::array<Word, WORD_COUNT> words;

	//TODO: use a vector for this instead of a fixed array? might be less performant but will save memory for large formats
	std::array<Block, MAX_PALETTE_SIZE> palette;

	unsigned short nextPaletteIndex = 0;

	inline unsigned short getArrayOffset(unsigned char x, unsigned char y, unsigned char z) const {
		return (x << (COORD_BIT_SIZE * 2)) | (z << COORD_BIT_SIZE) | y;
	}

	inline void find(unsigned char x, unsigned char y, unsigned char z, unsigned short &wordIdx, unsigned char &shift) const {
		assert(x < ARRAY_DIM && y < ARRAY_DIM && z < ARRAY_DIM);
		unsigned short idx = getArrayOffset(x, y, z);

		wordIdx = idx / BLOCKS_PER_WORD;
		shift = (idx % BLOCKS_PER_WORD) * BITS_PER_BLOCK;
	}

	inline unsigned short getPaletteOffset(unsigned char x, unsigned char y, unsigned char z) const {
		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		return (words[wordIdx] >> shift) & BLOCK_MASK;
	}

public:

	PalettedBlockArray() {
		memset(words.data(), 0, words.size());
	}

	PalettedBlockArray(std::vector<Word> &wordArray, std::vector<Block> &paletteEntries) {
		if (wordArray.size() != words.size()) {
			throw std::length_error("word array size should be exactly " + std::to_string(WORD_COUNT) + " words for a " + std::to_string(BITS_PER_BLOCK) + "bpb block array, got " + std::to_string(wordArray.size()) + " words");
		}
		if (paletteEntries.size() > MAX_PALETTE_SIZE) {
			throw std::length_error("palette size should be at most " + std::to_string(MAX_PALETTE_SIZE) + " entries for a " + std::to_string(BITS_PER_BLOCK) + "bpb block array, got " + std::to_string(paletteEntries.size()) + " entries");
		}
		if (paletteEntries.size() == 0) {
			throw std::length_error("palette cannot have a zero size");
		}

		memcpy(words.data(), wordArray.data(), words.size() * sizeof(Word));
		memcpy(palette.data(), paletteEntries.data(), paletteEntries.size() * sizeof(Block));
		nextPaletteIndex = (unsigned short)paletteEntries.size();
	}

	const Word *getWordArray(unsigned short &arraySize) const {
		arraySize = (unsigned short)words.size();
		return words.data();
	}

	const Block *getPalette(unsigned short &paletteSize) const {
		paletteSize = nextPaletteIndex;
		return palette.data();
	}

	unsigned short getPaletteSize() const {
		return nextPaletteIndex;
	}

	unsigned short getMaxPaletteSize() const {
		return MAX_PALETTE_SIZE;
	}

	unsigned short countUniqueBlocks() const {
		std::bitset<MAX_PALETTE_SIZE> hasFound;
		unsigned short count = 0;

		for (unsigned char x = 0; x < ARRAY_DIM; ++x) {
			for (unsigned char z = 0; z < ARRAY_DIM; ++z) {
				for (unsigned char y = 0; y < ARRAY_DIM; ++y) {
					unsigned short paletteOffset = getPaletteOffset(x, y, z);
					if (!hasFound[paletteOffset]) {
						++count;
						hasFound[paletteOffset] = true;
					}
				}
			}
		}

		return count;
	}

	uint8_t getBitsPerBlock() const {
		return BITS_PER_BLOCK;
	}

	Block get(unsigned char x, unsigned char y, unsigned char z) const {
		unsigned short offset = getPaletteOffset(x, y, z);
		assert(offset < nextPaletteIndex);
		return palette[offset];
	}

	bool set(unsigned char x, unsigned char y, unsigned char z, Block val) {
		//TODO (suggested by sandertv): check performance when recording last written block and palette offset - might improve performance for repetetive writes

		short offset = -1;
		for (short i = 0; i < nextPaletteIndex; ++i) {
			if (palette[i] == val) {
				offset = i;
				break;
			}
		}

		if (offset == -1) {
			if (nextPaletteIndex >= MAX_PALETTE_SIZE) {
				return false;
			}
			offset = (short)nextPaletteIndex++;
			palette[offset] = val;
			mayNeedGC = true;
		}

		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		words[wordIdx] = (words[wordIdx] & ~(BLOCK_MASK << shift)) | (offset << shift);
		return true;
	}

	bool replaceAll(Block from, Block to) {
		//TODO: clean up any duplicates
		bool retval = false;
		for (short i = 0; i < nextPaletteIndex; ++i) {
			if (palette[i] == from) {
				palette[i] = to;

				//don't return here, because there might be duplicated block states from previous replace operations
				retval = true;
			}
		}

		return retval;
	}

	bool convertFrom(IPalettedBlockArray &otherArray) {
		for (unsigned char x = 0; x < ARRAY_DIM; ++x) {
			for (unsigned char z = 0; z < ARRAY_DIM; ++z) {
				for (unsigned char y = 0; y < ARRAY_DIM; ++y) {
					if (!set(x, y, z, otherArray.get(x, y, z))) {
						return false;
					}
				}
			}
		}

		return true;
	}
};
