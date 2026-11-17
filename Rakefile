require 'erb'
require 'rake/clean'
require 'rake/loaders/makefile'
Rake.application.add_loader 'd', Rake::MakefileLoader.new

CC = 'gcc'
CFLAGS = ['-Wall']
LDFLAGS = []
INCLUDES = ['include', 'klib']
LIBS = []

SRC = FileList["src/**/*.c"]
OBJ = SRC.ext('.o')
DEP = SRC.ext('.d')
RESULT = File.basename(__dir__)

TESTS = FileList["test/test_*.c"]

CLEAN.include OBJ, DEP
CLOBBER.include RESULT

def build(dest, srcs)
  sh CC, *LDFLAGS, *LIBS.flat_map{ ['-L', _1] }, '-o', dest, *srcs
end

def compile(dest, src, test: false)
  incs = INCLUDES.flat_map{ ['-I', _1] }
  sh CC, *CFLAGS, *incs, '-MMD', '-MP', '-c', src, '-o', dest
end

desc "Build #{RESULT}"
task :build => RESULT
file RESULT => OBJ do |t|
  build t.name, t.prerequisites
end

RUNNER_TEMPLATE = <<EOS
#include "../Unity/src/unity.h"
<% functions.each do |func| %>
extern void <%= func %>(void);
<% end %>
int main(void) {
    UNITY_BEGIN();
    <% functions.each do |func| %>
    RUN_TEST(<%= func %>);
    <% end %>
    return UNITY_END();
}
EOS

def generate_testrunner(functions)
  ERB.new(RUNNER_TEMPLATE).result binding()
end

def parse_test_path(path)
  dir, file = File.split(path)
  base = File.basename(file, '.c')
  topic = base.delete_prefix('test_')
  {
    test:   File.join(dir, "#{base}"),
    runner: File.join(dir, "run_#{topic}"),
    topic:  topic,
    result: File.join(dir, base),
  }
end

def test_objs(test, runner)
  OBJ.dup.exclude('src/main.o').include("#{test}.o", "#{runner}.o", 'Unity/src/unity.o')
end

def extract_testfuncs(files)
  files.map{|file|
    src = File.open(file){|f| f.read }.gsub(/\/\*.*?\*\//m, '')
    src.lines.map{ /\A\s*void\s+(test_\w+)/ =~ it and $1 }
  }.flatten.compact.uniq
end

test_topics = []
namespace :test do
  TESTS.each do |path|
    h = parse_test_path(path)
    file h[:result] => test_objs(h[:test], h[:runner]) do |t|
      build t.name, t.prerequisites
    end
    file "#{h[:runner]}.c" => "#{h[:test]}.c" do |t|
      funcs = extract_testfuncs(t.prerequisites)
      File.write t.name, generate_testrunner(funcs)
    end
    test_topics << h[:topic]
    desc "Run #{h[:result]}"
    task h[:topic] => h[:result] do |t|
      sh(*t.prerequisites){|ok, stat|
        "#{t.name}: failed with status (#{stat.exitstatus})"
      }
    end
  end
end

desc "Run all tests"
task test: test_topics.map{ "test:#{it}" }

rule '.o' => '.c' do |t|
  compile t.name, t.source
end

DEP.select(&File.method(:exist?)).each(&method(:import))
