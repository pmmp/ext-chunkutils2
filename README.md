# ext-chunkutils2

[![Build Status](https://travis-ci.com/pmmp/ext-chunkutils2.svg?branch=master)](https://travis-ci.com/pmmp/ext-chunkutils2)

PHP extension prototype written in C++ implementing the new MCPE 1.2.13 subchunk format

## What is this?
This extension implements some performance-sensitive components of PocketMine-MP's internal chunk handling system in C++ for better performance and lower memory usage.

## What's in the extension?
At the time of writing:

- `\pocketmine\world\format\PalettedBlockArray`: This class implements paletted block-storages as per modern MCPE since 1.2.13.
- `\pocketmine\world\format\io\SubChunkConverter`: This class contains a series of helper methods for upgrading legacy world terrain.

## What's in the folders?
- `gsl`: Subtree merge of https://github.com/microsoft/GSL
- `lib`: Library code implementing various chunk components. The code in here is unfettered by PHP and can be used on its own.
- `src`: Binding code that glues together PHP and the C++ chunkutils2 components.
- `tests`: `.phpt` tests for the extension which can be run with PHP's `run-tests.php` tool (or `make test` when using PHP's build system)
