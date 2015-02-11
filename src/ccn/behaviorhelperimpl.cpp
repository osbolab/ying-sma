#include <sma/ccn/behaviorhelperimpl.hpp>

#include <random>
#include <cstddef>
#include <chrono>
#include <string>
#include <sma/async.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/contenthelper.hpp>

namespace sma
{

    BehaviorHelperImpl::BehaviorHelperImpl (CcnNode& node,
           std::chrono::milliseconds post_interest_freq,
           std::chrono::milliseconds post_content_freq,
           std::chrono::milliseconds download_content_freq)
        : BehaviorHelper (node)
        , interest_freq (post_interest_freq)
        , publish_freq (post_content_freq)
        , request_freq (download_content_freq)
    {}

    void BehaviorHelperImpl::behave ()
    {
      // wait for random time
      std::chrono::milliseconds rand_delay1(std::rand() % 10000); 
      std::chrono::milliseconds rand_delay2(std::rand() % 10000); 
      std::chrono::milliseconds rand_delay3(std::rand() % 10000); 

      asynctask (&BehaviorHelperImpl::behave_interest,this).do_in(rand_delay1);
      asynctask (&BehaviorHelperImpl::behave_publish, this).do_in(rand_delay2);
      asynctask (&BehaviorHelperImpl::behave_request, this).do_in(rand_delay3);
    }

    void BehaviorHelperImpl::behave_interest()
    {
      // clear local interest
      // random create_new interest.

      std::vector<ContentType> types;
      ContentType type ("onlyone");
      types.push_back (type);

      node.interests->create_local(types);

      // schedule next interest behavior
      float ratio = (rand()%60-30) / 100.0;  // alpha = 30%
      auto new_delay = interest_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_interest, this).do_in (new_delay);
    }

    void BehaviorHelperImpl::behave_publish()
    {

      char data[4 * 1024];
      std::memset (data, 'a', sizeof data);
      std::istringstream content_stream (std::string(data, sizeof data));
      ContentType type ("onlyone");

      auto metadata = node.content->create_new(
              {type},
              get_rand_str_n(16),
              content_stream);

      node.content->announce_metadata();

      // schedule next publish behavior
      float ratio = (rand()%60-30) / 100.0; 
      auto new_delay = publish_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_publish, this).do_in (new_delay);
    }

    void BehaviorHelperImpl::behave_request()
    {
      std::vector<ContentMetadata> meta_vec = node.content->metadata();
      // need rank
      std::size_t rand_index = rand() % meta_vec.size();
      Hash content_name = meta_vec[rand_index].hash;
      float utility_per_block = 1.0f;
      int num_of_blocks = meta_vec[rand_index].size;

      std::vector<BlockRequestArgs> requests;
      std::chrono::milliseconds ttl (20000);
      for (std::size_t i=0; i<num_of_blocks; i++)
      {

         requests.push_back (BlockRequestArgs(content_name,
                     i,
                     utility_per_block,
                     ttl,
                     node.id,
                     node.position()));
      }

      node.content->request (requests);
      
      // schedule next request behavior
      float ratio = (rand()%60 - 30) / 100.0;
      auto new_delay = request_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_request, this).do_in (new_delay);
    }

    std::string BehaviorHelperImpl::get_rand_str_n (std::size_t length)
    {
        int sz_dictionary = sizeof (BehaviorHelperImpl::alphanum) - 1;
        std::string result;
        while (length > 0)
        {
          result += alphanum[rand() % sz_dictionary];
          length--; 
        }
        return result;
    }

    
}
