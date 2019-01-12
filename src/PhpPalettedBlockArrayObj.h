#ifndef HAVE_PHP_PALETTED_BLOCK_ARRAY_OBJ_H
#define HAVE_PHP_PALETTED_BLOCK_ARRAY_OBJ_H

#include "lib/BlockArrayContainer.h"

extern "C" {
#include "php.h"
}

typedef struct {
	BlockArrayContainer<> container;
	zend_object std;
} paletted_block_array_obj;

#endif
