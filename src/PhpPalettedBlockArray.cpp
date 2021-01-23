#include <new>
#include <gsl/span>

#include "lib/BlockArrayContainer.h"
#include "ZendUtil.h"

#include "PhpPalettedBlockArrayObj.h"

extern "C" {
#include "php.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/php_var.h"
}

zend_class_entry *paletted_block_array_entry;
static zend_object_handlers paletted_block_array_handlers;

/* internal object methods */

static inline bool checkPaletteEntrySize(zend_long v) {
	Block casted = (Block)v;
	zend_long castedBack = (zend_long)casted;
	if (castedBack != v) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "value %zd is too large to be used as a palette entry", v);
		return false;
	}
	return true;
}

static bool paletted_block_array_from_data(zval *return_value, zend_long bitsPerBlock, zend_string *wordArrayZstr, HashTable *paletteHt) {
	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	gsl::span<uint8_t> wordArray((uint8_t*)ZSTR_VAL(wordArrayZstr), (uint8_t*)(ZSTR_VAL(wordArrayZstr) + ZSTR_LEN(wordArrayZstr)));
	std::vector<Block> palette;

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
			return false;
		}
		b = (Block)Z_LVAL_P(current);
		palette.push_back(b);
	}

	try {
		new(&intern->container) NormalBlockArrayContainer((uint8_t)bitsPerBlock, wordArray, palette);
		return true;
	}
	catch (std::exception& e) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%s", e.what());
		return false;
	}
}

static void paletted_block_array_get_bits_per_block(zval *object, zval *return_value) {
	ZVAL_LONG(return_value, fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object))->container.getBitsPerBlock());
}

static void paletted_block_array_get_word_array(zval *object, zval *return_value) {
	auto span = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object))->container.getWordArray();

	ZVAL_STRINGL(return_value, span.data(), span.size());
}

static void paletted_block_array_get_palette(zval *object, zval *return_value) {
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object));

	auto palette = intern->container.getPalette();

	array_init_size(return_value, palette.size());
	for (unsigned short i = 0; i < palette.size(); ++i) {
		add_index_long(return_value, i, palette[i]);
	}
}

/* internal object handlers */

static zend_object* paletted_block_array_new(zend_class_entry *class_type) {
	paletted_block_array_obj *object = (paletted_block_array_obj *)ecalloc(1, sizeof(paletted_block_array_obj) + zend_object_properties_size(class_type));

	zend_object_std_init(&object->std, class_type);
	object_properties_init(&object->std, class_type);

	object->std.handlers = &paletted_block_array_handlers;

	return &object->std;
}

static zend_object* paletted_block_array_clone(chunkutils2_handler_context *object) {
	paletted_block_array_obj *old_object = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_FROM_HANDLER_CONTEXT(object));
	paletted_block_array_obj *new_object = fetch_from_zend_object<paletted_block_array_obj>(paletted_block_array_new(Z_OBJ_FROM_HANDLER_CONTEXT(object)->ce));

	new(&new_object->container) NormalBlockArrayContainer(old_object->container);

	zend_objects_clone_members(&new_object->std, &old_object->std); //copy user-assigned properties

	return &new_object->std;
}

static void paletted_block_array_free(zend_object *obj) {
	paletted_block_array_obj *object = fetch_from_zend_object<paletted_block_array_obj>(obj);
	object->container.~BlockArrayContainer();

	//call the standard free handler
	zend_object_std_dtor(&object->std);
}

static int paletted_block_array_serialize(zval *obj, unsigned char **buffer, size_t *buf_len, zend_serialize_data *data) {
	paletted_block_array_obj *object = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(obj));

	smart_str buf = { 0 };
	php_serialize_data_t serialize_data = (php_serialize_data_t)data;
	PHP_VAR_SERIALIZE_INIT(serialize_data);

	zval zv;

	paletted_block_array_get_bits_per_block(obj, &zv);
	php_var_serialize(&buf, &zv, &serialize_data);
	zval_dtor(&zv);

	paletted_block_array_get_word_array(obj, &zv);
	php_var_serialize(&buf, &zv, &serialize_data);
	zval_dtor(&zv);

	paletted_block_array_get_palette(obj, &zv);
	php_var_serialize(&buf, &zv, &serialize_data);
	zval_dtor(&zv);

	ZVAL_ARR(&zv, zend_std_get_properties(HANDLER_CONTEXT_FROM_ZVAL(obj)));
	php_var_serialize(&buf, &zv, &serialize_data);

	PHP_VAR_SERIALIZE_DESTROY(serialize_data);
	*buffer = (unsigned char *)estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
	*buf_len = ZSTR_LEN(buf.s);
	zend_string_release(buf.s);

	return SUCCESS;
}

