#include "presentation/ConsoleIO.h"

#include <iostream>

namespace lms::presentation {

void ConsoleIO::printLine(const std::string& message) {
    std::cout << message << '\n';
}

} // namespace lms::presentation
