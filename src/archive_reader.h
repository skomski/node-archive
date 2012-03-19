// Copyright 2012 Karl Skomski MIT

#ifndef ARCHIVE_READER_H__
#define ARCHIVE_READER_H__

#include "archive_entry.h"
#include <string>

namespace nodearchive {
  class ArchiveReader : public node::ObjectWrap {
    public:
      static void Init(v8::Handle<v8::Object> target);
    private:
      ArchiveReader(const char* filename);
      ~ArchiveReader();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);

      static v8::Handle<v8::Value> Open(const v8::Arguments& args);
      static v8::Handle<v8::Value> NextEntry(const v8::Arguments& args);

      static async_rtn OpenWork(uv_work_t *job);
      static async_rtn OpenDone(uv_work_t *job);

      static async_rtn NextEntryWork(uv_work_t *job);
      static async_rtn NextEntryDone(uv_work_t *job);

      struct archive *archive_;
      std::string filename_;
  };
}

#endif
