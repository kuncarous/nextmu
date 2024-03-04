#include "mu_precompiled.h"
#include "mu_logger.h"
#include "mu_directories.h"

namespace MULogger
{
    std::unique_ptr<NConsoleLogger> ConsoleLogger;
    std::unique_ptr<NFileLogger> MainFileLogger;
    std::map<mu_utf8string, NLogTypeConfig> Types;

    const mu_boolean Load()
    {
        const mu_utf8string filename = MUDirectories::GetSharedData() + "logs.json";

        std::unique_ptr<QFile> file;
        if (mu_rwfromfile_swt(file, filename, QIODeviceBase::ReadOnly) == false)
        {
            mu_error("logs config missing ({})", filename);
            return false;
        }

        mu_isize fileLength = file->size();
        std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
        file->read(jsonBuffer.get(), fileLength);
        file->close();

        const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
        jsonBuffer.reset();
        auto document = nlohmann::json::parse(inputBuffer.c_str());
        if (document.is_discarded() == true || document.is_object() == false)
        {
            mu_error("logs config malformed ({})", filename);
            return false;
        }

        if (document.contains("types") == false)
        {
            mu_error("logs types missing ({})", filename);
            return false;
        }

        Types.clear();
        const auto &jtypes = document["types"];
        if (jtypes.is_array() == false)
        {
            mu_error("logs types wrong type ({})", filename);
            return false;
        }

        for (const auto &jtype : jtypes)
        {
            NLogTypeConfig type;

            const mu_utf8string id = jtype["id"].get<mu_utf8string>();
            type.Type = QString::fromUtf8(id.c_str());

            if (jtype.contains("colors")) {
                const auto &jcolors = jtype["colors"];

                if (jcolors.contains("background"))
                {
                    type.BackgroundColor = QString::fromUtf8(jcolors["background"].get<mu_utf8string>().c_str());
                }
                if (jcolors.contains("font"))
                {
                    type.FontColor = QString::fromUtf8(jcolors["font"].get<mu_utf8string>().c_str());
                }
                if (jcolors.contains("selected"))
                {
                    type.SelectedColor = QString::fromUtf8(jcolors["selected"].get<mu_utf8string>().c_str());
                }
                if (jcolors.contains("highlight"))
                {
                    type.HighlightColor = QString::fromUtf8(jcolors["highlight"].get<mu_utf8string>().c_str());
                }
            }

            Types.insert(std::make_pair(id, type));
        }

        return true;
    }

    const mu_boolean Initialize()
    {
        ConsoleLogger.reset(new_nothrow NConsoleLogger());
        MainFileLogger.reset(new_nothrow NFileLogger("./logs/main", ConsoleLogger.get()));
        return Load();
    }

    void Destroy()
    {
        ConsoleLogger.reset();
        Types.clear();
    }

    const NLogTypeConfig *GetType(const mu_utf8string type)
    {
        auto iter = Types.find(type);
        if (iter == Types.end()) return nullptr;
        return &iter->second;
    }

    NConsoleLogger *GetConsoleLogger()
    {
        return ConsoleLogger.get();
    }

    NFileLogger *GetMainFileLogger()
    {
        return MainFileLogger.get();
    }
};
