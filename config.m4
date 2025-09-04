dnl $Id$
dnl config.m4 for extension chunkutils2

PHP_ARG_ENABLE(chunkutils2, whether to enable chunkutils2 support,
[  --enable-chunkutils2                         Enable PocketMine ChunkUtils2 extension])

if test "$PHP_CHUNKUTILS2" != "no"; then
  PHP_REQUIRE_CXX()

  dnl the 6th parameter here is required for C++ shared extensions
  PHP_NEW_EXTENSION(chunkutils2, chunkutils2.cpp src/PhpLightArray.cpp src/PhpPalettedBlockArray.cpp src/PhpSubChunk.cpp src/PhpSubChunkConverter.cpp, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 -std=c++14 -fno-strict-aliasing -DGSL_THROW_ON_CONTRACT_VIOLATION=1, yes)
  PHP_SUBST(CHUNKUTILS2_SHARED_LIBADD)
  PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
  PHP_ADD_BUILD_DIR($ext_builddir/lib, 1)
  PHP_ADD_INCLUDE($ext_builddir)
  PHP_ADD_INCLUDE($ext_builddir/gsl/include)
fi
