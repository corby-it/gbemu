

#include "Cartridge.h"
#include <algorithm>
#include <unordered_map>


static const std::unordered_map<std::string_view, std::string_view> newLicenseeCodeMap = {
    { "00", "None" },
    { "01", "Nintendo R&D1" },
    { "08", "Capcom" },
    { "13", "Electronic Arts" },
    { "18", "Hudson Soft" },
    { "19", "b-ai" },
    { "20", "kss" },
    { "22", "pow" },
    { "24", "PCM Complete" },
    { "25", "san-x" },
    { "28", "Kemco Japan" },
    { "29", "seta" },
    { "30", "Viacom" },
    { "31", "Nintendo" },
    { "32", "Bandai" },
    { "33", "Ocean/Acclaim" },
    { "34", "Konami" },
    { "35", "Hector" },
    { "37", "Taito" },
    { "38", "Hudson" },
    { "39", "Banpresto" },
    { "41", "Ubi Soft" },
    { "42", "Atlus" },
    { "44", "Malibu" },
    { "46", "angel" },
    { "47", "Bullet-Proof" },
    { "49", "irem" },
    { "50", "Absolute" },
    { "51", "Acclaim" },
    { "52", "Activision" },
    { "53", "American sammy" },
    { "54", "Konami" },
    { "55", "Hi tech entertainment" },
    { "56", "LJN" },
    { "57", "Matchbox" },
    { "58", "Mattel" },
    { "59", "Milton Bradley" },
    { "60", "Titus" },
    { "61", "Virgin" },
    { "64", "LucasArts" },
    { "67", "Ocean" },
    { "69", "Electronic Arts" },
    { "70", "Infogrames" },
    { "71", "Interplay" },
    { "72", "Broderbund" },
    { "73", "sculptured" },
    { "75", "sci" },
    { "78", "THQ" },
    { "79", "Accolade" },
    { "80", "misawa" },
    { "83", "lozc" },
    { "86", "Tokuma Shoten Intermedia" },
    { "87", "Tsukuda Original" },
    { "91", "Chunsoft" },
    { "92", "Video system" },
    { "93", "Ocean/Acclaim" },
    { "95", "Varie" },
    { "96", "Yonezawa/s’pal" },
    { "97", "Kaneko" },
    { "99", "Pack in soft" },
    { "9H", "Bottom Up" },
    { "A4", "Konami (Yu-Gi-Oh!)" },
};


static const std::unordered_map<uint8_t, CartridgeType> cartTypeMap = {
    { 0x00, CartridgeType::NoMBC },
    { 0x01, CartridgeType::MBC1 },
    { 0x02, CartridgeType::MBC1Ram },
    { 0x03, CartridgeType::MBC1RamBattery },
    { 0x05, CartridgeType::MBC2 },
    { 0x06, CartridgeType::MBC2Battery },
    { 0x08, CartridgeType::RomRam },
    { 0x09, CartridgeType::RomRamBattery },
    { 0x0B, CartridgeType::MMM01 },
    { 0x0C, CartridgeType::MMM01Ram },
    { 0x0D, CartridgeType::MMM01RamBattery },
    { 0x0F, CartridgeType::MBC3TimerBattery },
    { 0x10, CartridgeType::MBC3TimerRamBattery },
    { 0x11, CartridgeType::MBC3 },
    { 0x12, CartridgeType::MBC3Ram },
    { 0x13, CartridgeType::MBC3RamBattery },
    { 0x19, CartridgeType::MBC5 },
    { 0x1A, CartridgeType::MBC5Ram },
    { 0x1B, CartridgeType::MBC5RamBattery },
    { 0x1C, CartridgeType::MBC5Rumble },
    { 0x1D, CartridgeType::MBC5RumbleRam },
    { 0x1E, CartridgeType::MBC5RumbleRamBattery },
    { 0x20, CartridgeType::MBC6 },
    { 0x22, CartridgeType::MBC7SensorRumbleRamBattery },
    { 0xFC, CartridgeType::PocketCamera },
    { 0xFD, CartridgeType::BandaiTama5 },
    { 0xFE, CartridgeType::HuC3 },
    { 0xFF, CartridgeType::HuC1RamBattery },
};


