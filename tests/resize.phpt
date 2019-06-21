--TEST--
Check that fast upwards resizing works as expected
--SKIPIF--
<?php
if(!extension_loaded("chunkutils2")) die("skip extension not loaded");
?>
--FILE--
<?php

$p1 = new \pocketmine\world\format\PalettedBlockArray(1);
for($i = 0; $i < 4096; ++$i){ //this triggers resizes all the way up to max capacity
	$p1->set($i & 0xf, ($i >> 4) & 0xf, ($i >> 8) & 0xf, $i);
}

for($i = 0; $i < 4096; ++$i){ //verify
	if(($actual = $p1->get($i & 0xf, ($i >> 4) & 0xf, ($i >> 8) & 0xf)) !== $i){
		die("mismatch: expected $i, got $actual");
	}
}
var_dump("good");
?>
--EXPECT--
string(4) "good"