/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BlockArrayContainer.h"

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_exceptions.h"

#include "php_mcpe_paletted_block_array.h"
}

zend_object_handlers paletted_block_array_handlers;
zend_class_entry *paletted_block_array_entry;

typedef struct {
	BlockArrayContainer<> *container;
	zend_object std;
} paletted_block_array_obj;

typedef uint32_t Word;
typedef unsigned int Block;

template<typename class_name>
static inline class_name * fetch_from_zend_object(zend_object *obj) {
	return (class_name *)((char *)obj - XtOffsetOf(class_name, std));
}

/* internal object handlers */

zend_object* paletted_block_array_new(zend_class_entry *class_type) {
	paletted_block_array_obj *object = (paletted_block_array_obj *)ecalloc(1, sizeof(paletted_block_array_obj) + zend_object_properties_size(class_type));

	zend_object_std_init(&object->std, class_type);
	object_properties_init(&object->std, class_type);

	object->std.handlers = &paletted_block_array_handlers;
	
	return &object->std;
}


void paletted_block_array_free(zend_object *obj) {
	paletted_block_array_obj *object = fetch_from_zend_object<paletted_block_array_obj>(obj);
	delete object->container;

	//call the standard free handler
	zend_object_std_dtor(&object->std);
}


/* PHP-land PalettedBlockArray methods */

ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, bitsPerBlock, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, wordArray, IS_STRING, 0)
	ZEND_ARG_ARRAY_INFO(0, palette, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, __construct) {
	zend_long bitsPerBlock = 1;
	zend_string *wordArrayZstr = nullptr;
	zval *paletteZarray = nullptr;

	//this should really be split into several overloads
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 3)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(bitsPerBlock)
		Z_PARAM_STR(wordArrayZstr)
		Z_PARAM_ARRAY(paletteZarray)
	ZEND_PARSE_PARAMETERS_END();

	if (wordArrayZstr != nullptr && paletteZarray == nullptr) {
		zend_throw_exception(spl_ce_InvalidArgumentException, "cannot specify a word array without a corresponding palette", 0);
		return;
	}
	if (wordArrayZstr != nullptr && (ZSTR_LEN(wordArrayZstr) % sizeof(Word)) != 0) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "word array length should be a multiple of %d, got a length of %d", sizeof(Word), ZSTR_LEN(wordArrayZstr));
		return;
	}

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));

	try {
		if (wordArrayZstr != nullptr && paletteZarray != nullptr) {
			//convert to vectors

			std::vector<Word> wordArray((Word *)ZSTR_VAL(wordArrayZstr), (Word *)(ZSTR_VAL(wordArrayZstr) + ZSTR_LEN(wordArrayZstr)));

			std::vector<Block> palette;
			HashTable *paletteHt = Z_ARRVAL_P(paletteZarray);
			HashPosition pos;
			zval *current;
			Block b;

			palette.reserve(paletteHt->nNumUsed);

			for (
				zend_hash_internal_pointer_reset_ex(paletteHt, &pos); 
				current = zend_hash_get_current_data_ex(paletteHt, &pos);
				zend_hash_move_forward_ex(paletteHt, &pos)
			) {
				b = (Block)Z_LVAL_P(current);
				palette.push_back(b);
			}

			auto container = new BlockArrayContainer<>((uint8_t)bitsPerBlock, wordArray, palette);
			intern->container = container;
		}
		else {
			auto container = new BlockArrayContainer<>((uint8_t)bitsPerBlock);
			intern->container = container;
		}
	}
	catch (std::exception& e) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, e.what());
	}
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getWordArray, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getWordArray) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));

	unsigned short wordCount;
	const Word *words = intern->container->getWordArray(wordCount);

	RETURN_STRINGL((const char *)words, wordCount * sizeof(Word));
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getPalette, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getPalette) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));

	unsigned short paletteSize;
	const Block *palette = intern->container->getPalette(paletteSize);

	array_init_size(return_value, paletteSize);
	for (unsigned short i = 0; i < paletteSize; ++i) {
		add_index_long(return_value, i, palette[i]);
	}
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getMaxPaletteSize, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getMaxPaletteSize) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container->getMaxPaletteSize());
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getBitsPerBlock, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getBitsPerBlock) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container->getBitsPerBlock());
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_get, 0, 3, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, get) {
	zend_long x, y, z;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container->get(x, y, z))
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray_set, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, val, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, set) {
	zend_long x, y, z, val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 4, 4)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
		Z_PARAM_LONG(val)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container->set(x, y, z, val);
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray_collectGarbage, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, force, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, collectGarbage) {
	zend_bool force = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(force)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container->collectGarbage(force ? true : false);
}

/* PHP-land PalettedBlockArray methods end */


zend_function_entry paletted_block_array_class_methods[] = {
	PHP_ME(PhpPalettedBlockArray, __construct, arginfo_PalettedBlockArray___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PhpPalettedBlockArray, getWordArray, arginfo_PalettedBlockArray_getWordArray, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getPalette, arginfo_PalettedBlockArray_getPalette, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getMaxPaletteSize, arginfo_PalettedBlockArray_getMaxPaletteSize, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getBitsPerBlock, arginfo_PalettedBlockArray_getBitsPerBlock, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, get, arginfo_PalettedBlockArray_get, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, set, arginfo_PalettedBlockArray_set, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, collectGarbage, arginfo_PalettedBlockArray_collectGarbage, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mcpe_paletted_block_array)
{
	memcpy(&paletted_block_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	paletted_block_array_handlers.offset = XtOffsetOf(paletted_block_array_obj, std);
	paletted_block_array_handlers.free_obj = paletted_block_array_free;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\level\\format\\PalettedBlockArray", paletted_block_array_class_methods);
	ce.create_object = paletted_block_array_new;
	paletted_block_array_entry = zend_register_internal_class(&ce);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mcpe_paletted_block_array)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mcpe_paletted_block_array)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mcpe_paletted_block_array support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ mcpe_paletted_block_array_functions[]
 *
 * Every user visible function must have an entry in mcpe_paletted_block_array_functions[].
 */
const zend_function_entry mcpe_paletted_block_array_functions[] = {
	PHP_FE_END	/* Must be the last line in mcpe_paletted_block_array_functions[] */
};
/* }}} */

/* {{{ mcpe_paletted_block_array_module_entry
 */
zend_module_entry mcpe_paletted_block_array_module_entry = {
	STANDARD_MODULE_HEADER,
	"mcpe_paletted_block_array",
	mcpe_paletted_block_array_functions,
	PHP_MINIT(mcpe_paletted_block_array),
	PHP_MSHUTDOWN(mcpe_paletted_block_array),
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	PHP_MINFO(mcpe_paletted_block_array),
	PHP_MCPE_PALETTED_BLOCK_ARRAY_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MCPE_PALETTED_BLOCK_ARRAY
extern "C" {
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mcpe_paletted_block_array)
}
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
