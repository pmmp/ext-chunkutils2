
#include "SubChunkConverter.h"

#define LOOP_1 \
	BlockArrayContainer<> *result = new BlockArrayContainer<>(16); \
	for (auto x = 0; x < 16; ++x) { \
		for (auto z = 0; z < 16; ++z) { \
			for (auto y = 0; y < 8; ++y) {

#define LOOP_2 \
				uint8_t metaByte = nibbleArray[metaIdx]; \
				result->set(x, y << 1, z, (byteArray[id1Idx] << 4) | (metaByte & 0xf)); \
				result->set(x, (y << 1) | 1, z, (byteArray[id2Idx] << 4) | ((metaByte >> 4) & 0xf)); \
			} \
		} \
	} \
	result->collectGarbage(true); \
	return result;

BlockArrayContainer<> *convertSubChunkXZY(uint8_t * byteArray, uint8_t * nibbleArray) {
	LOOP_1
		auto id1Idx = (x << 8) | (z << 4) | (y << 1);
		auto id2Idx = id1Idx | 1;
		auto metaIdx = id1Idx >> 1;
	LOOP_2
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
	LOOP_1
		auto id1Idx = (x << 11) | (z << 7) | (yOffset << 4) | (y << 1);
		auto id2Idx = id1Idx | 1;
		auto metaIdx = id1Idx >> 1;
	LOOP_2
}
