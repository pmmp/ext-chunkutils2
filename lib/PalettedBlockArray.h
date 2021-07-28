#pragma once
#ifndef HAVE_PALETTED_BLOCK_ARRAY_H
#define HAVE_PALETTED_BLOCK_ARRAY_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <array>
#include <climits>
#include <exception>
#include <string>
#include <unordered_set>
#include <vector>
#include <gsl/span>

#include "VanillaPaletteSize.h"

template<typename Block>
class IPalettedBlockArray {
protected:
	typedef uint32_t Word;

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

	virtual const gsl::span<const char> getWordArray() const = 0;

	virtual const gsl::span<const Block> getPalette() const = 0;
	virtual unsigned short getPaletteSize() const = 0;
	virtual unsigned short getMaxPaletteSize() const = 0;

	virtual unsigned short countUniqueBlocks() const = 0;

	virtual VanillaPaletteSize getBitsPerBlock() const = 0;
	virtual Block get(unsigned char x, unsigned char y, unsigned char z) const = 0;
	virtual bool set(unsigned char x, unsigned char y, unsigned char z, Block val) = 0;

	virtual unsigned short getPaletteOffset(unsigned char x, unsigned char y, unsigned char z) const = 0;

	virtual void replaceAll(Block from, Block to) = 0;

	virtual void convertFrom(const IPalettedBlockArray<Block> &otherArray) = 0;
	virtual void fastUpsize(const IPalettedBlockArray<Block> &otherArray) = 0;

	virtual IPalettedBlockArray<Block> *clone() const = 0;

	virtual ~IPalettedBlockArray() {}
};

template <VanillaPaletteSize BITS_PER_BLOCK, typename Block>
class PalettedBlockArray final : public IPalettedBlockArray<Block> {
private:
	using Base = IPalettedBlockArray<Block>;
	using Word = typename Base::Word;
	static const uint8_t BITS_PER_BLOCK_INT = static_cast<uint8_t>(BITS_PER_BLOCK);
	static const unsigned char BLOCKS_PER_WORD = sizeof(Word) * CHAR_BIT / BITS_PER_BLOCK_INT;
	static const Word BLOCK_MASK = (1 << BITS_PER_BLOCK_INT) - 1;
	static const unsigned short WORD_COUNT = Base::ARRAY_CAPACITY / BLOCKS_PER_WORD + (Base::ARRAY_CAPACITY % BLOCKS_PER_WORD ? 1 : 0);
public:
	static const unsigned int PAYLOAD_SIZE = WORD_COUNT * sizeof(Word);
	static const unsigned short MAX_PALETTE_SIZE = (1 << BITS_PER_BLOCK_INT) < Base::ARRAY_CAPACITY ? (1 << BITS_PER_BLOCK_INT) : Base::ARRAY_CAPACITY;

private:
	std::array<Word, WORD_COUNT> words;

	//TODO: use a vector for this instead of a fixed array? might be less performant but will save memory for large formats
	std::array<Block, MAX_PALETTE_SIZE> palette;

	unsigned short nextPaletteIndex = 0;

	inline unsigned short getArrayOffset(unsigned char x, unsigned char y, unsigned char z) const {
		return (x << (Base::COORD_BIT_SIZE * 2)) | (z << Base::COORD_BIT_SIZE) | y;
	}

	inline void find(unsigned char x, unsigned char y, unsigned char z, unsigned short &wordIdx, unsigned char &shift) const {
		assert(x < Base::ARRAY_DIM && y < Base::ARRAY_DIM && z < Base::ARRAY_DIM);
		unsigned short idx = getArrayOffset(x, y, z);

		wordIdx = idx / BLOCKS_PER_WORD;
		shift = (idx % BLOCKS_PER_WORD) * BITS_PER_BLOCK_INT;
	}

	inline unsigned short _getPaletteOffset(unsigned char x, unsigned char y, unsigned char z) const {
		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		return (words[wordIdx] >> shift) & BLOCK_MASK;
	}

	inline void _setPaletteOffset(unsigned char x, unsigned char y, unsigned char z, unsigned short offset) {
		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		words[wordIdx] = (words[wordIdx] & ~(BLOCK_MASK << shift)) | (offset << shift);
	}

public:

	PalettedBlockArray(Block block) {
		memset(words.data(), 0, sizeof(words));
		palette[nextPaletteIndex++] = block;
	}

