#ifndef ESAVEPATH_H
#define ESAVEPATH_H

#include <filesystem>
#include <string>

namespace eSavePath {
inline bool isDeletable(const std::string& folder,
                        const std::string& candidate) {
    std::error_code error;
    const auto root = std::filesystem::weakly_canonical(folder, error);
    if(error) return false;
    const auto file = std::filesystem::weakly_canonical(candidate, error);
    if(error || file.parent_path() != root) return false;
    if(file.extension() != ".ez") return false;
    return std::filesystem::is_regular_file(file, error) && !error;
}
}

#endif // ESAVEPATH_H
