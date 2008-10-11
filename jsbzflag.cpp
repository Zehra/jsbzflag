#include "bzfsAPI.h"
#include "stdio.h"
#define XP_UNIX
#include "mozjs/jsapi.h"

BZ_GET_PLUGIN_VERSION

static JSRuntime *rt;
static JSContext *cx;
static JSObject *global_object;

static char *ReadFile (const char *filename);
static char *code_buffer;

JSBool js_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    const char *string;
    
    if (!JS_ConvertArguments(cx, argc, argv, "s", &string))
        return JS_FALSE;

    printf("%s\n", string);

    return JS_TRUE;
}

static JSFunctionSpec global_functions_spec[] = {
    "print", js_print, 1, 0, 0,
    0,0,0,0,0
};

/* The class of the global object. */
static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

/* The error reporter callback. */
void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stderr, "*js error:%s:%u:%s\n",
            report->filename ? report->filename : "<no filename>",
            (unsigned int) report->lineno,
            message);
}

JSObject * build_position_array(JSContext *cx, float * pos){
    JSObject * p = JS_NewArrayObject(cx, 0, NULL);
    JS_AddRoot(cx, p);
    for (int i=0; i<3; i++) {
        JSBool ok;
        jsval v;
        JS_NewNumberValue(cx, pos[i], &v);
        ok = JS_SetElement(cx, p, i, &v);
    }
    return p;
}

JSBool read_position_array(JSContext *cx, jsval pos_jsval, float * pos){
    if (!JSVAL_IS_OBJECT(pos_jsval))
        return JS_FALSE;
    JSObject * p = JSVAL_TO_OBJECT(pos_jsval);
    for (int i=0; i<3; i++) {
        jsval v;
        if (!JS_GetElement(cx, p, i, &v))
            return JS_FALSE;
        jsdouble vd;
        if (!JS_ConvertArguments(cx, 1, &v, "d", &vd))
            return JS_FALSE;
        pos[i] = vd;
    }
    return JS_TRUE;
}


class GenericEventHandler : public bz_EventHandler
{
    public:
        //GenericEventHandler();
        //virtual ~GenericEventHandler();

        virtual void process(bz_EventData * event_data);

        virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
};

GenericEventHandler generic_event_handler;

void GenericEventHandler::process ( bz_EventData *event_data )
{
  switch (event_data->eventType)
  {
  default:
    // really WTF!!!!
    break;

  case bz_eGetPlayerSpawnPosEvent:
    {
      bz_GetPlayerSpawnPosEventData *spawn = (bz_GetPlayerSpawnPosEventData*)event_data;

      JSObject *e = JS_NewObject(cx, NULL, NULL, NULL);
      jsval v = OBJECT_TO_JSVAL(build_position_array(cx, spawn->pos));
      JS_SetProperty(cx, e, "pos", &v);

      void * mark;
      jsval * args = JS_PushArguments(cx, &mark, "o", e);
      if (args){
          jsval r;
          if (!JS_CallFunctionName(cx, JS_GetGlobalObject(cx), "bz_GetPlayerSpawnPosEvent", 1, args, &r))
              printf("called successfully\n");
          JS_PopArguments(cx, mark);
      }


      jsval pos;
      if (JS_GetProperty(cx, e, "pos", &pos))
          read_position_array(cx, pos, spawn->pos);

      //float randPos = rand()/(float)RAND_MAX;
      //spawn->pos[2] += randPos * spawnRange;
      spawn->handled = true;
    }
    break;
  }
}

class PluginHandler : public bz_APIPluginHandler
{
public:
  virtual bool handle (bzApiString plugin, bzApiString param);
};

bool
PluginHandler::handle (bzApiString plugin, bzApiString param)
{
  const char *filename = plugin.c_str ();
  char *buffer = ReadFile(filename);
  if (!buffer) {
      fprintf(stderr, "Could not open file %s.\n", filename);
      return false;
  }
  code_buffer = buffer;

  JSBool ok;
  jsval rval;

  ok = JS_EvaluateScript(cx, global_object, code_buffer, strlen(code_buffer), filename, 0, &rval);
  if (!ok){
      fprintf(stderr, "evaluating plugin file resulted in errors.\n");
      return false;
  }

  return true;
};

static PluginHandler *js_handler;


BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
      // I would use assert here, but "Assertion `3 == 2' failed" is really not a useful error at all
      if (BZ_API_VERSION != 16) {
        fprintf (stderr, "plugin currently wraps the version 5 API, but BZFS is exporting version %d. Please complain loudly\n", BZ_API_VERSION);
        abort ();
      }

      /* Create a JS runtime. */
    rt = JS_NewRuntime(8L * 1024L * 1024L);
    if (rt == NULL)
        return 1;

    /* Create a context. */
    cx = JS_NewContext(rt, 8192);
    if (cx == NULL)
        return 1;
    JS_SetOptions(cx, JSOPTION_VAROBJFIX);
    //JS_SetVersion(cx, JSVERSION_LATEST);
    JS_SetErrorReporter(cx, reportError);

    /* Create the global object. */
    global_object = JS_NewObject(cx, &global_class, NULL, NULL);
    if (global_object == NULL)
        return 1;

    /* Populate the global object with the standard globals,
       like Object and Array. */
    if (!JS_InitStandardClasses(cx, global_object))
        return 1;

    if (!JS_DefineFunctions(cx, global_object, global_functions_spec))
        return 1;

        
  bz_registerEvent(bz_eGetPlayerSpawnPosEvent,&generic_event_handler);

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
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

  bz_removeEvent(bz_eGetPlayerSpawnPosEvent,&generic_event_handler);

  return 0;
}

static char *
ReadFile (const char *filename)
{
  FILE *f = fopen (filename, "r");
  if (!f) {
      return NULL;
  }

  unsigned int pos = ftell (f);
  fseek (f, 0, SEEK_END);
  unsigned int len = ftell (f);
  fseek (f, pos, SEEK_SET);

  char *buffer = new char[len + 1];
  fread (buffer, 1, len, f);

  buffer[len] = '\0';

  fclose (f);
  return buffer;
}
