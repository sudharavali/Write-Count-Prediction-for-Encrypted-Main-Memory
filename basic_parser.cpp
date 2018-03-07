#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

int main() {
   
   unsigned long cacheLine;
   std::string instrAddr, memOp, memAddr;	
   std::unordered_map<unsigned long, int> mainMemory;
   std::ifstream infile("addr_traces");
   std::string fileLine;
   while (std::getline(infile, fileLine)) {
	std::istringstream traces(fileLine);
	traces >> instrAddr >> memOp >> memAddr;
	std::cout << memAddr << std::endl;
	cacheLine = std::stoul(memAddr, nullptr, 16) / 64;
	std::cout << cacheLine << std::endl;
	mainMemory[cacheLine] = 0;
   }
   std::cout << mainMemory.size() <<std::endl;

}
