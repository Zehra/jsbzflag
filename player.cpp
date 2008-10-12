#include <v8.h>
#include <bzfsAPI.h>

using namespace v8;

Handle<Value> Player_get_id(Local<String> name, const AccessorInfo& info) {
    return info.Holder()->GetInternalField(0);
}


Handle<Value> Player_get_callsign(Local<String> name, const AccessorInfo& info) {
    int id = info.Holder()->GetInternalField(0)->Int32Value();
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);
    if (!record) return Undefined();

    Handle<String> result = String::New(record->callsign.c_str());

    bz_freePlayerRecord(record);
    return result;
}

Handle<Value> Player_get_currentFlag(Local<String> name, const AccessorInfo& info) {
    int id = info.Holder()->GetInternalField(0)->Int32Value();
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);
    if (!record) return Undefined();

    Handle<Value> result;
    if (record->currentFlag.c_str() == NULL) {
        result = Undefined();
    } else {
        result = String::New(record->currentFlag.c_str());
    }

    bz_freePlayerRecord(record);
    return result;
}

Handle<Value> Player_get_spawned(Local<String> name, const AccessorInfo& info) {
    int id = info.Holder()->GetInternalField(0)->Int32Value();
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);
    if (!record) return Undefined();

    Handle<Value> result = Boolean::New(record->spawned);

    bz_freePlayerRecord(record);
    return result;
}
Handle<Value> Player_get_verified(Local<String> name, const AccessorInfo& info) {
    int id = info.Holder()->GetInternalField(0)->Int32Value();
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);
    if (!record) return Undefined();

    Handle<Value> result = Boolean::New(record->verified);

    bz_freePlayerRecord(record);
    return result;
}

Handle<Value> Player_new(const Arguments& args) {
    if (args.IsConstructCall() && args[0]->IsNumber())
        args.This()->SetInternalField(0, args[0]);
    return Undefined();
}


Handle<Function> make_Player_function() {
    HandleScope handle_scope;

    Handle<FunctionTemplate> result = FunctionTemplate::New(Player_new);
    Handle<ObjectTemplate> tmpl = result->InstanceTemplate();

    tmpl->SetInternalFieldCount(1);
  
    // Add accessors for each of the fields of the request.
    tmpl->SetAccessor(String::NewSymbol("id"), Player_get_id);
    tmpl->SetAccessor(String::NewSymbol("callsign"), Player_get_callsign);
    tmpl->SetAccessor(String::NewSymbol("currentFlag"), Player_get_currentFlag);
    tmpl->SetAccessor(String::NewSymbol("spawned"), Player_get_spawned);
    tmpl->SetAccessor(String::NewSymbol("verified"), Player_get_verified);
  
    // Again, return the result through the current handle scope.
    return handle_scope.Close(result->GetFunction());
}

