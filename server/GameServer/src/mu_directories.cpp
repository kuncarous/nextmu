#include "mu_precompiled.h"
#include "mu_directories.h"

namespace MUDirectories
{
    mu_utf8string GameData = "../data/";
    mu_utf8string ServerData = "./data/";
    mu_utf8string SharedData = "../data/";

    const mu_boolean Load()
    {
        const mu_utf8string filename = "./data/directories.json";

        std::unique_ptr<QFile> file;
        if (mu_rwfromfile_swt(file, filename, QIODeviceBase::ReadOnly) == false)
        {
            return true;
        }

        mu_isize fileLength = file->size();
        std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
        file->read(jsonBuffer.get(), fileLength);
        file->close();

        const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
        jsonBuffer.reset();
        auto document = nlohmann::json::parse(inputBuffer.c_str());
        if (document.is_discarded() == true)
        {
            MULogger::GetMainFileLogger()->Write(NLogType::Error, "directories file malformed ({})", filename);
            return false;
        }

        if (
            document.contains("game") == true &&
            document["game"].is_string() == true
        )
        {
            GameData = document["game"].get<mu_utf8string>();
        }

        if (
            document.contains("server") == true &&
            document["server"].is_string() == true
        )
        {
            ServerData = document["server"].get<mu_utf8string>();
        }

        if (
            document.contains("shared") == true &&
            document["shared"].is_string() == true
        )
        {
            SharedData = document["shared"].get<mu_utf8string>();
        }

        return true;
    }

    const mu_utf8string GetGameData()
    {
        return GameData;
    }

    const mu_utf8string GetServerData()
    {
        return ServerData;
    }

    const mu_utf8string GetSharedData()
    {
        return SharedData;
    }
};