static const std::unordered_map<uint8_t, std::string_view> oldLicenseeCodeMap = {
    { 0x00, "None" },
    { 0x01, "Nintendo" },
    { 0x08, "Capcom" },
    { 0x09, "Hot-B" },
    { 0x0A, "Jaleco" },
    { 0x0B, "Coconuts Japan" },
    { 0x0C, "Elite Systems" },
    { 0x13, "EA (Electronic Arts)" },
    { 0x18, "Hudsonsoft" },
    { 0x19, "ITC Entertainment" },
    { 0x1A, "Yanoman" },
    { 0x1D, "Japan Clary" },
    { 0x1F, "Virgin Interactive" },
    { 0x24, "PCM Complete" },
    { 0x25, "San-X" },
    { 0x28, "Kotobuki Systems" },
    { 0x29, "Seta" },
    { 0x30, "Infogrames" },
    { 0x31, "Nintendo" },
    { 0x32, "Bandai" },
    { 0x33, "Refer to the \"New licensee code\"" },
    { 0x34, "Konami" },
    { 0x35, "HectorSoft" },
    { 0x38, "Capcom" },
    { 0x39, "Banpresto" },
    { 0x3C, ".Entertainment i" },
    { 0x3E, "Gremlin" },
    { 0x41, "Ubisoft" },
    { 0x42, "Atlus" },
    { 0x44, "Malibu" },
    { 0x46, "Angel" },
    { 0x47, "Spectrum Holoby" },
    { 0x49, "Irem" },
    { 0x4A, "Virgin Interactive" },
    { 0x4D, "Malibu" },
    { 0x4F, "U.S. Gold" },
    { 0x50, "Absolute" },
    { 0x51, "Acclaim" },
    { 0x52, "Activision" },
    { 0x53, "American Sammy" },
    { 0x54, "GameTek" },
    { 0x55, "Park Place" },
    { 0x56, "LJN" },
    { 0x57, "Matchbox" },
    { 0x59, "Milton Bradley" },
    { 0x5A, "Mindscape" },
    { 0x5B, "Romstar" },
    { 0x5C, "Naxat Soft" },
    { 0x5D, "Tradewest" },
    { 0x60, "Titus" },
    { 0x61, "Virgin Interactive" },
    { 0x67, "Ocean Interactive" },
    { 0x69, "EA (Electronic Arts)" },
    { 0x6E, "Elite Systems" },
    { 0x6F, "Electro Brain" },
    { 0x70, "Infogrames" },
    { 0x71, "Interplay" },
    { 0x72, "Broderbund" },
    { 0x73, "Sculptered Soft" },
    { 0x75, "The Sales Curve" },
    { 0x78, "t.hq" },
    { 0x79, "Accolade" },
    { 0x7A, "Triffix Entertainment" },
    { 0x7C, "Microprose" },
    { 0x7F, "Kemco" },
    { 0x80, "Misawa Entertainment" },
    { 0x83, "Lozc" },
    { 0x86, "Tokuma Shoten Intermedia" },
    { 0x8B, "Bullet-Proof Software" },
    { 0x8C, "Vic Tokai" },
    { 0x8E, "Ape" },
    { 0x8F, "I’Max" },
    { 0x91, "Chunsoft Co." },
    { 0x92, "Video System" },
    { 0x93, "Tsubaraya Productions Co." },
    { 0x95, "Varie Corporation" },
    { 0x96, "Yonezawa/S’Pal" },
    { 0x97, "Kaneko" },
    { 0x99, "Arc" },
    { 0x9A, "Nihon Bussan" },
    { 0x9B, "Tecmo" },
    { 0x9C, "Imagineer" },
    { 0x9D, "Banpresto" },
    { 0x9F, "Nova" },
    { 0xA1, "Hori Electric" },
    { 0xA2, "Bandai" },
    { 0xA4, "Konami" },
    { 0xA6, "Kawada" },
    { 0xA7, "Takara" },
    { 0xA9, "Technos Japan" },
    { 0xAA, "Broderbund" },
    { 0xAC, "Toei Animation" },
    { 0xAD, "Toho" },
    { 0xAF, "Namco" },
    { 0xB0, "acclaim" },
    { 0xB1, "ASCII or Nexsoft" },
    { 0xB2, "Bandai" },
    { 0xB4, "Square Enix" },
    { 0xB6, "HAL Laboratory" },
    { 0xB7, "SNK" },
    { 0xB9, "Pony Canyon" },
    { 0xBA, "Culture Brain" },
    { 0xBB, "Sunsoft" },
    { 0xBD, "Sony Imagesoft" },
    { 0xBF, "Sammy" },
    { 0xC0, "Taito" },
    { 0xC2, "Kemco" },
    { 0xC3, "Squaresoft" },
    { 0xC4, "Tokuma Shoten Intermedia" },
    { 0xC5, "Data East" },
    { 0xC6, "Tonkinhouse" },
    { 0xC8, "Koei" },
    { 0xC9, "UFL" },
    { 0xCA, "Ultra" },
    { 0xCB, "Vap" },
    { 0xCC, "Use Corporation" },
    { 0xCD, "Meldac" },
    { 0xCE, ".Pony Canyon or" },
    { 0xCF, "Angel" },
    { 0xD0, "Taito" },
    { 0xD1, "Sofel" },
    { 0xD2, "Quest" },
    { 0xD3, "Sigma Enterprises" },
    { 0xD4, "ASK Kodansha Co." },
    { 0xD6, "Naxat Soft" },
    { 0xD7, "Copya System" },
    { 0xD9, "Banpresto" },
    { 0xDA, "Tomy" },
    { 0xDB, "LJN" },
    { 0xDD, "NCS" },
    { 0xDE, "Human" },
    { 0xDF, "Altron" },
    { 0xE0, "Jaleco" },
    { 0xE1, "Towa Chiki" },
    { 0xE2, "Yutaka" },
    { 0xE3, "Varie" },
    { 0xE5, "Epcoh" },
    { 0xE7, "Athena" },
    { 0xE8, "Asmik ACE Entertainment" },
    { 0xE9, "Natsume" },
    { 0xEA, "King Records" },
    { 0xEB, "Atlus" },
    { 0xEC, "Epic/Sony Records" },
    { 0xEE, "IGS" },
    { 0xF0, "A Wave" },
    { 0xF3, "Extreme Entertainment" },
    { 0xFF, "LJN" },
};



