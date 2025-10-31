#include "Parser.hpp"
#include <fstream>
#include <iomanip>

void Parser::parseMovesToFile(const std::vector<math137::Vector3f>& moves, std::ofstream& file) {

    for (size_t i = 0; i < moves.size(); i++)
    {
        //format the float so that all numbers have 4 decimal places
        file << std::fixed << std::setprecision(4);
        file << "N" << std::to_string(i) << "G01X" << moves[i].x() << "Y" << -moves[i].z() << "Z" << moves[i].y() << "\n";
    }
}