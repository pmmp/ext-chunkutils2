--TEST--
Test conversion of a legacy XZY chunk column
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$ids = random_bytes(32768);
$data = random_bytes(16384);

$checked = 0;
for($yOffset = 0; $yOffset < 8; ++$yOffset){
	$converted = \pocketmine\world\format\io\SubChunkConverter::convertSubChunkFromLegacyColumn($ids, $data, $yOffset);
	for($x = 0; $x < 16; ++$x){
		for($z = 0; $z < 16; ++$z){
			for($y = 0; $y < 16; ++$y){
				$yy = ($yOffset << 4) | $y;
				$idx = ($x << 11) | ($z << 7) | $yy;
				$dataIdx = $idx >> 1;

				$id = ord($ids[$idx]);
				$m = ord($data[$dataIdx]);
				if(($yy & 1) === 0){
					$meta = $m & 0x0f;
				}else{
					$meta = $m >> 4;
				}
				$expected = ($id << 4) | $meta;

				if($expected !== ($actual = $converted->get($x, $y, $z))){
					die("mismatch at $x $yy $z: expected $expected, got $actual");
				}
				$checked++;
			}
		}
	}
}

var_dump("$checked ok");
?>
--EXPECT--
string(8) "32768 ok"