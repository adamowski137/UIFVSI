#pragma once
#include <string>
#include <vector>
#include <Vector.hpp>


class Parser {
public:
    static void parseMovesToFile(const std::vector<math137::Vector3f>& moves, std::ofstream& file);
};