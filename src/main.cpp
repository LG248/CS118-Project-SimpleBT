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
    int64_t pieceLength = metainfo.getPieceLength(); // number of pieces
    std::vector<uint8_t> pieces = metainfo.getPieces(); // concatenated 20-byte SHA1 hash values
    //std::string name = metainfo.getName(); // file name
    //int64_t length = metainfo.getLength(); // length of file
    
    // url encode the hash
    sbt::ConstBufferPtr hashptr = metainfo.getHash();
    sbt::Buffer hashbuf1 = *hashptr;
    uint8_t* hashbuf2 = hashbuf1.buf();
    std::string info_hash = sbt::url::encode(hashbuf2, 16); // TODO what is size?
    std::cout << info_hash + "\n";
    
    /// Tracker request ///
    
    // Send HTTP GET request to send/receive the following info:
    // - client requests peer info from tracker
    // - client reports meta info to tracker (info_hash, ip, port, event)
    // - client reports status to tracker (uploaded, downloaded, lef)
    sbt::HttpRequest getReq;
    getReq.setMethod(sbt::HttpRequest::GET); // TODO idk what values to put
    getReq.setHost(announce);
    getReq.setPort(12345);
    getReq.setPath("/"); // what should be path?
    getReq.setVersion("1.0"); // should be 1.1?
    
    // TODO somehow add the &info_hash=...&downloaded=... stuff
    // and the headers would be good to
    
    
    /* TODO
     1. (short-term) get the GET request to have all the right parts
     2. send the GET request to the tracker
     3. parse the information you get back from the tracker
     4. peer info list
     
     */
    
    size_t reqLen = getReq.getTotalLength();
    char *metabuf = new char [reqLen];
    getReq.formatRequest(metabuf);
    std::cout << metabuf << std::endl; // TODO do stuff with the char buffer
    
    // these just print out GET request
    std::string metastr = metabuf;
    std::cout << metastr << std::endl;
    for (int i = 0; i < 100; i++)
    {
      std::cout << metabuf[i];
    }
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
