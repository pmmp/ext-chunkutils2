
#include "lib/SubChunkConverter.h"
#include "ZendUtil.h"
#include "PhpPalettedBlockArrayObj.h"

extern "C" {
#include "php.h"
}
#include "PhpPalettedBlockArray.h"

Block flattenData(uint8_t id, uint8_t meta) {
	return (id << 4) | meta;
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_SubChunkConverter_convertSubChunk, 0, 2, pocketmine\\level\\format\\PalettedBlockArray, 0)
ZEND_ARG_TYPE_INFO(0, idArray, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, metaArray, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpSubChunkConverter, convertSubChunkXZY) {
	zend_string *idArray;
	zend_string *metaArray;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(idArray)
		Z_PARAM_STR(metaArray)
	ZEND_PARSE_PARAMETERS_END();


	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));
	//TODO: needs length checks
	convertSubChunkXZY<Block>(&intern->container, (uint8_t*)idArray->val, (uint8_t*)metaArray->val, flattenData);
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
	//TODO: needs length checks
	convertSubChunkYZX<Block>(&intern->container, (uint8_t*)idArray->val, (uint8_t*)metaArray->val, flattenData);
}


ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_SubChunkConverter_convertSubChunkFromLegacyColumn, 0, 3, pocketmine\\level\\format\\PalettedBlockArray, 0)
ZEND_ARG_TYPE_INFO(0, idArray, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, metaArray, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, yOffset, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpSubChunkConverter, convertSubChunkFromLegacyColumn) {
	zend_string *idArray;
	zend_string *metaArray;
	zend_long yOffset;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_STR(idArray)
		Z_PARAM_STR(metaArray)
		Z_PARAM_LONG(yOffset)
	ZEND_PARSE_PARAMETERS_END();


	object_init_ex(return_value, paletted_block_array_entry);
	paletted_block_array_obj *intern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(return_value));
	//TODO: needs length checks
	//TODO: check for valid Y offset
	convertSubChunkFromLegacyColumn<Block>(&intern->container, (uint8_t*)idArray->val, (uint8_t*)metaArray->val, (uint8_t)yOffset, flattenData);
}

zend_function_entry subchunk_converter_class_methods[] = {
	PHP_ME(PhpSubChunkConverter, convertSubChunkXZY, arginfo_SubChunkConverter_convertSubChunk, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSubChunkConverter, convertSubChunkYZX, arginfo_SubChunkConverter_convertSubChunk, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSubChunkConverter, convertSubChunkFromLegacyColumn, arginfo_SubChunkConverter_convertSubChunkFromLegacyColumn, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

void register_sub_chunk_converter_class() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\level\\format\\io\\SubChunkConverter", subchunk_converter_class_methods);
	zend_register_internal_class(&ce);
}
