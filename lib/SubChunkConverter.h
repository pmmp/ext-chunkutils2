#ifndef HAVE_SUBCHUNK_CONVERTER_H
#define HAVE_SUBCHUNK_CONVERTER_H

#include "BlockArrayContainer.h"

#include <functional>
#include <new>
#include <gsl/span>

typedef gsl::span<uint8_t, 4096> LegacySubChunkIds;
typedef gsl::span<uint8_t, 2048> LegacySubChunkMetas;
typedef gsl::span<uint8_t, 32768> LegacyChunkColumnIds;
typedef gsl::span<uint8_t, 16384> LegacyChunkColumnMetas;

template<typename getIndexArg>
using getIndexFunc = void(*)(
	int x,
	int y,
	int z,
	unsigned short& id1Idx,
	unsigned short& id2Idx,
	unsigned short& metaIdx,
	getIndexArg extraArg
);
template<typename Block>
using mapperFunc = Block(*)(uint8_t id, uint8_t meta);

using coordinateSwapFunc = void(*)(const int x, const int y, const int z, int& rX, int& rY, int& rZ);

template<
	typename Block,
	typename getIndexArg,
	getIndexFunc<getIndexArg> getIndex,
	mapperFunc<Block> mapper,
	coordinateSwapFunc swapCoordinates
>
class ConverterVisited {
private:
	const getIndexArg extraArg;
	const uint8_t * idArray;
	const uint8_t * metaArray;

public:
	ConverterVisited(const uint8_t * idSpan, const uint8_t * metaSpan, getIndexArg extraArg) :
		idArray(idSpan),
		metaArray(metaSpan),
		extraArg(extraArg) {}

	template<uint8_t BITS_PER_BLOCK>
	void visit(PalettedBlockArray<BITS_PER_BLOCK, Block>& blockArray) {
		unsigned short id1Idx, id2Idx, metaIdx;
		int rX, rY, rZ;
		for (auto x = 0; x < 16; ++x) {
			for (auto z = 0; z < 16; ++z) {
				for (auto y = 0; y < 8; ++y) {
					getIndex(x, y, z, id1Idx, id2Idx, metaIdx, extraArg);

					uint8_t metaByte = metaArray[metaIdx];

					swapCoordinates(x, y << 1, z, rX, rY, rZ);
					blockArray.set(rX, rY, rZ, mapper(idArray[id1Idx], (metaByte & 0xf)));

					swapCoordinates(x, (y << 1) | 1, z, rX, rY, rZ);
					blockArray.set(rX, rY, rZ, mapper(idArray[id2Idx], ((metaByte >> 4) & 0xf)));
				}
			}
		}
	}

	void visit(IPalettedBlockArray<Block>& blockArray) {
		throw std::invalid_argument("unknown block array size");
	}
};

template<
	typename Block,
	typename getIndexArg,
	getIndexFunc<getIndexArg> getIndex,
	mapperFunc<Block> mapper,
	coordinateSwapFunc swapCoordinates
>
static inline void convert(BlockArrayContainer<Block> * result, const gsl::span<uint8_t> &idSpan, const gsl::span<uint8_t> &metaSpan, getIndexArg extraArg) {
	const uint8_t * idArray = idSpan.data();
	const uint8_t * metaArray = metaSpan.data();
	bool seen[4096] = {};
	unsigned short id1Idx, id2Idx, metaIdx;
	unsigned short unique = 0;
	for (auto x = 0; x < 16; ++x) {
		for (auto z = 0; z < 16; ++z) {
			for (auto y = 0; y < 8; ++y) {
				getIndex(x, y, z, id1Idx, id2Idx, metaIdx, extraArg);

				uint8_t metaByte = metaArray[metaIdx];
				unsigned short id1 = (idArray[id1Idx] << 4) | (metaByte & 0xf);
				unsigned short id2 = (idArray[id2Idx] << 4) | ((metaByte >> 4) & 0xf);
				if (!seen[id1]) {

					seen[id1] = true;
					unique++;
				}
				if (!seen[id2]) {

					seen[id2] = true;
					unique++;
				}
			}
		}
	}

	new(result) BlockArrayContainer<Block>(unique);

	ConverterVisited<Block, getIndexArg, getIndex, mapper, swapCoordinates> converter(idArray, metaArray, extraArg);
	result->template specializeForArraySize<void>(converter);
	result->setGarbageCollected();
}

static inline void getIndexSubChunkXZY(int x, int y, int z, unsigned short &id1Idx, unsigned short &id2Idx, unsigned short &metaIdx, int unused) {
	(void)unused;

	id1Idx = (x << 8) | (z << 4) | (y << 1);
	id2Idx = id1Idx | 1;
	metaIdx = id1Idx >> 1;
}

static inline void swapCoordinatesNoOp(int x, int y, int z, int &rX, int &rY, int &rZ) {
	rX = x;
	rY = y;
	rZ = z;
}

template<typename Block, Block(*mapper)(uint8_t id, uint8_t meta)>
void convertSubChunkXZY(BlockArrayContainer<Block> * result, const LegacySubChunkIds &idSpan, const LegacySubChunkMetas &metaSpan) {
	convert<Block, int, getIndexSubChunkXZY, mapper, swapCoordinatesNoOp>(result, idSpan, metaSpan, 0);
}

static inline void swapCoordinatesFromYZX(int x, int y, int z, int& rX, int& rY, int& rZ) {
	rX = y; //yes, this is intended!
	rY = x; //yes, this is also intended!
	rZ = z;
}

template<typename Block, Block(*mapper)(uint8_t id, uint8_t meta)>
void convertSubChunkYZX(BlockArrayContainer<Block> * result, const LegacySubChunkIds &idSpan, const LegacySubChunkMetas &metaSpan) {
	convert<Block, int, getIndexSubChunkXZY, mapper, swapCoordinatesFromYZX>(result, idSpan, metaSpan, 0);
}

static inline void getIndexLegacyColumnXZY(int x, int y, int z, unsigned short &id1Idx, unsigned short &id2Idx, unsigned short &metaIdx, uint8_t yOffset) {
	id1Idx = (x << 11) | (z << 7) | (yOffset << 4) | (y << 1);
	id2Idx = id1Idx | 1;
	metaIdx = id1Idx >> 1;
}

template<typename Block, Block(*mapper)(uint8_t id, uint8_t meta)>
void convertSubChunkFromLegacyColumn(BlockArrayContainer<Block> * result, const LegacyChunkColumnIds &idSpan, const LegacyChunkColumnMetas &metaSpan, const uint8_t yOffset) {
	convert<Block, uint8_t, getIndexLegacyColumnXZY, mapper, swapCoordinatesNoOp>(result, idSpan, metaSpan, yOffset);
}

#endif
