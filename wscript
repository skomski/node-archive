def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  conf.env.append_unique('CXXFLAGS', ['-Wall', '-O3', '-DNDEBUG'])

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.cxxflags = ['-Wall', '-g', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE']
  obj.target = 'binding'
  obj.source = 'src/binding.cc deps/libsnappy/snappy.cc deps/libsnappy/snappy-sinksource.cc'
  obj.includes = 'deps/libsnappy/'
  obj.install_path = None
