--TEST--
Test that PalettedBlockArray works correctly after serialize/unserialize
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$p1 = new \pocketmine\level\format\PalettedBlockArray(1);
var_dump($p1->get(0, 0, 0));

$p2 = unserialize(serialize($p1));
$p2->set(0, 0, 0, 2);
var_dump($p1->get(0, 0, 0));
var_dump($p2->get(0, 0, 0));
var_dump("ok");
?>
--EXPECT--
int(1)
int(1)
int(2)
string(2) "ok"
