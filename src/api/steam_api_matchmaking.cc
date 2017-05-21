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

void InitLobbyComparison(v8::Handle<v8::Object> exports) {
  v8::Local<v8::Object> lobby_comparison = Nan::New<v8::Object>();
  SET_TYPE(lobby_comparison, "EqualToOrLessThan", k_ELobbyComparisonEqualToOrLessThan);
  SET_TYPE(lobby_comparison, "LessThan", k_ELobbyComparisonLessThan);
  SET_TYPE(lobby_comparison, "Equal", k_ELobbyComparisonEqual);
  SET_TYPE(lobby_comparison, "GreaterThan", k_ELobbyComparisonGreaterThan);
  SET_TYPE(lobby_comparison, "EqualToOrGreaterThan", k_ELobbyComparisonEqualToOrGreaterThan);
  SET_TYPE(lobby_comparison, "NotEqual", k_ELobbyComparisonNotEqual);
  Nan::Persistent<v8::Object> constructor;
  constructor.Reset(lobby_comparison);
  Nan::Set(exports,
           Nan::New("LobbyComparison").ToLocalChecked(),
           lobby_comparison);
}

void InitLobbyDistanceFilter(v8::Handle<v8::Object> exports) {
  v8::Local<v8::Object> lobby_distance = Nan::New<v8::Object>();
  SET_TYPE(lobby_distance, "Close", k_ELobbyDistanceFilterClose);
  SET_TYPE(lobby_distance, "Default", k_ELobbyDistanceFilterDefault);
  SET_TYPE(lobby_distance, "Far", k_ELobbyDistanceFilterFar);
  SET_TYPE(lobby_distance, "Worldwide", k_ELobbyDistanceFilterWorldwide);
  Nan::Persistent<v8::Object> constructor;
  constructor.Reset(lobby_distance);
  Nan::Set(exports,
           Nan::New("LobbyDistanceFilter").ToLocalChecked(),
           lobby_distance);
}

