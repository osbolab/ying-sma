#pragma once

#include <utility>

namespace sma
{
//! A scoped lock on a mutable buffer entry.
/*! The next writable sequence number is claimed, but not assigned to the
 * entry until the writer is released.
 * Readers will be blocked on that entry because its sequence is lower than
 * theirs, and writers will be given a subsequent sequence number because
 * it's atomically incremented when assigned.
 */
template <typename T, typename E = typename T::entry_type>
struct WriteLock {
private:
  using Myt = WriteLock<T, E>;
  E& entry;

public:
  using data_type = decltype(std::declval<E>().data);
  using size_type = typename E::size_type;

  constexpr size_type capacity() { return E::capacity; }

  data_type& data;
  size_type& size;


  WriteLock(T& buf)
    : entry(buf.acquire())
    , data(entry.data)
    , size(entry.size)
  {
  }

  WriteLock(Myt const&) = delete;
  Myt& operator=(Myt const&) = delete;

  //! Release the buffer entry for reading.
  /*! This is guaranteed safe without locking because the sequence number
   * was atomically incremented when taken and readers are blocked on a cheap
   * ordered read until this commit takes place.
   */
  ~WriteLock() { entry.writing.clear(); }
};

//! A scoped lock of an immutable buffer entry.
/*! Unlimited readers may immutably reference an entry and it will not be
 * modified until they are all destructed.
 * This blocks writing, and since the ring buffer is neither destructive nor
 * dynamic may cause an overrun exception.
 */
template <typename T, typename E = typename T::entry_type>
struct ReadLock {
private:
  using Myt = ReadLock<T, E>;
  E* entry;

public:
  using const_data_pointer = typename E::value_type const*;
  using size_type = typename E::size_type;

  constexpr size_type capacity() { return E::capacity; }

  ReadLock(T& buf)
    : entry(buf.try_pop())
  {
  }

  ReadLock(Myt const&) = delete;
  Myt& operator=(Myt const&) = delete;

  //! Release a read lock on the entry.
  ~ReadLock()
  {
    if (entry)
      --entry->readers;
  }

  bool acquired() const { return entry != nullptr; }

  const_data_pointer cdata() const { return entry->data; }
  size_type size() const { return entry->size; }
};
}
