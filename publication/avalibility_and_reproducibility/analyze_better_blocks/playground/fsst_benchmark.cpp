#include <gflags/gflags.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "gflags/gflags.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/ranges.h"

#include "PerfEvent.hpp"
#include "PerfExternal.hpp"
#include "MMapvector.hpp"

#include "fsst.h"

using std::stringstream;

DEFINE_string(fsst_stats, "", "");
DEFINE_string(file_list_file, "pbi-string-columns.txt", "file-list");
DEFINE_bool(log_info, false, "log");

struct InputFiles {
  std::ifstream list;
  InputFiles(const std::string& filename) : list(filename)
  {
    std::cout << "file " << filename << std::endl;
  }

  bool next(std::string& output)
  {
    return !(std::getline(list, output).fail());
  }
};


std::string ensure_file(const std::string& object)
{
  static const std::string bucket = "s3://public-bi-benchmark/binary/";
  std::string outfile = "columns/" + object;
  stringstream _cmd;
  _cmd << "bash -c 'mkdir -p columns; test -f \"" << outfile
       << "\" && echo \"file exists, skipping download\" || (echo "
          "\"downloading file\"; aws s3 cp \""
       << bucket << object << "\" \"" << outfile << "\")' 1>&2";
  std::string cmd(_cmd.str());
  spdlog::info("running {}", cmd);
  system(cmd.c_str());
  return outfile;
}

using Column = Vector<std::string_view>;
constexpr size_t SCRATCH_SIZE = 3ul*1024*1024*1024;

struct FSSTRuntime {
    inline static unsigned char* compressed;
    inline static unsigned char* decompressed;

    unsigned char* input_data_start;
    size_t len_sum{0};
    Column& input;
    unsigned char ** input_buffers, **output_buffers;
    uint64_t* input_lengths, *output_lengths;

    explicit FSSTRuntime(Column& input)
        : input(input)
        , input_buffers(new unsigned char*[input.size()])
        , output_buffers(new unsigned char*[input.size()])
        , input_lengths(new uint64_t[input.size()])
        , output_lengths(new uint64_t[input.size()]) {

        if (!compressed) {
            compressed = new unsigned char[SCRATCH_SIZE];
        }

        if (!decompressed) {
            decompressed = new unsigned char[SCRATCH_SIZE];
        }

        input_data_start = reinterpret_cast<unsigned char*>(const_cast<char*>(input[0].data()));
        for (unsigned str_i = 0; str_i < input.size(); str_i++) {
            auto strview = input[str_i];
            auto* ptr = reinterpret_cast<unsigned char*>(const_cast<char*>(strview.data()));
            input_data_start = std::min(ptr, input_data_start);
            // fsst interface does not specify const :-<
            input_buffers[str_i] = ptr;
            input_lengths[str_i] = strview.size();
            len_sum += strview.size();
        }
    }

    ~FSSTRuntime() {
        delete[] input_lengths;
        delete[] input_buffers;
        delete[] output_lengths;
        delete[] output_buffers;
    }

    size_t size() const { return input.size(); }

    void assert_equal(size_t size) {
       die_if(memcmp(input_data_start, decompressed, size) == 0);
    }

    void clear(bool compr = true) {
        memset(decompressed, 0, SCRATCH_SIZE);
        if (compr) {
            memset(compressed, 0, SCRATCH_SIZE);
        }
    }
};

uint64_t compress(FSSTRuntime& rt, PerfEvent& e)
{
  uint64_t fsst_strings_used_space;
  e.setParam("phase", "compress");
  {
    //PerfEventBlock perf(e, rt.size());
    auto* write_ptr = rt.compressed;
    auto* encoder =
        fsst_create(rt.size(), rt.input_lengths, rt.input_buffers, 0);
    die_if(fsst_export(encoder, write_ptr) > 0);
    auto fsst_table_used_space = FSST_MAXHEADER;
    write_ptr += fsst_table_used_space;

    auto count = fsst_compress(encoder, rt.size(), rt.input_lengths,
                               rt.input_buffers, SCRATCH_SIZE - FSST_MAXHEADER,
                               write_ptr, rt.output_lengths, rt.output_buffers);
    die_if(count == rt.size());

    fsst_strings_used_space =
        rt.output_lengths[rt.size() - 1] +
        (rt.output_buffers[rt.size() - 1] - rt.output_buffers[0]);

    fsst_destroy(encoder);
  }
  return fsst_strings_used_space;
}

