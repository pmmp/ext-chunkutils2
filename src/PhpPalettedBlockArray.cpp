#include <new>
#include <gsl/span>

#include "lib/BlockArrayContainer.h"
#include "ZendUtil.h"

#include "PhpPalettedBlockArrayObj.h"
#include "stubs/pocketmine/world/format/PalettedBlockArray_arginfo.h"
#include "stubs/pocketmine/world/format/PalettedBlockArrayLoadException_arginfo.h"

extern "C" {
#include "php.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/php_var.h"
}

zend_class_entry *paletted_block_array_entry;
zend_class_entry* paletted_block_array_load_exception_entry;

static zend_object_handlers paletted_block_array_handlers;

/* internal object methods */

static inline bool checkPaletteEntrySize(zend_long v, zend_class_entry* exception_ce) {
	Block casted = (Block)v;
	zend_long castedBack = (zend_long)casted;
	if (castedBack != v) {
		zend_throw_exception_ex(exception_ce, 0, "value %zd is too large to be used as a palette entry", v);
		return false;
	}
	return true;
}

static bool palette_data_from_array(HashTable* paletteHt, std::vector<Block>& palette, zend_class_entry* exception_ce) {
	HashPosition pos;
	zval *current;
	Block b;

	palette.reserve(paletteHt->nNumUsed);
	for (
		zend_hash_internal_pointer_reset_ex(paletteHt, &pos);
		current = zend_hash_get_current_data_ex(paletteHt, &pos);
		zend_hash_move_forward_ex(paletteHt, &pos)
		) {

		if (!checkPaletteEntrySize(Z_LVAL_P(current), exception_ce)) {
			return false;
		}
		b = (Block)Z_LVAL_P(current);
		palette.push_back(b);
	}

	return true;
}

static bool palette_data_from_string(zend_string* paletteZstr, std::vector<Block>& palette, zend_class_entry* exception_ce) {
	if ((ZSTR_LEN(paletteZstr) % sizeof(Block)) != 0) {
		zend_throw_exception_ex(exception_ce, 0, "palette length in bytes must be a multiple of %zu, but have %zu bytes", sizeof(Block), ZSTR_LEN(paletteZstr));
		return false;
	}

	auto paletteEntries = ZSTR_LEN(paletteZstr) / sizeof(Block);
	Block* paletteValues = reinterpret_cast<Block*>(ZSTR_VAL(paletteZstr));
	palette.assign(paletteValues, paletteValues + paletteEntries);

	return true;
}

static bool paletted_block_array_from_data(zval *return_value, zend_long bitsPerBlock, zend_string *wordArrayZstr, HashTable *paletteHt, zend_string* paletteZstr) {
	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	gsl::span<uint8_t> wordArray((uint8_t*)ZSTR_VAL(wordArrayZstr), (uint8_t*)(ZSTR_VAL(wordArrayZstr) + ZSTR_LEN(wordArrayZstr)));
	std::vector<Block> palette;

	assert(paletteHt != nullptr || paletteZstr != nullptr);

	if (paletteHt != nullptr && !palette_data_from_array(paletteHt, palette, paletted_block_array_load_exception_entry)) {
		return false;
	} else if (paletteZstr != nullptr && !palette_data_from_string(paletteZstr, palette, paletted_block_array_load_exception_entry)) {
		return false;
	}

	try {
		new(&intern->container) NormalBlockArrayContainer((uint8_t)bitsPerBlock, wordArray, palette);
		return true;
	}
	catch (std::exception& e) {
		zend_throw_exception_ex(paletted_block_array_load_exception_entry, 0, "%s", e.what());
		return false;
	}
}

static void paletted_block_array_get_bits_per_block(zval *object, zval *return_value) {
	ZVAL_LONG(return_value, static_cast<uint8_t>(fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object))->container.getBitsPerBlock()));
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
		zval zv;
		ZVAL_LONG(&zv, palette[i]);
		zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &zv);
	}
}

static void paletted_block_array_get_palette_bytes(zval* object, zval* return_value) {
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object));

	auto palette = intern->container.getPalette();

	const Block* paletteValues = palette.data();

	ZVAL_STRINGL(return_value, reinterpret_cast<const char*>(paletteValues), palette.size_bytes());
}


