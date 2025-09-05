--TEST--
Test that PalettedBlockArray::fromData() works as expected with bytes
--FILE--
<?php

use pocketmine\world\format\PalettedBlockArray;
use pocketmine\world\format\PalettedBlockArrayLoadException;

$array = new PalettedBlockArray(0);
for($x = 0; $x < 16; $x++){
	for($y = 0; $y < 16; $y++){
		for($z = 0; $z < 16; $z++){
			$array->set($x, $y, $z, ($x << 8) | ($y << 4) | $z);
		}
	}
}

$array2 = PalettedBlockArray::fromData($array->getBitsPerBlock(), $array->getWordArray(), $array->getPaletteBytes());
$matched = 0;
for($x = 0; $x < 16; $x++){
	for($y = 0; $y < 16; $y++){
		for($z = 0; $z < 16; $z++){
			$expect = ($x << 8) | ($y << 4) | $z;
			if($array2->get($x, $y, $z) === $expect){
				$matched++;
			}else{
				echo "Mismatch at $x, $y, $z: actual " . $array->get($x, $y, $z) . ", expected " . $expect . "\n";
			}
		}
	}
}
var_dump($matched);

try{
    PalettedBlockArray::fromData($array->getBitsPerBlock(), $array->getWordArray(), $array->getPaletteBytes() . "\n");
    echo "This is not supposed to work\n";
}catch(PalettedBlockArrayLoadException $e){
    echo $e->getMessage() . "\n";
}
try{
    PalettedBlockArray::fromData($array->getBitsPerBlock(), $array->getWordArray(), substr($array->getPaletteBytes(), 0, -1));
    echo "This is not supposed to work\n";
}catch(PalettedBlockArrayLoadException $e){
    echo $e->getMessage() . "\n";
}
try{
    PalettedBlockArray::fromData($array->getBitsPerBlock(), $array->getWordArray(), "");
    echo "This is not supposed to work\n";
}catch(PalettedBlockArrayLoadException $e){
    //TODO: this exception type is confusing, but it comes from a different place than the bytes-handling, so it's currently expected
    echo $e->getMessage() . "\n";
}
?>
--EXPECT--
int(4096)
palette length in bytes must be a multiple of 4, but have 16385 bytes
palette length in bytes must be a multiple of 4, but have 16383 bytes
palette cannot have a zero size
