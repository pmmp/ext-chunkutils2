--TEST--
Tests that SubChunkConverter cannot be extended
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

var_dump(new class extends \pocketmine\world\format\io\SubChunkConverter{});

?>
--EXPECTF--
Fatal error: Class %s@anonymous %s final class %s in %s on line %d
