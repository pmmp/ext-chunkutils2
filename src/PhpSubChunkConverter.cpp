#include <gsl/span>

#include "lib/SubChunkConverter.h"
#include "ZendUtil.h"
#include "PhpPalettedBlockArrayObj.h"
#include "stubs/pocketmine/world/format/io/SubChunkConverter_arginfo.h"

extern "C" {
#include "php.h"
#include "Zend/zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"
}
#include "PhpPalettedBlockArray.h"

static Block flattenData(uint8_t id, uint8_t meta) {
	return (id << 4) | meta;
}

PHP_METHOD(PhpSubChunkConverter, convertSubChunkXZY) {
	zend_string *idArray;
	zend_string *metaArray;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(idArray)
		Z_PARAM_STR(metaArray)
	ZEND_PARSE_PARAMETERS_END();

	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	try {
		LegacySubChunkIds idSpan((uint8_t *)ZSTR_VAL(idArray), (uint8_t *)(ZSTR_VAL(idArray) + ZSTR_LEN(idArray)));
		LegacySubChunkMetas metaSpan((uint8_t *)ZSTR_VAL(metaArray), (uint8_t *)(ZSTR_VAL(metaArray) + ZSTR_LEN(metaArray)));
		convertSubChunkXZY<Block, &flattenData>(&intern->container, idSpan, metaSpan);
	}
	catch (gsl::fail_fast &e) {
		zend_throw_exception_ex(spl_ce_LengthException, 0, "Invalid data sizes (got %zu and %zu)", ZSTR_LEN(idArray), ZSTR_LEN(metaArray));
		return;
	}
}

//TODO: repetetive, clean it up :(

PHP_METHOD(PhpSubChunkConverter, convertSubChunkYZX) {
	zend_string *idArray;
	zend_string *metaArray;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(idArray)
		Z_PARAM_STR(metaArray)
	ZEND_PARSE_PARAMETERS_END();


	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	try {
		LegacySubChunkIds idSpan((uint8_t *)ZSTR_VAL(idArray), (uint8_t *)(ZSTR_VAL(idArray) + ZSTR_LEN(idArray)));
		LegacySubChunkMetas metaSpan((uint8_t *)ZSTR_VAL(metaArray), (uint8_t *)(ZSTR_VAL(metaArray) + ZSTR_LEN(metaArray)));
		convertSubChunkYZX<Block, &flattenData>(&intern->container, idSpan, metaSpan);
	}
	catch (gsl::fail_fast &e) {
		zend_throw_exception_ex(spl_ce_LengthException, 0, "Invalid data sizes (got %zu and %zu)", ZSTR_LEN(idArray), ZSTR_LEN(metaArray));
		return;
	}
}

PHP_METHOD(PhpSubChunkConverter, convertSubChunkFromLegacyColumn) {
	zend_string *idArray;
	zend_string *metaArray;
	zend_long yOffset;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_STR(idArray)
		Z_PARAM_STR(metaArray)
		Z_PARAM_LONG(yOffset)
	ZEND_PARSE_PARAMETERS_END();

	if (yOffset < 0 || yOffset > 7) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Y offset must be in range 0-7");
		return;
	}
	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));

	try {
		LegacyChunkColumnIds idSpan((uint8_t *)ZSTR_VAL(idArray), (uint8_t *)(ZSTR_VAL(idArray) + ZSTR_LEN(idArray)));
		LegacyChunkColumnMetas metaSpan((uint8_t *)ZSTR_VAL(metaArray), (uint8_t *)(ZSTR_VAL(metaArray) + ZSTR_LEN(metaArray)));

		//TODO: check for valid Y offset
		convertSubChunkFromLegacyColumn<Block, &flattenData>(&intern->container, idSpan, metaSpan, (uint8_t)yOffset);
	}
	catch (gsl::fail_fast &e) {
		zend_throw_exception_ex(spl_ce_LengthException, 0, "Invalid data sizes (got %zu and %zu)", ZSTR_LEN(idArray), ZSTR_LEN(metaArray));
		return;
	}
}

PHP_METHOD(PhpSubChunkConverter, __construct) {
	//NOOP
}

static zend_function_entry subchunk_converter_class_methods[] = {
	PHP_ME(PhpSubChunkConverter, convertSubChunkXZY, arginfo_class_pocketmine_world_format_io_SubChunkConverter_convertSubChunkXZY, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSubChunkConverter, convertSubChunkYZX, arginfo_class_pocketmine_world_format_io_SubChunkConverter_convertSubChunkYZX, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSubChunkConverter, convertSubChunkFromLegacyColumn, arginfo_class_pocketmine_world_format_io_SubChunkConverter_convertSubChunkFromLegacyColumn, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSubChunkConverter, __construct, arginfo_class_pocketmine_world_format_io_SubChunkConverter___construct, ZEND_ACC_CTOR | ZEND_ACC_PRIVATE)
	PHP_FE_END
};

void register_sub_chunk_converter_class() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\world\\format\\io\\SubChunkConverter", subchunk_converter_class_methods);
	auto registered = zend_register_internal_class(&ce);
	registered->ce_flags |= ZEND_ACC_FINAL;
}
