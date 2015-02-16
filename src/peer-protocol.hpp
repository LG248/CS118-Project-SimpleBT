//
//  peer-protocol.hpp
//  Peer protocol part of client (like being both server and client)
//
//  \author LG248
//
//

#ifndef ____peer_protocol__
#define ____peer_protocol__

#include "common.hpp"
#include "tracker-response.hpp" // PeerInfo class
#include "handshake.hpp"
#include "msg-base.hpp" // all other msgs
#include <iostream>

namespace sbt {
namespace msg {
public:
ClientPeer(const std::string& port,
       const std::string& torrent);

void
run(); // runs all the peer protocol stuff

// TODO need to listen() for incoming somewhere
//    and do the other server stuff

// any getters go here, ex/
//const std::string& getTrackerHost() {
//  return m_trackerHost;
//}


private:
  
  // checks whether there is already a connection to some peer
  bool alreadyConnected(PeerInfo peerinfo);
  
  // checks that peer isn't client itself
  bool notSelf(PeerInfo peerinfo);
  
  // update relevant data members when receiving a handshake
  // also returns filled in HandShake obj
  // decide whether to drop connection here?
  HandShake parseHandshake(IDKType stuff_recv_from_peer);

  HandShake recHandshake; // handshake received from peer
  peerHandShake.setInfoHash(m_metaInfo.getHash()); // TODO pass metainfo as param to ClientPeer(), add data members
  peerHandShake.setPeerId("SIMPLEBT.TEST.PEERID");
  ConstBufferPtr message = recHandshake.encode();
  const char* msgbuf = reinterpret_cast<const char*>(message->buf());
  ??? = handShake.decode(msgbuf);
  
  // send handshake to a peer
  void sendHandshake(HandShake handshake, PeerInfo peer);

  // parse messages with length, type, payload into MsgBody
  MsgBody parseMsg(IDKType stuff_recv_from_peer);

  // respond appropriately to each type of msg (bitfield, unchoke, etc)
  void processMsg(MsgBase msgbody); //
  //void processKeepAlive(); // don't handle keepalive
  void processChoke(Choke chokeBase);
  void processUnchoke(Unchoke unchokeBase);
  void processInterested(Interested intrBase);
  void processNotInterested(NotInterested nintrBase);
  void processHave(Have haveBase);
  void processBitfield(Bitfield bitfBase);
  void processRequest(Request reqMsgBase);
  
  // sends specified message to a peer
  void sendMsg(MsgBase msgObj, PeerInfo peer);


private:
  
  // info from main.cpp (tracker/metainfo)
  MetaInfo metainfo;
  std::string trackerHost;
  uint8_t trackerPort;
  std::String trackerFile;
  
  
  
  bool isClient; // if true, send handshake first. if false, wait for handshake.

  bool isHandshakeRec; // if has received handshake (should be first msg received)
  bool hasHandshaked; // if client has sent handshake
  bool hasBitfield;   // only get one bitfield, subsequent are error

  bool amInterested = 0;
  bool amChoking = 1; // i.e. is client choking peer
  bool peerInterested;
  bool peerChoking; // i.e. is client being choked by peer

  ??? outstandingRequests // use to check if incoming piece is the one requested



};
}} // namespace msg and sbt

#endif /* defined(____peer_protocol__) */
