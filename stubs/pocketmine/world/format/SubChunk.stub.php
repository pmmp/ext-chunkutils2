<?php

/*
 *
 *  ____            _        _   __  __ _                  __  __ ____
 * |  _ \ ___   ___| | _____| |_|  \/  (_)_ __   ___      |  \/  |  _ \
 * | |_) / _ \ / __| |/ / _ \ __| |\/| | | '_ \ / _ \_____| |\/| | |_) |
 * |  __/ (_) | (__|   <  __/ |_| |  | | | | | |  __/_____| |  | |  __/
 * |_|   \___/ \___|_|\_\___|\__|_|  |_|_|_| |_|\___|     |_|  |_|_|
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * @author PocketMine Team
 * @link http://www.pocketmine.net/
 *
 *
*/

namespace pocketmine\world\format;

class SubChunk{
	/**
	 * SubChunk constructor.
	 *
	 * @param PalettedBlockArray[] $blocks
	 */
	public function __construct(int $emptyBlockId, array $blocks, ?LightArray $skyLight = null, ?LightArray $blockLight = null){}

	/**
	 * Returns whether this subchunk contains any non-air blocks.
	 * This function will do a slow check, usually by garbage collecting first.
	 * This is typically useful for disk saving.
	 */
	public function isEmptyAuthoritative() : bool{}

	/**
	 * Returns a non-authoritative bool to indicate whether the chunk contains any blocks.
	 * This may report non-empty erroneously if the chunk has been modified and not garbage-collected.
	 */
	public function isEmptyFast() : bool{}

	/**
	 * Returns the block used as the default. This is assumed to refer to air.
	 * If all the blocks in a subchunk layer are equal to this block, the layer is assumed to be empty.
	 */
	public function getEmptyBlockId() : int{}

	public function getFullBlock(int $x, int $y, int $z) : int{}

	public function setFullBlock(int $x, int $y, int $z, int $block) : void{}

	/**
	 * @return PalettedBlockArray[]
	 */
	public function getBlockLayers() : array{}

	public function getHighestBlockAt(int $x, int $z) : ?int{}

	public function getBlockSkyLightArray() : LightArray{}

	public function setBlockSkyLightArray(LightArray $data) : void{}

	public function getBlockLightArray() : LightArray{}

	public function setBlockLightArray(LightArray $data) : void{}

	public function collectGarbage() : void{}
}
