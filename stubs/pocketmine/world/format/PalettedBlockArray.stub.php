<?php

/**
 * @generate-function-entries
 */

namespace pocketmine\world\format;

final class PalettedBlockArray{

	public function __construct(int $fillEntry){}

	/**
	 * @param int[] $palette
	 * @throws PalettedBlockArrayLoadException if the provided data is invalid in any way
	 */
	public static function fromData(int $bitsPerBlock, string $wordArray, array $palette) : \pocketmine\world\format\PalettedBlockArray{}

	public function getWordArray() : string{}

	/**
	 * @return int[]
	 */
	public function getPalette() : array{}

	/**
	 * The input array must be the same size as the current palette
	 * Keys are ignored. Only input order is considered.
	 * @param int[] $palette
	 */
	public function setPalette(array $palette) : void{}

	public function getMaxPaletteSize() : int{}

	public function getBitsPerBlock() : int{}

	public function get(int $x, int $y, int $z) : int{}

	public function set(int $x, int $y, int $z, int $val) : void{}

	public function replaceAll(int $oldVal, int $newVal) : void{}

	public function collectGarbage(bool $force = false) : void{}

	public static function getExpectedWordArraySize(int $bitsPerBlock) : int{}
}
