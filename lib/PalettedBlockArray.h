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
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <gsl/span>

#include "VanillaPaletteSize.h"
#include "Palette.h"

template<typename Block>
class IPalettedBlockArray {
protected:
	typedef uint32_t Word;

	using Coord = unsigned char;

	// Whether this block array may have unused entries in its palette. Indicates that garbage collection could reduce the size of the array
	bool mayNeedGC = false;
public:
	static const unsigned char COORD_BIT_SIZE = 4; //number of coordinate bits per dimension
	static const unsigned int COORD_MASK = ~(~0U << COORD_BIT_SIZE);
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
	virtual void setPalette(const gsl::span<Block>& newPalette) = 0;
	virtual unsigned short getPaletteSize() const = 0;
	virtual unsigned short getMaxPaletteSize() const = 0;

	virtual unsigned short countUniqueBlocks() const = 0;

	virtual VanillaPaletteSize getBitsPerBlock() const = 0;
	virtual Block get(Coord x, Coord y, Coord z) const = 0;
	virtual bool set(Coord x, Coord y, Coord z, Block val) = 0;

	virtual unsigned short getPaletteOffset(Coord x, Coord y, Coord z) const = 0;

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
	using Coord = typename Base::Coord;
	static const uint8_t BITS_PER_BLOCK_INT = static_cast<uint8_t>(BITS_PER_BLOCK);
	static const unsigned char BLOCKS_PER_WORD = sizeof(Word) * CHAR_BIT / BITS_PER_BLOCK_INT;
	static const Word BLOCK_MASK = (1 << BITS_PER_BLOCK_INT) - 1;
	static const unsigned short WORD_COUNT = Base::ARRAY_CAPACITY / BLOCKS_PER_WORD + (Base::ARRAY_CAPACITY % BLOCKS_PER_WORD ? 1 : 0);

	static const unsigned int MAX_PALETTE_OFFSET = 1 << BITS_PER_BLOCK_INT;

public:
	static const unsigned int PAYLOAD_SIZE = WORD_COUNT * sizeof(Word);
	static const size_t MAX_PALETTE_SIZE = MAX_PALETTE_OFFSET < Base::ARRAY_CAPACITY ? MAX_PALETTE_OFFSET : Base::ARRAY_CAPACITY;
private:
	using Palette = std::conditional_t<
		BITS_PER_BLOCK >= VanillaPaletteSize::BPB_6,
		LargePalette<MAX_PALETTE_SIZE, Block>,
		SmallPalette<MAX_PALETTE_SIZE, Block>
	>;

	std::array<Word, WORD_COUNT> words;

	Palette palette;

	inline unsigned short getArrayOffset(Coord x, Coord y, Coord z) const {
		return
			((x & Base::COORD_MASK) << (Base::COORD_BIT_SIZE * 2)) |
			((z & Base::COORD_MASK) << Base::COORD_BIT_SIZE) |
			(y & Base::COORD_MASK);
	}

	inline void find(Coord x, Coord y, Coord z, unsigned short &wordIdx, unsigned char &shift) const {
		unsigned short idx = getArrayOffset(x, y, z);

		wordIdx = idx / BLOCKS_PER_WORD;
		shift = (idx % BLOCKS_PER_WORD) * BITS_PER_BLOCK_INT;
	}

	inline unsigned short _getPaletteOffset(Coord x, Coord y, Coord z) const {
		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		return (words[wordIdx] >> shift) & BLOCK_MASK;
	}

	inline void _setPaletteOffset(Coord x, Coord y, Coord z, unsigned short offset) {
		unsigned short wordIdx;
		unsigned char shift;
		find(x, y, z, wordIdx, shift);

		words[wordIdx] = (words[wordIdx] & ~(BLOCK_MASK << shift)) | (offset << shift);
	}

