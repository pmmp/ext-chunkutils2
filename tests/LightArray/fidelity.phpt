--TEST--
Test fidelity of LightArray (read/write)
--SKIPIF--
<?php if(!extension_loaded('chunkutils2')) die('skip: extension not loaded'); ?>
--FILE--
<?php declare(strict_types=1);

namespace pocketmine\world\format;

$seed = mt_rand();
mt_srand($seed);

$values = [];
for($i = 0; $i < 4096; ++$i){
	$values[] = mt_rand(0, 15);
}

$array = LightArray::fill(0);
foreach($values as $i => $v){
	$array->set(($i >> 8) & 0xf, ($i >> 4) & 0xf, $i & 0xf, $v);
}

foreach($values as $i => $v){
	if($array->get(($i >> 8) & 0xf, ($i >> 4) & 0xf, $i & 0xf) !== $v){
		die("mismatched value at $i with seed $seed");
	}
}

echo "ok\n";
?>
--EXPECT--
ok
