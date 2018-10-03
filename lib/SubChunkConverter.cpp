
#include "SubChunkConverter.h"

#define LOOP(GET_INDEX) \
	bool seen[4096] = {}; \
	unsigned short id1Idx, id2Idx, metaIdx; \
	unsigned short unique = 0; \
	for (auto x = 0; x < 16; ++x) { \
		for (auto z = 0; z < 16; ++z) { \
			for (auto y = 0; y < 8; ++y) { \
				GET_INDEX \
				uint8_t metaByte = nibbleArray[metaIdx]; \
				unsigned short id1 = (byteArray[id1Idx] << 4) | (metaByte & 0xf); \
				unsigned short id2 = (byteArray[id2Idx] << 4) | ((metaByte >> 4) & 0xf); \
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
	BlockArrayContainer<> *result = new BlockArrayContainer<>(BlockArrayContainer<>::getMinBitsPerBlock(unique)); \
	for (auto x = 0; x < 16; ++x) { \
		for (auto z = 0; z < 16; ++z) { \
			for (auto y = 0; y < 8; ++y) { \
				GET_INDEX \
				uint8_t metaByte = nibbleArray[metaIdx]; \
				result->set(x, y << 1, z, (byteArray[id1Idx] << 4) | (metaByte & 0xf)); \
				result->set(x, (y << 1) | 1, z, (byteArray[id2Idx] << 4) | ((metaByte >> 4) & 0xf)); \
			} \
		} \
	} \
	return result;

BlockArrayContainer<> *convertSubChunkXZY(uint8_t * byteArray, uint8_t * nibbleArray) {
	LOOP(
		id1Idx = (x << 8) | (z << 4) | (y << 1);
		id2Idx = id1Idx | 1;
		metaIdx = id1Idx >> 1;
	)
}

static inline uint8_t *rotateByteArray(uint8_t *byteArray) {
	auto result = new uint8_t[4096];
	for (auto y = 0; y < 16; ++y) {
		for (auto z = 0; z < 16; ++z) {
			for (auto x = 0; x < 16; ++x) {
				result[(x << 8) | (z << 4) | y] = byteArray[(y << 8) | (z << 4) | x];
			}
		}
	}
	return result;
}

static inline uint8_t *rotateNibbleArray(uint8_t *nibbleArray) {
	auto result = new uint8_t[2048];

	auto i = 0;
	for (auto x = 0; x < 8; x++) {
		for (auto z = 0; z < 16; z++) {
			auto zx = ((z << 3) | x);
			for (auto y = 0; y < 8; y++) {
				auto byte1 = nibbleArray[(y << 8) | zx];
				auto byte2 = nibbleArray[(y << 8) | zx | 0x80];

				result[i] = ((byte2 << 4) | (byte1 & 0x0f));
				result[i | 0x80] = ((byte1 >> 4) | (byte2 & 0xf0));
				i++;
			}
		}
		i += 128;
	}

	return result;
}

BlockArrayContainer<> *convertSubChunkYZX(uint8_t * byteArray, uint8_t * nibbleArray) {
	auto rotatedByteArray = rotateByteArray(byteArray);
	auto rotatedNibbleArray = rotateNibbleArray(nibbleArray);
	auto result = convertSubChunkXZY(rotatedByteArray, rotatedNibbleArray);

	delete rotatedByteArray;
	delete rotatedNibbleArray;
	return result;
}

BlockArrayContainer<> *convertSubChunkFromLegacyColumn(uint8_t * byteArray, uint8_t * nibbleArray, uint8_t yOffset) {
	LOOP(
		id1Idx = (x << 11) | (z << 7) | (yOffset << 4) | (y << 1);
		id2Idx = id1Idx | 1;
		metaIdx = id1Idx >> 1;
	)
}
