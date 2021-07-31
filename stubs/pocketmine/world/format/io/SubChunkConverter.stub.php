<?php

namespace pocketmine\world\format\io;

final class SubChunkConverter{

	public static function convertSubChunkXZY(string $idArray, string $metaArray) : \pocketmine\world\format\PalettedBlockArray{}

	public static function convertSubChunkYZX(string $idArray, string $metaArray) : \pocketmine\world\format\PalettedBlockArray{}

	public static function convertSubChunkFromLegacyColumn(string $idArray, string $metaArray, int $yOffset) : \pocketmine\world\format\PalettedBlockArray{}

	private function __construct(){}
}