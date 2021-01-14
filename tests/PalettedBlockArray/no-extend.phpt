--TEST--
Tests that PalettedBlockArray cannot be extended
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

var_dump(new class(1) extends \pocketmine\world\format\PalettedBlockArray{});

?>
--EXPECTF--
Fatal error: Class %s@anonymous may not inherit from final class (pocketmine\world\format\PalettedBlockArray) in %s on line %d
