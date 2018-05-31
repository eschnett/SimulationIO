#include <Config.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace SimulationIO {

int versionMajor() { return SIMULATIONIO_VERSION_MAJOR; }
int versionMinor() { return SIMULATIONIO_VERSION_MINOR; }
int versionPatch() { return SIMULATIONIO_VERSION_PATCH; }

std::string Version() { return SIMULATIONIO_VERSION; }

void checkVersion(const char *header_version) {
  if (header_version != Version()) {
    std::cerr
        << "Version mismatch detected -- aborting.\n"
        << "  Include headers have version " << header_version << ",\n"
        << "  Linked library has version " << Version() << ".\n"
        << "(The versions of the include headers and linked libraries differ.\n"
        << "This points to an improperly installed library or\n"
        << "improperly installed application.)\n";
    std::exit(EXIT_FAILURE);
  }
}

} // namespace SimulationIO
