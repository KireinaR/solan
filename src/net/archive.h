#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Zips the given files into output_zip_path, flattening them to their
// basenames. Throws if any input is missing.
void create_zip(const std::vector<std::string> &filepaths, const std::string &output_zip_path);

// Combined on-disk size of the inputs, i.e. the payload before compression.
// Missing files count as zero.
uint64_t total_size(const std::vector<std::string> &filepaths);
