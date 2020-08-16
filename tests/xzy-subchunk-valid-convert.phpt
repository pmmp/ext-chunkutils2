--TEST--
Test conversion of a standard XZY subchunk
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$ids = random_bytes(4096);
$data = random_bytes(2048);

$t1 = \pocketmine\world\format\io\SubChunkConverter::convertSubChunkXZY($ids, $data);

$checked = 0;
for($x = 0; $x < 16; ++$x){
	for($z = 0; $z < 16; ++$z){
		for($y = 0; $y < 16; ++$y){
			$actual = $t1->get($x, $y, $z);
			
			$idx = ($x << 8) | ($z << 4) | $y;
			$meta = (ord($data[($x << 7) | ($z << 3) | ($y >> 1)]) >> (($y & 1) << 2)) & 0xf;
			$expected = (ord($ids[$idx]) << 4) | $meta;
			if($actual !== $expected){
				die("mismatch at $x $y $z: expected $expected, got $actual");
			}
			$checked++;
		}
	}
}

var_dump("$checked ok");
?>
--EXPECT--
string(7) "4096 ok"
