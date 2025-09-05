--TEST--
Test that working with palettes as strings works as expected
--FILE--
<?php

use pocketmine\world\format\PalettedBlockArray;

$array = new PalettedBlockArray(0);
for($x = 0; $x < 16; $x++){
	for($y = 0; $y < 16; $y++){
		for($z = 0; $z < 16; $z++){
			$array->set($x, $y, $z,  ($x << 8) | ($y << 4) | $z);
		}
	}
}
$expected = "";
for($i = 0; $i < 4096; $i++){
	$expected .= pack("V", $i);
}
var_dump($expected === $array->getPaletteBytes());
?>
--EXPECT--
bool(true)
