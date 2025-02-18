#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <vector>

#include "rocsift/sift.h"
#include "rocsift/kfd.h"

std::string get_cmdline(int pid)
{
  std::stringstream ss;
  ss << "/proc/" << std::to_string(pid) << "/cmdline";
  std::ifstream cmdline_file(ss.str());

  // proc/<pid>/cmdline uses null characters instead of spaces
  std::vector<char> raw;
  while (!cmdline_file.eof()) {
    char c;
    cmdline_file >> c;
    raw.push_back(c);
  }
  std::replace_if(
      raw.begin(), raw.end(), [](char foo) { return foo == '\0'; }, ' ');
  return std::string(raw.begin(), raw.end());
}

int main(int argc, char** argv)

{
  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    std::cerr << "failed to initialize sift"
              << "\n";
    return -1;
  }

  sift_kfd_proc_list_t list;
  rc = sift_kfd_get_proc_list(&list);
  if (rc != SIFT_STATUS_SUCCESS) {
    std::cerr << "failed to get KFD process list"
              << "\n";
    return -1;
  }

  std::cout << std::setfill(' ') << std::setw(8) << "PID"
            << " " << std::setw(8) << "PASID"
            << " "
            << "CMD"
            << "\n";
  for (size_t i = 0; i < list.n; i++) {
    sift_kfd_proc_t& proc = list.procs[i];
    std::cout << std::setw(8) << proc.pid << " " << std::setw(8) << proc.pasid << " "
              << get_cmdline(proc.pid) << "\n";
  }

  rc = sift_destroy();
  if (rc != SIFT_STATUS_SUCCESS) {
    std::cerr << "failed to close sift"
              << "\n";
    return -1;
  }

  return 0;
}
