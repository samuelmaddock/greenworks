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


JoinLobbyWorker::JoinLobbyWorker(
    Nan::Callback* success_callback,
    Nan::Callback* error_callback,
    CSteamID lobby_id)
        :SteamCallbackAsyncWorker(success_callback, error_callback),
        lobby_id_(lobby_id) {
}

void JoinLobbyWorker::Execute() {
  SteamAPICall_t lobby_result = SteamMatchmaking()->JoinLobby(lobby_id_);
  call_result_.Set(lobby_result, this, &JoinLobbyWorker::OnLobbyJoined);

  WaitForCompleted();
}

void JoinLobbyWorker::OnLobbyJoined(
    LobbyEnter_t* result,
    bool success) { // TODO: what is this bool?
  if (result->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess) {
    enter_response_ = static_cast<EChatRoomEnterResponse>(result->m_EChatRoomEnterResponse);
  } else {
    SetErrorMessage("Error on joining Steam matchmaking lobby.");
  }
  is_completed_ = true;
}

void JoinLobbyWorker::HandleOKCallback() {
  Nan::HandleScope scope;

  v8::Local<v8::Value> argv[] = {
    Nan::New(enter_response_) };
  callback->Call(1, argv);
}


RequestLobbyListWorker::RequestLobbyListWorker(
    Nan::Callback* success_callback,
    Nan::Callback* error_callback)
        :SteamCallbackAsyncWorker(success_callback, error_callback) {
}

void RequestLobbyListWorker::Execute() {
  SteamMatchmaking()->AddRequestLobbyListResultCountFilter(50);
  SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);
  
  SteamAPICall_t match_result = SteamMatchmaking()->RequestLobbyList();
  call_result_.Set(match_result, this, &RequestLobbyListWorker::OnLobbyMatchList);

  WaitForCompleted();
}

void RequestLobbyListWorker::OnLobbyMatchList(
    LobbyMatchList_t* result,
    bool ioFailure) {
  if (result) {
    num_lobbies_ = result->m_nLobbiesMatching;
  } else {
    SetErrorMessage("Error requesting Steam matchmaking lobbies.");
  }
  is_completed_ = true;
}

void RequestLobbyListWorker::HandleOKCallback() {
  Nan::HandleScope scope;

  v8::Local<v8::Value> argv[] = {
    Nan::New(num_lobbies_) };
  callback->Call(1, argv);
}

}  // namespace greenworks
