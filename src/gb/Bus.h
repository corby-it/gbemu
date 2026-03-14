

#ifndef GBEMU_SRC_GB_BUS_H_
#define GBEMU_SRC_GB_BUS_H_

#include "Ram.h"
#include "GbCommons.h"


// the GB has a 16-bit address bus that connects the CPU to everything else.
// Everything is memory mapped on the same bus


// ------------------------------------------------------------------------------------------------
// Bus
// ------------------------------------------------------------------------------------------------

enum class BusEvent {
    CpuExecHalt,
    CpuResumesFromHalt,
    HdmaStarted,
    HdmaStopped,
};


template <size_t Size>
class BusEventQueue {

    static_assert((Size > 0) && ((Size & (Size - 1)) == 0), "Size must be a power of 2!");


public:
    BusEventQueue()
        : mData{}
        , mWHead(0)
        , mRHead(0)
        , mCount(0)
    {}

    void push(BusEvent evt) {
        mData[mWHead] = evt;
        inc(mWHead);

        if (mCount < Size)
            mCount++;

        // if after incrementing the write head it's the same as
        // the read head it means the queue is full and we just lost the oldest element
        if (mWHead == mRHead)
            inc(mRHead);
    }

    void pop() {
        inc(mRHead);
        if (mCount > 0)
            mCount--;
    }

    BusEvent front() {
        return mData[mRHead];
    }

    uint32_t size() const { return mCount; }
    bool empty() const { return mCount == 0; }

private:
    void inc(uint32_t& head) {
        head = (head + 1) & (Size - 1);
    }

    std::array<BusEvent, Size> mData;

    uint32_t mWHead;
    uint32_t mRHead;
    uint32_t mCount;

    // mWHead is the place where a new item will be pushed
    // mRHead is the place on which front() and pop() will work on
    // if mWHead and mRHead are the same the queue is empty
};


class Bus : public ReadWriteIf {
public:
    virtual ~Bus() {}

    uint16_t read16(uint16_t addr) const;
    
    void write16(uint16_t addr, uint16_t val);


    void sendEvent(BusEvent evt) { mEvtQueue.push(evt); }

    
    BusEventQueue<16> mEvtQueue;


};



// ------------------------------------------------------------------------------------------------
// TestBus
// ------------------------------------------------------------------------------------------------

// the TestBus class is used for testing, it doesn't map the addresses to actual peripherals,
// it only has a 64kB ram where things can be read from and written to freely
class TestBus : public Bus {
public:

    uint8_t read8(uint16_t addr) const override {
        return mWram.read8(addr);
    }

    void write8(uint16_t addr, uint8_t val) override {
        mWram.write8(addr, val);
    }
    
private:
    Ram<64_KB> mWram;

};




#endif // GBEMU_SRC_GB_BUS_H_