EntryPointData CartridgeHeader::entryPoint() const
{
    EntryPointData ret;

    if (mRomBaseAddr) 
        memcpy(ret.data(), mRomBaseAddr + 0x100, ret.size());
    else 
        std::fill(ret.begin(), ret.end(), 0);

    return ret;
}

LogoData CartridgeHeader::logoData() const
{
    LogoData ret;

    if (mRomBaseAddr)
        memcpy(ret.data(), mRomBaseAddr + 0x104, ret.size());
    else
        std::fill(ret.begin(), ret.end(), 0);

    return ret;
}

std::string_view CartridgeHeader::title() const
{
    if (!mRomBaseAddr) 
        return {};

    auto start = mRomBaseAddr + 0x134;
    
    auto ret = std::string_view((char*)start, 15);

    // remove unnecessary '\0' at the end
    auto firstNull = ret.find_first_of('\0');
    if (firstNull == std::string_view::npos)
        return ret;

    ret.remove_suffix(15 - firstNull);

    return ret;
}

CGBFlag CartridgeHeader::cgbFlag() const
{
    if (!mRomBaseAddr)
        return CGBFlag::Unknown;

    auto val = mRomBaseAddr[0x143];
    if (val == 0x00)
        return CGBFlag::CGBIncompatible;
    if (val == 0x80)
        return CGBFlag::CGBCompatible;
    else if (val == 0xC0)
        return CGBFlag::CGBOnly;
    else if (val & 0x80 && (val & 0x08 || val & 0x04))
        return CGBFlag::PGBMode;
    else
        return CGBFlag::Unknown;
}

