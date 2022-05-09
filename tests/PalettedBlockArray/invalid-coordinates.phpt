--TEST--
Test PalettedBlockArray::get() and set() overflow behaviour on invalid coordinates
--DESCRIPTION--
To avoid throwing costly exceptions in hot paths, we truncate invalid coordinates to 4 bits, so they will overflow.
We can't do nothing about it because it would allow invalid coordinates to cause memory corruption.
This is the least impactful solution.
--EXTENSIONS--
chunkutils2
--FILE--
<?php

namespace pocketmine\world\format;

$p = new PalettedBlockArray(1);
$p->set(0, 0, 0, 2);

var_dump($p->get(0, 0, 0));
var_dump($p->get(16, 16, 16));
var_dump($p->get(32, 32, 32));

$p->set(16, 16, 16, 3);
var_dump($p->get(0, 0, 0));
?>
--EXPECT--
int(2)
int(2)
int(2)
int(3)