static int paletted_block_array_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buf, size_t buf_len, zend_unserialize_data *data) {
	php_unserialize_data_t unserialize_data = (php_unserialize_data_t)data;
	PHP_VAR_UNSERIALIZE_INIT(unserialize_data);

	int result = FAILURE;

	const unsigned char *start = buf;
	const unsigned char *end = buf + buf_len;
	zval *bitsPerBlock, *wordArray, *palette, *properties;

	bitsPerBlock = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(bitsPerBlock, &start, end, &unserialize_data) || Z_TYPE_P(bitsPerBlock) != IS_LONG) {
		zend_throw_exception(NULL, "Failed to unserialize bits-per-block value", 0);
		goto end;
	}

	wordArray = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(wordArray, &start, end, &unserialize_data) || Z_TYPE_P(wordArray) != IS_STRING) {
		zend_throw_exception(NULL, "Failed to unserialize word array", 0);
		goto end;
	}

	palette = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(palette, &start, end, &unserialize_data) || Z_TYPE_P(palette) != IS_ARRAY) {
		zend_throw_exception(NULL, "Failed to unserialize palette", 0);
		goto end;
	}

	if (!paletted_block_array_from_data(object, Z_LVAL_P(bitsPerBlock), Z_STR_P(wordArray), Z_ARRVAL_P(palette))) {
		goto end;
	}

	properties = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(properties, &start, end, &unserialize_data) || Z_TYPE_P(properties) != IS_ARRAY) {
		zend_throw_exception(NULL, "Failed to unserialize user properties", 0);
		goto end;
	}
	if (zend_hash_num_elements(Z_ARRVAL_P(properties)) != 0) {
		zend_hash_copy(zend_std_get_properties(HANDLER_CONTEXT_FROM_ZVAL(object)), Z_ARRVAL_P(properties),(copy_ctor_func_t)zval_add_ref);
	}
	result = SUCCESS;
end:
	PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
	return result;
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
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%s", e.what());
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_PalettedBlockArray_fromData, 0, 3, pocketmine\\world\\format\\PalettedBlockArray, 0)
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

	paletted_block_array_from_data(return_value, bitsPerBlock, wordArrayZstr, Z_ARRVAL_P(paletteZarray));
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getWordArray, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getWordArray) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_word_array(getThis(), return_value);
}


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getPalette, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpPalettedBlockArray, getPalette) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_palette(getThis(), return_value);
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

	paletted_block_array_get_bits_per_block(getThis(), return_value);
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
	RETURN_LONG(intern->container.get(x, y, z));
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PalettedBlockArray_getExpectedWordArraySize, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bitsPerBlock, IS_LONG, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(PhpPalettedBlockArray, getExpectedWordArraySize) {
	zend_long bitsPerBlock;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(bitsPerBlock)
	ZEND_PARSE_PARAMETERS_END();

	uint8_t casted = (uint8_t)bitsPerBlock;
	zend_long castedBack = (zend_long)casted;
	if (bitsPerBlock != castedBack) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "invalid bits-per-block: %zd", bitsPerBlock);
		return;
	}

	try {
		RETURN_LONG((zend_long)NormalBlockArrayContainer::getExpectedPayloadSize(casted));
	}
	catch (std::invalid_argument &e) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "%s", e.what());
	}
}

static zend_function_entry paletted_block_array_class_methods[] = {
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
	PHP_ME(PhpPalettedBlockArray, getExpectedWordArraySize, arginfo_PalettedBlockArray_getExpectedWordArraySize, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

void register_paletted_block_array_class() {
	memcpy(&paletted_block_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	paletted_block_array_handlers.offset = XtOffsetOf(paletted_block_array_obj, std);
	paletted_block_array_handlers.free_obj = paletted_block_array_free;
	paletted_block_array_handlers.clone_obj = paletted_block_array_clone;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\world\\format\\PalettedBlockArray", paletted_block_array_class_methods);
	ce.create_object = paletted_block_array_new;
	ce.serialize = paletted_block_array_serialize;
	ce.unserialize = paletted_block_array_unserialize;
	paletted_block_array_entry = zend_register_internal_class(&ce);
	paletted_block_array_entry->ce_flags |= ZEND_ACC_FINAL;
}