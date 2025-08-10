
#include "gb/GameBoyCore.h"
#include <nanobench.h>
#include <doctest/doctest.h>

//
//TEST_CASE("Benchmark read/write") {
//
//    GameBoyClassic gb;
//    uint16_t addr = 0x00;
//
//    ankerl::nanobench::Bench b;
//    b.title("Read/write access")
//        .unit("uint64_t")
//        .warmup(100)
//        .relative(true);
//    b.performanceCounters(true);
//
//    b.run("if-else", [&]() {
//        auto val = gb.read8(addr);
//        ankerl::nanobench::doNotOptimizeAway(val);
//    });
//
//
//}