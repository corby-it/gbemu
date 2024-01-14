
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
    bool LoadTextureFromTile(const TileData tile, GLuint* out_texture);

    uint8_t mData[TileData::size];
    TileData mTile;

};


#endif // GBEMU_SRC_UI_APP_H_