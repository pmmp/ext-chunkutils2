--TEST--
Test handling of bad data in SubChunkConverter::convertSubChunkFromLegacyColumn()
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
	[4096, 2049],
	[32769, 16384],
	[32768, 16383]
];

foreach($sizes as [$idSize, $dataSize]){
	$ids = str_repeat("\x00", $idSize);
	$data = str_repeat("\x00", $dataSize);
	try{
		\pocketmine\world\format\io\SubChunkConverter::convertSubChunkFromLegacyColumn($ids, $data, 0);
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
string(40) "Invalid data sizes (got 32769 and 16384)"
string(40) "Invalid data sizes (got 32768 and 16383)"
string(2) "ok"