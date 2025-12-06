#ifndef ROUTER_H
#define ROUTER_H 

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <ostream>
#include <sstream>

inline int LOG_LEVEL = 4;
#define TRACE   if (LOG_LEVEL > 5) { std::cout << "TRACE: "
#define DEBUG   if (LOG_LEVEL > 4) { std::cout << "DEBUG: "
#define INFO    if (LOG_LEVEL > 3) { std::cout << "INFO: "
#define WARNING if (LOG_LEVEL > 2) { std::cout << "WARNING: "
#define ERROR   if (LOG_LEVEL > 1) { std::cout << "ERROR: "
#define FATAL   if (LOG_LEVEL > 0) { std::cout << "FATAL: "
#define ENDL  " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; }

class IPv4{
private:
  uint32_t ipaddr;
public:
  IPv4() : ipaddr(0) {}
  IPv4(uint32_t x) : ipaddr(x) {}
  IPv4(const std::string &s) : ipaddr(dotToInt(s)) {}
  static uint32_t dotToInt(const std::string &s);
  std::string intToDot() const;
  uint32_t toInt() const {return ipaddr;}
  friend std::ostream& operator<<(std::ostream& os, const IPv4& ip);
};

struct Interface{
  std::string interface;
  IPv4 ipaddr;
  int mask;
  bool isIn(const IPv4 destIP) const;
};
struct Route{
  IPv4 network;
  int mask;
  IPv4 hop;
};
#endif