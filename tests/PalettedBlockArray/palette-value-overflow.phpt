--TEST--
Check behaviour of too-large palette entry value
--SKIPIF--
<?php 
if(!extension_loaded("chunkutils2")) die("skip extension not loaded");
if(PHP_INT_SIZE !== 8) die("skip only for 64-bit");
?>
--FILE--
<?php

$values = [PHP_INT_MIN, -1, PHP_INT_MAX, (1 << 32)];

foreach($values as $v){
	try{
		$palette = new \pocketmine\world\format\PalettedBlockArray($v);
		var_dump("bad");
	}catch(\InvalidArgumentException $e){
		echo $e->getMessage() . "\n";
	}

	try{
		$palette = \pocketmine\world\format\PalettedBlockArray::fromData(1, str_repeat("\x00", 512), [$v]);
		var_dump("bad");
	}catch(\InvalidArgumentException $e){
		echo $e->getMessage() . "\n";
	}
}

var_dump("ok");
?>
--EXPECT--
value -9223372036854775808 is too large to be used as a palette entry
value -9223372036854775808 is too large to be used as a palette entry
value -1 is too large to be used as a palette entry
value -1 is too large to be used as a palette entry
value 9223372036854775807 is too large to be used as a palette entry
value 9223372036854775807 is too large to be used as a palette entry
value 4294967296 is too large to be used as a palette entry
value 4294967296 is too large to be used as a palette entry
string(2) "ok"