static bool paletted_block_array_set_palette(zval* object, HashTable* paletteHt) {
	paletted_block_array_obj* intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(object));

	std::vector<Block> palette;
	if (!palette_data_from_array(paletteHt, palette, spl_ce_InvalidArgumentException)) {
		return false;
	}

	try {
		intern->container.setPalette(palette);
		return true;
	}
	catch (std::length_error& e) {
		zend_value_error("%s", e.what());
		return false;
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

static zend_object* paletted_block_array_clone(zend_object *object) {
	paletted_block_array_obj *old_object = fetch_from_zend_object<paletted_block_array_obj>(object);
	paletted_block_array_obj *new_object = fetch_from_zend_object<paletted_block_array_obj>(paletted_block_array_new(object->ce));

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

	ZVAL_ARR(&zv, zend_std_get_properties(Z_OBJ_P(obj)));
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

	if (!paletted_block_array_from_data(object, Z_LVAL_P(bitsPerBlock), Z_STR_P(wordArray), Z_ARRVAL_P(palette), nullptr)) {
		goto end;
	}

	properties = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(properties, &start, end, &unserialize_data) || Z_TYPE_P(properties) != IS_ARRAY) {
		zend_throw_exception(NULL, "Failed to unserialize user properties", 0);
		goto end;
	}
	if (zend_hash_num_elements(Z_ARRVAL_P(properties)) != 0) {
		zend_hash_copy(zend_std_get_properties(Z_OBJ_P(object)), Z_ARRVAL_P(properties),(copy_ctor_func_t)zval_add_ref);
	}
	result = SUCCESS;
end:
	PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
	return result;
}


/* PHP-land PalettedBlockArray methods */
#define PALETTED_BLOCK_ARRAY_METHOD(name) PHP_METHOD(pocketmine_world_format_PalettedBlockArray, name)

PALETTED_BLOCK_ARRAY_METHOD(__construct) {
	zend_long fillEntry;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(fillEntry)
	ZEND_PARSE_PARAMETERS_END();

	if (!checkPaletteEntrySize(fillEntry, spl_ce_InvalidArgumentException)) {
		return;
	}

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	try {
		new(&intern->container) NormalBlockArrayContainer((Block)fillEntry, 0);
	}
	catch (std::exception& e) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "%s", e.what());
	}
}

PALETTED_BLOCK_ARRAY_METHOD(fromData) {
	zend_long bitsPerBlock = 1;
	zend_string *wordArrayZstr;
	HashTable* paletteHt;
	zend_string* paletteZstr;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(bitsPerBlock)
		Z_PARAM_STR(wordArrayZstr)
		Z_PARAM_ARRAY_HT_OR_STR(paletteHt, paletteZstr)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_from_data(return_value, bitsPerBlock, wordArrayZstr, paletteHt, paletteZstr);
}

PALETTED_BLOCK_ARRAY_METHOD(getWordArray) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_word_array(getThis(), return_value);
}

PALETTED_BLOCK_ARRAY_METHOD(getPalette) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_palette(getThis(), return_value);
}

PALETTED_BLOCK_ARRAY_METHOD(getPaletteBytes) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_palette_bytes(getThis(), return_value);
}

PALETTED_BLOCK_ARRAY_METHOD(setPalette) {
	zval* paletteZarray;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY(paletteZarray)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_set_palette(getThis(), Z_ARRVAL_P(paletteZarray));
}

PALETTED_BLOCK_ARRAY_METHOD(getMaxPaletteSize) {
	zend_parse_parameters_none_throw();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container.getMaxPaletteSize());
}

PALETTED_BLOCK_ARRAY_METHOD(getBitsPerBlock) {
	zend_parse_parameters_none_throw();

	paletted_block_array_get_bits_per_block(getThis(), return_value);
}

PALETTED_BLOCK_ARRAY_METHOD(get) {
	zend_long x, y, z;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(intern->container.get(x, y, z));
}

PALETTED_BLOCK_ARRAY_METHOD(set) {
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

PALETTED_BLOCK_ARRAY_METHOD(replaceAll) {
	zend_long oldVal, newVal;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(oldVal)
		Z_PARAM_LONG(newVal)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container.replaceAll(oldVal, newVal);
}

PALETTED_BLOCK_ARRAY_METHOD(collectGarbage) {
	zend_bool force = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(force)
	ZEND_PARSE_PARAMETERS_END();

	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(getThis()));
	intern->container.collectGarbage(force ? true : false);
}

PALETTED_BLOCK_ARRAY_METHOD(getExpectedWordArraySize) {
	zend_long bitsPerBlock;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(bitsPerBlock)
	ZEND_PARSE_PARAMETERS_END();

	//TODO: this probably shouldn't be throwing InvalidArgumentException
	//the parameters given to this function will typically be coming from serialized
	//data, so it doesn't really make sense to declare them logically invalid
	//however using a different exception type will break BC

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

void register_paletted_block_array_class() {
	paletted_block_array_load_exception_entry = register_class_pocketmine_world_format_PalettedBlockArrayLoadException(spl_ce_RuntimeException);

	memcpy(&paletted_block_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	paletted_block_array_handlers.offset = XtOffsetOf(paletted_block_array_obj, std);
	paletted_block_array_handlers.free_obj = paletted_block_array_free;
	paletted_block_array_handlers.clone_obj = paletted_block_array_clone;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\world\\format\\PalettedBlockArray", class_pocketmine_world_format_PalettedBlockArray_methods);
	ce.create_object = paletted_block_array_new;
	ce.serialize = paletted_block_array_serialize;
	ce.unserialize = paletted_block_array_unserialize;
	paletted_block_array_entry = zend_register_internal_class(&ce);
	paletted_block_array_entry->ce_flags |= ZEND_ACC_FINAL;
}