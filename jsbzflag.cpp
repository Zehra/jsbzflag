#include "bzfsAPI.h"
#include "stdio.h"
#include <v8.h>

BZ_GET_PLUGIN_VERSION

using namespace v8;

#define new_str String::NewSymbol


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


void ReportException(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope;
  v8::String::Utf8Value exception(try_catch->Exception());
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    printf("%s\n", *exception);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptResourceName());
    int linenum = message->GetLineNumber();
    printf("%s:%i: %s\n", *filename, linenum, *exception);
    // Print line of source code.
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    printf("%s\n", *sourceline);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      printf(" ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      printf("^");
    }
    printf("\n");
  }
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result = v8::String::New(chars, size);
  delete[] chars;
  return result;
}

v8::Handle<v8::Value> js_print(const v8::Arguments& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    printf("%s", *str);
  }
  printf("\n");
  return v8::Undefined();
}


Handle<Value> js_getCurrentTime(const Arguments& args) {
    return Number::New(bz_getCurrentTime());
}

bool write_pos(Handle<Object> obj, Handle<Value> name, float * pos) {
    Handle<Array> p = Array::New(3);
    for (int i=0;i<3;i++) {
        p->Set(Integer::New(i), Number::New(pos[i]));
    }
    obj->Set(name, p);
    return true;
}

bool read_pos(Handle<Object> obj, Handle<Value> name, float * pos) {
    Handle<Value> pos_value = obj->Get(name);
    if (!pos_value->IsObject()) return false;
    Handle<Object> pos_object = pos_value->ToObject();
    for (int i=0;i<3;i++) {
        Handle<Value> v = pos_object->Get(Integer::New(i));
        if (v->IsNumber())
            pos[i] = v->NumberValue();
    }
    obj->Set(name, pos_object);
    return true;
}


bool write_bool(Handle<Object> obj, Handle<Value> name, bool value) {
    return obj->Set(name, Boolean::New(value));
}
bool read_bool(Handle<Object> obj, Handle<Value> name, bool &value) {
    Handle<Value> v = obj->Get(name);
    value = v->BooleanValue();
    return true;
}

bool write_float(Handle<Object> obj, Handle<Value> name, float value) {
    return obj->Set(name, Number::New(value));
}
bool read_float(Handle<Object> obj, Handle<Value> name, float &value) {
    Handle<Value> v = obj->Get(name);
    value = v->NumberValue(); // FIXME what happens if it can't be converted to a number?
    return true;
}

v8::Handle<v8::Value> js_sendTextMessage(const v8::Arguments& args) {
    v8::HandleScope handle_scope;
    int from = args[0]->Int32Value();
    int to = args[1]->Int32Value();
    String::Utf8Value str(args[2]);
    bz_sendTextMessagef(from, to, "%s", *str);
    return v8::Undefined();
}

class JS_Plugin : public bz_EventHandler
{
    public:
        //GenericEventHandler();
        //virtual ~JS_Plugin();

        virtual void process(bz_EventData * event_data);

        virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event

  bool initialize();
  void uninitialize();
  bool load_source(v8::Handle<v8::String> source, Handle<Value> file_name);
  bool load_file(char * filename);

  bool call_event(char * event_name, v8::Handle<v8::Value> data);

  Handle<Value> get_player(int player_id);

  v8::HandleScope handle_scope;
  v8::Persistent<v8::Context> context;
};


Handle<Value> JS_Plugin::get_player(int player_id) {
    // I'm not sure if there is any use for this function anymore...
    HandleScope handle_scope;
    v8::Handle<v8::Value> players_array = context->Global()->Get(new_str("players"));
    if (!players_array->IsObject()) return Undefined(); // TODO raise errors instead;
    v8::Handle<v8::Value> function_value = players_array->ToObject()->Get(new_str("get"));
    if (!function_value->IsFunction()) return Undefined();
    v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(function_value);

    const int argc = 1;
    v8::Handle<v8::Value> argv[argc] = {Number::New(player_id)};
    v8::Handle<v8::Value> result = function->Call(players_array->ToObject(), argc, argv);
    return handle_scope.Close(result);
}

bool JS_Plugin::initialize() {

    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    global->Set(v8::String::New("print"), v8::FunctionTemplate::New(js_print));
    global->Set(v8::String::New("getCurrentTime"), v8::FunctionTemplate::New(js_getCurrentTime));
    global->Set(v8::String::New("sendTextMessage"), v8::FunctionTemplate::New(js_sendTextMessage));
    context = v8::Context::New(NULL, global);  // TODO Dispose

    v8::Context::Scope context_scope(context);
    context->Global()->Set(String::NewSymbol("Player"), make_Player_function());
    if (!load_file("stdlib.js"))
        return false;

    bz_registerEvent(bz_eGetPlayerSpawnPosEvent,this);
    bz_registerEvent(bz_eUnknownSlashCommand,this);
    bz_registerEvent(bz_ePlayerJoinEvent,this);
    bz_registerEvent(bz_ePlayerPartEvent,this);
    bz_registerEvent(bz_ePlayerDieEvent,this);

    return true;
}

void JS_Plugin::uninitialize() {
    bz_removeEvent(bz_eGetPlayerSpawnPosEvent, this);
    bz_removeEvent(bz_eUnknownSlashCommand, this);
    bz_removeEvent(bz_ePlayerJoinEvent, this);
    bz_removeEvent(bz_ePlayerPartEvent, this);
    bz_removeEvent(bz_ePlayerDieEvent, this);
}

