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
#include "client.hpp"
#include "meta-info.hpp"
#include "./http/url-encoding.hpp"
#include "./http/http-request.hpp"
#include "tracker-response.hpp"
#include "./http/http-response.hpp"
#include "tracker-request-param.hpp"


/*
 TODO
 0) pass test 4 (started must be the first req)
 1) separate out code with comment blocks (equiv to functions)
 2) refactor/trim unnecessary code. figure out what lastTree is
 */



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

    // Initialise the client. TODO client doesn't do shit atm
    //sbt::Client client(argv[1], argv[2]);
    
    //// Initialize useful variables
    
    /* 
     // parsing metainfo vars
    sbt::MetaInfo metainfo; // holds info from .torrent file
    std::string host; // tracker host to use in GET request to tracker ("localhost")
    std::string port; // tracker port (12345), currently an int in my code
    std::string file; // don't think i need since we do single file mode 
     */
    
    uint16_t clientPort;

    bool isFirstReq = true; // only need event param for first
    bool isFirstResponse = true; // idk what for yet
    
    //int sockfd; // equiv to trackerSock in ref code, already init inside while(true)
    //int serverSock = -1; // for part 2?
    
    //fd_set readSocks; // only used in FD_CLR(sockfd, &readSocks), which just removes sockfd from the readSocks set. not sure if needed.
    
    //uint64_t interval; // eyyy, already init inside while(true)
    
    
    //// Get metainfo
    
    // ppen .torrent file and decode
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
    
  
    /// Tracker request parameters
    
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
    //std::string event = "no_event"; // indicates to leave event out of req
    std::string event = "started"; // ref code sets to started with first req, then leaves as started

    if (isFirstReq){
      event = "started";
      isFirstReq = false;
    }
    
    std::string reqParams = sbt::treq::formatTrackerParams(
                                                info_hash,
                                                peer_id,
                                                ip_str,
                                                port,
                                                uploaded,
                                                downloaded,
                                                left,
                                                event);
 
    
    // Send HTTP GET request to send/receive the following info:
    // - client requests peer info from tracker
    // - client reports meta info to tracker (info_hash, ip, port, event)
    // - client reports status to tracker (uploaded, downloaded, left)
    
    while (true) { // TODO what is the break condition? which part loops?

    //// Send tracker request
    // create path (tracker url, also encodes meta info and status)
    std::string path = announce + reqParams;
    
    // create HTTP request
    // TODO fix inputs for HTTP request based on reference client.cpp instead of hardcoding
    sbt::HttpRequest getReq;
    getReq.setMethod(sbt::HttpRequest::GET);
    getReq.setHost("localhost"); // TODO get from .torrent announce part, hardcode for now
    getReq.setPort(12345); // port of tracker TODO get from .torrent announce part, hardcode for now
    getReq.setPath(path); // all those params are in there
    getReq.setVersion("1.0"); // should be 1.0 or 1.1?
    getReq.addHeader("Accept-Language", "en-US");

    // get HTTP request as char buffer
    size_t reqLen = getReq.getTotalLength();
    char *reqBuf = new char [reqLen];
    getReq.formatRequest(reqBuf);
    //std::cout << reqBuf << std::endl; // TODO do stuff with the char buffer
    
    // put GET request as a string and print
    std::string reqStr = reqBuf;
    //std::cout << "~~ http request ~~\n" << reqStr << "\n\n" << std::endl;

    delete [] reqBuf; // delete buffer when done
    
    ///// start orig request sending code (also in client.cpp)

    /// Send the initial request to the tracker ///
    
    
    //// Connect tracker
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // uses TCP IP
    
    // format server socket address (not yet used?)
    // TODO don't hardcode values
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    

    //std::cout << "~~ about to try connecting ~~\n\n" << std::endl;
    
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      perror("connect");
      return 2;
    }
    
    //std::cout << "~~ about to get client id ~~\n\n" << std::endl;
    
    // TODO clientAddr and ipAddr not used yet
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
    //std::cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;
    
    
    // actual sending (recall metastr is full size, can send all at once)
    if (send(sockfd, reqStr.c_str(), reqStr.size(), 0) == -1) {
      perror("send");
      return 4;
    }

    
    
    // received data will be stored in buffer and ss
    char recvBuf[1000] = {0}; // buf holds data received
    std::stringstream recvss; // received buf is put into recss
    memset(recvBuf, '\0', sizeof(recvBuf)); // null terminate buffer
    
    ///// end orig sending code
    
    //////////////////////////////////////////////////////////////////////
    //// Receive response from tracker
    //void
    //Client::recvTrackerResponse()
    //{
    //bool m_isFirstRes = true;
    
    std::stringstream headerOs;
    std::stringstream bodyOs;
      
    char buf[512] = {0};          // read in 512 chars at a time
    char lastTree[3] = {0};       // ??? (print out buf to figure out what it is)
      
    bool hasEnd = false;          // did you get a message saying to close?
    bool hasParseHeader = false;  // has header been parsed into HttpResponse obj response?
    sbt::HttpResponse response;
      
    uint64_t bodyLength = 0;
    
    while (true) {
      memset(buf, '\0', sizeof(buf)); // null-terminate buffer
      memcpy(buf, lastTree, 3);       // set first three chars of buf to lastTree
      
      // read in (512 - 3) chars (skip first 3 chars)
      // res = size of buf received
      ssize_t res = recv(sockfd, buf + 3, 512 - 3, 0);
      
      if (res == -1) { // if error in recv
        perror("recv");
        return 5; // TODO make sure error return values are sequential
      }
      
      const char* endline = 0; // will point to end of header
      
      if (!hasEnd) {            // if end not found
        // memmem finds first occurance of "\r\n\r\n" (len 4) in buf
        endline = (const char*)memmem(buf, res, "\r\n\r\n", 4);
      }
      
      if (endline != 0) {      // if rnrn is found
        const char* headerEnd = endline + 4; // end of header after "\r\n\r\n"
        
        headerOs.write(buf + 3, (endline + 4 - buf - 3)); // write to ss, skipping lastTree
        
        if (headerEnd < (buf + 3 + res)) { // if headerEnd extends beyond end of http header
          bodyOs.write(headerEnd, (buf + 3 + res - headerEnd)); // write remainder to body
        }
        
        hasEnd = true;
      }
      
      else { // if rnrn not part of buf
        if (!hasEnd) { // if not yet at end of header (rnrn)
          memcpy(lastTree, buf + res, 3); // set lastTree to first 3 chars after buf
          headerOs.write(buf + 3, res);   // write header to ss
        }
        else // if done with header, write body
          bodyOs.write(buf + 3, res);
      }
      
      if (hasEnd) { // if at end of header
        if (!hasParseHeader) { // if header not parsed (put into http response object)
          response.parseResponse(headerOs.str().c_str(), headerOs.str().size());
          hasParseHeader = true;
          
          bodyLength = boost::lexical_cast<uint64_t>(response.findHeader("Content-Length"));
        }
      }
      
      // if header is parsed and body0s contains full body, done with recv
      if (hasParseHeader && bodyOs.str().size() >= bodyLength)
        break;
    }
    
    close(sockfd);
    fd_set m_readSocks; // taken from data member of client.hpp

    FD_CLR(sockfd, &m_readSocks);
    
    /// parse tracker response and get interval

    // convert body0s to ss, then wireDecode into dict
    sbt::bencoding::Dictionary dict;
    std::stringstream tss;
    tss.str(bodyOs.str());
    dict.wireDecode(tss);
  
    // tss is decoded body
    //std::cout << tss << std::endl;
  
    // get interval and peers
    sbt::TrackerResponse trackerResponse;
    trackerResponse.decode(dict);
    const std::vector<sbt::PeerInfo>& peers = trackerResponse.getPeers();
    uint64_t interval = trackerResponse.getInterval();
    
    //std::cout << "interval: " << interval << std::endl;
        
    // print out peer info for first response only
    if (isFirstResponse) {
      for (const auto& peer : peers) {
        std::cout << peer.ip << ":" << peer.port << std::endl;
      }
    }
    
    isFirstResponse = false;
  //}

  //////////////////////////////////////////////////// end ref client.cpp code
    // wait for interval before sending next request
    close(sockfd);
    sleep(interval);
  }// end while(true)
  } // end try
    
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }
        
  return 0;
}
