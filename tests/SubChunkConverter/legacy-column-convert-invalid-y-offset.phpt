--TEST--
Test handling of bad yOffset in SubChunkConverter::convertSubChunkFromLegacyColumn()
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$ids = str_repeat("0", 32768);
$data = str_repeat("0", 16384);

foreach([-1, PHP_INT_MIN, PHP_INT_MAX, 8, 9] as $offset){
	try{
		\pocketmine\world\format\io\SubChunkConverter::convertSubChunkFromLegacyColumn($ids, $data, $offset);
	}catch(\InvalidArgumentException $e){
		var_dump("bad: $offset");
	}
}

for($y = 0; $y < 8; ++$y){
	try{
		\pocketmine\world\format\io\SubChunkConverter::convertSubChunkFromLegacyColumn($ids, $data, $y);
		var_dump("ok: $y");
	}catch(\InvalidArgumentException $e){
		var_dump($e->getMessage());
	}
}

var_dump("ok");
?>
--EXPECT--
string(7) "bad: -1"
string(25) "bad: -9223372036854775808"
string(24) "bad: 9223372036854775807"
string(6) "bad: 8"
string(6) "bad: 9"
string(5) "ok: 0"
string(5) "ok: 1"
string(5) "ok: 2"
string(5) "ok: 3"
string(5) "ok: 4"
string(5) "ok: 5"
string(5) "ok: 6"
string(5) "ok: 7"
string(2) "ok"