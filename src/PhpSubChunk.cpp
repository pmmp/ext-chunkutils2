#include <new>
#include <vector>

#include "PhpPalettedBlockArray.h"
#include "PhpPalettedBlockArrayObj.h"
#include "PhpLightArray.h"
#include "PhpSubChunk.h"
#include "ZendUtil.h"
#include "stubs/pocketmine/world/format/SubChunk_arginfo.h"

extern "C" {
#include "php.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_hash.h"
}

typedef struct {
	zend_long emptyBlockId;
	std::vector<paletted_block_array_obj*> blockLayers;
	light_array_obj* blockLight;
	light_array_obj* skyLight;
	zend_object std;
} sub_chunk_obj;

zend_class_entry* sub_chunk_entry;
static zend_object_handlers sub_chunk_handlers;

static inline sub_chunk_obj* fetch_intern(zend_object* object) {
	return fetch_from_zend_object<sub_chunk_obj>(object);
}

#define sub_chunk_this() fetch_intern(Z_OBJ_P(getThis()))

static zend_object* sub_chunk_new(zend_class_entry* ce) {
	sub_chunk_obj* result = alloc_custom_zend_object<sub_chunk_obj>(ce, &sub_chunk_handlers);

	result->emptyBlockId = 0;
	new(&result->blockLayers) std::vector<paletted_block_array_obj*>();
	result->blockLight = nullptr;
	result->skyLight = nullptr;

	return &result->std;
}

static zend_object* sub_chunk_clone(zend_object* object) {
	sub_chunk_obj* const old_object = fetch_intern(object);
	sub_chunk_obj* const new_object = fetch_intern(sub_chunk_new(object->ce));

	new_object->emptyBlockId = old_object->emptyBlockId;
	new(&new_object->blockLayers) std::vector<paletted_block_array_obj*>();
	for (auto blockLayer : old_object->blockLayers) {
		auto clonedBlockLayer = fetch_from_zend_object<paletted_block_array_obj>(blockLayer->std.handlers->clone_obj(&blockLayer->std));
		new_object->blockLayers.push_back(clonedBlockLayer);
	}

	auto blockLight = old_object->blockLight;
	if (blockLight != nullptr) {
		new_object->blockLight = fetch_from_zend_object<light_array_obj>(blockLight->std.handlers->clone_obj(&blockLight->std));
	}
	auto skyLight = old_object->skyLight;
	if (skyLight != nullptr) {
		new_object->skyLight = fetch_from_zend_object<light_array_obj>(skyLight->std.handlers->clone_obj(&skyLight->std));
	}

	zend_objects_clone_members(&new_object->std, &old_object->std); //copy user-assigned properties

	return &new_object->std;
}

static void sub_chunk_free(zend_object* std) {
	auto object = fetch_intern(std);

	for (auto blockLayer : object->blockLayers) {
		OBJ_RELEASE(&blockLayer->std);
	}
	object->blockLayers.~vector();

	auto blockLight = object->blockLight;
	if (blockLight != nullptr) {
		OBJ_RELEASE(&blockLight->std);
	}

	auto skyLight = object->skyLight;
	if (skyLight != nullptr) {
		OBJ_RELEASE(&skyLight->std);
	}

	zend_object_std_dtor(std);
}

static int sub_chunk_serialize(zval* obj, unsigned char** buffer, size_t* buf_len, zend_serialize_data* data) {
	sub_chunk_obj* object = fetch_intern(Z_OBJ_P(obj));

	smart_str buf = { 0 };
	php_serialize_data_t serialize_data = (php_serialize_data_t)data;
	PHP_VAR_SERIALIZE_INIT(serialize_data);

	zval zv;

	ZVAL_LONG(&zv, object->emptyBlockId);
	php_var_serialize(&buf, &zv, &serialize_data);

	array_init_size(&zv, object->blockLayers.size());
	for (auto i : object->blockLayers) {
		zval zlayer;
		ZVAL_OBJ(&zlayer, &i->std);
		Z_ADDREF(zlayer);
		add_next_index_zval(&zv, &zlayer);
	}
	php_var_serialize(&buf, &zv, &serialize_data);
	zval_dtor(&zv);

	{
		auto blockLight = object->blockLight;
		if (blockLight != nullptr) {
			ZVAL_OBJ(&zv, &blockLight->std);
		} else {
			ZVAL_NULL(&zv);
		}
		php_var_serialize(&buf, &zv, &serialize_data);
	}
	{
		auto skyLight = object->skyLight;
		if (skyLight != nullptr) {
			ZVAL_OBJ(&zv, &skyLight->std);
		} else {
			ZVAL_NULL(&zv);
		}
		php_var_serialize(&buf, &zv, &serialize_data);
	}

	ZVAL_ARR(&zv, zend_std_get_properties(Z_OBJ_P(obj)));
	php_var_serialize(&buf, &zv, &serialize_data);

	PHP_VAR_SERIALIZE_DESTROY(serialize_data);
	*buffer = (unsigned char*)estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
	*buf_len = ZSTR_LEN(buf.s);
	zend_string_release(buf.s);

	return SUCCESS;
}

