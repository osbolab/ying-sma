#pragma once

#include <iosfwd>

namespace sma
{
struct ContentType;
struct ContentName;

struct MessageHeader;
struct ContentInfoMessage;

class AbstractContentHelper
{
public:
  virtual ~AbstractContentHelper() {}

  //! Receive a content metadata announcement.
  /*! A metadata announcement indicates that a piece of content is available
   * from the publisher named in the metadata.  The announcement may be
   * forwarded so the publisher may be an undefined number of hops remote.
   */
  virtual void receive(MessageHeader header, ContentAnnouncement msg) = 0;

  //! Store a piece of a content locally and broadcast a metadata announcement
  //! to the network.
  virtual void publish(ContentType type, ContentName name, std::istream& is)
      = 0;
};
}
