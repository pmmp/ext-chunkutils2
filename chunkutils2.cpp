/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "src/PhpLightArray.h"
#include "src/PhpPalettedBlockArray.h"
#include "src/PhpSubChunk.h"
#include "src/PhpSubChunkConverter.h"

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "php_chunkutils2.h" //this one has to be C always, so the engine can understand it
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(chunkutils2)
{
	register_light_array_class();
	register_paletted_block_array_class();
	register_sub_chunk_class();
	register_sub_chunk_converter_class();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(chunkutils2)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "chunkutils2 support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ chunkutils2_module_entry
 */
zend_module_entry chunkutils2_module_entry = {
	STANDARD_MODULE_HEADER,
	"chunkutils2",
	NULL,
	PHP_MINIT(chunkutils2),
	NULL, /* MSHUTDOWN */
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	PHP_MINFO(chunkutils2),
	PHP_CHUNKUTILS2_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CHUNKUTILS2
extern "C" {
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(chunkutils2)
}
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