static int sub_chunk_unserialize(zval* object, zend_class_entry* ce, const unsigned char* buf, size_t buf_len, zend_unserialize_data* data) {
	php_unserialize_data_t unserialize_data = (php_unserialize_data_t)data;
	PHP_VAR_UNSERIALIZE_INIT(unserialize_data);

	int result = FAILURE;

	const unsigned char* start = buf;
	const unsigned char* end = buf + buf_len;

	zval* emptyBlockId, * blockLayers, * zvBlockLayer, * blockLight, * skyLight, *properties;
	sub_chunk_obj* intern;
	emptyBlockId = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(emptyBlockId, &start, end, &unserialize_data) || Z_TYPE_P(emptyBlockId) != IS_LONG) {
		zend_throw_exception(NULL, "Failed to unserialize SubChunk::emptyBlockId", 0);
		goto end;
	}

	blockLayers = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(blockLayers, &start, end, &unserialize_data) || Z_TYPE_P(blockLayers) != IS_ARRAY) {
		zend_throw_exception(NULL, "Failed to unserialize SubChunk::blockLayers", 0);
		goto end;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(blockLayers), zvBlockLayer) {
		if (Z_TYPE_P(zvBlockLayer) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(zvBlockLayer), paletted_block_array_entry)) {
			zend_throw_exception_ex(NULL, 0, "SubChunk::blockLayers expects only objects of type %s", ZSTR_VAL(paletted_block_array_entry->name));
			goto end;
		}
	} ZEND_HASH_FOREACH_END();

	blockLight = var_tmp_var(&unserialize_data);
	if (
		!php_var_unserialize(blockLight, &start, end, &unserialize_data) || (
			Z_TYPE_P(blockLight) != IS_NULL &&
			(Z_TYPE_P(blockLight) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(blockLight), light_array_entry))
		)
	) {
		zend_throw_exception(NULL, "Failed to unserialize SubChunk::blockLight", 0);
		goto end;
	}

	skyLight = var_tmp_var(&unserialize_data);
	if (
		!php_var_unserialize(skyLight, &start, end, &unserialize_data) || (
			Z_TYPE_P(skyLight) != IS_NULL &&
			(Z_TYPE_P(skyLight) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(skyLight), light_array_entry))
		)
	) {
		zend_throw_exception(NULL, "Failed to unserialize SubChunk::skyLight", 0);
		goto end;
	}

	object_init_ex(object, sub_chunk_entry);
	intern = fetch_intern(Z_OBJ_P(object));
	intern->emptyBlockId = Z_LVAL_P(emptyBlockId);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(blockLayers), zvBlockLayer) {
		auto blockLayerIntern = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(zvBlockLayer));
		intern->blockLayers.push_back(blockLayerIntern);
		Z_ADDREF_P(zvBlockLayer);
	} ZEND_HASH_FOREACH_END();

	if (Z_TYPE_P(blockLight) == IS_OBJECT) {
		intern->blockLight = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(blockLight));
		Z_ADDREF_P(blockLight);
	}
	if (Z_TYPE_P(skyLight) == IS_OBJECT) {
		intern->skyLight = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(skyLight));
		Z_ADDREF_P(skyLight);
	}

	properties = var_tmp_var(&unserialize_data);
	if (!php_var_unserialize(properties, &start, end, &unserialize_data) || Z_TYPE_P(properties) != IS_ARRAY) {
		zend_throw_exception(NULL, "Failed to unserialize SubChunk user properties", 0);
		goto end;
	}
	if (zend_hash_num_elements(Z_ARRVAL_P(properties)) != 0) {
		zend_hash_copy(zend_std_get_properties(Z_OBJ_P(object)), Z_ARRVAL_P(properties), (copy_ctor_func_t)zval_add_ref);
	}
	result = SUCCESS;
