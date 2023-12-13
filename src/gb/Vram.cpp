

#include "Vram.h"
#include <cassert>


TileData VRam::getObjTile(uint8_t id, bool doubleHeight) const
{
	// the data for a tile is 16 bytes long, each tile is an 8x8 or 8x16 bitmap
	// where each pixel is 2 bits deep (8x8x2 = 128 bits = 16 bytes or 8x16x2 = 256 bits = 32 bytes)
	// each tile is identified by an id between 0 and 255
	// OBJ tiles are all located between 0x8000 and 0x8FFF
	// the address of a specific tile is 0x8000 + (id * 16)

	// if we're using double height mode, an odd id is rounded down to the previous even id
	// (hence the bitwise AND with 0xFE)
	
	size_t len;
	uint16_t addr;

	if (doubleHeight) {
		len = 32;
		addr = mStartAddr + ((id & 0xFE) * 16);
	}
	else {
		len = 16;
		addr = mStartAddr + (id * 16);
	}

	return TileData(addr, getPtr(addr), len);
}

TileData VRam::getBgTile(uint8_t id, bool hiMemArea, bool /*doubleHeight*/) const
{
	// the location in VRAM of a background tile is determined by its id and the addressing
	// mode (bit 4 of the LCDC register):
	// - when the bit is 0 the location is 0x8000 + (id * 16), the range is 0x8000 - 0x8FFF
	// - when the bit is 1 the location is 0x9000 + ((int8)id * 16), the range is 0x8800 - 0x97FF 

	// background and window tiles can't be 8x16, only 8x8

	uint16_t addr;

	if (hiMemArea) 
		addr = (mStartAddr + 0x1000) + ((int8_t)id * 16);
	else 
		addr = mStartAddr + id * 16;

	return TileData(addr, getPtr(addr), 16);
}

TileMap VRam::getTileMap(bool hi) const
{
	// there are two 32x32 tile maps in VRAM, one between 0x9800 and 0x9BFF
	// and one between 0x9C00 and 0x9FFF

	// which map is selected depends on LCDC bit 3

	// each one contains the IDs of the tiles used to compose the background
	// or the window
	// each map contains the IDs starting from the top left of the map

	uint16_t addr = hi ? 0x9C00 : 0x9800;

	return TileMap(addr, getPtr(addr));
}






OAMData OAMRam::getOAMData(uint8_t id) const
{
	// OAM data is stored between 0xFE00 and 0xFEA0 (160 bytes in total),
	// each OAM record is 4-bytes long so we 40 OAM record that can be 
	// stored simultaneously

	assert(id < 40);

	uint16_t addr = mStartAddr + (id * OAMData::size);

	return OAMData(addr, getPtr(addr));
}
