#ifndef HAVE_SUBCHUNK_CONVERTER_H
#define HAVE_SUBCHUNK_CONVERTER_H

#include "BlockArrayContainer.h"

BlockArrayContainer<> *convertSubChunkXZY(uint8_t * byteArray, uint8_t * nibbleArray);
BlockArrayContainer<> *convertSubChunkYZX(uint8_t * byteArray, uint8_t * nibbleArray);
BlockArrayContainer<> *convertSubChunkFromLegacyColumn(uint8_t * byteArray, uint8_t * nibbleArray, uint8_t yOffset);

#endif