end:
	PHP_VAR_UNSERIALIZE_DESTROY(unserialize_data);
	return result;
}

static void sub_chunk_collect_garbage(sub_chunk_obj* object) {
	std::vector<paletted_block_array_obj*> cleanedLayers;

	for (auto layer : object->blockLayers) {
		layer->container.collectGarbage(false, 0);

		bool keep = false;
		for (auto p : layer->container.getPalette()) {
			if (p != object->emptyBlockId) {
				keep = true;
				break;
			}
		}
		if (keep) {
			cleanedLayers.push_back(layer);
		} else {
			OBJ_RELEASE(&layer->std);
		}
	}
	object->blockLayers = cleanedLayers;

	auto skyLight = object->skyLight;
	if (skyLight != nullptr && skyLight->lightArray.isUniform(0)) {
		OBJ_RELEASE(&skyLight->std);
		object->skyLight = nullptr;
	}

	auto blockLight = object->blockLight;
	if (blockLight != nullptr && blockLight->lightArray.isUniform(0)) {
		OBJ_RELEASE(&blockLight->std);
		object->blockLight = nullptr;
	}
}

#define SUB_CHUNK_METHOD(name) PHP_METHOD(pocketmine_world_format_SubChunk, name)

SUB_CHUNK_METHOD(__construct) {
	zend_long emptyBlockId;
	HashTable* htBlockLayers;
	zval* skyLight = NULL;
	zval* blockLight = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 4)
		Z_PARAM_LONG(emptyBlockId)
		Z_PARAM_ARRAY_HT(htBlockLayers)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(skyLight, light_array_entry, 1, 0)
		Z_PARAM_OBJECT_OF_CLASS_EX(blockLight, light_array_entry, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	zval *layer;
	ZEND_HASH_FOREACH_VAL(htBlockLayers, layer) {
		if (Z_TYPE_P(layer) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(layer), paletted_block_array_entry)) {
			zend_type_error("Layers array should contain only %s objects", ZSTR_VAL(paletted_block_array_entry->name));
			return;
		}
	} ZEND_HASH_FOREACH_END();

	auto intern = sub_chunk_this();
	intern->emptyBlockId = emptyBlockId;

	ZEND_HASH_FOREACH_VAL(htBlockLayers, layer) {
		auto internLayer = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ_P(layer));
		intern->blockLayers.push_back(internLayer);
		Z_ADDREF_P(layer);
	} ZEND_HASH_FOREACH_END();

	if (skyLight != NULL) {
		intern->skyLight = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(skyLight));
		Z_ADDREF_P(skyLight);
	}
	if (blockLight != NULL) {
		intern->blockLight = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(blockLight));
		Z_ADDREF_P(blockLight);
	}
}

SUB_CHUNK_METHOD(isEmptyAuthoritative) {
	zend_parse_parameters_none_throw();

	auto intern = sub_chunk_this();
	sub_chunk_collect_garbage(intern);
	RETURN_BOOL(intern->blockLayers.empty());
}

SUB_CHUNK_METHOD(isEmptyFast) {
	zend_parse_parameters_none_throw();
	RETURN_BOOL(sub_chunk_this()->blockLayers.empty());
}

SUB_CHUNK_METHOD(getEmptyBlockId) {
	zend_parse_parameters_none_throw();
	RETURN_LONG(sub_chunk_this()->emptyBlockId);
}

SUB_CHUNK_METHOD(getFullBlock) {
	zend_long x, y, z;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
	ZEND_PARSE_PARAMETERS_END();

	auto intern = sub_chunk_this();
	if (intern->blockLayers.empty()) {
		RETURN_LONG(intern->emptyBlockId);
	} else {
		RETURN_LONG(intern->blockLayers[0]->container.get(x, y, z));
	}
}

