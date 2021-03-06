// Contains general utilities for the codebase.

#ifndef SRC_UTIL_UTIL_H_
#define SRC_UTIL_UTIL_H_

#include <string>
#include <vector>

#include "opencv2/core/core.hpp"

namespace super_resolution {
namespace util {

constexpr char kCodeVersion[] = "0.1 (dev)";

// Initializes the app. Processes all of the command line arguments with gflags
// and initializes logging with glog. Sets the usage message and app version.
void InitApp(int argc, char** argv, const std::string& usage_message = "");

// Returns the root directory where this project was compiled. This uses the
// root path preprocessor macro specified by CMake. If for some reason this
// flag isn't defined, a warning will be logged and the local directory (".")
// will be returned instead.
std::string GetRootCodeDirectory();

// Returns the absolute path on the computer this code was compiled on of given
// relative path within the root code directory. For example,
//   GetAbsoluteCodePath("src/super_resolution.cpp")
// would return, for example,
//   "/Users/richard/Code/SuperResolution/src/super_resolution.cpp".
// Requires compilation using the provided CMake file.
std::string GetAbsoluteCodePath(const std::string& relative_path);

// Returns a list of all files in the given directory. If no files are present,
// returns an empty list. Subdirectories and hidden files are not included in
// the listing.
std::vector<std::string> ListFilesInDirectory(const std::string& directory);

// Returns the index into a pixel array given its channel (band), row, and
// column coordinates. This assumes the standard channel-row-col ordering on an
// array containing image data.
int GetPixelIndex(
    const cv::Size& image_size,
    const int channel,
    const int row,
    const int col);

}  // namespace util
}  // namespace super_resolution

#endif  // SRC_UTIL_UTIL_H_
