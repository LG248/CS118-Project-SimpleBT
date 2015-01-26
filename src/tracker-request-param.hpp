//
//  tracker-request-param.hpp
//

/*
 * Simple function to format parameters for a tracker request, i.e.
 * "?<key1>=<value1>&<key2>=<value2>&..."
 */
#ifndef ____tracker_request_param__
#define ____tracker_request_param__

#include <iostream>
#include "common.hpp"

namespace sbt {
  namespace treq {
  
  // given params, put them together in the right format for GET path
  // and return as string.
  // omit event param by putting in "no_event"
  // Note: you cannot omit any other input params, or add additional params
  //       with this function.
  std::string
  formatTrackerParams(std::string info_hash,  // urlencoded metainfo
               std::string peer_id,  // urlencoded peer id for client
               std::string ip, // ip addr of client machine
               int port,       // port number for peer communication
               int64_t uploaded,   // bytes uploaded
               int64_t downloaded, // bytes downloaded
               int64_t left,       // bytes left
               std::string event   // "stopped", "started", "completed", "no_event"
    );
    

  }} // end namespace sbt and treq


#endif /* defined(____tracker_request_param__) */
