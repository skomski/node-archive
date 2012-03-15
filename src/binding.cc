// Copyright 2012 Karl Skomski MIT

#include <node.h>
#include <node_version.h>
#include <archive.h>
#include <archive_entry.h>
#include <iostream>

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

namespace nodearchive {

  struct request {
    std::string filename;
    std::string target;
    std::string error;
    v8::Persistent<v8::Function> callback;
  };

  void compress_work(uv_work_t *job) {
    request *req = static_cast<request*>(job->data);
  }

  void compress_done(uv_work_t *job) {
    v8::HandleScope scope;
    request *req = static_cast<request*>(job->data);
  }

  v8::Handle<v8::Value> compress(const v8::Arguments& args) {
    v8::HandleScope scope;
    request *req = new request;


    return scope.Close(v8::Undefined());
  }

  static int copy_data(struct archive *ar, struct archive *aw)
  {
    int r;
    const void *buff;
    size_t size;
    off_t offset;

    for (;;) {
      r = archive_read_data_block(ar, &buff, &size, &offset);
      if (r == ARCHIVE_EOF) {
        return (ARCHIVE_OK);
      }
      if (r != ARCHIVE_OK)
        return (r);
      r = archive_write_data_block(aw, buff, size, offset);
      if (r != ARCHIVE_OK) {
        return (r);
      }
    }
  }

  async_rtn decompress_work(uv_work_t *job) {
    request *req = static_cast<request*>(job->data);

    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);

    if ((r = archive_read_open_file(a, req->filename.c_str(), 10240))) {
      req->error = std::string(archive_error_string(a));
    } else {
      for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;

        if (r != ARCHIVE_OK) {
          req->error = std::string(archive_error_string(a));
          break;
        }

        const char* pathname = archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, (req->target + '/' + pathname).c_str());

        r = archive_write_header(ext, entry);

        if (r != ARCHIVE_OK) {
          req->error = std::string(archive_error_string(ext));
          break;
        } else {
          copy_data(a, ext);
          r = archive_write_finish_entry(ext);
          if (r != ARCHIVE_OK) {
            req->error = std::string(archive_error_string(ext));
            break;
          }
        }
      }
    }

    archive_read_close(a);
    archive_read_free(a);

    RETURN_ASYNC
  }

  async_rtn decompress_done(uv_work_t *job) {
    v8::HandleScope scope;
    request* req = static_cast<request*>(job->data);
    v8::Handle<v8::Value> argv[1];

    if (req->error.empty()) {
      argv[0] = v8::Local<v8::Value>::New(v8::Null());
    } else {
      argv[0] = v8::Exception::Error(
        v8::String::New(req->error.c_str()));
    }

    v8::TryCatch try_catch;
    req->callback->Call(v8::Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) node::FatalException(try_catch);

    req->callback.Dispose();
    delete req;
    RETURN_ASYNC_AFTER
  }

  v8::Handle<v8::Value> decompress(const v8::Arguments& args) {
    v8::HandleScope scope;

    request *req = new request;
    v8::Handle<v8::Value> arg0 = args[0];
    v8::String::Utf8Value filename(arg0);

    v8::Handle<v8::Value> arg1 = args[1];
    v8::String::Utf8Value target(arg1);

    req->filename = std::string(*filename);
    req->target   = std::string(*target);
    req->callback = v8::Persistent<v8::Function>::New(
      v8::Local<v8::Function>::Cast(args[2]));

    BEGIN_ASYNC(req, decompress_work, decompress_done);
    return scope.Close(v8::Undefined());
  }

  extern "C" void init(v8::Handle<v8::Object> target) {
    target->Set(v8::String::NewSymbol("compress"),
      v8::FunctionTemplate::New(compress)->GetFunction());
    target->Set(v8::String::NewSymbol("decompress"),
      v8::FunctionTemplate::New(decompress)->GetFunction());
  }
}
