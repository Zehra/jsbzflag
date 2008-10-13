// Copyright 2008 by Matthew Marshall <matthew@matthewmarshall.org>
// License: GPL

#include "bzfsAPI.h"
#include "stdio.h"
#include <v8.h>
#include "player.h"
#include "bz_functions.h"

BZ_GET_PLUGIN_VERSION

using namespace v8;

#define new_str String::NewSymbol

const char * path_to_stdlib = NULL;


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
  bool load_file(const char * filename);

  bool call_event(char * event_name, v8::Handle<v8::Value> data);

  Handle<Value> get_player(int player_id);

  v8::HandleScope handle_scope;
  v8::Persistent<v8::Context> context;
};

bool JS_Plugin::initialize() {

    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    global->Set(v8::String::New("print"), v8::FunctionTemplate::New(js_print));
    context = v8::Context::New(NULL, global);  // TODO Dispose

    v8::Context::Scope context_scope(context);
    context->Global()->Set(String::NewSymbol("Player"), make_Player_function());
    context->Global()->Set(String::NewSymbol("_bz"), make_bz_object());
    if (!load_file(path_to_stdlib))
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

bool JS_Plugin::load_file(const char * filename) {
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

  return js_plugin.load_file(filename);
};

static PluginHandler *js_handler;


BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
    if (BZ_API_VERSION != 16) {
      fprintf (stderr, "The jsbzflag plugin currently wraps the version 16 API, but BZFS is exporting version %d. Things may go wrong.\n", BZ_API_VERSION);
    }

    if (strlen(commandLine) < 1) {
        fprintf(stderr, "Cannot load jsbzflag plugin because the path to stdlib.js isn't given.\n");
        fprintf(stderr, "Load the plugin like this: -loadplugin /path/to/jsbzflag.so,/path/to/stdlib.js\n");
        abort();
    }
    path_to_stdlib = commandLine;
      
    //js_plugin = new JS_Plugin ();
    if (!js_plugin.initialize()) {
        fprintf(stderr, "There was a problem initializing the jsbzflag plugin.  Aborting.\n");
        abort();
    }

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


