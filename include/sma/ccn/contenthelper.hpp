#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/ccnfwd.hpp>

#include <iosfwd>

namespace sma
{
class ContentHelper : public Helper
{
public:
  ContentHelper(CcnNode& node);

  virtual ~ContentHelper() {}

  //! Receive a content metadata announcement.
  /*! A metadata announcement indicates that a piece of content is available
   * from the publisher named in the metadata.  The ann may be
   * forwarded so the publisher may be an undefined number of hops remote.
   */
  virtual void receive(MessageHeader header, ContentAnn msg) = 0;

  //! Store a piece of a content locally and broadcast a metadata announcement
  //! to the network.
  virtual void publish(ContentType type, ContentName name, std::istream& is)
      = 0;
};
}
