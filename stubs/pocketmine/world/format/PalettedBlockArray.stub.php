<?php

/**
 * @generate-function-entries
 */

namespace pocketmine\world\format;

final class PalettedBlockArray{

	public function __construct(int $fillEntry){}

	/**
	 * @param int[] $palette
	 */
	public static function fromData(int $bitsPerBlock, string $wordArray, array $palette) : \pocketmine\world\format\PalettedBlockArray{}

	public function getWordArray() : string{}

	/**
	 * @return int[]
	 */
	public function getPalette() : array{}

	public function getMaxPaletteSize() : int{}

	public function getBitsPerBlock() : int{}

	public function get(int $x, int $y, int $z) : int{}

	public function set(int $x, int $y, int $z, int $val) : void{}

	public function replaceAll(int $oldVal, int $newVal) : void{}

	public function collectGarbage(bool $force = false) : void{}

	public static function getExpectedWordArraySize(int $bitsPerBlock) : int{}
}
