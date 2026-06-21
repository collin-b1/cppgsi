#include "gsi_config.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string gsi_config_body(const std::string& name, const std::string& uri)
{
    std::ostringstream ss;
    ss << "\"" << name << "\"\n"
        << "{\n"
        << "    \"uri\"           \"" << uri << "\"\n"
        << "    \"timeout\"       \"1.0\"\n"
        << "    \"buffer\"        \"0.1\"\n"
        << "    \"throttle\"      \"0.1\"\n"
        << "    \"heartbeat\"     \"5.0\"\n"
        << "    \"data\"\n"
        << "    {\n"
        << "        \"map_round_wins\"         \"1\"\n"
        << "        \"map\"                    \"1\"\n"
        << "        \"player_id\"              \"1\"\n"
        << "        \"player_match_stats\"     \"1\"\n"
        << "        \"player_state\"           \"1\"\n"
        << "        \"player_weapons\"         \"1\"\n"
        << "        \"provider\"               \"1\"\n"
        << "        \"round\"                  \"1\"\n"
        << "        \"allgrenades\"            \"1\"\n"
        << "        \"allplayers_id\"          \"1\"\n"
        << "        \"allplayers_match_stats\" \"1\"\n"
        << "        \"allplayers_position\"    \"1\"\n"
        << "        \"allplayers_state\"       \"1\"\n"
        << "        \"allplayers_weapons\"     \"1\"\n"
        << "        \"bomb\"                   \"1\"\n"
        << "        \"phase_countdowns\"       \"1\"\n"
        << "        \"player_position\"        \"1\"\n"
        << "    }\n"
        << "}\n";
    return ss.str();
}

namespace fs = std::filesystem;

static fs::path steam_root()
{
#if defined(_WIN32)
    const char* pf86 = getenv("ProgramFiles(x86)");
    const char* pf = getenv("ProgramFiles");
    fs::path base = pf86 ? fs::path(pf86) : (pf ? fs::path(pf) : fs::path("C:/Program Files (x86)"));
    return base / "Steam";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) throw std::runtime_error("HOME environment variable not set");
    return fs::path(home) / "Library/Application Support/Steam";
#else // Linux
    const char* home = getenv("HOME");
    if (!home) throw std::runtime_error("HOME environment variable not set");
    fs::path candidate = fs::path(home) / ".steam/steam";
    if (fs::exists(candidate)) return candidate;
    return fs::path(home) / ".local/share/Steam";
#endif
}

fs::path cs2gsi::generate_gsi_config(const std::string& name, const std::string& uri)
{
    return generate_gsi_config(name, uri, steam_root());
}

fs::path cs2gsi::generate_gsi_config(const std::string& name, const std::string& uri, const fs::path& steam_path)
{
    fs::path cfg_dir = steam_path
        / "steamapps/common/Counter-Strike Global Offensive/game/csgo/cfg";

    if (!fs::exists(cfg_dir))
    {
        throw std::runtime_error("Could not find cfg directory.");
    }

    std::string safe_name = name;
    for (char& c : safe_name)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') c = '_';

    fs::path out = cfg_dir / ("gamestate_integration_" + safe_name + ".cfg");

    std::ofstream f(out);
    if (!f) throw std::runtime_error("Could not open file for writing: " + out.string());
    f << gsi_config_body(name, uri);
    if (!f.good()) throw std::runtime_error("Write failed: " + out.string());

    return out;
}
