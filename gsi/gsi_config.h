#pragma once
#include <filesystem>
#include <string>

namespace cs2gsi
{
    // Writes gamestate_integration_<name>.cfg into the CS2 cfg directory.
    // Returns the path of the written file. Throws std::runtime_error on failure.
    std::filesystem::path generate_gsi_config(const std::string& name,
                                              const std::string& uri = "http://127.0.0.1:3000");

    std::filesystem::path generate_gsi_config(const std::string& name, const std::string& uri,
                                              const std::filesystem::path& steam_path);
}
