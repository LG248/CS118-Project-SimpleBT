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

#include "client.hpp"
#include "meta-info.hpp"

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

    
    /// 1. Parse torrent file ///
    
    /*
     Contents of metainfo file:
     anounce: announce URL of tracker
     info:
      pieceLength = (int64_t) number of pieces
      pieces = (byte string) concatenated 20-byte SHA1 hash values
      name = (string) file name
      length = (int64_t) length of file
    */
    
    // Open .torrent file and decode
    std::fstream metaFile;
    metaFile = open(argv[1], std::fstream::in);
    if (metaFile == NULL)
    {
      std::cerr << "Unable to open file!\n";
      std::cerr << "Usage: simple-bt <port> <torrent_file>\n";
      return 1
    }
      
    sbt::MetaInfo metainfo;
    metainfo.wireDecode(metaFile);
    metaFile.close();
    
    // Get info fields
    int64_t pieceLength = metainfo.getPieceLength(); // number of pieces
    std::vector<uint8_t> pieces = metainfo.getPieces(); // concatenated 20-byte SHA1 hash values
    std::string name = metainfo.getName(); // file name
    int64_t length = metainfo.getLength(); // length of file
    
    
    
    /// 2. Check status of downloaded files ///
    
    /// 3. Get tracker info ///

    // (Send HTTP GET request to get peer info)
    
    /// 4. Get peer list and print ///
    
    
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
