#include "mu_precompiled.h"
#include "mu_resourcesmanager.h"
#include "mu_directories.h"
#include "mu_model.h"
#include <boost/algorithm/string/replace.hpp>

typedef std::unique_ptr<NModel> ModelPointer;

namespace MUResourcesManager
{
	std::map<mu_utf8string, ModelPointer> Models;

	const mu_boolean LoadModels(const mu_utf8string basePath, const nlohmann::json &models);

    const mu_boolean Load()
	{
        const mu_utf8string path = MUDirectories::GetGameData();
        const mu_utf8string filename = path + "resources.json";

        std::unique_ptr<QFile> file;
        if (mu_rwfromfile_swt(file, filename, QIODeviceBase::ReadOnly) == false)
        {
            MULogger::GetMainFileLogger()->Write(NLogType::Error, "failed to load resources file ({})", filename);
            return false;
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
            MULogger::GetMainFileLogger()->Write(NLogType::Error, "resources file malformed ({})", filename);
			return false;
        }

		if (document.contains("models"))
		{
			const auto models = document["models"];
			if (LoadModels(path, models) == false)
			{
				return false;
			}
        }

        MULogger::GetMainFileLogger()->Write(NLogType::Success, "successfully loaded resources ({})", path);

		return true;
	}

	void Destroy()
    {
		Models.clear();
    }

	const mu_boolean LoadModels(const mu_utf8string basePath, const nlohmann::json &models)
    {
		for (const auto &m : models)
		{
			const mu_utf8string id = m["id"];
			const mu_utf8string path = m["path"];

			std::unique_ptr<NModel> model(new_nothrow NModel());
			if (model->Load(id, basePath + path) == false)
			{
                MULogger::GetMainFileLogger()->Write(NLogType::Error, "failed to load model ({})", path);
				return false;
			}

			Models.insert(std::pair(id, std::move(model)));
		}

		return true;
    }

	NModel *GetModel(const mu_utf8string id)
	{
		auto iter = Models.find(id);
		if (iter == Models.end()) return nullptr;
		return iter->second.get();
	}
};
