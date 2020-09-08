--TEST--
Test that LightArray::getData() can be used to produce a new light array equivalent to the original
--SKIPIF--
<?php if(!extension_loaded('chunkutils2')) die('skip: extension not loaded'); ?>
--FILE--
<?php declare(strict_types=1);

namespace pocketmine\world\format;

$array = new LightArray(random_bytes(2048));

$array2 = new LightArray($array->getData());

$checked = 0;
for($x = 0; $x < 16; ++$x){
	for($z = 0; $z < 16; ++$z){
		for($y = 0; $y < 16; ++$y){
			if($array->get($x, $y, $z) !== $array2->get($x, $y, $z)){
				die("mismatch at $x $y $z");
			}
			$checked++;
		}
	}
}

echo "$checked ok\n";
?>
--EXPECT--
4096 ok