NAN_METHOD(RequestLobbyList) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsObject() ||
    !info[1]->IsFunction()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  auto options = info[0]->ToObject();

  auto filters_str = Nan::New("filters").ToLocalChecked();
  auto near_filters_str = Nan::New("nearFilters").ToLocalChecked();
  auto slots_str = Nan::New("slots").ToLocalChecked();
  auto distance_str = Nan::New("distance").ToLocalChecked();
  auto count_str = Nan::New("count").ToLocalChecked();
  auto members_str = Nan::New("compatibleMembers").ToLocalChecked();

  auto key_str = Nan::New("key").ToLocalChecked();
  auto value_str = Nan::New("value").ToLocalChecked();
  auto comparator_str = Nan::New("comparator").ToLocalChecked();

  if (options->HasOwnProperty(filters_str)) {
    auto filters = Nan::Get(options, filters_str).ToLocalChecked();

    if (!filters->IsArray()) {
      THROW_BAD_ARGS("Bad property 'filters', must be an array");
    }

    auto filters_array = v8::Local<v8::Array>::Cast(filters);

    for (unsigned int i = 0; i < filters_array->Length(); i++) {
      v8::Handle<v8::Value> filter = filters_array->Get(i);

      if (!filter->IsObject()) {
        continue;
      }

      auto filter_obj = v8::Local<v8::Object>::Cast(filter);

      if (!filter_obj->HasOwnProperty(key_str) ||
          !filter_obj->HasOwnProperty(value_str) ||
          !filter_obj->HasOwnProperty(comparator_str)) {
        THROW_BAD_ARGS("Invalid filter object");
      }

      auto key = Nan::Get(filter_obj, key_str).ToLocalChecked();
      auto value = Nan::Get(filter_obj, value_str).ToLocalChecked();
      auto comparator = Nan::Get(filter_obj, comparator_str).ToLocalChecked();

      if (!key->IsString() || !(value->IsString() || value->IsInt32()) ||
          !comparator->IsInt32()) {
        THROW_BAD_ARGS("Invalid filter properties");
      }

      auto comparator_enum = static_cast<ELobbyComparison>(comparator->Int32Value());
      std::string key_string(*(v8::String::Utf8Value(key)));

      if (value->IsString()) {
        std::string value_string(*(v8::String::Utf8Value(value)));
        SteamMatchmaking()->AddRequestLobbyListStringFilter(
          key_string.c_str(), value_string.c_str(), comparator_enum);
      } else {
        SteamMatchmaking()->AddRequestLobbyListNumericalFilter(
          key_string.c_str(), value->Int32Value(), comparator_enum);
      }
    }
  }

  if (options->HasOwnProperty(near_filters_str)) {
    auto filters = Nan::Get(options, near_filters_str).ToLocalChecked();

    if (!filters->IsArray()) {
      THROW_BAD_ARGS("Bad property 'nearFilters', must be an array");
    }

    auto filters_array = v8::Local<v8::Array>::Cast(filters);

    for (unsigned int i = 0; i < filters_array->Length(); i++) {
      v8::Handle<v8::Value> filter = filters_array->Get(i);

      if (!filter->IsObject()) {
        continue;
      }

      auto filter_obj = v8::Local<v8::Object>::Cast(filter);

      if (!filter_obj->HasOwnProperty(key_str) ||
          !filter_obj->HasOwnProperty(value_str)) {
        THROW_BAD_ARGS("Invalid near filter object");
      }

      auto key = Nan::Get(filter_obj, key_str).ToLocalChecked();
      auto value = Nan::Get(filter_obj, value_str).ToLocalChecked();

      if (!key->IsString() || !value->IsInt32()) {
        THROW_BAD_ARGS("Invalid near filter properties");
      }

      std::string key_string(*(v8::String::Utf8Value(key)));

      SteamMatchmaking()->AddRequestLobbyListNearValueFilter(
        key_string.c_str(), value->Int32Value());
    }
  }

  if (options->HasOwnProperty(slots_str)) {
    auto slots = Nan::Get(options, slots_str).ToLocalChecked();

    if (!slots->IsInt32()) {
      THROW_BAD_ARGS("Invalid slots filter");
    }

    SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(
      slots->Int32Value());
  }

  if (options->HasOwnProperty(distance_str)) {
    auto distance = Nan::Get(options, distance_str).ToLocalChecked();

    if (!distance->IsInt32()) {
      THROW_BAD_ARGS("Invalid distance filter");
    }

    auto distance_enum = static_cast<ELobbyDistanceFilter>(distance->Int32Value());
    SteamMatchmaking()->AddRequestLobbyListDistanceFilter(distance_enum);
  }

  if (options->HasOwnProperty(count_str)) {
    auto count = Nan::Get(options, count_str).ToLocalChecked();

    if (!count->IsInt32()) {
      THROW_BAD_ARGS("Invalid count filter");
    }

    SteamMatchmaking()->AddRequestLobbyListResultCountFilter(
      count->Int32Value());
  }

  if (options->HasOwnProperty(members_str)) {
    auto members = Nan::Get(options, members_str).ToLocalChecked();

    if (!members->IsString()) {
      THROW_BAD_ARGS("Invalid compatible members filter");
    }

    std::string lobby_id_str(*(v8::String::Utf8Value(members)));
    CSteamID lobby_id(utils::strToUint64(lobby_id_str));
    if (!lobby_id.IsValid()) {
      THROW_BAD_ARGS("Invalid compatible members filter");
    }

    SteamMatchmaking()->AddRequestLobbyListCompatibleMembersFilter(
      lobby_id);
  }

  Nan::Callback* success_callback =
      new Nan::Callback(info[1].As<v8::Function>());
  Nan::Callback* error_callback = NULL;

  Nan::AsyncQueueWorker(new greenworks::RequestLobbyListWorker(
      success_callback, error_callback));
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetLobbyByIndex) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  int lobby_idx = info[0]->Int32Value();

  CSteamID lobby_id = SteamMatchmaking()->GetLobbyByIndex(lobby_idx);
  info.GetReturnValue().Set(greenworks::SteamID::Create(lobby_id));
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

