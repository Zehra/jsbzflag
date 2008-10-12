/* These are functions that pretty much directly wrap the C++ api, accessable 
 * in javascript by the _bz object.  Most of these are wrapped in some way or
 * another in stdlib.js, so they generally aren't used directly by plugins.
 */

#include <v8.h>
#include <bzfsAPI.h>

using namespace v8;

Handle<Value> js_getCurrentTime(const Arguments& args) {
    return Number::New(bz_getCurrentTime());
}

Handle<Value> js_sendTextMessage(const v8::Arguments& args) {
    HandleScope handle_scope;
    int from = args[0]->Int32Value();
    int to = args[1]->Int32Value();
    String::Utf8Value str(args[2]);
    bz_sendTextMessagef(from, to, "%s", *str);
    return v8::Undefined();
}

Handle<Value> js_givePlayerFlag(const v8::Arguments& args) {
    HandleScope handle_scope;
    int playerId = args[0]->Int32Value();
    String::AsciiValue flagType(args[1]);
    bool force = args[2]->BooleanValue();
    if (bz_givePlayerFlag(playerId, *flagType, force))
        return True();
    else
        return False();
}

Handle<Value> js_removePlayerFlag(const v8::Arguments& args) {
    HandleScope handle_scope;
    int playerId = args[0]->Int32Value();
    if (bz_removePlayerFlag(playerId))
        return True();
    else
        return False();
}

Handle<Value> js_killPlayer(const v8::Arguments& args) {
    HandleScope handle_scope;
    int playerId = args[0]->Int32Value();
    bool spawnOnBase = args.Length() > 1 ? args[1]->BooleanValue() : false;
    int killerId = args.Length() > 2 ? args[2]->Int32Value() : -1;
    char* flagType = args.Length() > 3 ? *String::AsciiValue(args[3]) : NULL;
    if (bz_killPlayer(playerId, spawnOnBase, killerId, flagType))
        return True();
    else
        return False();
}

Handle<Object> make_bz_object() {
    Handle<Object> bz = Object::New();

    #define FUNCTION(name) bz->Set(String::NewSymbol(#name), FunctionTemplate::New(js_##name)->GetFunction())
    FUNCTION(getCurrentTime);
    FUNCTION(sendTextMessage);
    FUNCTION(givePlayerFlag);
    FUNCTION(removePlayerFlag);
    FUNCTION(killPlayer);
    #undef FUNCTION
    return bz;
}

