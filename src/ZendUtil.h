#ifndef HAVE_BITARRAY_ZEND_UTIL_CPP_H
#define HAVE_BITARRAY_ZEND_UTIL_CPP_H

extern "C" {
#include "php.h"
}

template<typename class_name>
static inline class_name * fetch_from_zend_object(zend_object *obj) {
	return (class_name *)((char *)obj - XtOffsetOf(class_name, std));
}

#endif

