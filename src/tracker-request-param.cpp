//
//  tracker-request-param.cpp
//
#include "tracker-request-param.hpp"

namespace sbt {
  namespace treq {

//returns path of form "?info_hash=<value1>&peer_id=<value2>&..."
std::string
formatTrackerParams(std::string info_hash, // urlencoded metainfo
                    std::string peer_id, // url encoded peer id for client
                    std::string ip, // ip of client machine
                    int port, // port number for peer communication
                    int_64t uploaded, // bytes uploaded
                    int_64t downloaded, // bytes downloaded
                    int_64t left, // bytes left
                    std::string event // "stopped", "started", "completed"
                    )
{
  std::string params =
    "?info_hash=" + info_hash +
    "&peer_id=" + peer_id +
    "&ip=" + ip +
    "&port=" + std::to_string(port) +
    "&uploaded=" + std::to_string(uploaded) +
    "&downloaded=" + std::to_string(downloaded) +
    "&left=" + std::to_string(left) +
  "&event=" + event;
  
  return params;
}

  }}


