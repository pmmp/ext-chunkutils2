--TEST--
Test behaviour of PalettedBlockArray::getExpectedWordArraySize()
--SKIPIF--
<?php 
if(!extension_loaded("chunkutils2")) die("skip extension not loaded");
?>
--FILE--
<?php

for($i = 0; $i < 20; ++$i){
	try{
		var_dump(\pocketmine\world\format\PalettedBlockArray::getExpectedWordArraySize($i));
	}catch(\InvalidArgumentException $e){
		echo $e->getMessage() . "\n";
	}
}

//an outlier to make sure invalid-argument checks are working
try{
	var_dump(\pocketmine\world\format\PalettedBlockArray::getExpectedWordArraySize(257));
}catch(\InvalidArgumentException $e){
	echo $e->getMessage() . "\n";
}
var_dump("ok");
?>
--EXPECT--
invalid bits-per-block: 0
int(512)
int(1024)
int(1640)
int(2048)
int(2732)
int(3280)
invalid bits-per-block: 7
int(4096)
invalid bits-per-block: 9
invalid bits-per-block: 10
invalid bits-per-block: 11
invalid bits-per-block: 12
invalid bits-per-block: 13
invalid bits-per-block: 14
invalid bits-per-block: 15
int(8192)
invalid bits-per-block: 17
invalid bits-per-block: 18
invalid bits-per-block: 19
invalid bits-per-block: 257
string(2) "ok"