#ifndef HAVE_PHP_SUB_CHUNK_ARGINFO_H
#define HAVE_PHP_SUB_CHUNK_ARGINFO_H

#include "ZendUtil.h"
#include "PhpLightArray.h"

extern "C" {
#include "php.h"
}


CUSTOM_ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX_STR(arginfo_SubChunk_getLightArray, 0, 0, light_array_classname, 0)
ZEND_END_ARG_INFO();



ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_SubChunk_setLightArray, 0, 1, IS_VOID, 0)
	CUSTOM_ZEND_ARG_OBJ_INFO_STR(0, data, light_array_classname, 0)
ZEND_END_ARG_INFO()

#define parse_args_SubChunk_setLightArray() \
	zval* data; \
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1) \
		Z_PARAM_OBJECT_OF_CLASS(data, light_array_entry) \
	ZEND_PARSE_PARAMETERS_END();

#endif
