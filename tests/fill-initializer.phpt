--TEST--
Check freshly allocated paletted array is filled with the given initializer
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php
const FILL_VALUE = 6;
$array = new \pocketmine\world\format\PalettedBlockArray(FILL_VALUE);

$checked = 0;
for($x = 0; $x < 16; ++$x){
	for($z = 0; $z < 16; ++$z){
		for($y = 0; $y < 16; ++$y){
			$entry = $array->get($x, $y, $z);
			if($entry !== FILL_VALUE){
				die("wrong value at $x $y $z: expected " . FILL_VALUE . " got $entry");
			}
			$checked++;
		}
	}
}

var_dump("$checked ok");
?>
--EXPECT--
string(7) "4096 ok"