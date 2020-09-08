--TEST--
Test that LightArray::fill() produces the correct results
--SKIPIF--
<?php if(!extension_loaded('chunkutils2')) die('skip: extension not loaded'); ?>
--FILE--
<?php declare(strict_types=1);

namespace pocketmine\world\format;

$checks = 0;
for($i = 0; $i < 16; ++$i){
	$array = LightArray::fill($i);
	for($x = 0; $x < 16; ++$x){
		for($z = 0; $z < 16; ++$z){
			for($y = 0; $y < 16; ++$y){
				if($array->get($x, $y, $z) !== $i){
					die("bad value at $x $y $z");
				}
				$checks++;
			}
		}
	}
}

echo "$checks ok\n";
?>
--EXPECT--
65536 ok