NAN_METHOD(GetLobbyDataCount) {
  Nan::HandleScope scope;
  if (info.Length() < 1 || !info[0]->IsString()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  info.GetReturnValue().Set(
      SteamMatchmaking()->GetLobbyDataCount(lobby_id));
}

NAN_METHOD(GetLobbyDataByIndex) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() ||
      !info[1]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  int lobby_data_idx = info[1]->Int32Value();

  char key_buf[256];
  char val_buf[256];
  int buf_len = 256;

  bool success = SteamMatchmaking()->GetLobbyDataByIndex(lobby_id,
    lobby_data_idx, &key_buf[0], buf_len, &val_buf[0], buf_len);

  if (!success) {
    THROW_BAD_ARGS("Error getting lobby data");
  }

  v8::Local<v8::Array> data_pair = Nan::New<v8::Array>(2);
  data_pair->Set(0, Nan::New<v8::String>(&key_buf[0]).ToLocalChecked());
  data_pair->Set(1, Nan::New<v8::String>(&val_buf[0]).ToLocalChecked());

  info.GetReturnValue().Set(data_pair);
}

NAN_METHOD(SendLobbyChatMsg) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() ||
      !node::Buffer::HasInstance(info[1])) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  char* msg_buf = node::Buffer::Data(info[1]);
  size_t msg_buf_size = node::Buffer::Length(info[1]);

  info.GetReturnValue().Set(
      SteamMatchmaking()->SendLobbyChatMsg(lobby_id, msg_buf, msg_buf_size));
}


NAN_METHOD(GetLobbyChatEntry) {
  Nan::HandleScope scope;
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsInt32()) {
    THROW_BAD_ARGS("Bad arguments");
  }

  std::string lobby_id_str(*(v8::String::Utf8Value(info[0])));
  CSteamID lobby_id(utils::strToUint64(lobby_id_str));
  if (!lobby_id.IsValid()) {
    THROW_BAD_ARGS("Lobby Steam ID is invalid");
  }

  int chat_id = info[1]->Int32Value();

  CSteamID chat_steam_id;
  EChatEntryType chat_entry_type;

  char buf[4096];
  int buf_size = 4096; // max chat msg

  int num_bytes = SteamMatchmaking()->GetLobbyChatEntry(lobby_id, chat_id, &chat_steam_id,
    &buf, buf_size, &chat_entry_type);

  auto msg_buf = Nan::CopyBuffer(buf, num_bytes);
  auto chat_steam_id_str = utils::uint64ToString(chat_steam_id.ConvertToUint64());

  v8::Local<v8::Object> chat_entry = Nan::New<v8::Object>();
  Nan::Set(chat_entry,
    Nan::New("steamId").ToLocalChecked(),
    Nan::New(chat_steam_id_str).ToLocalChecked());
  Nan::Set(chat_entry,
    Nan::New("message").ToLocalChecked(),
    msg_buf.ToLocalChecked());

  info.GetReturnValue().Set(chat_entry);
}

void RegisterAPIs(v8::Handle<v8::Object> exports) {
  InitLobbyType(exports);
  InitLobbyComparison(exports);
  InitLobbyDistanceFilter(exports);

  Nan::Set(exports,
           Nan::New("requestLobbyList").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(RequestLobbyList)->GetFunction());
  Nan::Set(exports,
           Nan::New("getLobbyByIndex").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(GetLobbyByIndex)->GetFunction());
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
  Nan::Set(exports,
           Nan::New("getLobbyDataCount").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(GetLobbyDataCount)->GetFunction());
  Nan::Set(exports,
           Nan::New("getLobbyDataByIndex").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(GetLobbyDataByIndex)->GetFunction());
  Nan::Set(exports,
           Nan::New("sendLobbyChatMsg").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(SendLobbyChatMsg)->GetFunction());
  Nan::Set(exports,
           Nan::New("getLobbyChatEntry").ToLocalChecked(),
           Nan::New<v8::FunctionTemplate>(GetLobbyChatEntry)->GetFunction());
}

SteamAPIRegistry::Add X(RegisterAPIs);

}  // namespace
}  // namespace api
}  // namespace greenworks
