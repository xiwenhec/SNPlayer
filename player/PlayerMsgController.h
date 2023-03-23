//
// Created by sivin on 23-1-14.
//

#ifndef SIVINPLAYER_PLAYERMSGCONTROLLER_H
#define SIVINPLAYER_PLAYERMSGCONTROLLER_H

#include <cstdint>
#include <string>
#include <mutex>
#include <memory>
#include "deque"

namespace Sivin {

  class SivinPlayer;

  enum class PlayerMsgType {
    INVALID = -1,
    SET_DATASOURCE = 0,

    INTERNAL_FIRST = 0x100,
    INTERNAL_RENDERED = INTERNAL_FIRST,
    INTERNAL_VIDEO_CLEAN_FRAME,
    INTERNAL_VIDEO_HOLD_ON
  };

  struct PlayerDataSourceMsg {
    std::string *url;
  };

  union PlayerMsgContent {
    PlayerDataSourceMsg dataSource;
  };

  struct QueueMsg {
    PlayerMsgType msgType;
    PlayerMsgContent msgContent;
    int64_t msgTime;
  };


  class IPlayerMsgProcessor {
  public:
    virtual ~IPlayerMsgProcessor() = default;

  public:
    virtual void processSetDataSourceMsg(const std::string &url) = 0;

    virtual void processPrepareMsg() = 0;
  };


  class PlayerMsgController {
  public:
    explicit PlayerMsgController(IPlayerMsgProcessor &processor);

    ~PlayerMsgController();

    void putMsg(PlayerMsgType type, const PlayerMsgContent &msgContent);

    int processMsg();

    bool isContainMsg(PlayerMsgType type);

    bool empty();

    void clear();

  private:
    void recycleMsg(QueueMsg &msg);

    void distributeMsg(PlayerMsgType msgType, const PlayerMsgContent &msgContent);

  private:
    std::mutex mMutex;
    std::deque<QueueMsg> mMsgQueue{};

    IPlayerMsgProcessor &mMsgProcessor;
  };
}


#endif //SIVINPLAYER_PLAYERMSGCONTROLLER_H
