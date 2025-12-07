#include "router.h"
#include <vector>
#include <fstream>
#include <regex>
std::ofstream outFile;
std::ostream* out = &std::cout;
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
std::vector<Route> routeConfig;
void dumpInterfaces(const std::vector<Interface>& interfaces){
  for(const auto& entry : interfaces){
    std::cout << "Name: " << entry.interface << " Address: " << entry.ipaddr << " Prefix: " << entry.prefix << std::endl; 
  }
}
void dumpRoutes(const std::vector<Route>& routes){
  for(const auto& entry : routes){
    std::cout << "Address: " << entry.network << " Prefix: " << entry.prefix << " Next Hop: " << entry.hop << std::endl; 
  }
}
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

// ********************************************************************
// *check if directly connected or do longested prefix
// *print out the decision of interface/next hop or if it is unreachebal.
// ********************************************************************
uint32_t createMask(uint32_t prefixNum){
  uint32_t mask = 0xFFFFFFFF;
  if(prefixNum == 0){
    mask = 0;
    return mask;
  }
  mask = mask << (32-prefixNum);

  //uint32_t bits1 = 0;
  //bits1 = ~bits1;
  //if(prefixNum != 0){
  //  mask = (bits1 << (32-prefixNum));
  //}else{
  //  mask = 0;
  //}
  return htonl(mask);
}

bool interfaceMatch(const IPv4& destIP,const Interface& interface){
  uint32_t interface_prefix = interface.prefix;
  uint32_t imask = createMask(interface_prefix);
  uint32_t dip = htonl(destIP.getIpInt());
  uint32_t iip = htonl(interface.ipaddr.getIpInt());
  //std::cout << "Testing directly connected match on interface " << interface.ipaddr.intToDot() << " with prefix " << interface_prefix << ", and destination ip " << destIP.intToDot()<< std::endl;
  //std::cout << "Created mask from prefix: " << imask << "bitset32: " << std::bitset<32>(ntohl(imask)) <<std::endl;
  bool match = (dip & imask) == (iip & imask );
  if(match){
    //std::cout << "Conclusion: Interface is a match" << std::endl;
    return true;
  }
  //std::cout << "Conclusion: Interface NOT a match" << std::endl;
  return false;
}

bool routeMatch(const IPv4& destIP, const Route& route){
  uint32_t route_prefix = route.prefix;
  uint32_t rmask = createMask(route_prefix);
  uint32_t dip = htonl(destIP.getIpInt());
  uint32_t rip = htonl(route.network.getIpInt());
  //std::cout << "Testing route match on route" << route.network.intToDot() << " with prefix " << route_prefix << ", and destination ip " << destIP.intToDot() << std::endl;
  //std::cout << "Created mask from prefix: " << rmask << "bitset32: " << std::bitset<32>(ntohl(rmask)) <<std::endl;
  bool match = (dip & rmask) == (rip & rmask);
  if(match){
    //std::cout << "Conclusion: Route is a match" << std::endl;
    return true;
  }
  //std::cout << "Conclusion: Route NOT a match" << std::endl;
  return false;
}

