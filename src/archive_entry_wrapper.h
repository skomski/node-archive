// Copyright 2012 Karl Skomski MIT

#ifndef ARCHIVE_ENTRY_H__
#define ARCHIVE_ENTRY_H__

#include <node.h>
#include "node_buffer.h"
#include <archive.h>
#include <archive_entry.h>
#include "helpers.h"

namespace nodearchive {
  class ArchiveEntryWrapper : public node::ObjectWrap {
    public:
      static void Init(v8::Handle<v8::Object> target);
      static v8::Handle<v8::Value> NewInstance(archive *archive, archive_entry *entry);
    private:
      ArchiveEntryWrapper(archive *archive, archive_entry *entry);
      ~ArchiveEntryWrapper();

      static v8::Handle<v8::Value> New(const v8::Arguments& args);
      static v8::Handle<v8::Value> NextChunk(const v8::Arguments& args);

      static async_rtn NextChunkWork(uv_work_t *job);
      static async_rtn NextChunkDone(uv_work_t *job);

      struct archive *archive_;
      struct archive_entry *entry_;
      static v8::Persistent<v8::Function> constructor;
  };
}

#endif
