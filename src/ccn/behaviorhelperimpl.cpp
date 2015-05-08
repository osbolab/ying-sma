#include <sma/ccn/behaviorhelperimpl.hpp>

#include <random>
#include <cstddef>
#include <chrono>
#include <string>
#include <sma/async.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/contenthelper.hpp>
#include <sma/schedule/forwardscheduler.hpp>
#include <unordered_set>
#include <algorithm>
#include <sma/util/event.hpp>
#include <sma/io/log>
#include <sma/util/hash.hpp>

namespace sma
{	
    static const char const_alphanum[] =
       { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghipqrstuvwxyz"};
	
    static const std::vector<std::string> const_categories = 
      {"type1", "type2"};
	
	const char* BehaviorHelperImpl::alphanum = const_alphanum;
	
	const std::vector<std::string> BehaviorHelperImpl::categories = const_categories;

    constexpr std::size_t BehaviorHelperImpl::max_requests;

    BehaviorHelperImpl::BehaviorHelperImpl (CcnNode& host_node,
           std::chrono::milliseconds post_interest_freq,
           std::chrono::milliseconds post_content_freq,
           std::chrono::milliseconds download_content_freq)
        : BehaviorHelper (host_node)
        , interest_freq (post_interest_freq)
        , publish_freq (post_content_freq)
        , request_freq (download_content_freq)
        , request_times (0)
    {
      node.content->on_content_complete() += std::bind(&BehaviorHelperImpl::on_content, 
              this, 
              std::placeholders::_1);
    }
    
    bool BehaviorHelperImpl::on_content (Hash hash)
    {
      auto content_req = content_req_record.find(hash);
      assert(content_req != content_req_record.end());
      // Print complete log if not do that yet.
      if ((content_req->second).accessed == false) {
/*        std::vector<ContentMetadata> meta_vec = node.content->metadata();
        auto metaIt = meta_vec.begin();
        while (metaIt != meta_vec.end()) {
          if (metaIt->hash == hash)
            break;
          metaIt++;
        }

      if (metaIt != meta_vec.end()) {
        auto from_id = metaIt->publisher;
        int hops = metaIt->hops;

        node.log.i ("Complete download content %v (published by %v, %v hops away)", 
                hash,
                from_id,
                hops);
        // Don't print the log for other block_arrived_event
      } else {
        node.log.i ("Complete download content %v ", hash); 
      }
*/
        auto current_time = sma::chrono::system_clock::now();
        auto before_deadline 
            = std::chrono::duration_cast<std::chrono::milliseconds>((content_req->second).deadline - current_time).count();
        node.log.i ("Complete download content %v, published by %v, %v hops away, utility %v, %v left",
                hash, 
                (content_req->second).from_id,
                (content_req->second).hops,
                (content_req->second).utility,
                before_deadline
                ); 
        (content_req->second).accessed = true;
      }

      return true; 
    }

    void BehaviorHelperImpl::behave ()
    {
      // wait for random time
      std::chrono::milliseconds rand_delay1(interest_freq); 
      std::chrono::milliseconds rand_delay2(publish_freq);
      std::chrono::milliseconds rand_delay3(request_freq); 


      asynctask (&BehaviorHelperImpl::behave_interest,this).do_in(rand_delay1);
      asynctask (&BehaviorHelperImpl::behave_publish, this).do_in(rand_delay2);
      asynctask (&BehaviorHelperImpl::behave_request, this).do_in(rand_delay3);
    }

    void BehaviorHelperImpl::behave_interest()
    {
      // clear local interest
      node.interests->clear_local();
      // random create_new interest.

      int num_of_types = 1; //rand() % categories.size() + 1;
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


//      std::vector<ContentType> types;
//      types.push_back (ContentType("type1"));
//      types.push_back (ContentType("type2"));
//      types.push_back (ContentType("type4"));
	  
      if (types.size() > 0)
        node.interests->create_local(types);

      // schedule next interest behavior
      float ratio = (rand()%60-30) / 100.0;  // alpha = 30%
      auto new_delay = interest_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_interest, this).do_in (new_delay);
    }

    void BehaviorHelperImpl::behave_publish()
    {
      float min_blocks = 1.0;
      float max_blocks = 1.0;

      std::size_t n_blocks  = min_blocks 
            + static_cast <float> (rand()) 
            / (static_cast <float> (RAND_MAX)/(max_blocks-min_blocks));


      char* data = new char [1024 * n_blocks];



      std::string rand_str = get_rand_str_n (1024 * n_blocks ).c_str();
      std::strncpy (data, rand_str.c_str(), rand_str.size());
      std::istringstream content_stream (std::string(n_blocks, 1024 * n_blocks));
	  
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
              n_blocks * 1024);

      delete[] data;

//      node.content->announce_metadata();

      // schedule next publish behavior
      float ratio = (rand()%60-30) / 100.0; 
      auto new_delay = publish_freq * (1 + ratio);
      asynctask (&BehaviorHelperImpl::behave_publish, this).do_in (new_delay);
    }

    void BehaviorHelperImpl::behave_request()
    {

      std::vector<ContentMetadata> meta_vec = node.content->remote_metadata();

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
        NodeId from_id = meta_vec[rand_index].publisher;
        int hops = meta_vec[rand_index].hops;

        float min_util = 0.1f;
        float max_util = 1.0f;

        float utility_per_block = min_util 
            + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)/(max_util-min_util));

        float min_ttl = 10 * node.sched->get_sched_interval();
        float max_ttl = 10 * node.sched->get_sched_interval();
        float ttl_per_block = min_ttl
            + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)/(max_ttl - min_ttl));

        auto current_time = sma::chrono::system_clock::now();
        auto dl = current_time 
            + std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::milliseconds(static_cast<std::uint32_t>(ttl_per_block)));

        // reset the content_req_record, as it will deemed
        // as a new request. a local hit will be expected.
        auto content_req = content_req_record.find(content_name);
        if (content_req != content_req_record.end())
          (content_req->second).accessed = false;
        else
          content_req_record.insert(
                  {content_name, 
                    {false, utility_per_block, current_time, dl, hops, from_id}
                  });


        node.log.d("Request content %v (published by %v, %v hops away) with utility %v with ttl %v(ms)", 
                   content_name,
                   from_id,
                   hops,
                   utility_per_block,
                   ttl_per_block);
				   
        node.content->request_content (content_name,
		                               utility_per_block,
		                               static_cast<std::uint32_t> (ttl_per_block));
 
      }

      if (request_times++ < max_requests) {
        // schedule next request behavior
        float ratio = (rand()%60 - 30) / 100.0;
        auto new_delay = request_freq * (1 + ratio);
        asynctask (&BehaviorHelperImpl::behave_request, this).do_in (new_delay);
      }
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
