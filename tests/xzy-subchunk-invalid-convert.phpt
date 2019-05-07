--TEST--
Test handling of bad data in SubChunkConverter::convertSubChunkXZY()
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$sizes = [
	[0, 0],
	[1, 1],
	[4096, 4096],
	[2048, 2048],
	[4097, 2048],
	[4096, 2049]
];

foreach($sizes as [$idSize, $dataSize]){
	$ids = str_repeat("\x00", $idSize);
	$data = str_repeat("\x00", $dataSize);
	try{
		\pocketmine\world\format\io\SubChunkConverter::convertSubChunkXZY($ids, $data);
	}catch(\LengthException $e){
		var_dump($e->getMessage());
	}
}

var_dump("ok");
?>
--EXPECT--
string(32) "Invalid data sizes (got 0 and 0)"
string(32) "Invalid data sizes (got 1 and 1)"
string(38) "Invalid data sizes (got 4096 and 4096)"
string(38) "Invalid data sizes (got 2048 and 2048)"
string(38) "Invalid data sizes (got 4097 and 2048)"
string(38) "Invalid data sizes (got 4096 and 2049)"
string(2) "ok"