	void locateAndReportInvalidOffset() const {
		//Slow path, to allow giving detailed errors when a problem has already been detected by the fast path
		auto blockCount = 0;
		const auto max = palette.size();

		for (auto wordIdx = 0; wordIdx < words.size(); wordIdx++) {
			const auto word = words[wordIdx];
			for (
				auto shift = 0;
				blockCount < Base::ARRAY_CAPACITY && shift < BLOCKS_PER_WORD * BITS_PER_BLOCK_INT;
				blockCount++, shift += BITS_PER_BLOCK_INT
			) {
				const auto offset = (word >> shift) & BLOCK_MASK;
				if (offset >= max) {
					std::ostringstream ss;

					const auto blockIdx = (wordIdx * BLOCKS_PER_WORD) + (shift / BITS_PER_BLOCK_INT);
					const auto x = (blockIdx >> (Base::COORD_BIT_SIZE * 2)) & Base::COORD_MASK;
					const auto z = (blockIdx >> Base::COORD_BIT_SIZE) & Base::COORD_MASK;
					const auto y = blockIdx & Base::COORD_MASK;

					ss << "offset table contains invalid offset " << offset << " at position " << x << "," << y << "," << z << " (max valid offset: " << (max - 1) << ")";
					throw std::range_error(ss.str());
				}
			}
		}
	}

#ifdef _MSC_VER
	//If this function gets inlined, MSVC 16.29 does not want to vectorize it :(
	__declspec(noinline)
#endif
	bool fastValidate() const {
		Word invalid = 0;

		Word expected = 0;
		int max = palette.size();
		for (unsigned int shift = 0; shift < BLOCKS_PER_WORD * BITS_PER_BLOCK_INT; shift += BITS_PER_BLOCK_INT) {
			expected |= ((max - 1) << shift);
		}

		//Fast path - use carry-out vectors to detect invalid offsets
		//This trick is borrowed from https://devblogs.microsoft.com/oldnewthing/20190301-00/?p=101076
		//We can't detect exactly where the error is with this approach, but we leave that up to the slow code if we detect an error
		for (auto wordIdx = 0; wordIdx < words.size(); wordIdx++) {
			const auto word = words[wordIdx];

			if (BITS_PER_BLOCK_INT == 1) {
				//For 1 bits-per-block, we can only reach this point if the palette isn't full, which means that only offset 0 is set.
				//This means we can save a few instructions and just check the word directly for non-zero bits.
				invalid |= word;
			} else {
				Word carryVector = (~expected & word) | (~(expected ^ word) & (expected - word));
				invalid |= carryVector;
			}
		}

		return invalid == 0;
	}

	void validate() const {
		if (MAX_PALETTE_OFFSET == MAX_PALETTE_SIZE && palette.size() >= MAX_PALETTE_SIZE) {
			//Every possible offset representable is valid, therefore no validation is required
			//this is an uncommon case, but more frequent in small palettes, which is a win because small palettes are more
			//expensive to verify than big ones due to more bitwise operations needed to extract the offsets
			//for 16 bpb, offsets may point beyond the end of the palette even at full capacity, so they must always be checked
			return;
		}

		if (!fastValidate()) {
			//If we detected an error, use the old, slow method to locate the (first) source of errors
			//this allows giving a detailed error message
			//if the palette is valid, we can avoid this slow path entirely, which is the most likely outcome.
			locateAndReportInvalidOffset();
		}
	}

public:

	PalettedBlockArray(Block block) : palette(block) {
		memset(words.data(), 0, sizeof(words));
	}

	PalettedBlockArray(gsl::span<uint8_t> &wordArray, std::vector<Block> &paletteEntries) : palette(paletteEntries) {
		if (wordArray.size() != sizeof(words)) {
			//TODO: known-size span can replace this check
			throw std::length_error("word array size should be exactly " + std::to_string(sizeof(words)) + " bytes for a " + std::to_string(BITS_PER_BLOCK_INT) + "bpb block array, got " + std::to_string(wordArray.size()) + " bytes");
		}
		memcpy(words.data(), wordArray.data(), sizeof(words));

		validate();

		this->mayNeedGC = true;
	}

