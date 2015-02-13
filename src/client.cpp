/*
 * client.cpp
 * Part of CS118 Simple-BT project
 *
 * main.cpp parses metainfo file and formats initial tracker request
 * client.hpp implements these steps:
 * - sends tracker request (may need to format too, after initial request)
 * - receive and parse tracker response
 * - print formatted peer info
 *
 * \author LG248
 *
 */

// TODO this is my old code for the tracker request/response steps, not based on the reference

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include "./http/http-request.hpp"
#include "tracker-response.hpp"
#include "./http/http-response.hpp"
#include "tracker-request-param.hpp"
#include "client.hpp"


/// Send the initial request to the tracker ///
// Socket code modified from client.cpp posted by Yingdi Yu
// http://irl.cs.ucla.edu/~yingdi/cs118/proj1/client.cpp

// create a socket using TCP IP
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

// format server socket address
struct sockaddr_in serverAddr;
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(12345);     // short, network byte order
serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

// connect to the server
if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
  perror("connect");
  return 2;
}

// get client address from sockfd
struct sockaddr_in clientAddr;
socklen_t clientAddrLen = sizeof(clientAddr);
if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
  perror("getsockname");
  return 3;
}

// display ipaddr as string
char ipstr[INET_ADDRSTRLEN] = {'\0'};
inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
std::cout << "Set up a connection from: " << ipstr << ":" <<
ntohs(clientAddr.sin_port) << std::endl;


// send/receive data to/from connection
bool isEnd = false;
std::string input = metastr; // input is data to send
char recbuf[3000] = {0}; // buf holds data received
/* TODO in a while loop, keep receiving stuff from buffer and
 adding on to the stringstream until you reach some end of file signal
 (maybe a newline or \r\n?), or parse content length from file?
 (not sure what max received buf size should be)
 */
std::stringstream recss; // received buf is put into ss

while (!isEnd) {
  memset(recbuf, '\0', sizeof(recbuf)); // null terminate buffer
  
  // sending
  if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
    perror("send");
    return 4;
  }
  
  // receiving
  if (recv(sockfd, recbuf, sizeof(recbuf), 0) == -1) {
    perror("recv");
    return 5;
  }
  
  // print out the recbuf
  std::cout << "size of recbuf: " << sizeof(recbuf) << std::endl;
  std::cout << "printing tracker response buffer" << std::endl;
  std::cout << recbuf << std::endl;
  
  // add to string stream
  recss << recbuf << std::endl;
  if (recss.str() == "close\n") // TODO does "close" replace header or is it after header?
    break;
  // roughly end of client.cpp-based code
  
  
  /// parsing tracker response ///
  // (consists of HTTP header and bencoded dict of peers)
  
  std::stringstream headerss;
  std::stringstream msgss;
  
  // parse header
  sbt::HttpResponse respHeader;
  respHeader.parseResponse(headerss.str().c_str(), headerss.str().size());
  uint64_t contentLen = boost::lexical_cast<uint64_t>(
                                                      respHeader.findHeader("Content-Length"));
  
  std::cout << "This is what I have for header: " << std::endl;
  std::cout << headerss << std::endl;
  std::cout << "This is content length: " << contentLen << std::endl;
  
  // parse message body
  sbt::TrackerResponse trackerResp;
  sbt::bencoding::Dictionary respDict;
  respDict.wireDecode(ss);
  trackerResp.decode(respDict);
  
  if (trackerResp.isFailure())       // handle failure
  {
    std::cerr << "Tracker failure:" << std::endl;
    std::cerr << trackerResp.getFailure() << std::endl;
    break; // TODO figure out what failure indicates and if you should return
  }
  
  uint64_t interval = trackerResp.getInterval(); // how long to wait
  std::vector<sbt::PeerInfo> peerVector = trackerResp.getPeers();
  
  recss.str(""); // clear received ss (set to "")
  
  
}

close(sockfd);


/*
 sbt::HttpResponse respHeader;
 resp.parseResponse(headerBuf, sizeof(headerBuf)); // idk size
 size_t respSize = resp.getTotalLength();
 const std::string statusCode = respHeader.getStatusCode();
 const std::string statusMsg = respHeader.getStatusMsg();
 std::cout << "status code and message" << std::endl;
 std::cout << statusCode << std::endl;
 std::cout << statusMsg << std::endl;
 */


/* TODO
 1. x (short-term) get the GET request to have all the right parts
 2. x send the GET request to the tracker
 2.5. get info back from the tracker (TODO figure out buffer size)
 3. parse the information you get back from the tracker
 4. peer info list
 5. Do 2-4 in a while loop so client is periodically messaging tracker
 */


/// Get peer list and print ///
return 0;

}
catch (std::exception& e)
{
  std::cerr << "exception: " << e.what() << "\n";
}