bool JS_Plugin::load_source(v8::Handle<v8::String> source, Handle<Value> file_name) {
    v8::Context::Scope context_scope(context);
    v8::HandleScope scope;
    v8::TryCatch try_catch;
    v8::Handle<v8::Script> script = v8::Script::Compile(source, file_name);
    if (script.IsEmpty()) {
        ReportException(&try_catch);
        return false;
    }
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
        ReportException(&try_catch);
        return false;
    }
    return true;
}

bool JS_Plugin::load_file(char * filename) {
    v8::HandleScope scope;
    return load_source(ReadFile(filename), String::New(filename));
}

JS_Plugin js_plugin;

bool JS_Plugin::call_event(char * event_name, v8::Handle<v8::Value> data) {
    v8::Context::Scope context_scope(this->context);
    v8::HandleScope scope;
    v8::Handle<v8::Value> event_namespace = context->Global()->Get(new_str("events"));
    if (!event_namespace->IsObject()) return false;
    v8::Handle<v8::Value> event_object = event_namespace->ToObject()->Get(new_str(event_name));
    if (!event_object->IsObject()) return false;
    v8::Handle<v8::Value> function_object = event_object->ToObject()->Get(new_str("call"));
    if (!function_object->IsFunction()) return false;
    v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(function_object);

    v8::TryCatch try_catch;
    const int argc = 1;
    v8::Handle<v8::Value> argv[argc] = {data};
    v8::Handle<v8::Value> result = function->Call(event_object->ToObject(), argc, argv);
    if (result.IsEmpty()) {
        ReportException(&try_catch);
        return false;
    }
    return true;
}

void JS_Plugin::process ( bz_EventData *event_data )
{
    v8::Context::Scope context_scope(this->context);
    Handle<Object> data = Object::New();
    switch (event_data->eventType)
    {
    default:
      break;
  
    case bz_eGetPlayerSpawnPosEvent:
      {

        bz_GetPlayerSpawnPosEventData *event = (bz_GetPlayerSpawnPosEventData*)event_data;
        write_pos(data, new_str("pos"), event->pos);
        write_bool(data, new_str("handled"), event->handled);
        write_float(data, new_str("rot"), event->rot);
        write_float(data, new_str("time"), event->time);
        //data->Set(new_str("player"), get_player(event->playerID));
        data->Set(new_str("playerID"), Integer::New(event->playerID));

        if (!call_event("getPlayerSpawnPos", data))
            printf("Calling event failed!\n");

        read_pos(data, new_str("pos"), event->pos);
        read_bool(data, new_str("handled"), event->handled);
        read_float(data, new_str("rot"), event->rot);
      }
      break;

    case bz_eUnknownSlashCommand:
      {
        bz_UnknownSlashCommandEventData *event = (bz_UnknownSlashCommandEventData*)event_data;
          data->Set(new_str("message"), String::New(event->message.c_str()));
          data->Set(new_str("fromID"), Integer::New(event->from));
          data->Set(new_str("handled"), Boolean::New(event->handled));
          data->Set(new_str("time"), Number::New(event->time));
        if (!call_event("unknownSlashCommand", data))
            printf("Calling event failed!\n");
        event->handled = data->Get(new_str("handled"))->BooleanValue();

      }
      break;

    case bz_ePlayerJoinEvent:
      {
        bz_PlayerJoinPartEventData *event = (bz_PlayerJoinPartEventData*)event_data;
        data->Set(new_str("playerID"), Integer::New(event->playerID));
        data->Set(new_str("time"), Number::New(event->time));
        if (!call_event("playerJoin", data))
            printf("Calling event failed!\n");
      }
      break;
    case bz_ePlayerPartEvent:
      {
        bz_PlayerJoinPartEventData *event = (bz_PlayerJoinPartEventData*)event_data;
        data->Set(new_str("playerID"), Integer::New(event->playerID));
        data->Set(new_str("time"), Number::New(event->time));
        data->Set(new_str("reason"), String::New(event->reason.c_str()));
        if (!call_event("playerPart", data))
            printf("Calling event failed!\n");
      }
      break;
    case bz_ePlayerDieEvent:
      {

        bz_PlayerDieEventData *event = (bz_PlayerDieEventData*)event_data;
        data->Set(new_str("playerID"), Integer::New(event->playerID));
        data->Set(new_str("killerID"), Integer::New(event->killerID));
        data->Set(new_str("shotID"), Integer::New(event->shotID));
        write_pos(data, new_str("pos"), event->pos);
        data->Set(new_str("time"), Number::New(event->time));
        data->Set(new_str("rot"), Number::New(event->rot));
        data->Set(new_str("flagKilledWith"), String::New(event->flagKilledWith.c_str()));

        if (!call_event("playerDie", data))
            printf("Calling event failed!\n");
      }
      break;
    }
}

class PluginHandler : public bz_APIPluginHandler
{
public:
  virtual bool handle(bzApiString plugin, bzApiString param);

};



bool
PluginHandler::handle (bzApiString plugin, bzApiString param)
{
  const char *filename = plugin.c_str ();

  return js_plugin.load_file((char *)filename);
};

static PluginHandler *js_handler;


BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
    if (BZ_API_VERSION != 16) {
      fprintf (stderr, "plugin currently wraps the version 16 API, but BZFS is exporting version %d. Please complain loudly\n", BZ_API_VERSION);
      abort ();
    }

      
    //js_plugin = new JS_Plugin ();
    if (!js_plugin.initialize())
        return 1;

    js_handler = new PluginHandler ();
    if (!bz_registerCustomPluginHandler ("js", js_handler))
      fprintf (stderr, "couldn't register custom plugin handler\n");
    return 0;
}

BZF_PLUGIN_CALL
int
bz_Unload (void)
{
    js_plugin.uninitialize();

  return 0;
}


