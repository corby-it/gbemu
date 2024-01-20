
#ifndef GBEMU_SRC_UI_APP_H_
#define GBEMU_SRC_UI_APP_H_


#include "AppBase.h"
#include "gb/Vram.h"


class App : public AppBase {
public:
    App();
    virtual ~App() {}

    void startup() override {}
    void update() override;


private:

    uint8_t mTileData[TileData::size];
    TileData mTile;

    uint8_t mTileMapData[TileMap::size];
    TileMap mMap;

    RgbBuffer mTileRgbBuffer;
    RgbBuffer mMapRgbBuffer;

};


#endif // GBEMU_SRC_UI_APP_H_