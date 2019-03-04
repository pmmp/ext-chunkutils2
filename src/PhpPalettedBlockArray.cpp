#include <new>

#include "lib/BlockArrayContainer.h"
#include "ZendUtil.h"

#include "PhpPalettedBlockArrayObj.h"

extern "C" {
#include "php.h"
#include "Zend/zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"
}

zend_class_entry *paletted_block_array_entry;
zend_object_handlers paletted_block_array_handlers;

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
	object->container.~BlockArrayContainer();

	//call the standard free handler
	zend_object_std_dtor(&object->std);
}

static inline bool checkPaletteEntrySize(zend_long v) {
	Block casted = (Block)v;
	zend_long castedBack = (zend_long)casted;
	if (castedBack != v) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "value %zd is too large to be used as a palette entry", v);
		return false;
	}
	return true;
}

/* PHP-land PalettedBlockArray methods */

ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fillEntry, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, __construct) {
	zend_long fillEntry;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(fillEntry)
	ZEND_PARSE_PARAMETERS_END();

	if (!checkPaletteEntrySize(fillEntry)) {
		return;
	}

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	try {
		new(&intern->container) NormalBlockArrayContainer((Block)fillEntry);
	}
	catch (std::exception& e) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, e.what());
	}
}


ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_PalettedBlockArray_fromData, 0, 3, pocketmine\\level\\format\\PalettedBlockArray, 0)
	ZEND_ARG_TYPE_INFO(0, bitsPerBlock, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, wordArray, IS_STRING, 0)
	ZEND_ARG_ARRAY_INFO(0, palette, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, fromData) {
	zend_long bitsPerBlock = 1;
	zend_string *wordArrayZstr;
	zval *paletteZarray;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(bitsPerBlock)
		Z_PARAM_STR(wordArrayZstr)
		Z_PARAM_ARRAY(paletteZarray)
	ZEND_PARSE_PARAMETERS_END();

	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	std::vector<char> wordArray(ZSTR_VAL(wordArrayZstr), (ZSTR_VAL(wordArrayZstr) + ZSTR_LEN(wordArrayZstr)));
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

		if (!checkPaletteEntrySize(Z_LVAL_P(current))) {
			zval_ptr_dtor(return_value);
			return;
		}
		b = (Block)Z_LVAL_P(current);
		palette.push_back(b);
	}

	try {
		new(&intern->container) NormalBlockArrayContainer((uint8_t)bitsPerBlock, wordArray, palette);
	}
	catch (std::exception& e) {
		zval_ptr_dtor(return_value);
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, e.what());
	}
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getWordArray, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getWordArray) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));

	unsigned int length;
	const char *words = intern->container.getWordArray(length);

	RETURN_STRINGL(words, length);
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getPalette, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getPalette) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));

	unsigned short paletteSize;
	const Block *palette = intern->container.getPalette(paletteSize);

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
	RETURN_LONG(intern->container.getMaxPaletteSize());
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getBitsPerBlock, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getBitsPerBlock) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container.getBitsPerBlock());
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
	RETURN_LONG(intern->container.get(x, y, z))
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
	intern->container.set(x, y, z, val);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray_replace, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, val, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, replace) {
	zend_long offset, val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(offset)
		Z_PARAM_LONG(val)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container.replace(offset, val);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_PalettedBlockArray_replaceAll, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, oldVal, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, newVal, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, replaceAll) {
	zend_long oldVal, newVal;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(oldVal)
		Z_PARAM_LONG(newVal)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container.replaceAll(oldVal, newVal);
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
	intern->container.collectGarbage(force ? true : false);
}

zend_function_entry paletted_block_array_class_methods[] = {
	PHP_ME(PhpPalettedBlockArray, __construct, arginfo_PalettedBlockArray___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PhpPalettedBlockArray, fromData, arginfo_PalettedBlockArray_fromData, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpPalettedBlockArray, getWordArray, arginfo_PalettedBlockArray_getWordArray, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getPalette, arginfo_PalettedBlockArray_getPalette, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getMaxPaletteSize, arginfo_PalettedBlockArray_getMaxPaletteSize, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, getBitsPerBlock, arginfo_PalettedBlockArray_getBitsPerBlock, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, get, arginfo_PalettedBlockArray_get, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, set, arginfo_PalettedBlockArray_set, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, replace, arginfo_PalettedBlockArray_replace, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, replaceAll, arginfo_PalettedBlockArray_replaceAll, ZEND_ACC_PUBLIC)
	PHP_ME(PhpPalettedBlockArray, collectGarbage, arginfo_PalettedBlockArray_collectGarbage, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void register_paletted_block_array_class() {
	memcpy(&paletted_block_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	paletted_block_array_handlers.offset = XtOffsetOf(paletted_block_array_obj, std);
	paletted_block_array_handlers.free_obj = paletted_block_array_free;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\level\\format\\PalettedBlockArray", paletted_block_array_class_methods);
	ce.create_object = paletted_block_array_new;
	paletted_block_array_entry = zend_register_internal_class(&ce);
}