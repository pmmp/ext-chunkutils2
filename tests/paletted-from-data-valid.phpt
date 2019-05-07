--TEST--
Test PalettedBlockArray::fromData() correctly checks payload sizes
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

foreach([1,2,3,4,5,6,8,16] as $bitsPerBlock){
	$blocksPerWord = intdiv(32, $bitsPerBlock);
	$payloadSize = (intdiv(4096, $blocksPerWord) + ((4096 % $blocksPerWord) !== 0 ? 1 : 0)) * 4;

	$palette = \pocketmine\world\format\PalettedBlockArray::fromData($bitsPerBlock, str_repeat("\x00", $payloadSize), [$bitsPerBlock]);
	var_dump($palette->get(0, 0, 0));

	try{
		$palette = \pocketmine\world\format\PalettedBlockArray::fromData($bitsPerBlock, str_repeat("\x00", $payloadSize - 1), [$bitsPerBlock]);
		var_dump($palette->get(0, 0, 0));
	}catch(\RuntimeException $e){
		echo "size - 1 is not allowed\n";
	}

	try{
		$palette = \pocketmine\world\format\PalettedBlockArray::fromData($bitsPerBlock, str_repeat("\x00", $payloadSize + 1), [$bitsPerBlock]);
		var_dump($palette->get(0, 0, 0));
	}catch(\RuntimeException $e){
		echo "size + 1 is not allowed\n";
	}

	try{
		$palette = \pocketmine\world\format\PalettedBlockArray::fromData($bitsPerBlock, "", [$bitsPerBlock]);
		var_dump($palette->get(0, 0, 0));
	}catch(\RuntimeException $e){
		echo "empty payload is not allowed\n";
	}
}
var_dump("ok");
?>
--EXPECT--
int(1)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(2)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(3)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(4)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(5)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(6)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(8)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
int(16)
size - 1 is not allowed
size + 1 is not allowed
empty payload is not allowed
string(2) "ok"
