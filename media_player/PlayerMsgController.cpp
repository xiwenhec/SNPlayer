//
// Created by sivin on 23-1-14.
//

#include "PlayerMsgController.h"
#include "utils/SNLog.h"
#include <algorithm>

namespace Sivin {

#define REPLACE_NONE 0
#define REPLACE_ALL (-1)
#define REPLACE_LAST (-2)

#define ADD_LOCK \
  std::unique_lock<std::mutex> lock { mMutex }

  static int getReplaceType(PlayerMsgType type) {
    switch (type) {
      case PlayerMsgType::SET_DATASOURCE:
      case PlayerMsgType::PREPARE:
        return REPLACE_ALL;
      default:
        return REPLACE_NONE;
    }
  }

  PlayerMsgController::PlayerMsgController(IPlayerMsgProcessor &processor) : mMsgProcessor(processor) {}

  PlayerMsgController::~PlayerMsgController() {}

  void PlayerMsgController::putMsg(PlayerMsgType type, const PlayerMsg &msg) {
    QueueMsg queueMsg{};
    queueMsg.msgType = type;
    queueMsg.msg = msg;

    int replaceType = getReplaceType(type);
    ADD_LOCK;
    switch (replaceType) {
      case REPLACE_ALL://删除掉消息队列中c重复的所有消息，只留下当前新插入的消息
        for (auto iter = mMsgQueue.begin(); iter != mMsgQueue.end();) {
          if (iter->msgType == type) {
            recycleMsg(*iter);
            iter = mMsgQueue.erase(iter);
          } else {
            ++iter;
          }
        }
        break;
      case REPLACE_LAST:
        if (!mMsgQueue.empty() && mMsgQueue.back().msgType == type) {
          recycleMsg(mMsgQueue.back());
          mMsgQueue.pop_back();
        }
        break;
      default:
        break;
    }
    mMsgQueue.push_back(queueMsg);
  }

  void PlayerMsgController::recycleMsg(QueueMsg &msg) {
    if (msg.msgType == PlayerMsgType::SET_DATASOURCE) {
      delete msg.msg.dataSource.url;
      msg.msg.dataSource.url = nullptr;
    }
  }

  /**
    分发处理消息队列里的消息，返回处理的消息个数
  */
  int PlayerMsgController::processMsg() {
    //首先将所有待处理的消息，转移到一个新的消息队列中，这样在消息处理期间不影响新的消息加入
    std::deque<QueueMsg> processQueue{};

    ADD_LOCK;
    for (auto iter = mMsgQueue.begin(); iter != mMsgQueue.end();) {
      processQueue.push_back(*iter);
      iter = mMsgQueue.erase(iter);
    }
    lock.unlock();
    //一次性处理所有消息
    int count = 0;
    for (auto &queueMsg: processQueue) {
      distributeMsg(queueMsg.msgType, queueMsg.msg);
      recycleMsg(queueMsg);
      if (queueMsg.msgType < PlayerMsgType::INTERNAL_FIRST) {
        count++;
      }
    }
    processQueue.clear();
    return count;
  }

  void PlayerMsgController::distributeMsg(PlayerMsgType msgType, const PlayerMsg &msg) {
    switch (msgType) {
      case PlayerMsgType::SET_DATASOURCE:
        mMsgProcessor.processSetDataSourceMsg(*msg.dataSource.url);
        break;
      case PlayerMsgType::PREPARE:
        mMsgProcessor.processPrepareMsg();
        break;
      default:
        break;
    }
  }

  bool PlayerMsgController::isContainMsg(PlayerMsgType type) {
    ADD_LOCK;
    return std::ranges::any_of(mMsgQueue.begin(), mMsgQueue.end(),
                               [type](QueueMsg &msg) { return type == msg.msgType; });
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

}// namespace Sivin