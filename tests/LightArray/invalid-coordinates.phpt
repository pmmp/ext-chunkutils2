--TEST--
Test that LightArray handles invalid coordinates appropriately
--DESCRIPTION--
We don't want to throw exceptions in hot paths, so we truncate invalid coordinates to 4 bits.
This intentionally produces overflow behaviour, as the least impactful way to avoid memory corruption issues.
--EXTENSIONS--
chunkutils2
--FILE--
<?php

namespace pocketmine\world\format;

$p = LightArray::fill(0);

$p->set(0, 0, 0, 15);

var_dump($p->get(0, 0, 0));
var_dump($p->get(16, 16, 16));
var_dump($p->get(32, 32, 32));

$p->set(16, 16, 16, 8);
var_dump($p->get(0, 0, 0));

?>
--EXPECT--
int(15)
int(15)
int(15)
int(8)