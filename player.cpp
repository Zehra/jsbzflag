// Copyright 2008 by Matthew Marshall <matthew@matthewmarshall.org>
// License: GPL

#include <v8.h>
#include <bzfsAPI.h>

using namespace v8;


Handle<Value> build_pos(float * pos) {
    Handle<Array> p = Array::New(3);
    for (int i=0;i<3;i++) 
        p->Set(Integer::New(i), Number::New(pos[i]));
    return p;
}

Handle<Value> build_string(bzApiString & s){
    if (s == NULL || s.c_str() == NULL) 
        return Undefined();
    return String::New(s.c_str());
}

Handle<Value> Player_get_id(Local<String> name, const AccessorInfo& info) {
    return info.Holder()->GetInternalField(0);
}

Handle<Value> Player_get_currentFlag(Local<String> name, const AccessorInfo& info) {
    int id = info.Holder()->GetInternalField(0)->Int32Value();
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);
    if (!record) return Undefined();

    Handle<Value> result = build_string(record->currentFlag);

    bz_freePlayerRecord(record);
    return result;
}

#define GETTER(func_name, expression) \
Handle<Value> func_name(Local<String> name, const AccessorInfo& info) {\
    int id = info.Holder()->GetInternalField(0)->Int32Value();\
    bz_PlayerRecord *record = bz_getPlayerByIndex(id);\
    if (!record) return Undefined();\
    Handle<Value> result = (expression);\
    bz_freePlayerRecord(record);\
    return result;\
}
#define BOOL_GETTER(member) GETTER(Player_get_##member, (Boolean::New(record->member)))
#define INT_GETTER(member) GETTER(Player_get_##member, (Integer::New(record->member)))
#define FLOAT_GETTER(member) GETTER(Player_get_##member, (Number::New(record->member)))
#define STRING_GETTER(member) GETTER(Player_get_##member, (build_string(record->member)))

STRING_GETTER(callsign);
STRING_GETTER(ipAddress);
STRING_GETTER(email);
FLOAT_GETTER(rot);
GETTER(Player_get_pos, (build_pos(record->pos)));
BOOL_GETTER(spawned);
BOOL_GETTER(verified);
BOOL_GETTER(globalUser);
BOOL_GETTER(admin);
BOOL_GETTER(op);
INT_GETTER(lag);
INT_GETTER(wins);
INT_GETTER(losses);
INT_GETTER(teamKills);
INT_GETTER(team);

// TODO properties: team, flagHistory, groups

#undef BOOL_GETTER
#undef INT_GETTER
#undef FLOAT_GETTER
#undef STRING_GETTER
#undef GETTER

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
    tmpl->SetAccessor(String::NewSymbol("ipAddress"), Player_get_ipAddress);
    tmpl->SetAccessor(String::NewSymbol("email"), Player_get_email);
    tmpl->SetAccessor(String::NewSymbol("currentFlag"), Player_get_currentFlag);
    tmpl->SetAccessor(String::NewSymbol("spawned"), Player_get_spawned);
    tmpl->SetAccessor(String::NewSymbol("verified"), Player_get_verified);
    tmpl->SetAccessor(String::NewSymbol("globalUser"), Player_get_globalUser);
    tmpl->SetAccessor(String::NewSymbol("admin"), Player_get_admin);
    tmpl->SetAccessor(String::NewSymbol("op"), Player_get_op);
    tmpl->SetAccessor(String::NewSymbol("lag"), Player_get_lag);
    tmpl->SetAccessor(String::NewSymbol("wins"), Player_get_wins);
    tmpl->SetAccessor(String::NewSymbol("losses"), Player_get_losses);
    tmpl->SetAccessor(String::NewSymbol("teamKills"), Player_get_teamKills);
    tmpl->SetAccessor(String::NewSymbol("rot"), Player_get_rot);
    tmpl->SetAccessor(String::NewSymbol("pos"), Player_get_pos);
    tmpl->SetAccessor(String::NewSymbol("teamID"), Player_get_team);
  
    // Again, return the result through the current handle scope.
    return handle_scope.Close(result->GetFunction());
}

