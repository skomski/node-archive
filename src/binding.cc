// Copyright 2012 Karl Skomski MIT

#include <snappy.h>
#include <node_buffer.h>

namespace nodesnappy {

  struct request {
    const char* input_data;
    size_t input_length;
    char* output_data;
    size_t output_length;
    v8::Persistent<v8::Function> callback;
  };

  void compress_work(uv_work_t *job) {
    request *req = static_cast<request*>(job->data);

    size_t max_length = snappy::MaxCompressedLength(req->input_length);

    req->output_data = new char[max_length];

    snappy::RawCompress(
        req->input_data,
        req->input_length,
        req->output_data,
        &req->output_length);
  }

  void compress_done(uv_work_t *job) {
    v8::HandleScope scope;

    request *req = static_cast<request*>(job->data);

    v8::Handle<v8::Object> buffer = node::Buffer::New(
      req->output_data, req->output_length)->handle_;

    v8::Handle<v8::Value> argv[2] = {
      v8::Local<v8::Value>::New(v8::Null()), buffer };

    v8::TryCatch try_catch;
    req->callback->Call(v8::Context::GetCurrent()->Global(), 2, argv);
    if (try_catch.HasCaught()) node::FatalException(try_catch);

    req->callback.Dispose();
    delete[] req->output_data;
    delete req;
    delete job;
  }

  v8::Handle<v8::Value> compress(const v8::Arguments& args) {
    v8::HandleScope scope;

    request *req = new request;

    v8::Local<v8::Object> input = args[0]->ToObject();
    req->input_length = node::Buffer::Length(input);
    req->input_data = node::Buffer::Data(input);
    req->callback = v8::Persistent<v8::Function>::New(
      v8::Local<v8::Function>::Cast(args[1]));

    uv_work_t *job = new uv_work_t;
    job->data = req;
    uv_queue_work(uv_default_loop(), job, compress_work, compress_done);

    return scope.Close(v8::Undefined());
  }

  void uncompress_work(uv_work_t *job) {
    request *req = static_cast<request*>(job->data);

    if (!snappy::GetUncompressedLength(
          req->input_data,
          req->input_length,
          &req->output_length)) {
      req->output_length = -1u;
      return;
    }

    req->output_data = new char[req->output_length];

    if (!snappy::RawUncompress(
          req->input_data,
          req->input_length,
          req->output_data)) {
      req->output_length = -1u;
    }
  }

  void uncompress_done(uv_work_t *job) {
    v8::HandleScope scope;

    request* req = static_cast<request*>(job->data);

    v8::Handle<v8::Value> argv[2];

    if (req->output_length != -1u) {
      argv[0] = v8::Local<v8::Value>::New(v8::Null());
      argv[1] = node::Buffer::New(
          req->output_data,
          req->output_length)->handle_;
    } else {
      argv[0] = v8::Exception::Error(
        v8::String::New("Buffer could not be decompressed"));
      argv[1] = v8::Local<v8::Value>::New(v8::Null());
    }

    v8::TryCatch try_catch;
    req->callback->Call(v8::Context::GetCurrent()->Global(), 2, argv);
    if (try_catch.HasCaught()) node::FatalException(try_catch);

    req->callback.Dispose();
    delete[] req->output_data;
    delete req;
    delete job;
  }

  v8::Handle<v8::Value> uncompress(const v8::Arguments& args) {
    v8::HandleScope scope;

    request *req = new request;

    v8::Local<v8::Object> input = args[0]->ToObject();

    req->input_length = node::Buffer::Length(input);
    req->input_data = node::Buffer::Data(input);
    req->callback = v8::Persistent<v8::Function>::New(
      v8::Local<v8::Function>::Cast(args[1]));

    uv_work_t *uncompressJob = new uv_work_t;
    uncompressJob->data = req;
    uv_queue_work(
      uv_default_loop(),
      uncompressJob,
      uncompress_work,
      uncompress_done);

    return scope.Close(v8::Undefined());
  }

  void Init(v8::Handle<v8::Object> target) {
    target->Set(v8::String::NewSymbol("compress"),
      v8::FunctionTemplate::New(compress)->GetFunction());
    target->Set(v8::String::NewSymbol("uncompress"),
      v8::FunctionTemplate::New(uncompress)->GetFunction());
  }

  NODE_MODULE(binding, Init)
}
