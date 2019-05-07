--TEST--
Test that PalettedBlockArray works correctly after cloning
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$p1 = new \pocketmine\world\format\PalettedBlockArray(1);
var_dump($p1->get(0, 0, 0));
$p1->test = "hi";

$p2 = clone $p1;
var_dump($p2->test === $p1->test);
$p2->set(0, 0, 0, 2);
var_dump($p1->get(0, 0, 0));
var_dump($p2->get(0, 0, 0));
var_dump("ok");
?>
--EXPECT--
int(1)
bool(true)
int(1)
int(2)
string(2) "ok"
