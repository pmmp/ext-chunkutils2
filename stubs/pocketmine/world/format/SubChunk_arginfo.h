/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b63b79246936b10749522729edee252c4eb5368e */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, emptyBlockId, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, blocks, IS_ARRAY, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, skyLight, pocketmine\\world\\format\\LightArray, 1, "null")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, blockLight, pocketmine\\world\\format\\LightArray, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_isEmptyAuthoritative, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pocketmine_world_format_SubChunk_isEmptyFast arginfo_class_pocketmine_world_format_SubChunk_isEmptyAuthoritative

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_getEmptyBlockId, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_getFullBlock, 0, 3, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_setFullBlock, 0, 4, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, block, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_getBlockLayers, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_getHighestBlockAt, 0, 2, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_getBlockSkyLightArray, 0, 0, pocketmine\\world\\format\\LightArray, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_setBlockSkyLightArray, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, data, pocketmine\\world\\format\\LightArray, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pocketmine_world_format_SubChunk_getBlockLightArray arginfo_class_pocketmine_world_format_SubChunk_getBlockSkyLightArray

#define arginfo_class_pocketmine_world_format_SubChunk_setBlockLightArray arginfo_class_pocketmine_world_format_SubChunk_setBlockSkyLightArray

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_SubChunk_collectGarbage, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
