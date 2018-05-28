/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

void FSEvents::emitEvent(const char *path, UInt32 flags, UInt64 uid) {
  napi_value argv[3];
  napi_status status;

  napi_handle_scope scope;
  status = napi_open_handle_scope(env_, &scope);
  status = napi_create_string_utf8(env_, path, NAPI_AUTO_LENGTH, &argv[0]);
  assert(status == napi_ok);

  status = napi_create_uint32(env_, flags, &argv[1]);
  assert(status == napi_ok);

  status = napi_create_int64(env_, (int64_t)uid, &argv[2]);
  assert(status == napi_ok);

  napi_value handler;
  status = napi_get_reference_value(env_, this->handlerRef, &handler);
  assert(status == napi_ok);

  napi_value jsthis;
  status = napi_get_reference_value(env_, this->jsContextRef, &jsthis);
  assert(status == napi_ok);

  status = napi_call_function(env_, jsthis, handler, 3, (const napi_value *)argv, nullptr);

  napi_close_handle_scope(env_, scope);
}


napi_value FSEvents::Start(napi_env env, napi_callback_info info) {
  napi_value jsthis;
  napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
  assert(status == napi_ok);
  FSEvents* fse;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&fse));
  assert(status == napi_ok);
  fse->asyncStart();
  fse->threadStart();
  return jsthis;
}

napi_value FSEvents::Stop(napi_env env, napi_callback_info info) {
  napi_value jsthis;
  napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
  assert(status == napi_ok);
  FSEvents* fse;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&fse));
  assert(status == napi_ok);
  fse->threadStop();
  fse->asyncStop();
  return jsthis;
}

// 保证参数是string
napi_value FSEvents::New(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  napi_value jsthis;
  napi_status status;
  status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
  assert(status == napi_ok);
  size_t len;
  char path[1024];
  napi_valuetype vt;
  status = napi_typeof(env, args[0], &vt);
  assert(vt == napi_string);

  status = napi_get_value_string_utf8(env, args[0], (char *) path, 1024, &len);
  assert(status == napi_ok);
  FSEvents* fse = new FSEvents(path);

  if (argc == 2) {
    napi_valuetype vt;
    status = napi_typeof(env, args[1], &vt);
    assert(vt == napi_function);
    status = napi_create_reference(env, args[1], 1, &fse->handlerRef);
    assert(status == napi_ok);

    status = napi_create_reference(env, jsthis, 1, &fse->jsContextRef);
    assert(status == napi_ok);
  }
  fse->env_ = env;
  status = napi_wrap(
    env,
    jsthis,
    reinterpret_cast<void*>(fse),
    FSEvents::Destructor,
    nullptr,
    &fse->wrapper_ 
  );
  assert(status == napi_ok);
  return jsthis;
}


void FSEvents::Destructor(napi_env env, void* nativeFSEvents, void* /*finalize_hint*/) {
  reinterpret_cast<FSEvents*>(nativeFSEvents)->~FSEvents();
}
