#include "archive_reader_wrapper.h"

extern "C" void init(v8::Handle<v8::Object> target) {
  nodearchive::ArchiveReaderWrapper::Init(target);
  nodearchive::ArchiveEntryWrapper::Init(target);
}