std::string_view CartridgeHeader::newLicenseeCode() const
{
    if (!mRomBaseAddr)
        return { "" };

    auto start = mRomBaseAddr + 0x144;
    std::string_view code((char*)start, 2);

    auto it = newLicenseeCodeMap.find(code);

    if (it != newLicenseeCodeMap.end())
        return it->second;
    else
        return { "" };
}

SGBFlag CartridgeHeader::sgbFlag() const
{
    if (!mRomBaseAddr)
        return SGBFlag::Unknown;

    switch (mRomBaseAddr[0x146]) {
    case 0x00: return SGBFlag::GB;
    case 0x03: return SGBFlag::SGB;
    default:
        return SGBFlag::Unknown;
    }
}

CartridgeType CartridgeHeader::cartType() const
{
    if (!mRomBaseAddr)
        return CartridgeType::Unknown;

    auto it = cartTypeMap.find(mRomBaseAddr[0x147]);

    if (it != cartTypeMap.end())
        return it->second;
    else
        return CartridgeType::Unknown;
}

uint32_t CartridgeHeader::romSize() const
{
    if (!mRomBaseAddr)
        return 0;

    auto val = mRomBaseAddr[0x148];
    return 32 * 1024 * (1 << val);
}

uint32_t CartridgeHeader::ramSize() const
{
    if (!mRomBaseAddr)
        return 0;
    
    switch (mRomBaseAddr[0x149]) {
    case 0x02: return 8 * 1024;
    case 0x03: return 32 * 1024;
    case 0x04: return 128 * 1024;
    case 0x05: return 64 * 1024;
    default:
        return 0;
    }
}

DestCode CartridgeHeader::destCode() const
{
    if (!mRomBaseAddr)
        return DestCode::Unknown;

    switch (mRomBaseAddr[0x14A]) {
    case 0x00: return DestCode::Japan;
    case 0x01: return DestCode::World;
    default:
        return DestCode::Unknown;
    }
}

std::string_view CartridgeHeader::oldLicenseeCode() const
{
    if (!mRomBaseAddr)
        return { "" };

    auto it = oldLicenseeCodeMap.find(mRomBaseAddr[0x14B]);

    if (it != oldLicenseeCodeMap.end())
        return it->second;
    else 
        return { "" };
}

uint8_t CartridgeHeader::maskRomVersionNum() const
{
    if (!mRomBaseAddr)
        return 0;

    return mRomBaseAddr[0x14C];
}

uint8_t CartridgeHeader::headerChecksum() const
{
    if (!mRomBaseAddr)
        return 0;

    return mRomBaseAddr[0x14D];
}

uint16_t CartridgeHeader::globalChecksum() const
{
    if (!mRomBaseAddr)
        return 0;

    // this global checksum is stored in big endian
    auto lo = mRomBaseAddr[0x14F];
    auto hi = mRomBaseAddr[0x14E];
    
    return hi << 8 | lo;
}


bool CartridgeHeader::verifyHeaderChecksum() const
{
    if (!mRomBaseAddr)
        return false;

    // algorithm from https://gbdev.io/pandocs/The_Cartridge_Header.html

    uint8_t checksum = 0;
    for (uint16_t addr = 0x0134; addr <= 0x014C; ++addr) {
        checksum = checksum - mRomBaseAddr[addr] - 1;
    }

    return headerChecksum() == checksum;
}

bool CartridgeHeader::verifyGlobalChecksum() const
{
    if (!mRomBaseAddr)
        return false;

    // algorithm from https://gbdev.io/pandocs/The_Cartridge_Header.html
    uint16_t sum = 0;
    for (uint16_t addr = 0x100; addr < 0x14D; ++addr) {
        sum += mRomBaseAddr[addr];
    }

    return globalChecksum() == sum;
}
