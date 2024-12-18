--TEST--
Test that PalettedBlockArray::setPalette() works as expected
--FILE--
<?php

use pocketmine\world\format\PalettedBlockArray;

$p1 = new PalettedBlockArray(1);
$p1->set(0, 0, 0, 2); //now we have 2 entries

echo "replace palette\n";
$p1->setPalette([3, 4]);
var_dump($p1->get(0, 0, 0)); //4
var_dump($p1->get(0, 0, 1)); //3

try{
    echo "empty not accepted\n";
    $p1->setPalette([]);
}catch(\ValueError $e){
    echo $e->getMessage() . "\n";
}
try{
    echo "too many entries not accepted\n";
    $p1->setPalette([1, 2, 3]);
}catch(\ValueError $e){
    echo $e->getMessage() . "\n";
}
$p1->set(0, 0, 2, 5);
echo "now 3 entries should be accepted:\n";
//now 3 entries should be accepted:
$p1->setPalette([3, 4, 5]);
var_dump($p1->get(0, 0, 2)); //5


echo "special test for 0 bpb\n";
$p2 = new PalettedBlockArray(1);
$p2->setPalette([2]);
var_dump($p2->get(0, 0, 0)); //2

try{
    echo "empty not accepted\n";
    $p2->setPalette([]);
}catch(\ValueError $e){
    echo $e->getMessage() . "\n";
}
try{
    echo "too many entries not accepted\n";
    $p2->setPalette([1, 2, 3]);
}catch(\ValueError $e){
    echo $e->getMessage() . "\n";
}
?>
--EXPECT--
replace palette
int(4)
int(3)
empty not accepted
new palette must be the same size as the old one, expected 2 but received 0
too many entries not accepted
new palette must be the same size as the old one, expected 2 but received 3
now 3 entries should be accepted:
int(5)
special test for 0 bpb
int(2)
empty not accepted
new palette must be the same size as the old one, expected 1 but received 0
too many entries not accepted
new palette must be the same size as the old one, expected 1 but received 3
