// Copyright 2012 Karl Skomski MIT

#include "archive_entry.h"


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

  ArchiveEntry::ArchiveEntry(archive *archive, archive_entry *entry) :
    archive_(archive),
    entry_(entry) {

  }
  ArchiveEntry::~ArchiveEntry() {
    //archive_entry_free(entry_);
  }

  Persistent<Function> ArchiveEntry::constructor;

  void ArchiveEntry::Init(Handle<Object> target) {
    Local<FunctionTemplate> entry_template = FunctionTemplate::New(New);
    entry_template->SetClassName(String::NewSymbol("ArchiveEntry"));
    entry_template->InstanceTemplate()->SetInternalFieldCount(1);

    entry_template->PrototypeTemplate()->Set(String::NewSymbol("nextChunk"),
        FunctionTemplate::New(NextChunk)->GetFunction());

    constructor = Persistent<Function>::New(entry_template->GetFunction());
    target->Set(String::NewSymbol("Entry"), constructor);
  }

  Handle<Value> ArchiveEntry::New(const Arguments& args) {
    HandleScope scope;

    Local<External> parent_archive = Local<External>::Cast(args[0]);
    Local<External> entry = Local<External>::Cast(args[1]);

    ArchiveEntry* reader = new ArchiveEntry(
        static_cast<archive*>(parent_archive->Value()),
        static_cast<archive_entry*>(entry->Value()));

    reader->Wrap(args.This());

    return scope.Close(args.This());
  }

  Handle<Value> ArchiveEntry::NewInstance(archive *archive, archive_entry *entry) {
    HandleScope scope;

    Local<Value> argv[2] = { External::New(archive), External::New(entry) };
    Local<Object> instance = constructor->NewInstance(2, argv);

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

  struct extract_request {
    ArchiveEntry *archive_entry;
    const char* error_string;
    const void* output_data;
    size_t output_length;
    off_t offset;
    bool eof;
  };

  async_rtn ArchiveEntry::NextChunkWork(uv_work_t *job) {
    extract_request *req = static_cast<extract_request*>(job->data);

    int return_value = archive_read_data_block(
        req->archive_entry->archive_,
        &req->output_data,
        &req->output_length,
        &req->offset);

    if (return_value == ARCHIVE_EOF) {
      req->eof = true;
    } else if (return_value != ARCHIVE_OK) {
      req->error_string = archive_error_string(req->archive_entry->archive_);
    }

    RETURN_ASYNC
  }

  async_rtn ArchiveEntry::NextChunkDone(uv_work_t *job) {
    v8::HandleScope scope;
    extract_request *req = static_cast<extract_request*>(job->data);

    if(req->eof == true) {
      helpers::Emit(req->archive_entry->handle_, "end", Undefined());
    } else if (req->error_string != NULL) {
      helpers::EmitError(req->archive_entry->handle_, req->error_string);
    } else {
       v8::Handle<v8::Object> buffer = node::Buffer::New(
      (char*)req->output_data, req->output_length)->handle_;
       helpers::Emit(req->archive_entry->handle_, "data", buffer);
    }

    delete req;
    RETURN_ASYNC_AFTER
  }

  Handle<Value> ArchiveEntry::NextChunk(const Arguments& args) {
    HandleScope scope;
    ArchiveEntry* entry = ObjectWrap::Unwrap<ArchiveEntry>(args.This());

    if (archive_entry_size(entry->entry_) > 0) {
      extract_request *req = new extract_request;
      req->archive_entry = entry;
      req->error_string = NULL;
      req->eof = false;

      BEGIN_ASYNC(req, NextChunkWork, NextChunkDone);
      entry->Ref();
    } else {
      helpers::Emit(entry->handle_, "end", Undefined());
    }

    return scope.Close(Undefined());
  }
}