void select_route(IPv4 destinationIP){
  Interface* lp_directCon = nullptr;
  for(auto& interface : interfaceConfig){
    if(interfaceMatch(destinationIP, interface)){
      //yay
      if(lp_directCon == nullptr || interface.prefix > lp_directCon->prefix){
        lp_directCon = &interface;
      }
    }
  }
  if(lp_directCon){
    (*out) << destinationIP << ": " << lp_directCon->interface << " -> " << destinationIP << std::endl;
    return;
  }
  
  //if not check routes
  Route* lp_route = nullptr;
  for(auto& route : routeConfig){
    if(routeMatch(destinationIP, route)){
      if((lp_route == nullptr) || (route.prefix > lp_route->prefix)){
        lp_route = &route;
      }
    }
  }
  if(lp_route == nullptr){
    //if still havent found, unreachable
    (*out) << destinationIP << ": unreachable" << std::endl;
    return;
  }

  Interface* lp_nexthop = nullptr;
  for(auto& interface : interfaceConfig){
    if(interfaceMatch(lp_route->hop, interface)){
      //yay instread
      if(lp_nexthop == nullptr || interface.prefix > lp_nexthop->prefix){
        lp_nexthop = &interface;
      }
    }
  }
  if(lp_nexthop){
    (*out) << destinationIP << ": " << lp_nexthop->interface << " -> " << lp_route->hop << std::endl;
    return;
  }
  //still????
  //unreachable
  (*out) << destinationIP << ": unreachable2" << std::endl;
  return;

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
    std::cerr << "Missing config and route file content" << std::endl;
    exit(1);
  }
  //std::cout << "Log Level " << LOG_LEVEL << std::endl;


  // ********************************************************************
  // *read interfaces and routes
  // ********************************************************************
  std::regex icReg(R"(\s*([a-zA-Z0-9]+)\s+(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/(\d{1,2})\s*)");
  std::regex rcReg(R"(\s*(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/(\d{1,2})\s+(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\s*)");

  std::ifstream ic(configFile);
  if(ic.is_open()){
    std::string wholeLineC;
    while(std::getline(ic, wholeLineC)){
      //std::cout << wholeLineC << std::endl;
      //ignore garbage
      std::string ccpy = wholeLineC;
      auto startC = ccpy.find_first_not_of(" \t");
      if(startC == std::string::npos || ccpy[startC] == '#'){
        continue;
      }
      //fill structure from reg exp
      std::smatch imatch;
      if(std::regex_match(wholeLineC, imatch, icReg)){
        std::string interface_name = imatch[1];
        std::string interface_address = imatch[2];
        int interface_prefix = std::stoi(imatch[3]);

        Interface newInterface;
        newInterface.interface = interface_name;
        newInterface.ipaddr = IPv4(interface_address);
        newInterface.prefix = static_cast<uint32_t>(interface_prefix);
        interfaceConfig.push_back(newInterface);
      }else{
        std::cout << "something wrong with interface regex in this line: " << wholeLineC << std::endl;
      }
    }
  }
  std::ifstream rc(routeFile);
  if(rc.is_open()){
    std::string wholeLineR;
    while(std::getline(rc, wholeLineR)){
      //std::cout << wholeLineR << std::endl;
      //ignore garbage
      std::string rcpy = wholeLineR;
      auto startR = rcpy.find_first_not_of(" \t");
      if(startR == std::string::npos || rcpy[startR] == '#'){
        continue;
      }
      //fill structure from reg exp
      std::smatch rmatch;
      if(std::regex_match(wholeLineR, rmatch, rcReg)){
        std::string route_network = rmatch[1];
        int route_prefix = std::stoi(rmatch[2]);
        std::string route_nexthop= rmatch[3];

        Route newRoute;
        newRoute.network = IPv4(route_network);
        newRoute.prefix = static_cast<uint32_t>(route_prefix);
        newRoute.hop = IPv4(route_nexthop);
        routeConfig.push_back(newRoute);
      }else{
        std::cout << "something wrong with route regex in this line: " << wholeLineR << std::endl;
      }
    }
  }

  // ****************
  // * test structure
  // ****************
  //std::cout << "Dumping Interfaces" << std::endl;
  //dumpInterfaces(interfaceConfig);
  //std::cout << "\nDumping Routes" << std::endl;
  //dumpRoutes(routeConfig);
  //std::cout << std::endl;

  // ********************************************************************
  // * read dest ip addresses
  // ********************************************************************
  if(!outputFile.empty()){
    outFile.open(outputFile);
    if(!outFile){
      std::cerr << "Something wrong with output file" << std::endl;
      exit(1);
    }
    out = &outFile;
  }
  if(!inputFile.empty()){
    std::ifstream in(inputFile);
    if(in.is_open()){
      std::string wholeLineIn;
      while(std::getline(in,wholeLineIn)){
        std::string icpy = wholeLineIn;
        auto startI = icpy.find_first_not_of(" \t");
        if(startI == std::string::npos || icpy[startI] == '#'){
          continue;
        }
        std::string ip_line = icpy.substr(startI);
        IPv4 readDestination = IPv4(ip_line);
        select_route(readDestination);
      }
    }
  }else{
    std::string wholeLineIn;
    while(std::getline(std::cin, wholeLineIn)){
      std::string icpy = wholeLineIn;
      auto startI = icpy.find_first_not_of(" \t");
      if(startI == std::string::npos || icpy[startI] == '#'){
        continue;
      }
      std::string ip_line = icpy.substr(startI);
      IPv4 readDestination = IPv4(ip_line);
      select_route(readDestination);
    }
  }
  return 0;
}