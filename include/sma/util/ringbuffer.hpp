#pragma once

#include <mutex>
#include <atomic>

#include <cstdint>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <type_traits>


namespace sma
{
template <typename T>
class RingBuffer
{
  static_assert(std::is_default_constructible<T>::value,
                "Ring buffer element type must be default constructible for"
                " preallocation.");

  //! The buffered type.
  using value_type = T;
  //! A pointer to the buffered type.
  using pointer = T*;
  //! A reference to the buffered type.
  using reference = T&;
  //! An immutable reference to the buffered type.
  using const_reference = T const&;
  //! An unsigned type that can index all elements in the buffer.
  using size_type = std::size_t;
  //! Monotonically increasing sequence counter.
  using mono_type = std::uint64_t;
  //! Read/write ordered monotonic sequence number.
  using sequence = std::atomic<mono_type>;
  //! Produces the real array index when ANDed with a monotonic sequence.
  mono_type idx_mask;


  //! A read/write synchronized entry in the buffer containing the value type.
  struct Entry {
    Entry() = default;

    Entry(Entry const&) = delete;
    Entry& operator=(Entry const&) = delete;

    //! The value type is value-initialized at allocation and modified in place.
    value_type t;
    //! The entry's sequence number indicates the freshness of its contents.
    /*! Readers will not read an entry with an older sequence number than their
     * own; if the next entry has a lower sequence then the buffer is empty.
     */
    sequence seq;
    //! The number of readers holding locks on the entry.
    /*! Writers will not claim an entry with readers. If the next entry to be
     * written is read-locked then the buffer is overrun.
     */
    std::atomic_size_t readers;
    //! \a set if a writer has claimed the entry.
    /*! No other reader or writer will proceed to claim the entry while set.
     */
    std::atomic_flag writing = ATOMIC_FLAG_INIT;
  };

public:
  // clang-format off
  //! Claim a buffer entry for writing and release it on destruction.
  /*! The next writable sequence number is claimed, but not assigned to the
   * entry until the writer is released.
   * Readers will be blocked on that entry because its sequence is lower than
   * theirs, and writers will be given a subsequent sequence number because
   * it's atomically incremented when assigned.
   */
  struct WriteLock {
    friend class RingBuffer<T>;

    WriteLock(WriteLock&& r)
      : entry(r.entry)
    { r.entry = nullptr; }

    WriteLock& operator=(WriteLock&& r)
    {
      entry = r.entry;
      r.entry = nullptr;
      return *this;
    }

    //! Release the buffer entry by setting its sequence number.
    /*! This is guaranteed safe without locking because the sequence number
     * was atomically incremented when taken and readers are blocked on a cheap
     * ordered read until this commit takes place.
     */
    ~WriteLock()
    { if (entry) entry->writing.clear(); }

    //! Get a reference to the claimed buffer entry.
    /*! Dereferencing this reference outside the lifetime of the current object
     * is undefined behavior; mutating that value is strictly illegal.
     */
    reference operator*()
    { assert(entry); return entry->t; }

    //! Dereference the claimed buffer entry.
    reference operator->()
    { assert(entry); return entry->t; }

  private:
    WriteLock(Entry& entry)
      : entry(&entry)
    { }

    //! Claimed entry at which readers are blocked while this object is live.
    Entry* entry;
  };

  //! Lock the referenced entry against writing for the lifetime of this object.
  /*! Unlimited readers may immutably reference an entry and it will not be
   * modified until they are all destructed.
   * This blocks writing, and since the ring buffer is neither destructive nor
   * dynamic may cause an overrun exception.
   */
  struct ReadLock {
    friend class RingBuffer<T>;

    //! Transfer the read lock on the entry without incrementing it.
    ReadLock(ReadLock&& r)
      : entry(r.entry)
      { r.entry = nullptr; }

    //! Take an additional read lock on the entry.
    ReadLock(ReadLock const& r)
      : entry(r.entry)
    { ++entry->readers; }

    //! Transfer the read lock on the entry without incrementing it.
    ReadLock& operator=(ReadLock&& r)
    {
      entry = r.entry;
      r.entry = nullptr;
      return *this;
    }

    //! Take an additional read lock on the entry.
    ReadLock& operator=(ReadLock const& r)
    {
      entry = r.entry;
      ++entry->readers;
      return *this;
    }

    //! Release a read lock on the entry.
    /*! The entry is not unlocked until all readers have been destroyed. */
    ~ReadLock()
    { if (entry) --entry->readers; }

    //! Get an immutable reference to the buffer entry.
    /*! Dereferencing this reference outside the lifetime of the current object
     * is undefined behavior.
     */
    T const& operator*() const
    { assert(entry); return entry->t; }

    //! Dereference the buffer entry.
    T const& operator->() const
    { assert(entry); return entry->t; }

  private:
    //! The default reader represents no entry.
    /*! This may be returned by a nonblocking read when the buffer is empty.
     * Dereferencing the default reader is undefined.
     */
    ReadLock() = default;
    ReadLock(Entry& entry)
      : entry(&entry)
    { }

    Entry* entry{nullptr};
  };
  // clang-format on


  //! Construct a new buffer, default-value-initializing all of its elements.
  RingBuffer(size_type size)
    : entries(new Entry[size]())
    , idx_mask(size - 1)
  {
    assert(size != 0 && ((size & (size - 1)) == 0));
  }

  //! Delete the buffered elements without regard for open readers or writers.
  ~RingBuffer() { delete[] entries; }

  //! Get the number of unread elements in the buffer.
  size_type size() { return next_write - next_read - 1; }

  //! Claim the next available buffer element for writing for the lifetime of
  //! the returned object.
  /*! This is equivalent to lazily emplacing an element in a queue such that it
   * is guaranteed to be read before subsequently emplaced elements.
   * Consequently, while further writes may take place concurrently, all reads
   * will be blocked at this entry until it has committed.
   */
  WriteLock claim()
  {
    // Try until we find a slot not being written to.
    // This may occur if every slot is being written to, in which case we busy
    // loop waiting for one to open.
    mono_type seq;
    Entry* entry;
    do {
      // Claim the next sequence number and the according buffer slot.
      seq = next_write++;
      // The sequence numbers outrun the buffer indices by one because the
      // readers need to start behind the first writer.
      entry = entries + ((seq - 1) & idx_mask);
      // If another writer set the flag then find a new slot.
    } while (entry->writing.test_and_set());
    // We have set the writing flag

    if (entry->readers != 0)
      throw std::runtime_error(
          "Ring buffer overrun (the next slot to write is being read).");

    return WriteLock(*entry);
  }

  std::pair<ReadLock, bool> try_pop()
  {
    // Pretend to claim the next read position and, if we're successful,
    // CAS until we actually commit it.
    mono_type seq = next_read;
    Entry* entry;
    while (true) {
      entry = entries + (seq & idx_mask);
      ++entry->readers;
      // Writers are now blocked from claiming it, but one may have it already.
      if (entry->writing.test_and_set())
        return std::make_pair(ReadLock(), false);

      entry->writing.clear();

      // Since we just pretended to increment the read position (in case we
      // failed), someone else might have succeeded while we tried.
      if (next_read.compare_exchange_weak(seq, seq + 1))
        break;
      // We failed the CAS; remove ourselves from the entry we chose and retry.
      --entry->readers;
    }

    return std::make_pair(ReadLock(*entry), true);
  }

private:
  Entry* entries;
  //! The next sequence number to be written.
  sequence next_write{1};
  //! The low-water mark for readers.
  /*! Readers will not enter an entry with a sequence number <= this value. */
  sequence next_read{0};
};
}
