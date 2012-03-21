// Copyright 2012 Karl Skomski MIT

#include "./archive_reader.h"

extern "C" void init(v8::Handle<v8::Object> target) {
  nodearchive::ArchiveReader::Init(target);
  nodearchive::ArchiveEntryWrapper::Init(target);
}
