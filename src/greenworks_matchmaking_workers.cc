// Copyright (c) 2014 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "greenworks_matchmaking_workers.h"

#include <algorithm>

#include "nan.h"
#include "steam/steam_api.h"
#include "v8.h"

#include "greenworks_utils.h"

namespace greenworks {

CreateLobbyWorker::CreateLobbyWorker(
    Nan::Callback* success_callback,
    Nan::Callback* error_callback,
    ELobbyType lobby_type,
    int max_members)
        :SteamCallbackAsyncWorker(success_callback, error_callback),
        lobby_type_(lobby_type),
        max_members_(max_members) {
}

void CreateLobbyWorker::Execute() {
  SteamAPICall_t lobby_result = SteamMatchmaking()->CreateLobby(lobby_type_, max_members_);
  call_result_.Set(lobby_result, this, &CreateLobbyWorker::OnLobbyCreated);

  WaitForCompleted();
}

void CreateLobbyWorker::OnLobbyCreated(
    LobbyCreated_t* result,
    bool success) { // TODO: what is this bool?
  if (result->m_eResult == k_EResultOK) {
    lobby_steam_id_ = result->m_ulSteamIDLobby;
  } else {
    SetErrorMessage("Error on creating Steam matchmaking lobby.");
  }
  is_completed_ = true;
}

void CreateLobbyWorker::HandleOKCallback() {
  Nan::HandleScope scope;

  v8::Local<v8::Value> argv[] = {
      Nan::New(utils::uint64ToString(lobby_steam_id_)).ToLocalChecked() };
  callback->Call(1, argv);
}

}  // namespace greenworks
