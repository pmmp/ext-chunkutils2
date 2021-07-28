--TEST--
Test that LightArray::isUniform() works correctly
--SKIPIF--
<?php if(!extension_loaded('chunkutils2')) die('skip: extension not loaded'); ?>
--FILE--
<?php declare(strict_types=1);

namespace pocketmine\world\format;

$array = LightArray::fill(0);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

$array->set(0, 0, 0, 1);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

$array->set(0, 0, 0, 0);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));

$array->set(10, 10, 10, 2);
var_dump($array->isUniform(0));
var_dump($array->isUniform(1));
var_dump($array->isUniform(2));
echo "ok\n";
?>
--EXPECT--
bool(true)
bool(false)
bool(false)
bool(false)
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
ok
