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

namespace sbt {
  namespace treq {
  
  // given params, put them together in the right format for GET path
  // and return as string
  // Note: you cannot omit any of the input params, or add additional params
  //       with this function.
  std::string
  formatTrackerParams(std::string info_hash, // urlencoded metainfo
               std::string peer_id, // urlencoded peer id for client
               std::string ip, // ip addr of client machine
               int port, // port number for peer communication
               int_64t uploaded, // bytes uploaded
               int_64t downloaded, // bytes downloaded
               int_64t left, // bytes left
               std::string event // "stopped", "started", "completed"
               )
  
  

  }} // end namespace sbt and treq


#endif /* defined(____tracker_request_param__) */