	PalettedBlockArray(gsl::span<uint8_t> &wordArray, std::vector<Block> &paletteEntries) {
		if (wordArray.size() != sizeof(words)) {
			//TODO: known-size span can replace this check
			throw std::length_error("word array size should be exactly " + std::to_string(sizeof(words)) + " bytes for a " + std::to_string(BITS_PER_BLOCK_INT) + "bpb block array, got " + std::to_string(wordArray.size()) + " bytes");
		}
		if (paletteEntries.size() > MAX_PALETTE_SIZE) {
			throw std::length_error("palette size should be at most " + std::to_string(MAX_PALETTE_SIZE) + " entries for a " + std::to_string(BITS_PER_BLOCK_INT) + "bpb block array, got " + std::to_string(paletteEntries.size()) + " entries");
		}
		if (paletteEntries.size() == 0) {
			throw std::length_error("palette cannot have a zero size");
		}

		memcpy(words.data(), wordArray.data(), sizeof(words));
		memcpy(palette.data(), paletteEntries.data(), paletteEntries.size() * sizeof(Block));
		nextPaletteIndex = (unsigned short)paletteEntries.size();
		this->mayNeedGC = true;
	}

	PalettedBlockArray(const PalettedBlockArray &otherArray) {
		memcpy(words.data(), otherArray.words.data(), sizeof(words));
		memcpy(palette.data(), otherArray.palette.data(), sizeof(palette));
		nextPaletteIndex = otherArray.nextPaletteIndex;
		this->mayNeedGC = otherArray.mayNeedGC;
	}

	const gsl::span<const char> getWordArray() const {
		return gsl::span<const char>((const char *) words.data(), sizeof(words));
	}

	const gsl::span<const Block> getPalette() const {
		return gsl::span<const Block>(palette.data(), nextPaletteIndex);
	}

	unsigned short getPaletteSize() const {
		return nextPaletteIndex;
	}

	unsigned short getMaxPaletteSize() const {
		return MAX_PALETTE_SIZE;
	}

	unsigned short countUniqueBlocks() const {
		std::unordered_set<Block> hasFound;

		for (unsigned char x = 0; x < Base::ARRAY_DIM; ++x) {
			for (unsigned char z = 0; z < Base::ARRAY_DIM; ++z) {
				for (unsigned char y = 0; y < Base::ARRAY_DIM; ++y) {
					auto inserted = hasFound.insert(palette[_getPaletteOffset(x, y, z)]).second;
					if (inserted && hasFound.size() == getPaletteSize()) {
						break;
					}
				}
			}
		}

		return hasFound.size();
	}

	VanillaPaletteSize getBitsPerBlock() const {
		return BITS_PER_BLOCK;
	}

	Block get(unsigned char x, unsigned char y, unsigned char z) const {
		unsigned short offset = _getPaletteOffset(x, y, z);
		assert(offset < nextPaletteIndex);
		return palette[offset];
	}

	bool set(unsigned char x, unsigned char y, unsigned char z, Block val) {
		//TODO (suggested by sandertv): check performance when recording last written block and palette offset - might improve performance for repetetive writes

		short offset = -1;
		bool needGC = true;
		for (short i = 0; i < nextPaletteIndex; ++i) {
			if (palette[i] == val) {
				offset = i;
				break;
			}
		}

		if (offset == -1) {
			if (nextPaletteIndex >= MAX_PALETTE_SIZE) {
				if (MAX_PALETTE_SIZE < Base::ARRAY_CAPACITY || this->mayNeedGC) {
					return false;
				}
				//overwrite existing offset on fully used, non-dirty palette
				offset = _getPaletteOffset(x, y, z);
				//we skip GC because:
				//- we know this block isn't already in the palette
				//- we know every block in the array has its own palette entry (palette full and not dirty), therefore we must be overwriting an entry that's only used by 1 block anyway.
				needGC = false;
			} else {
				offset = (short)nextPaletteIndex++;
			}
			palette[offset] = val;
		}

		_setPaletteOffset(x, y, z, offset);
		this->mayNeedGC = needGC;
		return true;
	}

	unsigned short getPaletteOffset(unsigned char x, unsigned char y, unsigned char z) const {
		return _getPaletteOffset(x, y, z);
	}

	void replaceAll(Block from, Block to) {
		//TODO: clean up any duplicates
		for (short i = 0; i < nextPaletteIndex; ++i) {
			if (palette[i] == from) {
				palette[i] = to;

				//don't return here, because there might be duplicated block states from previous replace operations
			}
		}
		this->mayNeedGC = true; //operation might create duplicate entries
	}

