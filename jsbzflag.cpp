#include "bzfsAPI.h"
#include "stdio.h"
#include <v8.h>

BZ_GET_PLUGIN_VERSION

using namespace v8;

#define new_str String::New

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
        //virtual ~GenericEventHandler();

        virtual void process(bz_EventData * event_data);

        virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event

  bool initialize();
  bool load_source(v8::Handle<v8::String> source);
  bool load_file(char * filename);

  bool call_event(char * event_name, v8::Handle<v8::Value> data);

  v8::HandleScope handle_scope;
  v8::Persistent<v8::Context> context;
};

bool JS_Plugin::initialize() {
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    global->Set(v8::String::New("print"), v8::FunctionTemplate::New(js_print));
    global->Set(v8::String::New("getCurrentTime"), v8::FunctionTemplate::New(js_getCurrentTime));
    global->Set(v8::String::New("sendTextMessage"), v8::FunctionTemplate::New(js_sendTextMessage));
    context = v8::Context::New(NULL, global);  // TODO Dispose

    v8::Context::Scope context_scope(context);
    load_file("stdlib.js");

    return true;
}

bool JS_Plugin::load_source(v8::Handle<v8::String> source) {
    v8::HandleScope scope;
    v8::Context::Scope context_scope(context);
    v8::Handle<v8::Script> script = v8::Script::Compile(source);
    v8::Handle<v8::Value> result = script->Run();
    return true;
}

bool JS_Plugin::load_file(char * filename) {
    v8::HandleScope scope;
    return load_source(ReadFile(filename));
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

    const int argc = 1;
    v8::Handle<v8::Value> argv[argc] = {data};
    v8::Handle<v8::Value> result = function->Call(event_object->ToObject(), argc, argv);
    // TODO check for exceptions?
    return true;
}

void JS_Plugin::process ( bz_EventData *event_data )
{
    v8::Context::Scope context_scope(this->context);
    bz_GetPlayerSpawnPosEventData *event = (bz_GetPlayerSpawnPosEventData*)event_data;
    Handle<Object> data = Object::New();
    switch (event_data->eventType)
    {
    default:
      break;
  
    case bz_eGetPlayerSpawnPosEvent:
      {

        write_pos(data, new_str("pos"), event->pos);
        write_bool(data, new_str("handled"), event->handled);
        write_float(data, new_str("rot"), event->rot);
        write_float(data, new_str("time"), event->time);

        if (!call_event("getPlayerSpawnPos", data))
            printf("Calling event failed!\n");

        read_pos(data, new_str("pos"), event->pos);
        read_bool(data, new_str("handled"), event->handled);
        read_float(data, new_str("rot"), event->rot);
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
    js_plugin.initialize();
    bz_registerEvent(bz_eGetPlayerSpawnPosEvent,&js_plugin);

    js_handler = new PluginHandler ();
    if (!bz_registerCustomPluginHandler ("js", js_handler))
      fprintf (stderr, "couldn't register custom plugin handler\n");
    return 0;
}

BZF_PLUGIN_CALL
int
bz_Unload (void)
{
  //delete [] code_buffer;

  bz_removeEvent(bz_eGetPlayerSpawnPosEvent,&js_plugin);

  return 0;
}