	PalettedBlockArray(const PalettedBlockArray &otherArray) : palette(otherArray.palette) {
		memcpy(words.data(), otherArray.words.data(), sizeof(words));
		this->mayNeedGC = otherArray.mayNeedGC;
	}

	const gsl::span<const char> getWordArray() const {
		return gsl::span<const char>((const char *) words.data(), sizeof(words));
	}

	const gsl::span<const Block> getPalette() const {
		return palette.getPalette();
	}

	void setPalette(const gsl::span<Block>& newPalette) {
		if (newPalette.size() != palette.size()) {
			throw std::length_error(
				"new palette must be the same size as the old one, expected " +
				std::to_string(palette.size()) +
				" but received " +
				std::to_string(newPalette.size())
			);
		}
		palette = Palette(newPalette);
	}

	unsigned short getPaletteSize() const {
		return palette.size();
	}

	unsigned short getMaxPaletteSize() const {
		return MAX_PALETTE_SIZE;
	}

	unsigned short countUniqueBlocks() const {
		std::unordered_set<Block> hasFound;

		for (Coord x = 0; x < Base::ARRAY_DIM; ++x) {
			for (Coord z = 0; z < Base::ARRAY_DIM; ++z) {
				for (Coord y = 0; y < Base::ARRAY_DIM; ++y) {
					auto inserted = hasFound.insert(palette.get(_getPaletteOffset(x, y, z))).second;
					if (inserted && hasFound.size() == palette.size()) {
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

	Block get(Coord x, Coord y, Coord z) const {
		unsigned short offset = _getPaletteOffset(x, y, z);
		assert(offset < palette.size());
		return palette.get(offset);
	}

	bool set(Coord x, Coord y, Coord z, Block val) {
		int offset = palette.addOrLookup(val);
		bool needGC = true;

		if (offset == -1) {
			if (MAX_PALETTE_SIZE < Base::ARRAY_CAPACITY || this->mayNeedGC) {
				return false;
			}
			//overwrite existing offset on fully used, non-dirty palette
			offset = _getPaletteOffset(x, y, z);
			//we skip GC because:
			//- we know this block isn't already in the palette
			//- we know every block in the array has its own palette entry (palette full and not dirty), therefore we must be overwriting an entry that's only used by 1 block anyway.
			needGC = false;
			palette.set(offset, val);
		}

		_setPaletteOffset(x, y, z, offset);
		this->mayNeedGC = needGC;
		return true;
	}

	unsigned short getPaletteOffset(Coord x, Coord y, Coord z) const {
		return _getPaletteOffset(x, y, z);
	}

	void replaceAll(Block from, Block to) {
		//TODO: clean up any duplicates
		for (short i = 0; i < palette.size(); ++i) {
			if (palette.get(i) == from) {
				palette.set(i, to);

				//don't return here, because there might be duplicated block states from previous replace operations
			}
		}
		this->mayNeedGC = true; //operation might create duplicate entries
	}

	void convertFrom(const IPalettedBlockArray<Block> &otherArray) {
		for (Coord x = 0; x < Base::ARRAY_DIM; ++x) {
			for (Coord z = 0; z < Base::ARRAY_DIM; ++z) {
				for (Coord y = 0; y < Base::ARRAY_DIM; ++y) {
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
		this->palette = Palette(otherPalette);

		for (Coord x = 0; x < Base::ARRAY_DIM; ++x) {
			for (Coord z = 0; z < Base::ARRAY_DIM; ++z) {
				for (Coord y = 0; y < Base::ARRAY_DIM; ++y) {
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
	using Coord = typename IPalettedBlockArray<Block>::Coord;
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

	void setPalette(const gsl::span<Block>& newPalette) {
		if (newPalette.size() != 1) {
			throw std::length_error("new palette must be the same size as the old one, expected 1 but received " + std::to_string(newPalette.size()));
		}
		block = newPalette[0];
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

	Block get(Coord, Coord, Coord) const {
		return block;
	}

	bool set(Coord, Coord, Coord, Block val) {
		return val == block;
	}

	unsigned short getPaletteOffset(Coord, Coord, Coord) const {
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
