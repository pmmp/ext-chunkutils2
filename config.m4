dnl $Id$
dnl config.m4 for extension mcpe_paletted_block_array

PHP_ARG_ENABLE(mcpe_paletted_block_array, whether to enable mcpe_paletted_block_array support,
[  --enable-mcpe-paletted-block-array           Enable PocketMine chunk block-arrays extension])

if test "$PHP_MCPE_PALETTED_BLOCK_ARRAY" != "no"; then
  PHP_REQUIRE_CXX()
  PHP_NEW_EXTENSION(mcpe_paletted_block_array, mcpe_paletted_block_array.cpp, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
