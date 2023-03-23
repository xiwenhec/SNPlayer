//
// Created by sivin on 23-1-14.
//

#include <algorithm>
#include "PlayerMsgController.h"

namespace Sivin {

#define REPLACE_NONE 0
#define REPLACE_ALL (-1)
#define REPLACE_LAST (-2)
#define SEEK_REPEAT_TIME 500 //ms
#define ADD_LOCK std::unique_lock<std::mutex> lock{mMutex}

  static int getRepeatTimeMS(PlayerMsgType type) {
    switch (type) {
      case PlayerMsgType::SET_DATASOURCE:
        return REPLACE_ALL;
      default:
        return REPLACE_NONE;
    }
  }

  PlayerMsgController::PlayerMsgController(IPlayerMsgProcessor &processor) : mMsgProcessor(processor) {
  }

  PlayerMsgController::~PlayerMsgController() {
  }

  void PlayerMsgController::putMsg(PlayerMsgType type, const PlayerMsgContent &msgContent) {
    QueueMsg msg{};
    msg.msgType = type;
    msg.msgContent = msgContent;
    int repeatTime = getRepeatTimeMS(type);

    ADD_LOCK;
    switch (repeatTime) {
      case REPLACE_NONE:
        for (auto iter = mMsgQueue.begin(); iter != mMsgQueue.end(); ++iter) {
          if (iter->msgType == PlayerMsgType::SET_DATASOURCE) {
            recycleMsg(*iter);
            mMsgQueue.erase(iter);
          }
        }
        break;
      default:
        break;
    }
    mMsgQueue.push_back(msg);
  }

  void PlayerMsgController::recycleMsg(QueueMsg &msg) {
    if (msg.msgType == PlayerMsgType::SET_DATASOURCE) {
      delete msg.msgContent.dataSource.url;
      msg.msgContent.dataSource.url = nullptr;
    }
  }

  int PlayerMsgController::processMsg() {
    std::deque<QueueMsg> processQueue{};
    ADD_LOCK;
    for (auto iter = mMsgQueue.begin(); iter != mMsgQueue.end(); ++iter) {
      processQueue.push_back(*iter);
      mMsgQueue.erase(iter);
    }
    lock.unlock();

    int count = 0;
    for (auto &msg: processQueue) {
      distributeMsg(msg.msgType, msg.msgContent);
      recycleMsg(msg);
      if (msg.msgType < PlayerMsgType::INTERNAL_FIRST) {
        count++;
      }
    }

    processQueue.clear();

    return count;
  }

  void PlayerMsgController::distributeMsg(PlayerMsgType msgType, const PlayerMsgContent &msgContent) {
    switch (msgType) {
      case PlayerMsgType::SET_DATASOURCE:
        mMsgProcessor.processSetDataSourceMsg(*msgContent.dataSource.url);
        break;
      default:
        break;
    }
  }

  bool PlayerMsgController::isContainMsg(PlayerMsgType type) {
    ADD_LOCK;
    return std::ranges::any_of(mMsgQueue.begin(), mMsgQueue.end(),
                               [type](QueueMsg &msg) {
                                 return type == msg.msgType;
                               });
  }

  bool PlayerMsgController::empty() {
    ADD_LOCK;
    return mMsgQueue.empty();
  }

  void PlayerMsgController::clear() {
    ADD_LOCK;
    for (auto &msg: mMsgQueue) {
      recycleMsg(msg);
    }
    mMsgQueue.clear();
  }


}