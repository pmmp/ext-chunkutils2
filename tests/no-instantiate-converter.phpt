--TEST--
Tests that SubChunkConverter cannot be instantiated
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

var_dump(new \pocketmine\world\format\io\SubChunkConverter);
?>
--EXPECTF--
Fatal error: Uncaught Error: Call to private pocketmine\world\format\io\SubChunkConverter::__construct() from invalid context in %s:%d
Stack trace:
#0 {main}
  thrown in %s on line %d