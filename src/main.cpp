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

#include <fstream>
#include "client.hpp"
#include "meta-info.hpp"
#include "./http/url-encoding.hpp"
#include "./http/http-request.hpp"
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
    
    // url encode the hash
    sbt::ConstBufferPtr hashptr = metainfo.getHash();
    sbt::Buffer hashbuf1 = *hashptr;
    uint8_t* hashbuf2 = hashbuf1.buf();
    std::string info_hash = sbt::url::encode(hashbuf2, 16); // TODO what is size?
    
    // other params
    std::string peer_id // urlencoded peer id for client
    std::string ip // ip of client machine
    int port = std::atoi(argv[1]); // port number for peer communication
    int_64t uploaded = 0; // bytes uploaded
    int_64t downloaded = 0; // bytes downloaded
    int_64t left = length; // bytes left
    std::string event // "stopped", "started", "completed"
    
    std::string reqParams = sbt::formatTrackerParams(
                                                info_hash,
                                                peer_id,
                                                ip,
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
    getReq.setHost('COOL_localhost'); // TODO get from .torrent announce part, hardcode for now
    getReq.setPort(54321); // port of tracker TODO get from .torrent announce part, hardcode for now
    getReq.setPath(path); // all those params are in there
    getReq.setVersion("1.0"); // or should be 1.1?
    getReq.addHeader("Accept-Language", "en-US");

    
    
    /* TODO
     1. (short-term) get the GET request to have all the right parts
     2. send the GET request to the tracker
     3. parse the information you get back from the tracker
     4. peer info list
     5. Do 2-4 in a while loop so client is periodically messaging tracker
     */
    
    size_t reqLen = getReq.getTotalLength();
    char *metabuf = new char [reqLen];
    getReq.formatRequest(metabuf);
    std::cout << metabuf << std::endl; // TODO do stuff with the char buffer
    
    // these just print out GET request
    std::string metastr = metabuf;
    std::cout << metastr << std::endl;
    //for (int i = 0; i < 100; i++)
    //{
    //  std::cout << metabuf[i];
    //}
    std::cout << std::endl;
    delete [] metabuf;
    
    
    
    /// Get peer list and print ///
    return 0;
    
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
