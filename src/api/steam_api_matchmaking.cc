// Copyright (c) 2016 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <memory>

#include "nan.h"
#include "steam/steam_api.h"
#include "v8.h"

#include "greenworks_async_workers.h"
#include "steam_api_registry.h"
#include "steam_id.h"

namespace greenworks {
namespace api {
namespace {

void InitLobbyType(v8::Handle<v8::Object> exports) {
  v8::Local<v8::Object> lobby_type = Nan::New<v8::Object>();
  SET_TYPE(lobby_type, "Private", k_ELobbyTypePrivate);
  SET_TYPE(lobby_type, "FriendsOnly", k_ELobbyTypeFriendsOnly);
  SET_TYPE(lobby_type, "Public", k_ELobbyTypePublic);
  SET_TYPE(lobby_type, "Invisible", k_ELobbyTypeInvisible);
  Nan::Persistent<v8::Object> constructor;
  constructor.Reset(lobby_type);
  Nan::Set(exports,
           Nan::New("LobbyType").ToLocalChecked(),
           lobby_type);
}

NAN_METHOD(CreateLobby) {
  Nan::HandleScope scope;
  if (info.Length() < 3 || !info[0]->IsInt32() || !info[1]->IsInt32() ||
      !info[2]->IsFunction()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  ELobbyType lobby_type = static_cast<ELobbyType>(info[0]->Int32Value());
  if (lobby_type < k_ELobbyTypePrivate || lobby_type > k_ELobbyTypeInvisible) {
    THROW_BAD_ARGS("Bad arguments");
  }

  int max_members = info[1]->Int32Value();

  Nan::Callback* success_callback =
      new Nan::Callback(info[2].As<v8::Function>());
  Nan::Callback* error_callback = NULL;

  Nan::AsyncQueueWorker(new greenworks::CreateLobbyWorker(
      success_callback, error_callback, lobby_type, max_members));
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(JoinLobby) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsString()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  Nan::Callback* success_callback =
      new Nan::Callback(info[2].As<v8::Function>());
  Nan::Callback* error_callback = NULL;

  Nan::AsyncQueueWorker(new greenworks::JoinLobbyWorker(
      success_callback, error_callback, lobby_id));
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(LeaveLobby) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsString()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  SteamMatchmaking()->LeaveLobby(lobby_id);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(InviteUserToLobby) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsString()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  std::string steam_id_str(*(v8::String::Utf8Value(info[1])));
  CSteamID steam_id(utils::strToUint64(steam_id_str));
  if (!steam_id.IsValid()) {
    THROW_BAD_ARGS("User Steam ID is invalid");
  }

  info.GetReturnValue().Set(
      SteamMatchmaking()->InviteUserToLobby(lobby_id, steam_id));
}

void RegisterAPIs(v8::Handle<v8::Object> exports) {
  InitLobbyType(exports);

  Nan::Set(exports,
           Nan::New("createLobby").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(CreateLobby)->GetFunction());
  Nan::Set(exports,
           Nan::New("joinLobby").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(JoinLobby)->GetFunction());
  Nan::Set(exports,
           Nan::New("leaveLobby").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(LeaveLobby)->GetFunction());
  Nan::Set(exports,
           Nan::New("inviteUserToLobby").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(InviteUserToLobby)->GetFunction());
}

SteamAPIRegistry::Add X(RegisterAPIs);

}  // namespace
}  // namespace api
}  // namespace greenworks
