//
//  peer-protocol.cpp
//  
//
//  Created by L on 2/13/15.
//
//

#include "peer-protocol.h"


//// CONNECT TO EACH PEER

// skip peers that already have connection (whether through connect or accept)
// don't connect to client itself (peer list may have client ip and port)
// more peers may be in subsequent tracker responses


// PEER PROTOCOL

// handshake - include client peerid. see handshake.hpp

// bitfield - initiator sends bitfield to B after handshake, B sends own bitfield back

// unchoke
// interested
    // unchoke/interested: acording to received bitfield, determine if other side has missing piece.
    // if so, send Interested msg. else, do nothing.
    // when receiving Interested msg, return Unchoke msg.
    // when receiving Unchoke msg, start sending Request msg.


// request - one req per piece
    // when receiving Request, read data from file and generate Piece msg
    // NOTE: the “length” of request is set to “piecelength” in the torrent file 
// piece
    // when receiving Piece msg, check piece with hash from .torrent
    // if verified ok, write to file and send Have to all peers. else drop piece and request it again
// have
    // when sender of piece receives Have, sender updates ul'd.
    // when receiver of piece receives Have, updates dl'd

// ignore keep-alive, choke, not interested, cancel
