//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_FRAMEWORKERROR_H
#define SIVINPLAYER_FRAMEWORKERROR_H

#define SNRET_SUCCESS 0

#define SNRET_ERROR -1

#define SNRET_AGAIN -100

namespace Sivin {
  class SNRet {
  public:
    enum class Status {
      SUCCESS = 0,
      ERROR = -30,
      AGAIN,
      EOS,
    };

  public:
    SNRet(Status st = Status::SUCCESS, int val_ = 0) : status(st), val(val_) {}

    bool operator==(const SNRet &ret) {
      return ret.status == status;
    }

    bool operator!=(const SNRet &ret) {
      return ret.status != status;
    }

  public:
    Status status;
    int val{0};
  };

  enum NetworkError {

  };
}// namespace Sivin

#endif//SIVINPLAYER_FRAMEWORKERROR_H
