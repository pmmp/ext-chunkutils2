dnl $Id$
dnl config.m4 for extension mcpe_paletted_block_array

PHP_ARG_ENABLE(mcpe_paletted_block_array, whether to enable mcpe_paletted_block_array support,
[  --enable-mcpe-paletted-block-array           Enable PocketMine chunk block-arrays extension])

if test "$PHP_MCPE_PALETTED_BLOCK_ARRAY" != "no"; then
  PHP_REQUIRE_CXX()

  dnl apparently PHP_REQUIRE_CXX() isn't good enough if we're compiling as a shared library
  PHP_ADD_LIBRARY(stdc++, 1, MCPE_PALETTED_BLOCK_ARRAY_SHARED_LIBADD)
  PHP_SUBST(MCPE_PALETTED_BLOCK_ARRAY_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mcpe_paletted_block_array, mcpe_paletted_block_array.cpp lib/SubChunkConverter.cpp src/PhpPalettedBlockArray.cpp src/PhpSubChunkConverter.cpp, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -std=c++11)
  PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
  PHP_ADD_BUILD_DIR($ext_builddir/lib, 1)
  PHP_ADD_INCLUDE($ext_builddir)
fi