	void convertFrom(const IPalettedBlockArray<Block> &otherArray) {
		for (unsigned char x = 0; x < Base::ARRAY_DIM; ++x) {
			for (unsigned char z = 0; z < Base::ARRAY_DIM; ++z) {
				for (unsigned char y = 0; y < Base::ARRAY_DIM; ++y) {
					if (!set(x, y, z, otherArray.get(x, y, z))) {
						throw std::range_error("out of palette space");
					}
				}
			}
		}
	}
private:
	template<typename BlockArray>
	void _fastUpsize(const BlockArray& otherArray) {
		auto otherPalette = otherArray.getPalette();
		nextPaletteIndex = otherPalette.size();
		std::copy(otherPalette.data(), otherPalette.data() + otherPalette.size(), palette.data());
		for (unsigned char x = 0; x < Base::ARRAY_DIM; ++x) {
			for (unsigned char z = 0; z < Base::ARRAY_DIM; ++z) {
				for (unsigned char y = 0; y < Base::ARRAY_DIM; ++y) {
					_setPaletteOffset(x, y, z, otherArray.getPaletteOffset(x, y, z));
				}
			}
		}

		//we directly copy palette from the old variant; it might have holes
		this->mayNeedGC = otherArray.needsGarbageCollection();
	}

public:
	void fastUpsize(const IPalettedBlockArray<Block> &otherArray) {
#define BPB_MACRO(i) \
		if (BITS_PER_BLOCK_INT >= i) { \
			if (auto casted = dynamic_cast<const PalettedBlockArray<VanillaPaletteSize::BPB_##i, Block>*>(&otherArray)) { \
				return _fastUpsize(*casted); \
			} \
		}
		MACRO_PER_BITS_PER_BLOCK
#undef BPB_MACRO

		//fallback for alternative implementations
		if (BITS_PER_BLOCK_INT < static_cast<uint8_t>(otherArray.getBitsPerBlock())) {
			throw std::logic_error("Cannot upsize from a larger palette. Something has gone horribly wrong.");
		}
		_fastUpsize(otherArray);
	}

	Base *clone() const {
		return new PalettedBlockArray(*this);
	}
};

template <typename Block>
class PalettedBlockArray<VanillaPaletteSize::BPB_0, Block> final : public IPalettedBlockArray<Block> {
private:
	Block block;
public:
	static const unsigned int PAYLOAD_SIZE = 0;
	static const unsigned int MAX_PALETTE_SIZE = 1;

	PalettedBlockArray(Block block) {
		this->block = block;
	}

	PalettedBlockArray(gsl::span<uint8_t>& wordArray, std::vector<Block>& paletteEntries) {
		if (wordArray.size() != PAYLOAD_SIZE) {
			throw std::invalid_argument("empty word array expected for zero bits-per-block");
		}
		if (paletteEntries.size() != MAX_PALETTE_SIZE) {
			throw std::invalid_argument("expected exactly 1 palette entry for zero bits-per-block");
		}

		block = paletteEntries[0];
	}

	PalettedBlockArray(const PalettedBlockArray& other) {
		block = other.block;
	}

	const gsl::span<const char> getWordArray() const {
		return gsl::span<const char>();
	}

	const gsl::span<const Block> getPalette() const {
		return gsl::span<const Block>(&block, 1);
	}

	unsigned short getPaletteSize() const {
		return 1;
	}

	unsigned short getMaxPaletteSize() const {
		return 1;
	}

	unsigned short countUniqueBlocks() const {
		return 1;
	}

	VanillaPaletteSize getBitsPerBlock() const {
		return VanillaPaletteSize::BPB_0;
	}

	Block get(unsigned char x, unsigned char y, unsigned char z) const {
		return block;
	}

	bool set(unsigned char x, unsigned char y, unsigned char z, Block val) {
		return val == block;
	}

	unsigned short getPaletteOffset(unsigned char x, unsigned char y, unsigned char z) const {
		return 0;
	}

	void replaceAll(Block from, Block to) {
		if (block == from) {
			block = to;
		}
	}

	void convertFrom(const IPalettedBlockArray<Block>& otherArray) {
		if (otherArray.countUniqueBlocks() > MAX_PALETTE_SIZE) {
			throw std::range_error("palette too large");
		}
		block = otherArray.get(0, 0, 0);
	}

	void fastUpsize(const IPalettedBlockArray<Block>& otherArray) {
		throw std::logic_error("Upsizing to zero-bits-per-block doesn't make any sense. Something has gone horribly wrong.");
	}

	IPalettedBlockArray<Block>* clone() const {
		return new PalettedBlockArray(*this);
	}
};
#endif
