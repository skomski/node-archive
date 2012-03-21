// Copyright 2012 Karl Skomski MIT

#include "./archive_entry_wrapper.h"

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
  using v8::Date;

  ArchiveEntryWrapper::ArchiveEntryWrapper(archive *archive) :
    archive_(archive) {
  }

  ArchiveEntryWrapper::~ArchiveEntryWrapper() {}

  Persistent<Function> ArchiveEntryWrapper::constructor;

  void ArchiveEntryWrapper::Init(Handle<Object> target) {
    Local<FunctionTemplate> entry_template = FunctionTemplate::New(New);
    entry_template->SetClassName(String::NewSymbol("ArchiveEntryWrapper"));
    entry_template->InstanceTemplate()->SetInternalFieldCount(1);

    entry_template->PrototypeTemplate()->Set(String::NewSymbol("nextChunk"),
        FunctionTemplate::New(NextChunk)->GetFunction());

    constructor = Persistent<Function>::New(entry_template->GetFunction());
    target->Set(String::NewSymbol("Entry"), constructor);
  }

  Handle<Value> ArchiveEntryWrapper::New(const Arguments& args) {
    HandleScope scope;

    Local<External> parent_archive = Local<External>::Cast(args[0]);

    ArchiveEntryWrapper* reader = new ArchiveEntryWrapper(
        static_cast<archive*>(parent_archive->Value()));

    reader->Wrap(args.This());

    return scope.Close(args.This());
  }

  Handle<Value> ArchiveEntryWrapper::NewInstance(
      archive *archive, archive_entry *entry) {
    HandleScope scope;

    Local<Value> argv[1] = { External::New(archive) };
    Local<Object> instance = constructor->NewInstance(1, argv);

    if (archive_format_name(archive) != NULL) {
      instance->Set(
          String::NewSymbol("format"),
          String::New(archive_format_name(archive)));
    }

    if (archive_entry_pathname(entry) != NULL) {
      instance->Set(
          String::NewSymbol("path"),
          String::New(archive_entry_pathname(entry)));
    }

    if (archive_entry_size_is_set(entry) != 0) {
      instance->Set(
          String::NewSymbol("size"),
          Integer::New(archive_entry_size(entry)));
    }

    if (archive_entry_mtime_is_set(entry) != 0) {
      instance->Set(
          String::NewSymbol("mtime"),
          Date::New(archive_entry_mtime(entry) * 1000));
    }

    if (archive_entry_ctime_is_set(entry) != 0) {
      instance->Set(
          String::NewSymbol("ctime"),
          Date::New(archive_entry_ctime(entry) * 1000));
    }

    if (archive_entry_atime_is_set(entry) != 0) {
      instance->Set(
          String::NewSymbol("atime"),
          Date::New(archive_entry_atime(entry) * 1000));
    }

    instance->Set(
        String::NewSymbol("gid"),
        Integer::New(archive_entry_gid(entry)));

    instance->Set(
        String::NewSymbol("uid"),
        Integer::New(archive_entry_uid(entry)));

    instance->Set(
        String::NewSymbol("access"),
        Integer::New(archive_entry_perm(entry)));

    instance->Set(
        String::NewSymbol("type"),
        Integer::New(archive_entry_filetype(entry)));

    return scope.Close(instance);
  }

  struct ExtractRequest {
    ArchiveEntryWrapper *entry_wrapper;
    const char* error_string;
    const void* output_data;
    size_t output_length;
    off_t offset;
  };

  async_rtn ArchiveEntryWrapper::NextChunkWork(uv_work_t *job) {
    ExtractRequest *req = static_cast<ExtractRequest*>(job->data);

    int return_value = archive_read_data_block(
        req->entry_wrapper->archive_,
        &req->output_data,
        &req->output_length,
        &req->offset);

    if (return_value != ARCHIVE_EOF && return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->entry_wrapper->archive_);
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveEntryWrapper::NextChunkDone(uv_work_t *job) {
    v8::HandleScope scope;
    ExtractRequest *req = static_cast<ExtractRequest*>(job->data);

    if (req->error_string != NULL) {
      helpers::EmitError(req->entry_wrapper->handle_, req->error_string);
    } else {
      if (req->output_data == NULL) {
        helpers::Emit(req->entry_wrapper->handle_, "end", Undefined());
      } else {
        char *data = static_cast<char*>(const_cast<void*>(req->output_data));
        Handle<Object> buffer = node::Buffer::New(
          data, req->output_length)->handle_;

        helpers::Emit(req->entry_wrapper->handle_, "data", buffer);
      }
    }

    req->entry_wrapper->Unref();
    delete req;
    RETURN_ASYNC_AFTER
  }

  Handle<Value> ArchiveEntryWrapper::NextChunk(const Arguments& args) {
    HandleScope scope;
    ArchiveEntryWrapper* entry = ObjectWrap::Unwrap<ArchiveEntryWrapper>(
        args.This());

    ExtractRequest *req = new ExtractRequest;
    req->entry_wrapper = entry;
    req->error_string  = NULL;
    req->output_data   = NULL;

    BEGIN_ASYNC(req, NextChunkWork, NextChunkDone);
    entry->Ref();

    return scope.Close(Undefined());
  }
}
