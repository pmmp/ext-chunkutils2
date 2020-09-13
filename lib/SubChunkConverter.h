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

#define LOOP(GET_INDEX) \
	const uint8_t * idArray = idSpan.data(); \
	const uint8_t * metaArray = metaSpan.data(); \
	bool seen[4096] = {}; \
	unsigned short id1Idx, id2Idx, metaIdx; \
	unsigned short unique = 0; \
	for (auto x = 0; x < 16; ++x) { \
		for (auto z = 0; z < 16; ++z) { \
			for (auto y = 0; y < 8; ++y) { \
				GET_INDEX \
				uint8_t metaByte = metaArray[metaIdx]; \
				unsigned short id1 = (idArray[id1Idx] << 4) | (metaByte & 0xf); \
				unsigned short id2 = (idArray[id2Idx] << 4) | ((metaByte >> 4) & 0xf); \
				if(!seen[id1]){ \
					seen[id1] = true; \
					unique++; \
				} \
				if(!seen[id2]){ \
					seen[id2] = true; \
					unique++; \
				} \
			} \
		} \
	} \
	new(result) BlockArrayContainer<Block>(unique); \
	for (auto x = 0; x < 16; ++x) { \
		for (auto z = 0; z < 16; ++z) { \
			for (auto y = 0; y < 8; ++y) { \
				GET_INDEX \
				uint8_t metaByte = metaArray[metaIdx]; \
				result->set(x, y << 1, z, mapper(idArray[id1Idx], (metaByte & 0xf))); \
				result->set(x, (y << 1) | 1, z, mapper(idArray[id2Idx], ((metaByte >> 4) & 0xf))); \
			} \
		} \
	} \
	result->setGarbageCollected(); \

template<typename Block>
void convertSubChunkXZY(BlockArrayContainer<Block> * result, const LegacySubChunkIds &idSpan, const LegacySubChunkMetas &metaSpan, const std::function<Block(uint8_t id, uint8_t meta)> mapper) {
	LOOP(
		id1Idx = (x << 8) | (z << 4) | (y << 1);
		id2Idx = id1Idx | 1;
		metaIdx = id1Idx >> 1;
	)
}

static inline void rotateByteArray(const LegacySubChunkIds &byteArray, const LegacySubChunkIds &resultArray) {
	const auto data = byteArray.data();
	auto result = resultArray.data();
	for (auto y = 0; y < 16; ++y) {
		for (auto z = 0; z < 16; ++z) {
			for (auto x = 0; x < 16; ++x) {
				result[(x << 8) | (z << 4) | y] = data[(y << 8) | (z << 4) | x];
			}
		}
	}
}

static inline void rotateNibbleArray(const LegacySubChunkMetas &nibbleArray, const LegacySubChunkMetas &resultArray) {
	const auto data = nibbleArray.data();
	auto result = resultArray.data();
	auto i = 0;
	for (auto x = 0; x < 8; x++) {
		for (auto z = 0; z < 16; z++) {
			auto zx = ((z << 3) | x);
			for (auto y = 0; y < 8; y++) {
				uint8_t byte1 = data[(y << 8) | zx];
				uint8_t byte2 = data[(y << 8) | zx | 0x80];

				result[i] = ((byte2 << 4) | (byte1 & 0x0f));
				result[i | 0x80] = ((byte1 >> 4) | (byte2 & 0xf0));
				i++;
			}
		}
		i += 128;
	}
}

template<typename Block>
void convertSubChunkYZX(BlockArrayContainer<Block> * result, const LegacySubChunkIds &idSpan, const LegacySubChunkMetas &metaSpan, const std::function<Block(uint8_t id, uint8_t meta)> mapper) {
	uint8_t rotatedByteArray[4096];
	uint8_t rotatedNibbleArray[2048];
	LegacySubChunkIds rotatedIds(rotatedByteArray);
	LegacySubChunkMetas rotatedMetas(rotatedNibbleArray);

	rotateByteArray(idSpan, rotatedIds);
	rotateNibbleArray(metaSpan, rotatedMetas);
	convertSubChunkXZY<Block>(result, rotatedIds, rotatedMetas, mapper);
}

template<typename Block>
void convertSubChunkFromLegacyColumn(BlockArrayContainer<Block> * result, const LegacyChunkColumnIds &idSpan, const LegacyChunkColumnMetas &metaSpan, const uint8_t yOffset, const std::function<Block(uint8_t id, uint8_t meta)> mapper) {
	LOOP(
		id1Idx = (x << 11) | (z << 7) | (yOffset << 4) | (y << 1);
		id2Idx = id1Idx | 1;
		metaIdx = id1Idx >> 1;
	)
}

#endif
