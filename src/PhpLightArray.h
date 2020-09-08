#ifndef HAVE_PHP_LIGHT_ARRAY_H
#define HAVE_PHP_LIGHT_ARRAY_H

extern "C" {
#include "php.h"
}

PHP_METHOD(PhpLightArray, __construct);
PHP_METHOD(PhpLightArray, fill);
PHP_METHOD(PhpLightArray, get);
PHP_METHOD(PhpLightArray, set);
PHP_METHOD(PhpLightArray, getData);
PHP_METHOD(PhpLightArray, collectGarbage);

void register_light_array_class();

#endif
