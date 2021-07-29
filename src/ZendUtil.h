#ifndef HAVE_BITARRAY_ZEND_UTIL_CPP_H
#define HAVE_BITARRAY_ZEND_UTIL_CPP_H

extern "C" {
#include "php.h"
}

template<typename class_name>
static inline class_name * fetch_from_zend_object(zend_object *obj) {
	return (class_name *)((char *)obj - XtOffsetOf(class_name, std));
}

template<typename class_name>
static class_name* alloc_custom_zend_object(zend_class_entry* ce, zend_object_handlers *handlers) {
	class_name* object = (class_name*)emalloc(sizeof(class_name) + zend_object_properties_size(ce));

	zend_object_std_init(&object->std, ce);
	object_properties_init(&object->std, ce);

	object->std.handlers = handlers;

	return object;
}

#if PHP_VERSION_ID >= 80000
typedef zend_object chunkutils2_handler_context;
#define Z_OBJ_FROM_HANDLER_CONTEXT(context) (context)
#define HANDLER_CONTEXT_FROM_ZVAL(zv) Z_OBJ_P(zv)
#else
typedef zval chunkutils2_handler_context;
#define Z_OBJ_FROM_HANDLER_CONTEXT(context) Z_OBJ_P((context))
#define HANDLER_CONTEXT_FROM_ZVAL(zv) (zv)
#endif

//custom version of ZEND_ARG_OBJ_INFO that accepts strings directly, instead of barewords - needed for macro usage
#if PHP_VERSION_ID < 80000
#define CUSTOM_ZEND_ARG_OBJ_INFO_STR(pass_by_ref, name, classname, allow_null)  { #name, ZEND_TYPE_ENCODE_CLASS_CONST(classname, allow_null), pass_by_ref, 0 },
#else
#define CUSTOM_ZEND_ARG_OBJ_INFO_STR(pass_by_ref, name, classname, allow_null) \
	{ #name, ZEND_TYPE_INIT_CLASS_CONST(classname, allow_null, _ZEND_ARG_INFO_FLAGS(pass_by_ref, 0)), NULL },
#endif

#define CUSTOM_ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX_STR(name, return_reference, required_num_args, class_name, allow_null) \
	static const zend_internal_arg_info name[] = { \
		{ (const char*)(zend_uintptr_t)(required_num_args), \
			ZEND_TYPE_INIT_CLASS_CONST(class_name, allow_null, _ZEND_ARG_INFO_FLAGS(return_reference, 0)), NULL },

#endif

