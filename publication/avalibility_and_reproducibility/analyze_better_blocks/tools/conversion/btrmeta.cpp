//
// Created by david on 29.04.22.
//
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

#include "gflags/gflags.h"

#include "datablock/BtrReader.hpp"
#include "datablock/schemes/CSchemePool.hpp"
#include "utils/Utils.hpp"

DEFINE_string(btr, "btr", "Directory with btr input");
DEFINE_int32(chunk, -1, "Chunk to inspect");
DEFINE_int32(column, -1, "Column to inspect");
DEFINE_string(fsst_stats, "", ""); // unused, defined to make linker not break

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::filesystem::path btr_dir = FLAGS_btr;
    cengine::db::CSchemePool::refresh();

    std::vector<char> raw_file_metadata;
    const cengine::db::FileMetadata *file_metadata;
    {
        auto metadata_path = btr_dir / "metadata";
        std::ifstream file(metadata_path, std::ios::binary | std::ios::ate);
        std::streamsize filesize = file.tellg();
        file.seekg(0, std::ios::beg);
        raw_file_metadata.resize(filesize);
        file.read(raw_file_metadata.data(), filesize);
        if (file.bad()) {
            throw Generic_Exception("Reading metadata failed");
        }
        file_metadata = reinterpret_cast<const cengine::db::FileMetadata *>(raw_file_metadata.data());
    }

    vector<u32> columns;
    if (FLAGS_column != -1) {
        columns.push_back(FLAGS_column);
    } else {
        u32 num_columns = file_metadata->num_columns;
        columns.resize(num_columns);
        std::iota(columns.begin(), columns.end(), 0);
    }

    std::unordered_map<u32, std::pair<ColumnType, std::unordered_map<std::string, std::vector<u32>>>> m;

    u32 total_tuples = 0;
    u32 chunk_i = 0;
    for (u32 column_i : columns) {
        for (u32 part_i = 0; part_i < file_metadata->parts[column_i].num_parts; part_i++) {
            thread_local std::vector<char> compressed_data;
            auto path = btr_dir / ("column" + std::to_string(column_i) + "_part" + std::to_string(part_i));
            cengine::Utils::readFileToMemory(path.string(), compressed_data);
            cengine::db::BtrReader reader(compressed_data.data());
            for (u32 part_chunk_i = 0; part_chunk_i < reader.getChunkCount(); part_chunk_i++) {
                if (FLAGS_chunk == -1 || FLAGS_chunk == chunk_i) {
                    total_tuples += reader.getTupleCount(part_chunk_i);
                    auto scheme_description = reader.getSchemeDescription(part_chunk_i);
                    if (m.count(column_i) == 0) {
                        m[column_i] = {reader.getColumnType(), {}};
                    }

                    if (m[column_i].second.count(scheme_description) == 0) {
                        // First time this scheme occurs
                        m[column_i].second[scheme_description] = {};
                    }
                    m[column_i].second[scheme_description].push_back(chunk_i);
                }
                chunk_i++;
            }
        }
    }

    // General info
    std::cout << "Looked at " << (FLAGS_chunk == -1 ? file_metadata->num_chunks : 1) << " chunks with " << columns.size() << " columns, a total of " << total_tuples << " tuples" << std::endl;
    std::cout << std::endl;
    for (auto &[column, p] : m) {
        auto &[type, scheme_map] = p;
        std::cout << "Column " << column << " with type " << ConvertTypeToString(type) << ":" << std::endl;
        for (auto &[scheme, chunks] : scheme_map) {
            std::sort(chunks.begin(), chunks.end());
            std::cout << chunks.size() << " chunks [ ";
            for (auto chunk : chunks) {
                std::cout << chunk << " ";
            }
            std::cout << "] use" << std::endl;
            std::cout << scheme << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}
