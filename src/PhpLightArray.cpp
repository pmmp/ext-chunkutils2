#include <new>

#include "lib/LightArray.h"

#include "PhpLightArray.h"
#include "ZendUtil.h"

extern "C" {
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_exceptions.h"
}

zend_class_entry* light_array_entry;
static zend_object_handlers light_array_handlers;

/* internal object handlers */

static zend_object* light_array_new(zend_class_entry* class_type) {
	light_array_obj* object = alloc_custom_zend_object<light_array_obj>(class_type, &light_array_handlers);

	return &object->std;
}

static zend_object* light_array_clone(chunkutils2_handler_context* object) {
	light_array_obj* old_object = fetch_from_zend_object<light_array_obj>(Z_OBJ_FROM_HANDLER_CONTEXT(object));
	light_array_obj* new_object = fetch_from_zend_object<light_array_obj>(light_array_new(Z_OBJ_FROM_HANDLER_CONTEXT(object)->ce));
	new_object->lightArray = old_object->lightArray; //this calls the copy assignment operator

	zend_objects_clone_members(&new_object->std, &old_object->std); //copy user-assigned properties

	return &new_object->std;
}

static void light_array_free(zend_object* obj) {
	//we might need to call lightArray destructor here in future

	zend_object_std_dtor(obj);
}

static int light_array_serialize(zval* obj, unsigned char** buffer, size_t* buf_len, zend_serialize_data* data) {
	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(obj));

	auto span = object->lightArray.getRawData();

	*buffer = reinterpret_cast<unsigned char*>(emalloc(span.size_bytes()));
	memcpy(*buffer, span.data(), span.size_bytes());
	*buf_len = span.size_bytes();

	return SUCCESS;
}

static int light_array_unserialize(zval* obj, zend_class_entry* ce, const unsigned char* buf, size_t buf_len, zend_unserialize_data* data) {
	if (buf_len != LightArray::DATA_SIZE) {
		return FAILURE;
	}
	object_init_ex(obj, ce);

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(obj));

	gsl::span<const uint8_t, LightArray::DATA_SIZE> span(reinterpret_cast<const uint8_t*>(buf), LightArray::DATA_SIZE);

	new (&object->lightArray) LightArray(span);

	return SUCCESS;
}

void light_array_fill(light_array_obj* object, zend_long level) {
	new (&object->lightArray) LightArray(static_cast<uint8_t>(level));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_PhpLightArray___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, __construct) {
	zend_string* payload;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(payload);
	ZEND_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(payload) != LightArray::DATA_SIZE) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Payload size must be %zu bytes, but got %zu bytes", LightArray::DATA_SIZE, ZSTR_LEN(payload));
	}

	static_assert(sizeof(ZSTR_VAL(payload)[0]) == sizeof(uint8_t), "this assert is probably useless, but it doesn't hurt to be sure");
	gsl::span<const uint8_t, LightArray::DATA_SIZE> span(reinterpret_cast<const uint8_t *>(ZSTR_VAL(payload)), LightArray::DATA_SIZE);

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(getThis()));

	new (&object->lightArray) LightArray(span);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_PhpLightArray_fill, 0, 1, pocketmine\\world\\format\\LightArray, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, fill) {
	zend_long level;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(level)
	ZEND_PARSE_PARAMETERS_END();

	if (level > LightLevel::MAX) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Light level must be max %u", LightLevel::MAX);
	}

	object_init_ex(return_value, light_array_entry);
	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(return_value));
	light_array_fill(object, level);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PhpLightArray_get, 0, 3, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, get) {
	zend_long x, y, z;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
	ZEND_PARSE_PARAMETERS_END();

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(getThis()));
	RETURN_LONG(static_cast<zend_long>(object->lightArray.get(x & 0xf, y & 0xf, z & 0xf)));
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PhpLightArray_set, 0, 4, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, z, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, set) {
	zend_long x, y, z, level;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 4, 4)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
		Z_PARAM_LONG(level)
	ZEND_PARSE_PARAMETERS_END();

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(getThis()));
	object->lightArray.set(x & 0xf, y & 0xf, z & 0xf, static_cast<uint8_t>(level));
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PhpLightArray_getData, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, getData) {
	zend_parse_parameters_none_throw();

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(getThis()));

	auto span = object->lightArray.getRawData();

	RETURN_STRINGL(reinterpret_cast<const char*>(span.data()), span.size_bytes());
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PhpLightArray_collectGarbage, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, collectGarbage) {
	zend_parse_parameters_none_throw();

	//TODO: we currently don't bother trying to save memory with copy-on-write tricks, to reduce implementation complexity.
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_PhpLightArray_isUniform, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpLightArray, isUniform) {
	zend_long value;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(value)
	ZEND_PARSE_PARAMETERS_END();

	if (value > LightLevel::MAX) {
		zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Light level must be max %u", LightLevel::MAX);
	}

	auto object = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(getThis()));
	RETURN_BOOL(object->lightArray.isUniform(value));
}

static zend_function_entry light_array_class_methods[] = {
	PHP_ME(PhpLightArray, __construct, arginfo_PhpLightArray___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(PhpLightArray, fill, arginfo_PhpLightArray_fill, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpLightArray, get, arginfo_PhpLightArray_get, ZEND_ACC_PUBLIC)
	PHP_ME(PhpLightArray, set, arginfo_PhpLightArray_set, ZEND_ACC_PUBLIC)
	PHP_ME(PhpLightArray, getData, arginfo_PhpLightArray_getData, ZEND_ACC_PUBLIC)
	PHP_ME(PhpLightArray, collectGarbage, arginfo_PhpLightArray_collectGarbage, ZEND_ACC_PUBLIC)
	PHP_ME(PhpLightArray, isUniform, arginfo_PhpLightArray_isUniform, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void register_light_array_class() {
	memcpy(&light_array_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	light_array_handlers.offset = XtOffsetOf(light_array_obj, std);
	light_array_handlers.free_obj = light_array_free;
	light_array_handlers.clone_obj = light_array_clone;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, light_array_classname, light_array_class_methods);
	ce.create_object = light_array_new;
	ce.serialize = light_array_serialize;
	ce.unserialize = light_array_unserialize;
	light_array_entry = zend_register_internal_class(&ce);
	light_array_entry->ce_flags |= ZEND_ACC_FINAL;
}
