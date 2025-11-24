#include "Parser.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

void Parser::parseMovesToFile(const std::vector<math137::Vector3f>& moves, std::ofstream& file) {

    for (size_t i = 0; i < moves.size(); i++)
    {
        //format the float so that all numbers have 4 decimal places
        file << std::fixed << std::setprecision(4);
        file << "N" << std::to_string(i) << "G01X" << moves[i].x() << "Y" << -moves[i].z() << "Z" << moves[i].y() << "\n";
    }
}

std::vector<std::tuple<int, uint16_t, uint16_t>> Parser::ParseConfig(const std::string& filename) {
    std::vector<std::tuple<int, uint16_t, uint16_t>> configValues;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }
    while (file) {
        int val1;
        uint16_t val2, val3;
        file >> val1 >> val2 >> val3;
        configValues.emplace_back(val1, val2, val3);
    }

    return configValues;
}