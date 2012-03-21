// Copyright 2012 Karl Skomski MIT

#ifndef SRC_ARCHIVE_READER_H_
#define SRC_ARCHIVE_READER_H_

#include "./archive_entry_wrapper.h"
#include <string>

namespace nodearchive {
  class ArchiveReader : public node::ObjectWrap {
    public:
      static void Init(v8::Handle<v8::Object> target);
    private:
      explicit ArchiveReader(const char* filename, size_t blocksize);
      ~ArchiveReader();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);

      static v8::Handle<v8::Value> Open(const v8::Arguments& args);
      static v8::Handle<v8::Value> NextEntry(const v8::Arguments& args);

      static async_rtn OpenWork(uv_work_t *job);
      static async_rtn OpenDone(uv_work_t *job);

      static async_rtn NextEntryWork(uv_work_t *job);
      static async_rtn NextEntryDone(uv_work_t *job);

      archive *archive_;
      std::string filename_;
      size_t blocksize_;
  };
}

#endif  // SRC_ARCHIVE_READER_H_
