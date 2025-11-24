#pragma once
#include <string>
#include <vector>
#include <Vector.hpp>


class Parser {
public:
    static void parseMovesToFile(const std::vector<math137::Vector3f>& moves, std::ofstream& file);
    static std::vector<std::tuple<int, uint16_t, uint16_t>> ParseConfig(const std::string& filename);
};