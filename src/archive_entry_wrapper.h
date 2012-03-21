// Copyright 2012 Karl Skomski MIT

#ifndef SRC_ARCHIVE_ENTRY_WRAPPER_H_
#define SRC_ARCHIVE_ENTRY_WRAPPER_H_

#include <archive.h>
#include <archive_entry.h>

#include <node.h>
#include "./node_buffer.h"

#include "./helpers.h"

namespace nodearchive {
  class ArchiveEntryWrapper : public node::ObjectWrap {
    public:
      static void Init(v8::Handle<v8::Object> target);
      static v8::Handle<v8::Value> NewInstance(
          archive *archive,
          archive_entry *entry);

    private:
      explicit ArchiveEntryWrapper(archive *archive);
      ~ArchiveEntryWrapper();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);
      static v8::Handle<v8::Value> NextChunk(const v8::Arguments& args);

      static async_rtn NextChunkWork(uv_work_t *job);
      static async_rtn NextChunkDone(uv_work_t *job);

      archive *archive_;
      static v8::Persistent<v8::Function> constructor;
  };
}

#endif  // SRC_ARCHIVE_ENTRY_WRAPPER_H_
