#pragma once

#include <sma/util/hash.hpp>
#include <sma/ccn/contentstore.hpp>
#include <sma/ccn/storedcontent.hpp>
#include <sma/ccn/storedblock.hpp>

#include <cstdint>
#include <cstdlib>
#include <iosfwd>

#include <memory>
#include <unordered_map>


namespace sma
{
class MemoryContentStore : public ContentStore
{
private:
  class Block : public StoredBlock
  {
  public:
    Block(std::uint32_t index, std::uint32_t size);
    ~Block();

    virtual bool complete() const override { return gaps_.empty(); }
    virtual StoredBlock::gaps_vec gaps() const override { return gaps_; }

    virtual std::size_t read(std::uint8_t* dst,
                             std::size_t src_off,
                             std::size_t size) const override;

    virtual void insert(std::size_t dst_off,
                        std::uint8_t const* src,
                        std::size_t size) override;

    std::uint8_t* data_;
    StoredBlock::gaps_vec gaps_;
  };

public:
  virtual ~MemoryContentStore() {}

  virtual block_map* find(Hash content) override;

  virtual Hash store_from(std::istream& in, std::uint32_t block_size) override;

private:
  std::unordered_map<Hash, block_map> content;
};
}
