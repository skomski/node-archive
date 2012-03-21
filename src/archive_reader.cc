// Copyright 2012 Karl Skomski MIT

#include "./archive_reader.h"

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

  ArchiveReader::ArchiveReader(const char* filename, size_t blocksize) {
    filename_   = std::string(filename);
    blocksize_ = blocksize;

    archive_ = archive_read_new();
    archive_read_support_filter_all(archive_);
    archive_read_support_format_all(archive_);
  }

  ArchiveReader::~ArchiveReader() {}

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

    ArchiveReader* reader = new ArchiveReader(
        *String::Utf8Value(args[0]),
        args[1]->ToInteger()->Value());

    reader->Wrap(args.This());

    return scope.Close(args.This());
  }

  struct OpenRequest {
    ArchiveReader* reader;
    const char* error_string;
    const char* compression_name;
    v8::Persistent<v8::Function> callback;
  };

  async_rtn ArchiveReader::OpenWork(uv_work_t *job) {
    OpenRequest *req = static_cast<OpenRequest*>(job->data);

    int return_value = archive_read_open_filename(
        req->reader->archive_,
        req->reader->filename_.c_str(),
        req->reader->blocksize_);

    if (return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->reader->archive_);
    } else {
      req->compression_name = archive_compression_name(req->reader->archive_);
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveReader::OpenDone(uv_work_t *job) {
    v8::HandleScope scope;
    OpenRequest *req = static_cast<OpenRequest*>(job->data);

    if (req->error_string != NULL) {
      helpers::EmitError(req->reader->handle_, req->error_string);
    } else {
      Local<Object> object = Object::New();

      if (req->compression_name != NULL) {
        object->Set(
          String::NewSymbol("compression"),
          String::New(req->compression_name));
      }

      Handle<Value> argv[1] = { object };

      TryCatch try_catch;
      req->callback->Call(Context::GetCurrent()->Global(), 1, argv);
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

    OpenRequest *req = new OpenRequest();
    req->reader = reader;
    req->error_string = NULL;
    req->callback = Persistent<Function>::New(
        Local<Function>::Cast(args[0]));

    BEGIN_ASYNC(req, OpenWork, OpenDone);
    reader->Ref();

    return scope.Close(Undefined());
  }

  struct NextEntryRequest {
    ArchiveReader *reader;
    archive_entry *entry;
    bool eof;
    const char    *error_string;
  };

  async_rtn ArchiveReader::NextEntryWork(uv_work_t *job) {
    NextEntryRequest *req = static_cast<NextEntryRequest*>(job->data);

    int return_value = archive_read_next_header(
        req->reader->archive_,
        &req->entry);

    if (return_value == ARCHIVE_EOF) {
      req->eof = true;
    } else if (return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->reader->archive_);
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveReader::NextEntryDone(uv_work_t *job) {
    v8::HandleScope scope;
    NextEntryRequest *req = static_cast<NextEntryRequest*>(job->data);

    if (req->error_string != NULL) {
      helpers::EmitError(req->reader->handle_, req->error_string);
    } else {
      if (req->eof == true) {
        helpers::Emit(req->reader->handle_, "end", Undefined());
        archive_read_close(req->reader->archive_);
        archive_read_free(req->reader->archive_);
      } else {
        Handle<Value> entry_wrapper = ArchiveEntryWrapper::NewInstance(
          req->reader->archive_,
          req->entry);
        helpers::Emit(req->reader->handle_, "entry", entry_wrapper);
      }
    }

    req->reader->Unref();
    delete req;
    RETURN_ASYNC_AFTER
  }

  Handle<Value> ArchiveReader::NextEntry(const Arguments& args) {
    HandleScope scope;
    ArchiveReader* reader = ObjectWrap::Unwrap<ArchiveReader>(args.This());

    NextEntryRequest *req = new NextEntryRequest();
    req->reader = reader;
    req->error_string = NULL;
    req->entry = NULL;
    req->eof = false;

    BEGIN_ASYNC(req, NextEntryWork, NextEntryDone);
    reader->Ref();

    return scope.Close(Undefined());
  }
}
