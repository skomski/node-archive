#include "archive_reader.h"

extern "C" void init(v8::Handle<v8::Object> target) {
  nodearchive::ArchiveReader::Init(target);
  nodearchive::ArchiveEntry::Init(target);
}
