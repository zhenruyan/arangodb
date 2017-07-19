//
// IResearch search engine 
// 
// Copyright (c) 2016 by EMC Corporation, All Rights Reserved
// 
// This software contains the intellectual property of EMC Corporation or is licensed to
// EMC Corporation from third parties. Use of this software and the intellectual property
// contained therein is expressly limited to the terms and conditions of the License
// Agreement under which it is provided by or on behalf of EMC.
// 

#include "tests_shared.hpp"
#include "formats/formats_10.hpp"
#include "store/memory_directory.hpp"

#include "index/index_meta.hpp"

#include <boost/algorithm/string/predicate.hpp>

using namespace iresearch;

namespace tests {
namespace detail {

void index_meta_read_write(iresearch::directory& dir, iresearch::format& codec) {
  auto writer = codec.get_index_meta_writer();

  /* check that there are no files in
 * a directory */
  std::vector<std::string> files;
  auto list_files = [&files] (std::string& name) {
    files.emplace_back(std::move(name));
    return true;
  };
  ASSERT_TRUE(dir.visit(list_files));
  ASSERT_TRUE(files.empty());

  /* create index metadata and
   * write it into the specified
   * directory */
  iresearch::index_meta meta_orig;

  ASSERT_TRUE(writer->prepare(dir, meta_orig));

  /* we should increase meta generation
   * after we write to directory */
  EXPECT_EQ(1, meta_orig.generation());

  /* check that files was successfully
   * written to directory */
  files.clear();
  ASSERT_TRUE(dir.visit(list_files));
  EXPECT_EQ(1, files.size());
  EXPECT_EQ(files[0], iresearch::string_ref("pending_segments_1"));

  writer->commit();

  /* create index metadata and
   * read it from specified the
   * directory */
  iresearch::index_meta meta_read;
  {
    std::string segments_file;

    auto reader = codec.get_index_meta_reader();
    const bool index_exists = reader->last_segments_file(dir, segments_file);

    ASSERT_TRUE(index_exists);
    reader->read(dir, meta_read, segments_file);
  }

  EXPECT_EQ(meta_orig.counter(), meta_read.counter());
  EXPECT_EQ(meta_orig.generation(), meta_read.generation());
  EXPECT_EQ(meta_orig.size(), meta_read.size());
}

} // detail
} // tests

TEST(index_meta_tests, memory_directory_read_write) {
  iresearch::version10::format codec;
  iresearch::memory_directory dir;
  tests::detail::index_meta_read_write(dir, codec);
}

TEST(index_meta_tests, ctor) {
  iresearch::index_meta meta;
  EXPECT_EQ(0, meta.counter());
  EXPECT_EQ(0, meta.size());
  EXPECT_EQ(type_limits<type_t::index_gen_t>::invalid(), meta.generation());
}

TEST(index_meta_tests, last_generation) {
  const char prefix[] = "segments_";

  std::vector<std::string> names;
  names.emplace_back("segments_387");
  names.emplace_back("segments_622");
  names.emplace_back("segments_314");
  names.emplace_back("segments_933");
  names.emplace_back("segments_660");
  names.emplace_back("segments_966");
  names.emplace_back("segments_074");
  names.emplace_back("segments_057");
  names.emplace_back("segments_836");
  names.emplace_back("segments_282");
  names.emplace_back("segments_882");
  names.emplace_back("segments_191");
  names.emplace_back("segments_965");
  names.emplace_back("segments_164");
  names.emplace_back("segments_117");
  
  // get max value
  uint64_t max = 0;
  for (const auto& s : names) {
    int num = atoi(s.c_str() + sizeof(prefix) - 1);
    if (num > max) {
      max = num;
    }
  }
  
  // populate directory
  iresearch::memory_directory dir;
  for (auto& name : names) {
    auto out = dir.create(name);
    ASSERT_FALSE(!out);
  }

  iresearch::version10::format codec;
  std::string last_seg_file;

  auto reader = codec.get_index_meta_reader();
  const bool index_exists = reader->last_segments_file(dir, last_seg_file);
  const std::string expected_seg_file = "segments_" + std::to_string(max);

  ASSERT_TRUE(index_exists);
  EXPECT_EQ(expected_seg_file, last_seg_file);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------