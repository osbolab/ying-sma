#pragma once

#include <atomic>
#include <mutex>

#include <cstdint>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <type_traits>

namespace sma
{
template <typename T, std::size_t Size>
class RingBuffer
{
  static_assert(std::is_default_constructible<T>::value,
                "Ring buffer element type must be default constructible for"
                " preallocation.");
  static_assert(Size != 0, "Ring buffer size must be nonzero.");
  static_assert((Size & (Size - 1)) == 0,
                "Ring buffer size must be a power of two.");

  using BufT = RingBuffer<T, Size>;
  using mono_type = std::uint64_t;
  using size_type = decltype(Size);
  static constexpr size_type idx_mask = Size - 1;


  struct Slot {
    Slot() = default;

    Slot(Slot const&) = delete;
    Slot& operator=(Slot const&) = delete;

    T t;
    bool reading = false;
  };

public:
  // clang-format off
  struct SlotWriter {
    friend class RingBuffer<T, Size>;

    SlotWriter(SlotWriter&& r)
      : slot(r.slot), slots(r.slots)
    {
      r.slot = nullptr;
      r.slots = nullptr;
    }
    SlotWriter& operator=(SlotWriter&& r)
    {
      slot = r.slot;
      slots = r.slots;
      r.slot = nullptr;
      r.slots = nullptr;
      return *this;
    }

    ~SlotWriter()
    { if (slots) slots->releasew(); }

    T& operator*()
    { assert(slot); return slot->t; }

    T& operator->()
    { assert(slot); return slot->t; }

  private:
    SlotWriter(BufT& slots, Slot& slot)
      : slots(&slots), slot(&slot) {}

    Slot* slot;
    BufT* slots;
  };

  struct SlotReader {
    friend class RingBuffer<T, Size>;

    SlotReader(SlotReader&& r)
      : slot(r.slot)
      { r.slot = nullptr; }

    SlotReader& operator=(SlotReader&& r)
    {
      slot = r.slot;
      r.slot = nullptr;
      return *this;
    }

    ~SlotReader()
    { if (slot) slot->reading = false; }

    T const& operator*() const
    { assert(slot); return slot->t; }

    T const& operator->() const
    { assert(slot); return slot->t; }

  private:
    SlotReader() = default;
    SlotReader(Slot& slot)
      : slot(&slot)
    {
      assert(!slot.reading);
      slot.reading = true;
    }

    Slot* slot{nullptr};
  };
  // clang-format on

  friend struct SlotWriter;


  RingBuffer()
    : slots(new Slot[Size]())
    , r(-1)
  {
  }

  ~RingBuffer() { delete[] slots; }

  size_type size() { return w - r; }

  SlotWriter claim()
  {
    size_type iw = w & idx_mask;
    // Since the reader peeks at W before incrementing, this can only happen
    // if the previous write incremented W to match R and the reader still
    // hasn't cleared anything.
    if (iw == r)
      throw std::runtime_error("Writer overran reader in ring buffer.");

    return SlotWriter(*this, slots[iw]);
  }

  std::pair<bool, SlotReader> try_pop()
  {
    mono_type rprev = r;
    mono_type rnew;
    do {
      // Don't commit the advance if it would underrun the writer.
      rnew = rprev + 1;
      if ((rnew & idx_mask) == (w & idx_mask))
        return std::make_pair(false, SlotReader());
      // Don't commit if another reader advanced out from under us
    } while (!r.compare_exchange_weak(rprev, rnew));

    // Any other reader to make it to this point will have a different value
    // for rnew.
    return std::make_pair(true, SlotReader(slots[rnew & idx_mask]));
  }

private:
  void releasew()
  {
    ++w;
  }

  Slot* slots;
  // The next write position is the current write index.
  std::atomic<mono_type> w{0};
  // The next read position is the next index if the write pointer has advanced.
  // Start with the read pointer waiting for index 0.
  std::atomic<mono_type> r;
};
}
