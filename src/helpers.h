// Copyright 2012 Karl Skomski MIT

#ifndef SRC_HELPERS_H_
#define SRC_HELPERS_H_

#include <node.h>
#include <node_version.h>

#if NODE_VERSION_AT_LEAST(0, 5, 6)
  #define BEGIN_ASYNC(_data, async, after) \
    uv_work_t *_req = new uv_work_t; \
    _req->data = _data; \
    uv_queue_work(uv_default_loop(), _req, async, after);
  typedef void async_rtn;
  #define RETURN_ASYNC return;
  #define RETURN_ASYNC_AFTER delete job;
#else
  #define BEGIN_ASYNC(data, async, after) \
    ev_ref(EV_DEFAULT_UC); \
    eio_custom(async, EIO_PRI_DEFAULT, after, data);
  typedef int async_rtn;
  typedef eio_req uv_work_t;
  #define RETURN_ASYNC return 0;
  #define RETURN_ASYNC_AFTER \
    ev_unref(EV_DEFAULT_UC); \
    RETURN_ASYNC;
#endif

namespace helpers {
  void Emit(
       v8::Handle<v8::Object> source,
       const char* event,
       v8::Handle<v8::Value> object);

  void EmitError(
      v8::Handle<v8::Object> source,
      const char* error_message);

  void MakeCallback(v8::Handle<v8::Object> object,
                    const char* method,
                    int argc,
                    v8::Handle<v8::Value> argv[]);
}

#endif  // SRC_HELPERS_H_
