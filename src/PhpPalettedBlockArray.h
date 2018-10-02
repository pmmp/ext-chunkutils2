#ifndef HAVE_PHP_PALETTED_BLOCK_ARRAY_H
#define HAVE_PHP_PALETTED_BLOCK_ARRAY_H

extern "C" {
#include "php.h"
}

PHP_METHOD(PhpPalettedBlockArray, __construct);
PHP_METHOD(PhpPalettedBlockArray, fromData);
PHP_METHOD(PhpPalettedBlockArray, getWordArray);
PHP_METHOD(PhpPalettedBlockArray, getPalette);
PHP_METHOD(PhpPalettedBlockArray, getMaxPaletteSize);
PHP_METHOD(PhpPalettedBlockArray, getBitsPerBlock);
PHP_METHOD(PhpPalettedBlockArray, get);
PHP_METHOD(PhpPalettedBlockArray, set);
PHP_METHOD(PhpPalettedBlockArray, collectGarbage);
PHP_METHOD(PhpPalettedBlockArray, replace);
PHP_METHOD(PhpPalettedBlockArray, replaceAll);

extern zend_class_entry *paletted_block_array_entry;

void register_paletted_block_array_class();

#endif
