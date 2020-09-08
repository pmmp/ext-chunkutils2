--TEST--
Palette offsets getting incorrectly overwritten on full palettes
--SKIPIF--
<?php if(!extension_loaded("chunkutils2")) die("skip extension not loaded"); ?>
--FILE--
<?php

$p = new \pocketmine\world\format\PalettedBlockArray(9998);

for($i = 0; $i < 4096; ++$i){
	$p->set($i & 0xf, ($i >> 4) & 0xf, ($i >> 8) & 0xf, $i);
}

$p000 = $p->get(0, 0, 0);
var_dump($p000);
//create an unused entry and point 0,0,1 to the same offset as 0,0,0
$p->set(0, 0, 1, 0);
var_dump($p->get(0, 0, 0));
var_dump($p->get(0, 0, 1));

$p->set(0, 0, 1, 9999); //this wasn't in the palette before, so it trashes a previous entry
var_dump($p->get(0, 0, 1));
var_dump($p->get(0, 0, 0) === $p000); //fidelity is broken
?>
--EXPECT--
int(0)
int(0)
int(0)
int(9999)
bool(true)
