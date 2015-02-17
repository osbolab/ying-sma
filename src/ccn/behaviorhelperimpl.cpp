#include <sma/ccn/behaviorhelperimpl.hpp>

#include <random>
#include <cstddef>
#include <chrono>
#include <string>
#include <sma/async.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/contenthelper.hpp>
#include <unordered_set>

namespace sma
{	
    static const char const_alphanum[] =
       { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghipqrstuvwxyz"};
	
    static const std::vector<std::string> const_categories = 
      {"type1", "type2", "type3", "type4", "type5"};
	
	const char* BehaviorHelperImpl::alphanum = const_alphanum;
	
	const std::vector<std::string> BehaviorHelperImpl::categories = const_categories;

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
      node.interests->clear_local();
      // random create_new interest.

	  std::vector<ContentType> types;
	  ContentType type ("type1");;
      types.push_back (type);
	  

/*      int num_of_types = 1; //rand() % categories.size() + 1;
      std::unordered_set <std::string> type_strs;

      while (num_of_types > 0)
      {
         int type_index = rand() % categories.size();
         type_strs.insert (categories[type_index]);
         num_of_types--;
      }

      auto it = type_strs.begin();
      std::vector<ContentType> types;

      while (it != type_strs.end())
      {
        ContentType type (*it);
        types.push_back (type);
        it++;
      }
*/	  
	  
      node.interests->create_local(types);

      // schedule next interest behavior
      float ratio = (rand()%60-30) / 100.0;  // alpha = 30%
      auto new_delay = interest_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_interest, this).do_in (new_delay);
    }

    void BehaviorHelperImpl::behave_publish()
    {

      char data[4 * 1024];
      std::string rand_str = get_rand_str_n (sizeof data).c_str();
      std::strncpy (data, rand_str.c_str(), rand_str.size());
      std::istringstream content_stream (std::string(data, sizeof data));
	  
      int num_of_types = rand() % categories.size() + 1;
      std::unordered_set <std::string> type_strs;

      while (num_of_types > 0)
      {
         int type_index = rand() % categories.size();
         type_strs.insert (categories[type_index]);
         num_of_types--;
      }

      auto it = type_strs.begin();
      std::vector<ContentType> types;

      while (it != type_strs.end())
      {
        ContentType type (*it);
        types.push_back (type);
        it++;
      }
	  
      auto metadata = node.content->create_new(
              types,
              get_rand_str_n(16),
              data,
              sizeof data);

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
      for (auto it=meta_vec.begin(); it!=meta_vec.end();)
      {
        if (it->publisher == node.id)
            it = meta_vec.erase(it);
        else
            it++;
      }

      std::size_t total_metas = meta_vec.size();
     

      if (total_metas != 0)
      {
        std::size_t rand_index = rand() % total_metas;
        Hash content_name = meta_vec[rand_index].hash;
        float utility_per_block = 1.0f;
        int num_of_blocks = 1+((meta_vec[rand_index].size-1)/meta_vec[rand_index].block_size);

        std::vector<BlockRequestArgs> requests;
        std::chrono::milliseconds ttl (20000);
        for (std::size_t i=0; i<num_of_blocks; i++)
        {

          requests.push_back (BlockRequestArgs(BlockRef(content_name,i),
                     utility_per_block,
                     ttl,
                     node.id,
                     node.position(),
                     true));
        }

        node.content->request (requests);
      }
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
