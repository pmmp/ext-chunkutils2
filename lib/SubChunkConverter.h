#ifndef HAVE_SUBCHUNK_CONVERTER_H
#define HAVE_SUBCHUNK_CONVERTER_H

#include "BlockArrayContainer.h"

void convertSubChunkXZY(BlockArrayContainer<> * where, uint8_t * byteArray, uint8_t * nibbleArray);
void convertSubChunkYZX(BlockArrayContainer<> * where, uint8_t * byteArray, uint8_t * nibbleArray);
void convertSubChunkFromLegacyColumn(BlockArrayContainer<> * where, uint8_t * byteArray, uint8_t * nibbleArray, uint8_t yOffset);

#endif
