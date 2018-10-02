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

#include "src/PhpPalettedBlockArray.h"
#include "src/PhpSubChunkConverter.h"

extern "C" {
#include "php.h"
#include "ext/standard/info.h"
#include "php_mcpe_paletted_block_array.h" //this one has to be C always, so the engine can understand it
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mcpe_paletted_block_array)
{
	register_paletted_block_array_class();
	register_sub_chunk_converter_class();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mcpe_paletted_block_array)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mcpe_paletted_block_array support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ mcpe_paletted_block_array_module_entry
 */
zend_module_entry mcpe_paletted_block_array_module_entry = {
	STANDARD_MODULE_HEADER,
	"mcpe_paletted_block_array",
	NULL,
	PHP_MINIT(mcpe_paletted_block_array),
	NULL, /* MSHUTDOWN */
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	PHP_MINFO(mcpe_paletted_block_array),
	PHP_MCPE_PALETTED_BLOCK_ARRAY_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MCPE_PALETTED_BLOCK_ARRAY
extern "C" {
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(mcpe_paletted_block_array)
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
