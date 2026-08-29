#include "finelemethod/input/abaqus_input_file.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace finelemethod::input
{
std::string read_abaqus_input_file(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Unable to open ABAQUS input file: " + path.string());
    }

    std::string contents{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (file.bad())
    {
        throw std::runtime_error("Unable to read ABAQUS input file: " + path.string());
    }
    if (contents.empty())
    {
        throw std::runtime_error("ABAQUS input file is empty: " + path.string());
    }
    return contents;
}
} // namespace finelemethod::input
