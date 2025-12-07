#include "router.h"
#include <vector>
#include <fstream>
#include <regex>
uint32_t IPv4::dotToInt(const std::string &s){
  uint32_t oct[4];
  char dot;
  std::stringstream sd(s);
  if(!(sd >> oct[0] >> dot >> oct[1] >> dot >> oct[2] >> dot >> oct[3])){
    std::cerr << "bad format for string ipv4" << std::endl;
  }
  uint32_t convInt = (oct[0] << 24) | (oct[1] << 16) | (oct[2] << 8 | oct[3]);
  return convInt;
}
std::vector<Interface> interfaceConfig;
std::vector<Route> routingTable;
std::string IPv4::intToDot() const{
  //bitwise and is &
  uint32_t oct1 = (ipaddr >> 24) & 0xFF;
  uint32_t oct2 = (ipaddr >> 16) & 0xFF;
  uint32_t oct3 = (ipaddr >> 8) & 0xFF;
  uint32_t oct4 = ipaddr & 0xFF;
  std::string convString = std::to_string(oct1) + "." + std::to_string(oct2) + "." + std::to_string(oct3) + "." + std::to_string(oct4);
  return convString;
}

std::ostream& operator<<(std::ostream& os, const IPv4& ip){
  return os << ip.intToDot();
}

std::string configFile;
std::string routeFile;
std::string inputFile = "";
std::string outputFile = "";

int main (int argc, char *argv[]) {
  
//-c <interface configuration file>
//-r <route table>
//-i <input file>. If the input file is not specified, read from stdin
//-o <output file> If the output file is not specified, write to stdout
//-d <debug level> If the debug level is not specified, set the level to 4.
//
  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  while ((opt = getopt(argc,argv,"c:r:i:o:d:")) != -1) {
    switch (opt) {
    case 'c':
      configFile = optarg;
      break;
    case 'r':
      routeFile = optarg;
      break;
    case 'i':
      inputFile = optarg;
      break;
    case 'o':
      outputFile = optarg;
      break;
    case 'd':
      LOG_LEVEL = atoi(optarg);;
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -c [Config File] -r [Route File] -i [Input File] -o [Output File] -d [Debug Level]" << std::endl;
      exit(-1);
    }
  }
  if(configFile.empty() || routeFile.empty()){
    std::cerr << "Missing config and route file content\n" << std::endl;
  }
  std::cout << "Log Level " << LOG_LEVEL << std::endl;


  // ********************************************************************
  // * Read configuration
  // ********************************************************************
  //which interfaces (eth0...) exist and ips/subnets they have
  //which networks it canr each through next hop (route table)
  std::regex icReg(R"(\s*([a-zA-Z0-9]+)\s+(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/(\d{1,2})\s*)");
  std::regex rcReg(R"(\s*(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/(\d{1,2})\s+(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\s*)");

  std::ifstream ic(configFile);
  if(ic.is_open()){
    std::string wholeLineC;
    while(std::getline(ic, wholeLineC)){
      std::cout << wholeLineC << std::endl;
      std::string ccpy = wholeLineC;
      auto startC = ccpy.find_first_not_of(" \t");
      if(ccpy[startC] == '#' || ccpy[startC] == std::string::npos){
        continue;
      }
      std::smatch imatch;
      if(std::regex_match(wholeLineC, imatch, icReg)){
        std::string interface_name = imatch[1];
        std::string interface_address = imatch[2];
        int interface_prefix = std::stoi(imatch[3]);

        Interface newInterface;
        newInterface.interface = interface_name;
        newInterface.ipaddr = IPv4(interface_address);
        newInterface.mask = interface_prefix;
        interfaceConfig.push_back(newInterface);
      }else{
        std::cout << "something wrong with regex in this line: " << wholeLineC << std::endl;
      }
    }
  }
  std::ifstream rc(routeFile);
  if(rc.is_open()){
    std::string wholeLineR;
    while(std::getline(rc, wholeLineR)){
      std::cout << wholeLineR << std::endl;
    }
  }
  // ********************************************************************
  // * read packets
  // ********************************************************************
  //dest ip addresses
  if(!inputFile.empty()){
    std::ifstream in(inputFile);
    if(in.is_open()){
      std::string wholeLineIn;
      while(std::getline(in,wholeLineIn)){
        
      }
    }
  }
  // ********************************************************************
  // * for every dest ip (line by line of input)
  // ********************************************************************
  //if belongs to directly connected subnet (interfaces)
      //send directly thru that interface to that ip
    //else
      //find best route by doing longest prefix match


  // ********************************************************************
  // * output
  // ********************************************************************
  //print out the decision of interface/next hop or if it is unreachebal.
}