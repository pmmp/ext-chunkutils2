<?php

/**
 * @generate-function-entries
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