constexpr size_t decomp_cnt = 1e9;

void test_individual(FSSTRuntime& rt, PerfEvent& e, uint64_t& fsst_strings_used_space) {
    if(fsst_strings_used_space == 0) { fsst_strings_used_space = compress(rt, e); };
    e.setParam("phase", "decompress");
    uint64_t decompressed_size;
    size_t repeat = std::max(1ul, decomp_cnt/rt.size());
    {
        PerfEventBlock perf(e, repeat*rt.size());
    //PerfExternalBlock perf;
        auto* read_ptr = rt.compressed;
        auto* write_ptr = rt.decompressed;

        fsst_decoder_t decoder;
        auto header_bytes = fsst_import(&decoder, read_ptr);
        die_if(header_bytes > 0 && header_bytes < FSST_MAXHEADER);
        read_ptr += FSST_MAXHEADER;


        for (auto i = 0u; i != repeat; ++i) {

        write_ptr =  rt.decompressed;
        read_ptr = rt.compressed + FSST_MAXHEADER;
          //#pragma GCC unroll 32
          for (auto str_i = 0u; str_i != rt.size(); ++str_i) {
            auto len = rt.output_lengths[str_i];
            auto bytes = fsst_decompress(&decoder, len, read_ptr, SCRATCH_SIZE,
                                         write_ptr);
            // assert(bytes == rt.input_lengths[str_i]);
            read_ptr += len;
            write_ptr += bytes;
          }
        }
        decompressed_size = write_ptr - rt.decompressed;
    }
    rt.assert_equal(decompressed_size);
}

void test_batch(FSSTRuntime& rt, PerfEvent& e, uint64_t& fsst_strings_used_space) {
    if(fsst_strings_used_space == 0) { fsst_strings_used_space = compress(rt, e); };
    e.setParam("phase", "decompress");
    uint64_t decompressed_size;

    size_t repeat = std::max(1ul, decomp_cnt/rt.size());
    {
        PerfEventBlock perf(e, repeat*rt.size());
        //PerfExternalBlock perf;
        auto* read_ptr = rt.compressed;
        auto* write_ptr = rt.decompressed;

        fsst_decoder_t decoder;
        auto header_bytes = fsst_import(&decoder, read_ptr);
        die_if(header_bytes > 0 && header_bytes < FSST_MAXHEADER);
        read_ptr += FSST_MAXHEADER;

        auto compressed_strings = read_ptr;

        for (auto i = 0u; i != repeat; ++i) {
        write_ptr =  rt.decompressed;
        read_ptr = rt.compressed + FSST_MAXHEADER;
        write_ptr += fsst_decompress(
            &decoder, fsst_strings_used_space, compressed_strings,
            SCRATCH_SIZE, write_ptr);
        }
        decompressed_size = write_ptr - rt.decompressed;
    }
    rt.assert_equal(decompressed_size);
}



int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    spdlog::set_level(FLAGS_log_info ? spdlog::level::info : spdlog::level::warn);
    PerfEvent perf;
    std::cerr << "using input file " << FLAGS_file_list_file << std::endl;

    InputFiles file_list(FLAGS_file_list_file);

    std::string nextfile;
    int i = 0;
    while (file_list.next(nextfile)) {
      if (i++ > 10) { break; }

      std::string uncompfile = ensure_file(nextfile);
      Column input(uncompfile.c_str());

      FSSTRuntime rt(input);
      uint64_t compressed_size = compress(rt, perf);

      perf.setParam("column", nextfile);
      perf.setParam("strlen", ((double)rt.len_sum)/rt.size());
      perf.setParam("compr", ((double)rt.len_sum)/compressed_size);
      perf.setParam("count", ((double)rt.len_sum)/compressed_size);

      perf.setParam("type", "single");
      test_individual(rt, perf, compressed_size);

      rt.clear(false);

      perf.setParam("type", "single");
      test_individual(rt, perf, compressed_size);

      rt.clear(false);

      perf.setParam("type", "batch");
      test_batch(rt, perf, compressed_size);

      rt.clear();
    }
}
