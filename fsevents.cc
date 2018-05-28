/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "uv.h"
#include "v8.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include <iostream>
#include <vector>

#include <node_api.h>

#include "src/storage.cc"
int counter = 0;

#define DECLARE_NAPI_METHOD(name, func)                          \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

namespace fse {
  class FSEvents {
  public:
    explicit FSEvents(const char *path);
    ~FSEvents();

    uv_mutex_t mutex;

    // async.cc
    uv_async_t async;
    void asyncStart();
    void asyncTrigger();
    void asyncStop();

    // thread.cc
    uv_thread_t thread;
    CFRunLoopRef threadloop;
    void threadStart();
    static void threadRun(void *ctx);
    void threadStop();

    // methods.cc - internal
    // Nan::AsyncResource async_resource;
    void emitEvent(const char *path, UInt32 flags, UInt64 id);

    // Common
    CFArrayRef paths;
    std::vector<fse_event*> events;

    // NAPI exposed
    static napi_value Init(napi_env env, napi_value exports);

    // methods.cc - exposed by NAPI
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);

    // wrappers
    napi_env env_;
    napi_ref wrapper_;
    // handler store
    napi_ref handlerRef;
    napi_ref jsContextRef;

    int id;

    // destructor for v8 call
    static void Destructor(napi_env, void* , void* /*finalize_hint*/);
  };
}

using namespace fse;

FSEvents::FSEvents(const char *path): env_(nullptr), wrapper_(nullptr), id(++counter) {
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, "./build", kCFStringEncodingUTF8) };
  paths = CFArrayCreate(NULL, (const void **)&dirs, 1, NULL);
  threadloop = NULL;
  if (uv_mutex_init(&mutex)) abort();
}

FSEvents::~FSEvents() {
  CFRelease(paths);
  uv_mutex_destroy(&mutex);
  napi_delete_reference(env_, wrapper_);
  napi_delete_reference(env_, handlerRef);
  napi_delete_reference(env_, jsContextRef);
}


#ifndef kFSEventStreamEventFlagItemCreated
#define kFSEventStreamEventFlagItemCreated 0x00000010
#endif

#include "src/async.cc"
#include "src/thread.cc"
#include "src/constants.cc"
#include "src/methods.cc"

static napi_value Init(napi_env env, napi_value exports) {
  napi_value new_exports;
  napi_status status;

  status = napi_create_object(env, &new_exports);
  assert(status == napi_ok);

  status = napi_set_named_property(env, new_exports, "Constants", Constants(env));
  assert(status == napi_ok);

  napi_property_descriptor properties[] = {
    DECLARE_NAPI_METHOD("start", FSEvents::Start),
    DECLARE_NAPI_METHOD("stop", FSEvents::Stop)
  };

  napi_value cons;
  status = napi_define_class(env, "FSEvents", NAPI_AUTO_LENGTH, FSEvents::New, nullptr, 2, properties, &cons);
  assert(status == napi_ok);

  status = napi_set_named_property(env, new_exports, "FSEvents", cons);
  assert(status == napi_ok);
  return new_exports;
}

// NODE_MODULE(fse, FSEvents::Initialize)
NAPI_MODULE(fse, Init)
