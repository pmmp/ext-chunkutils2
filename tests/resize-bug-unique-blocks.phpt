--TEST--
Check that fast upwards resizing works as expected when there are unused palette entries
--SKIPIF--
<?php
if(!extension_loaded("chunkutils2")) die("skip extension not loaded");
?>
--FILE--
<?php

$p1 = new \pocketmine\world\format\PalettedBlockArray(0);
for($i = 0; $i < 4096; ++$i){ //this triggers resizes all the way up to max capacity
	$p1->set($i & 0xf, ($i >> 4) & 0xf, ($i >> 8) & 0xf, 1);
}
$p1->set(0, 0, 0, 2);
if(($actual = $p1->get(0, 0, 0)) !== 2){
	die("mismatch: expected 2, got $actual");
}
var_dump("good");
?>
--EXPECT--
string(4) "good"
