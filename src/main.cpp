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
    ;endl
    std::cout << "~~ opened file, about to parse ~~" << std::endl;
    
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
    // TODO refactor tracker param code with get/setter
    
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
    // TODO fix inputs for HTTP request based on reference client.cpp
    sbt::HttpRequest getReq;
    getReq.setMethod(sbt::HttpRequest::GET);
    getReq.setHost("localhost"); // TODO get from .torrent announce part, hardcode for now
    getReq.setPort(12345); // port of tracker TODO get from .torrent announce part, hardcode for now
    getReq.setPath(path); // all those params are in there
    getReq.setVersion("1.0"); // should be 1.0 or 1.1?
    getReq.addHeader("Accept-Language", "en-US");

    // get HTTP request as char buffer
    size_t reqLen = getReq.getTotalLength();
    char *metabuf = new char [reqLen];
    getReq.formatRequest(metabuf);
    std::cout << metabuf << std::endl; // TODO do stuff with the char buffer
    
    // these just convert GET request buf to string and print
    std::string metastr = metabuf;
    std::cout << "~~ http request ~~\n" << metastr << "\n\n" << std::endl;

    delete [] metabuf; // delete buffer when done
    
    ///// start orig request sending code (also in client.cpp)
    // TODO why is connection refused? print everything. idk why it doesn't work now.
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
    
    std::cout << "~~ about to try connecting ~~\n\n" << std::cout
    
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      perror("connect");
      return 2;
    }
    
    std::cout << "~~ about to get client id ~~\n\n" << std::cout
    
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
    //char recbuf[3000] = {0}; // buf holds data received
    /* TODO in a while loop, keep receiving stuff from buffer and
     adding on to the stringstream until you reach some end of file signal
     (maybe a newline or \r\n?), or parse content length from file?
     (not sure what max received buf size should be)
     */
    std::stringstream recss; // received buf is put into ss
    
    //while (!isEnd) {
      memset(recbuf, '\0', sizeof(recbuf)); // null terminate buffer
      
      // sending
      if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
        perror("send");
        return 4;
      //}

    ///// end orig sending code
    
    //////////////////////////////////////////////////////////////////////
    
    //void
    //Client::recvTrackerResponse()
    //{
    bool m_isFirstRes = true;
    
      std::stringstream headerOs;
      std::stringstream bodyOs;
      
      char buf[512] = {0};          // read in 512 chars at a time
      char lastTree[3] = {0};       // ??? (print out buf to figure out what it is)
      
      bool hasEnd = false;          // did you get a message saying to close?
      bool hasParseHeader = false;  // has header been parsed into HttpResponse obj response?
      HttpResponse response;
      
      uint64_t bodyLength = 0;
      
      while (true) {
        memset(buf, '\0', sizeof(buf)); // null-terminate buffer
        memcpy(buf, lastTree, 3);       // set first three chars of buf to lastTree
        
        // read in (512 - 3) chars (skip first 3 chars)
        // res = size of buf received
        ssize_t res = recv(sockfd, buf + 3, 512 - 3, 0);
        
        if (res == -1) { // if error in recv
          perror("recv");
          return;
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
      FD_CLR(m_trackerSock, &m_readSocks);
      
      /// parse tracker response and get interval
  
      // convert body0s to ss, then wireDecode into dict
      bencoding::Dictionary dict;
      std::stringstream tss;
      tss.str(bodyOs.str());
      dict.wireDecode(tss);
    
    
    std::cout << tss << std::endl;
    
      // get interval and peers
      TrackerResponse trackerResponse;
      trackerResponse.decode(dict);
      const std::vector<PeerInfo>& peers = trackerResponse.getPeers();
      m_interval = trackerResponse.getInterval();
      
      // print out peer info for first response only
      if (m_isFirstRes) {
        for (const auto& peer : peers) {
          std::cout << peer.ip << ":" << peer.port << std::endl;
        }
      }
      
      m_isFirstRes = false;
    //}

    
    ////////////////////////////////////////////////////
    
  return 0;
}
