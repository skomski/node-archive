// Copyright 2012 Karl Skomski MIT

#include "archive_reader.h"

namespace nodearchive {
  using v8::Object;
  using v8::Handle;
  using v8::Local;
  using v8::Persistent;
  using v8::Value;
  using v8::HandleScope;
  using v8::FunctionTemplate;
  using v8::String;
  using v8::Array;
  using v8::Function;
  using v8::TryCatch;
  using v8::Context;
  using v8::Arguments;
  using v8::Integer;
  using v8::Exception;
  using v8::Undefined;
  using v8::External;

  ArchiveReader::ArchiveReader(const char* filename) {
    filename_ = filename;

    archive_ = archive_read_new();
    archive_read_support_filter_all(archive_);
    archive_read_support_format_all(archive_);
  }
  ArchiveReader::~ArchiveReader() {
  }

  void ArchiveReader::Init(Handle<Object> target) {
    Local<FunctionTemplate> reader_template = FunctionTemplate::New(New);
    reader_template->SetClassName(String::NewSymbol("ArchiveReader"));
    reader_template->InstanceTemplate()->SetInternalFieldCount(1);

    reader_template->PrototypeTemplate()->Set(String::NewSymbol("open"),
        FunctionTemplate::New(Open)->GetFunction());

    reader_template->PrototypeTemplate()->Set(String::NewSymbol("nextEntry"),
        FunctionTemplate::New(NextEntry)->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(
        reader_template->GetFunction());
    target->Set(String::NewSymbol("Reader"), constructor);
  }

  Handle<Value> ArchiveReader::New(const Arguments& args) {
    HandleScope scope;

    ArchiveReader* reader = new ArchiveReader(*v8::String::Utf8Value(args[0]));
    reader->Wrap(args.This());

    return scope.Close(args.This());
  }

  struct open_request {
    ArchiveReader* reader;
    const char* error_string;
    v8::Persistent<v8::Function> callback;
  };

  async_rtn ArchiveReader::OpenWork(uv_work_t *job) {
    open_request *req = static_cast<open_request*>(job->data);

    int return_value = archive_read_open_filename(
        req->reader->archive_,
        req->reader->filename_.c_str(),
        10240);

    if (return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->reader->archive_);
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveReader::OpenDone(uv_work_t *job) {
    v8::HandleScope scope;
    open_request *req = static_cast<open_request*>(job->data);

    if (req->error_string != NULL) {
      helpers::EmitError(req->reader->handle_, req->error_string);
    } else {
      Handle<Value> argv[1] = { Undefined() };

      TryCatch try_catch;
      req->callback->Call(v8::Context::GetCurrent()->Global(), 1, argv);
      if (try_catch.HasCaught()) node::FatalException(try_catch);
    }


    req->callback.Dispose();
    req->reader->Unref();
    delete req;
    RETURN_ASYNC_AFTER
  }

  Handle<Value> ArchiveReader::Open(const Arguments& args) {
    HandleScope scope;
    ArchiveReader* reader = ObjectWrap::Unwrap<ArchiveReader>(args.This());

    open_request *req = new open_request;
    req->reader = reader;
    req->error_string = NULL;
    req->callback = Persistent<Function>::New(
        Local<Function>::Cast(args[0]));

    BEGIN_ASYNC(req, OpenWork, OpenDone);
    reader->Ref();

    return scope.Close(Undefined());
  }

  struct next_entry_request {
    ArchiveReader* reader;
    struct archive_entry *entry;
    bool eof;
    const char* error_string;
  };

  async_rtn ArchiveReader::NextEntryWork(uv_work_t *job) {
    next_entry_request *req = static_cast<next_entry_request*>(job->data);


    int return_value = archive_read_next_header(
        req->reader->archive_,
        &req->entry);

    if (return_value == ARCHIVE_EOF) {
      req->eof = true;
      RETURN_ASYNC
    }

    if (return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->reader->archive_);
      RETURN_ASYNC
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveReader::NextEntryDone(uv_work_t *job) {
    v8::HandleScope scope;
    next_entry_request *req = static_cast<next_entry_request*>(job->data);

    if (req->eof == true) {
      helpers::Emit(req->reader->handle_, "end", Undefined());
      archive_read_close(req->reader->archive_);
      archive_read_free(req->reader->archive_);
    } else if (req->error_string != NULL) {
      helpers::EmitError(req->reader->handle_, req->error_string);
    } else {
      Handle<Value> entry = ArchiveEntry::NewInstance(
          req->reader->archive_,
          req->entry);
      helpers::Emit(req->reader->handle_, "entry", entry);
    }

    req->reader->Unref();
    delete req;
    RETURN_ASYNC_AFTER
  }

  Handle<Value> ArchiveReader::NextEntry(const Arguments& args) {
    HandleScope scope;
    ArchiveReader* reader = ObjectWrap::Unwrap<ArchiveReader>(args.This());

    next_entry_request *req = new next_entry_request;
    req->reader = reader;
    req->eof    = false;
    req->error_string = NULL;

    BEGIN_ASYNC(req, NextEntryWork, NextEntryDone);
    reader->Ref();

    return scope.Close(Undefined());
  }
}