SUB_CHUNK_METHOD(setFullBlock) {
	zend_long x, y, z, block;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 4, 4)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(y)
		Z_PARAM_LONG(z)
		Z_PARAM_LONG(block)
	ZEND_PARSE_PARAMETERS_END();

	auto intern = sub_chunk_this();
	paletted_block_array_obj* blockLayer;
	if (intern->blockLayers.empty()) {
		zval zvNewLayer;
		object_init_ex(&zvNewLayer, paletted_block_array_entry);
		blockLayer = fetch_from_zend_object<paletted_block_array_obj>(Z_OBJ(zvNewLayer));
		if (!paletted_block_array_fill(blockLayer, intern->emptyBlockId)) {
			return;
		}
		intern->blockLayers.push_back(blockLayer);
	} else {
		blockLayer = intern->blockLayers[0];
	}
	blockLayer->container.set(x, y, z, block);
}

SUB_CHUNK_METHOD(getBlockLayers) {
	zend_parse_parameters_none_throw();

	auto intern = sub_chunk_this();
	array_init_size(return_value, intern->blockLayers.size());
	for (auto layer : intern->blockLayers) {
		zval zv;
		ZVAL_OBJ(&zv, &layer->std);
		Z_ADDREF(zv);
		add_next_index_zval(return_value, &zv);
	}
}

SUB_CHUNK_METHOD(getHighestBlockAt) {
	zend_long x, z;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(x)
		Z_PARAM_LONG(z)
	ZEND_PARSE_PARAMETERS_END();

	auto intern = sub_chunk_this();
	if (intern->blockLayers.empty()) {
		RETURN_LONG(-1);
	}

	auto layerZero = intern->blockLayers[0];
	for (auto y = 15; y >= 0; y--) {
		if (layerZero->container.get(x, y, z) != intern->emptyBlockId) {
			RETURN_LONG(y);
		}
	}
	RETURN_LONG(-1);
}

static inline void ensure_light_array_initialized(light_array_obj** lightArray) {
	if (*lightArray == nullptr) {
		zval zv;
		object_init_ex(&zv, light_array_entry);
		auto lightArrayIntern = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(&zv));
		light_array_fill(lightArrayIntern, 0);
		*lightArray = lightArrayIntern;
	}
}

SUB_CHUNK_METHOD(getBlockSkyLightArray) {
	zend_parse_parameters_none_throw();

	auto intern = sub_chunk_this();
	ensure_light_array_initialized(&intern->skyLight);
	RETVAL_OBJ(&intern->skyLight->std);
	Z_ADDREF_P(return_value);
}

SUB_CHUNK_METHOD(getBlockLightArray) {
	zend_parse_parameters_none_throw();

	auto intern = sub_chunk_this();
	ensure_light_array_initialized(&intern->blockLight);
	RETVAL_OBJ(&intern->blockLight->std);
	Z_ADDREF_P(return_value);
}

#define macro_SubChunk_setLightArray(lightArrayFieldName) \
	zval* data; \
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1) \
		Z_PARAM_OBJECT_OF_CLASS(data, light_array_entry) \
	ZEND_PARSE_PARAMETERS_END(); \
	auto intern = sub_chunk_this(); \
	if (intern->lightArrayFieldName != nullptr) { \
		OBJ_RELEASE(&intern->lightArrayFieldName->std); \
	} \
	intern->lightArrayFieldName = fetch_from_zend_object<light_array_obj>(Z_OBJ_P(data)); \
	Z_ADDREF_P(data);

SUB_CHUNK_METHOD(setBlockSkyLightArray) {
	macro_SubChunk_setLightArray(skyLight);
}

SUB_CHUNK_METHOD(setBlockLightArray) {
	macro_SubChunk_setLightArray(blockLight);
}

SUB_CHUNK_METHOD(collectGarbage) {
	zend_parse_parameters_none_throw();

	sub_chunk_collect_garbage(sub_chunk_this());
}

void register_sub_chunk_class() {
	memcpy(&sub_chunk_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sub_chunk_handlers.offset = XtOffsetOf(sub_chunk_obj, std);
	sub_chunk_handlers.free_obj = sub_chunk_free;
	sub_chunk_handlers.clone_obj = sub_chunk_clone;

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "pocketmine\\world\\format\\SubChunk", class_pocketmine_world_format_SubChunk_methods);
	ce.create_object = sub_chunk_new;
	ce.serialize = sub_chunk_serialize;
	ce.unserialize = sub_chunk_unserialize;
	sub_chunk_entry = zend_register_internal_class(&ce);
	sub_chunk_entry->ce_flags |= ZEND_ACC_FINAL;
}
