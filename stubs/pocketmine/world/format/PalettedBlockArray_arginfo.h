/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 474344accc11ae782ff1b7d3ba695bce5bfa3fc1 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fillEntry, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_fromData, 0, 3, pocketmine\\world\\format\\PalettedBlockArray, 0)
	ZEND_ARG_TYPE_INFO(0, bitsPerBlock, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, wordArray, IS_STRING, 0)
	ZEND_ARG_TYPE_MASK(0, palette, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_getWordArray, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_getPalette, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pocketmine_world_format_PalettedBlockArray_getPaletteBytes arginfo_class_pocketmine_world_format_PalettedBlockArray_getWordArray

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_setPalette, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, palette, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_getMaxPaletteSize, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pocketmine_world_format_PalettedBlockArray_getBitsPerBlock arginfo_class_pocketmine_world_format_PalettedBlockArray_getMaxPaletteSize

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_get, 0, 3, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_set, 0, 4, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, val, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_replaceAll, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, oldVal, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, newVal, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_collectGarbage, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, force, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_PalettedBlockArray_getExpectedWordArraySize, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, bitsPerBlock, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, __construct);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, fromData);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getWordArray);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getPalette);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getPaletteBytes);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, setPalette);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getMaxPaletteSize);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getBitsPerBlock);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, get);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, set);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, replaceAll);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, collectGarbage);
ZEND_METHOD(pocketmine_world_format_PalettedBlockArray, getExpectedWordArraySize);

static const zend_function_entry class_pocketmine_world_format_PalettedBlockArray_methods[] = {
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, __construct, arginfo_class_pocketmine_world_format_PalettedBlockArray___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, fromData, arginfo_class_pocketmine_world_format_PalettedBlockArray_fromData, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getWordArray, arginfo_class_pocketmine_world_format_PalettedBlockArray_getWordArray, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getPalette, arginfo_class_pocketmine_world_format_PalettedBlockArray_getPalette, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getPaletteBytes, arginfo_class_pocketmine_world_format_PalettedBlockArray_getPaletteBytes, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, setPalette, arginfo_class_pocketmine_world_format_PalettedBlockArray_setPalette, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getMaxPaletteSize, arginfo_class_pocketmine_world_format_PalettedBlockArray_getMaxPaletteSize, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getBitsPerBlock, arginfo_class_pocketmine_world_format_PalettedBlockArray_getBitsPerBlock, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, get, arginfo_class_pocketmine_world_format_PalettedBlockArray_get, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, set, arginfo_class_pocketmine_world_format_PalettedBlockArray_set, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, replaceAll, arginfo_class_pocketmine_world_format_PalettedBlockArray_replaceAll, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, collectGarbage, arginfo_class_pocketmine_world_format_PalettedBlockArray_collectGarbage, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_PalettedBlockArray, getExpectedWordArraySize, arginfo_class_pocketmine_world_format_PalettedBlockArray_getExpectedWordArraySize, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};
