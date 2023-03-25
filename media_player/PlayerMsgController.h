//
// Created by sivin on 23-1-14.
//

#ifndef SIVINPLAYER_PLAYERMSGCONTROLLER_H
#define SIVINPLAYER_PLAYERMSGCONTROLLER_H

#include "deque"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace Sivin {

  class SivinPlayer;

  enum class PlayerMsgType {
    INVALID = -1,
    SET_DATASOURCE = 0,
    PREPARE,

    INTERNAL_FIRST = 0x100,
    INTERNAL_RENDERED = INTERNAL_FIRST,
    INTERNAL_VIDEO_CLEAN_FRAME,
    INTERNAL_VIDEO_HOLD_ON
  };

  //TODO:这里需要优化
  struct PlayerDataSourceMsg {
    //拥有该资源的所有权
    std::string *url;
  };

  union PlayerMsg {
    PlayerDataSourceMsg dataSource;
  };

  struct QueueMsg {
    PlayerMsgType msgType;
    PlayerMsg msg;
    int64_t msgTime;
  };


  class IPlayerMsgProcessor {
  public:
    virtual ~IPlayerMsgProcessor() = default;

  public:
    virtual void processSetDataSourceMsg(const std::string &url) = 0;

    virtual void processPrepareMsg() = 0;
  };


  //该类主要是负责转发上层发送到player的各种消息
  class PlayerMsgController {
  public:
    explicit PlayerMsgController(IPlayerMsgProcessor &processor);

    ~PlayerMsgController();

    void putMsg(PlayerMsgType type, const PlayerMsg &msg);

    int processMsg();

    bool isContainMsg(PlayerMsgType type);

    bool empty();

    void clear();

  private:
    void recycleMsg(QueueMsg &msg);

    void distributeMsg(PlayerMsgType msgType, const PlayerMsg &msg);

  private:
    std::mutex mMutex;
    std::deque<QueueMsg> mMsgQueue{};

    IPlayerMsgProcessor &mMsgProcessor;
  };
}// namespace Sivin


#endif// SIVINPLAYER_PLAYERMSGCONTROLLER_H
