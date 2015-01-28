/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California
 *
 * This file is part of Simple BT.
 * See AUTHORS.md for complete list of Simple BT authors and contributors.
 *
 * NSL is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NSL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NSL, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Yingdi Yu <yingdi@cs.ucla.edu>
 * \author LG248
 */

// includes from client.cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include "client.hpp"
#include "meta-info.hpp"
#include "./http/url-encoding.hpp"
#include "./http/http-request.hpp"
#include "tracker-response.hpp"
#include "./http/http-response.hpp"
#include "tracker-request-param.hpp"


int
main(int argc, char** argv)
{
  try
  {
    // Check command line arguments.
    if (argc != 3)
    {
      std::cerr << "Usage: simple-bt <port> <torrent_file>\n";
      return 1;
    }

    // Initialise the client.
    sbt::Client client(argv[1], argv[2]);

    
    /// Parse torrent file ///
    
    // Open .torrent file and decode
    std::fstream metaFile;
    metaFile.open(argv[2], std::fstream::in);
    if (metaFile == NULL)
    {
      std::cerr << "Unable to open file: " << argv[2] << "\n";
      std::cerr << "Usage: simple-bt <port> <torrent_file>\n";
      return 1;
    }
      
    sbt::MetaInfo metainfo;
    metainfo.wireDecode(metaFile);
    metaFile.close();
    
    // Get announce and info fields
    std::string announce = metainfo.getAnnounce(); // announce URL of tracker
    //int64_t pieceLength = metainfo.getPieceLength(); // number of pieces
    //std::vector<uint8_t> pieces = metainfo.getPieces(); // concatenated 20-byte SHA1 hash values
    //std::string name = metainfo.getName(); // file name
    int64_t length = metainfo.getLength(); // length of file
    
    
    /// Tracker request parameters ///
    
    // url encode the info hash
    sbt::ConstBufferPtr hashptr = metainfo.getHash();
    sbt::Buffer hashbuf1 = *hashptr;
    uint8_t* hashbuf2 = hashbuf1.buf();
    std::string info_hash = sbt::url::encode(hashbuf2, 20); // 20-byte encoding
    //std::cout << info_hash << std::endl; // print info_hash

    
    // other params
    std::string peer_id = "somepeer";        // urlencoded peer id for client
    std::string ip_str = "127.0.0.1";        // ip of client machine
    int port = std::atoi(argv[1]);  // port number for peer communication
    int64_t uploaded = 0;           // bytes uploaded, 0 to start
    int64_t downloaded = 0;         // bytes downloaded
    int64_t left = length;          // bytes left (whole file to start) TODO right?
    std::string event = "started";  // initial set to started, later remove
    
    std::string reqParams = sbt::treq::formatTrackerParams(
                                                info_hash,
                                                peer_id,
                                                ip_str,
                                                port,
                                                uploaded,
                                                downloaded,
                                                left,
                                                event);
 
    /// Make tracker request ///
    
    // Send HTTP GET request to send/receive the following info:
    // - client requests peer info from tracker
    // - client reports meta info to tracker (info_hash, ip, port, event)
    // - client reports status to tracker (uploaded, downloaded, lef)
    
    // create path (tracker url, also encodes meta info and status)
    std::string path = announce + reqParams;
    
    // create HTTP request
    sbt::HttpRequest getReq;
    getReq.setMethod(sbt::HttpRequest::GET);
    getReq.setHost("localhost"); // TODO get from .torrent announce part, hardcode for now
    getReq.setPort(12345); // port of tracker TODO get from .torrent announce part, hardcode for now
    getReq.setPath(path); // all those params are in there
    getReq.setVersion("1.0"); // or should be 1.1?
    getReq.addHeader("Accept-Language", "en-US");

    // get HTTP request as char buffer
    size_t reqLen = getReq.getTotalLength();
    char *metabuf = new char [reqLen];
    getReq.formatRequest(metabuf);
    std::cout << metabuf << std::endl; // TODO do stuff with the char buffer
    
    // these just convert GET request buf to string and print
    std::string metastr = metabuf;
    std::cout << metastr << std::endl;

    delete [] metabuf; // delete buffer when done
    
    
    
    /// Send the initial request to the tracker ///
    // Socket code modified from client.cpp posted by Yingdi Yu
    // http://irl.cs.ucla.edu/~yingdi/cs118/proj1/client.cpp
    
    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // format server socket address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);     // short, network byte order
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
    std::stringstream ss; // buf is put into ss
    
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
      ss << recbuf << std::endl;
      if (ss.str() == "close\n") // TODO does "close" replace header or is it after header?
        break;
      // roughly end of client.cpp-based code
      /*

      // parse tracker response (consists of header and bencoded dict of peers)
      // TODO separate header from dict (using /r/n?)
       //YYu: http-response is only for parsing http response header. The tracker-response is only for parsing the msg-body.
      //Yyu: see W2 discussion for how to find separation between header and body
      
      // parse header (TODO implement)
      sbt::HttpResponse respHeader;
      respHeader.parseHeaders(headerBuf, someSize);
      std::string contentLenStr = findHeader(someKeyString);
      int contentLength = atoi(contentLenStr); //TODO int? size_t? uint64_t?
      
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
      
      ss.str(""); // clear ss (set to "")
      */
      
    }
    
    close(sockfd);
    
    

     sbt::HttpResponse respHeader;
     resp.parseResponse(headerBuf, sizeof(headerBuf)); // idk size
     size_t respSize = resp.getTotalLength();
     const std::string statusCode = respHeader.getStatusCode();
     const std::string statusMsg = respHeader.getStatusMsg();
     std::cout << "status code and message" << std::endl;
     std::cout << statusCode << std::endl;
     std::cout << statusMsg << std::endl;

    
    
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

  return 0;
}
