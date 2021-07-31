/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: d6aa0d1e437e8a3301b5b808074b5872c7de5182 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_pocketmine_world_format_LightArray___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_fill, 0, 1, pocketmine\\world\\format\\LightArray, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_get, 0, 3, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_set, 0, 4, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_getData, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_collectGarbage, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pocketmine_world_format_LightArray_isUniform, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pocketmine_world_format_LightArray, __construct);
ZEND_METHOD(pocketmine_world_format_LightArray, fill);
ZEND_METHOD(pocketmine_world_format_LightArray, get);
ZEND_METHOD(pocketmine_world_format_LightArray, set);
ZEND_METHOD(pocketmine_world_format_LightArray, getData);
ZEND_METHOD(pocketmine_world_format_LightArray, collectGarbage);
ZEND_METHOD(pocketmine_world_format_LightArray, isUniform);


static const zend_function_entry class_pocketmine_world_format_LightArray_methods[] = {
	ZEND_ME(pocketmine_world_format_LightArray, __construct, arginfo_class_pocketmine_world_format_LightArray___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_LightArray, fill, arginfo_class_pocketmine_world_format_LightArray_fill, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(pocketmine_world_format_LightArray, get, arginfo_class_pocketmine_world_format_LightArray_get, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_LightArray, set, arginfo_class_pocketmine_world_format_LightArray_set, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_LightArray, getData, arginfo_class_pocketmine_world_format_LightArray_getData, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_LightArray, collectGarbage, arginfo_class_pocketmine_world_format_LightArray_collectGarbage, ZEND_ACC_PUBLIC)
	ZEND_ME(pocketmine_world_format_LightArray, isUniform, arginfo_class_pocketmine_world_format_LightArray_isUniform, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};
