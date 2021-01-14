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

#endif

