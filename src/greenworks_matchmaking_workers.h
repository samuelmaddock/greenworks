// Copyright (c) 2014 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SRC_GREENWORKS_MATCHMAKING_WORKERS_H_
#define SRC_GREENWORKS_MATCHMAKING_WORKERS_H_

#include "steam_async_worker.h"

#include <vector>

#include "steam/steam_api.h"

namespace greenworks {

class CreateLobbyWorker : public SteamCallbackAsyncWorker {
 public:
  CreateLobbyWorker(Nan::Callback* success_callback,
                  Nan::Callback* error_callback,
                  ELobbyType lobby_type,
                  int max_members);
  void OnLobbyCreated(LobbyCreated_t* result, bool success);

  // Override Nan::AsyncWorker methods.
  virtual void Execute();
  virtual void HandleOKCallback();

 private:
  ELobbyType lobby_type_;
  int max_members_;
  uint64_t lobby_steam_id_;
  CCallResult<CreateLobbyWorker, LobbyCreated_t> call_result_;
};

class JoinLobbyWorker : public SteamCallbackAsyncWorker {
 public:
  JoinLobbyWorker(Nan::Callback* success_callback,
                  Nan::Callback* error_callback,
                  CSteamID lobby_id);
  void OnLobbyJoined(LobbyEnter_t* result, bool success);

  // Override Nan::AsyncWorker methods.
  virtual void Execute();
  virtual void HandleOKCallback();

 private:
  CSteamID lobby_id_;
  EChatRoomEnterResponse enter_response_;
  CCallResult<JoinLobbyWorker, LobbyEnter_t> call_result_;
};

class RequestLobbyListWorker : public SteamCallbackAsyncWorker {
 public:
  RequestLobbyListWorker(Nan::Callback* success_callback,
                  Nan::Callback* error_callback);
  void OnLobbyMatchList(LobbyMatchList_t* result, bool success);

  // Override Nan::AsyncWorker methods.
  virtual void Execute();
  virtual void HandleOKCallback();

 private:
  uint32 num_lobbies_;
  CCallResult<RequestLobbyListWorker, LobbyMatchList_t> call_result_;
};

}  // namespace greenworks

#endif  // SRC_GREENWORKS_MATCHMAKING_WORKERS_H_
