

#ifndef GBEMU_SRC_TESTS_TESTUTILS_H_
#define GBEMU_SRC_TESTS_TESTUTILS_H_

#include <vector>
#include <filesystem>


std::filesystem::path getTestRoot();

std::vector<uint8_t> readFile(const std::filesystem::path& path);


#endif // GBEMU_SRC_TESTS_TESTUTILS_H_