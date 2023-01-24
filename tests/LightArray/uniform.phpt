--TEST--
Test that LightArray::isUniform() works correctly
--SKIPIF--
<?php if(!extension_loaded('chunkutils2')) die('skip: extension not loaded'); ?>
--FILE--
<?php declare(strict_types=1);

namespace pocketmine\world\format;

echo "fill 0\n";
$array = LightArray::fill(0);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

echo "first is 1\n";
$array->set(0, 0, 0, 1);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

echo "all are y%2\n";
for($x = 0; $x < 16; $x++) {
	for($y = 0; $y < 16; $y++) {
		for($z = 0; $z < 16; $z++) {
			$array->set($x, $y, $z, $y % 2);
		}
	}
}
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

echo "all are 1-(y%2)\n";
for($x = 0; $x < 16; $x++) {
	for($y = 0; $y < 16; $y++) {
		for($z = 0; $z < 16; $z++) {
			$array->set($x, $y, $z, 1 - ($y % 2));
		}
	}
}
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

echo "all are 1\n";
for($x = 0; $x < 16; $x++) {
	for($y = 0; $y < 16; $y++) {
		for($z = 0; $z < 16; $z++) {
			$array->set($x, $y, $z, 1);
		}
	}
}
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

echo "one of the coordinates is 2\n";
$array->set(10, 10, 10, 2);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));
var_dump($array->isUniform(2));
echo "ok\n";
?>
--EXPECT--
fill 0
bool(true)
bool(false)
first is 1
bool(false)
bool(false)
all are y%2
bool(false)
bool(false)
all are 1-(y%2)
bool(false)
bool(false)
all are 1
bool(false)
bool(true)
one of the coordinates is 2
bool(false)
bool(false)
bool(false)
ok
