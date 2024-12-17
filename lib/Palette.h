#ifndef HAVE_PALETTE_H
#define HAVE_PALETTE_H

template <size_t MAX_PALETTE_SIZE>
class PaletteUtils final {
public:
	static void checkSize(size_t size) {
		if (size > MAX_PALETTE_SIZE) {
			throw std::length_error("palette size should be at most " + std::to_string(MAX_PALETTE_SIZE) + " entries, but received " + std::to_string(size) + " entries");
		}
		if (size == 0) {
			throw std::length_error("palette cannot have a zero size");
		}
	}
};

/*
* Small palettes use fixed arrays to avoid vector indirections and allocations
* Performance is more important here because the amount of wasted memory is insignificant
* regardless of the number of elements in the palette.
*
* Lookups are done with a linear search, which is faster than using a hash map for small
* numbers of elements.
*/
template <size_t MAX_PALETTE_SIZE, typename Block>
class SmallPalette final {
private:
	std::array<Block, MAX_PALETTE_SIZE> palette;
	unsigned short nextPaletteIndex = 0;

	void initFromData(gsl::span<const Block> paletteEntries) {
		PaletteUtils<MAX_PALETTE_SIZE>::checkSize(paletteEntries.size());

		memcpy(palette.data(), paletteEntries.data(), paletteEntries.size() * sizeof(Block));
		nextPaletteIndex = (unsigned short)paletteEntries.size();
	}
public:
	SmallPalette(Block block) {
		palette[nextPaletteIndex++] = block;
	}

	SmallPalette(std::vector<Block>& paletteEntries) {
		initFromData(gsl::span<const Block>(paletteEntries.data(), paletteEntries.size()));
	}

	SmallPalette(gsl::span<const Block>& paletteEntries) {
		initFromData(paletteEntries);
	}

	SmallPalette(const SmallPalette& otherArray) {
		memcpy(palette.data(), otherArray.palette.data(), sizeof(palette));
		nextPaletteIndex = otherArray.nextPaletteIndex;
	}

	Block get(unsigned int offset) const {
		return palette[offset];
	}

	void set(unsigned int offset, Block val) {
		palette[offset] = val;
	}

	const gsl::span<const Block> getPalette() const {
		return gsl::span<const Block>(palette.data(), nextPaletteIndex);
	}

	size_t size() const {
		return nextPaletteIndex;
	}

	int addOrLookup(Block val) {
		for (int offset = 0; offset < nextPaletteIndex; ++offset) {
			if (palette[offset] == val) {
				return offset;
			}
		}

		if (nextPaletteIndex >= MAX_PALETTE_SIZE) {
			return -1;
		}
		int offset = nextPaletteIndex++;
		palette[offset] = val;
		return offset;
	}
};

/*
* For large numbers of elements, it's common for a significant fraction of the palette's
* capacity to be unused, so using a fixed array like SmallPalette would waste a lot of
* memory. We use a vector instead and pay a small performance penalty.
*
* We use the memory we saved to instead keep a hash map of block -> offset for lookups,
* which significantly improves performance for large palettes. (We only technically save
* memory as long as the palette is less than half (?) full, but the performance benefits
* remain in any case.)
*/
template <size_t MAX_PALETTE_SIZE, typename Block>
class LargePalette final {
private:
	std::vector<Block> palette;
	std::unordered_map<Block, unsigned int> blockToOffset;

	void initFromData(gsl::span<const Block> paletteEntries) {
		PaletteUtils<MAX_PALETTE_SIZE>::checkSize(paletteEntries.size());

		palette = std::vector<Block>(paletteEntries.begin(), paletteEntries.end());
		for (unsigned int i = 0; i < palette.size(); ++i) {
			blockToOffset[palette[i]] = i;
		}
	}

public:
	LargePalette(Block block) {
		palette.push_back(block);
		blockToOffset[block] = 0;
	}

	LargePalette(std::vector<Block>& paletteEntries) {
		initFromData(gsl::span<const Block>(paletteEntries.data(), paletteEntries.size()));
	}

	LargePalette(gsl::span<const Block>& paletteEntries) {
		initFromData(paletteEntries);
	}

	LargePalette(const LargePalette& otherArray) {
		palette = otherArray.palette;
		blockToOffset = otherArray.blockToOffset;
	}

	Block get(unsigned int offset) const {
		return palette[offset];
	}

	void set(unsigned int offset, Block val) {
		palette[offset] = val;
	}

	const gsl::span<const Block> getPalette() const {
		return gsl::span<const Block>(palette.data(), palette.size());
	}

	size_t size() const {
		return palette.size();
	}

	int addOrLookup(Block val) {
		auto it = blockToOffset.find(val);
		if (it != blockToOffset.end()) {
			return it->second;
		}

		if (palette.size() >= MAX_PALETTE_SIZE) {
			return -1;
		}
		int offset = palette.size();
		palette.push_back(val);
		blockToOffset[val] = offset;
		return offset;
	}
};
#endif
