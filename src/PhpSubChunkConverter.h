#ifndef HAVE_PHP_SUBCHUNK_CONVERTER_H
#define HAVE_PHP_SUBCHUNK_CONVERTER_H

extern "C" {
#include "php.h"
}

PHP_METHOD(PhpSubChunkConverter, convertSubChunkXZY);
PHP_METHOD(PhpSubChunkConverter, convertSubChunkYZX);
PHP_METHOD(PhpSubChunkConverter, convertSubChunkFromLegacyColumn);

void register_sub_chunk_converter_class();

#endif
