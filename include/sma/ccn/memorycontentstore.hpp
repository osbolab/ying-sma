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

  class Record : public StoredContent
  {
  public:
    Record(std::uint32_t size,
           std::uint32_t block_size,
           std::vector<std::unique_ptr<Block>> blocks);

    virtual std::size_t read(std::uint8_t* dst,
                             std::size_t size) const override;

    virtual StoredBlock* block(std::uint32_t index) override;
    virtual StoredBlock const* cblock(std::uint32_t index) const override;

    std::unordered_map<std::uint32_t, std::unique_ptr<Block>> blocks;
  };

public:
  virtual ~MemoryContentStore() {}

  virtual StoredContent* find(Hash content) override;

  virtual StoredContent* store_from(std::istream& in,
                                    std::uint32_t block_size) override;

private:
  std::unordered_map<Hash, std::unique_ptr<Record>> content;
};
}
