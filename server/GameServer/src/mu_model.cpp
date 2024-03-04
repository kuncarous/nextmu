#include "mu_precompiled.h"
#include "shared_binaryreader.h"
#include "mu_model.h"
#include "mu_crypt.h"
#include "mu_resourcesmanager.h"
#include <glm/gtc/type_ptr.hpp>

std::map<mu_utf8string, NAnimationModifierType> AnimationModifierMap = {
	{ "move_speed", NAnimationModifierType::MoveSpeed },
	{ "attack_speed", NAnimationModifierType::AttackSpeed },
};

#define MAPPING_VALUE_FROM_STRING(name, map, default_value) \
constexpr auto name##Default = default_value; \
NEXTMU_INLINE const auto name##FromString(const mu_utf8string value) \
{ \
	auto iter = map.find(value); \
	if (iter == map.end()) return default_value; \
	return iter->second; \
}

MAPPING_VALUE_FROM_STRING(AnimationModifier, AnimationModifierMap, NAnimationModifierType::None)

const mu_utf8string ProgramDefault = "mesh_normal";

NModel::~NModel()
{
}

const mu_boolean NModel::Load(const mu_utf8string id, const mu_utf8string path)
{
    Id = id;

	const mu_utf8string filename = path + "model.json";
    std::unique_ptr<QFile> file;
    if (mu_rwfromfile_swt(file, filename, QIODeviceBase::ReadOnly) == false)
    {
        mu_error("model missing ({})", filename);
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
		mu_error("model json malformed ({})", filename);
		return false;
	}

	const auto model = document["model"].get<mu_utf8string>();
	if (LoadModel(path + model) == false)
	{
		mu_error("failed to load model ({})", filename);
		return false;
	}

	LoadBoundingBoxes(path + model);

	if (document.contains("bone_head"))
	{
		BoneHead = document["bone_head"].get<mu_int16>();
	}

	if (document.contains("body_height"))
	{
		BodyHeight = document["body_height"].get<mu_float>();
	}

	if (document.contains("bones"))
	{
		const auto &jbones = document["bones"];
		for (const auto &jbone : jbones)
		{
			const auto id = jbone["id"].get<mu_utf8string>();
			const auto bone = jbone["bone"].get<mu_uint32>();

			BonesById.insert(std::make_pair(id, bone));
		}
	}

	if (document.contains("animations"))
	{
		const auto &janimations = document["animations"];
		for (const auto &janimation : janimations)
		{
			const auto id = janimation["id"].get<mu_utf8string>();
			const auto index = janimation["index"].get<mu_uint32>();

			auto &animation = Animations[index];
			animation.Id = id;

			if (janimation.contains("loop"))
			{
				animation.Loop = janimation["loop"].get<mu_boolean>();
			}

			if (janimation.contains("play_speed"))
			{
				animation.PlaySpeed = janimation["play_speed"].get<mu_float>();
			}

			if (janimation.contains("modifier"))
			{
				animation.Modifier = AnimationModifierFromString(janimation["modifier"].get<mu_utf8string>());
			}

			AnimationsById.insert(std::make_pair(id, index));
		}
	}

	return true;
}

const mu_boolean NModel::LoadModel(mu_utf8string path)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

    std::unique_ptr<QFile> file;
    if (mu_rwfromfile_swt(file, path, QIODeviceBase::ReadOnly) == false)
    {
        mu_error("model missing ({})", path);
        return false;
    }

    mu_isize fileLength = file->size();
    std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
    file->read(reinterpret_cast<mu_char*>(buffer.get()), fileLength);
    file->close();

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));

	reader.Skip(3); // BMD bytes
	const mu_uint8 version = reader.Read<mu_uint8>();
	if (version == 12)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[encryptedSize]);
		XorDecrypt(decryptedBuffer.get(), reader.GetPointer(), encryptedSize);
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), encryptedSize);
	}
	else if (version == 14)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		mu_uint32 decryptedSize = CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, nullptr);
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[decryptedSize]);
		CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, decryptedBuffer.get());
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), static_cast<mu_uint32>(decryptedSize));
	}

	reader.Skip(32); // Name bytes

	const mu_uint32 numMeshes = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numBones = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numActions = static_cast<mu_uint32>(reader.Read<mu_int16>());

	if (numBones >= static_cast<mu_int16>(MaxBones))
	{
		mu_error("model exceed max bones limit ({})", path);
		return false;
	}

	Meshes.resize(numMeshes);
	BoneName.resize(numBones);
	BoneInfo.resize(numBones);
	Animations.resize(numActions);
	BoundingBoxes.resize(numBones);

	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];

		const mu_uint32 numVertices = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numNormals = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTexCoords = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTriangles = static_cast<mu_uint32>(reader.Read<mu_int16>());

		const mu_uint32 texture = static_cast<mu_uint32>(reader.Read<mu_int16>());
		mu_assert(texture == m); // Why would this variable even exists?

		mesh.Vertices.resize(numVertices);
		mesh.Normals.resize(numNormals);
		mesh.TexCoords.resize(numTexCoords);
		mesh.Triangles.resize(numTriangles);

		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			vertex.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			vertex.Position[0] = reader.Read<mu_float>();
			vertex.Position[1] = reader.Read<mu_float>();
			vertex.Position[2] = reader.Read<mu_float>();
		}

		for (mu_uint32 n = 0; n < numNormals; ++n)
		{
			auto &normal = mesh.Normals[n];
			normal.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			normal.Normal[0] = reader.Read<mu_float>();
			normal.Normal[1] = reader.Read<mu_float>();
			normal.Normal[2] = reader.Read<mu_float>();
			reader.Skip(4);
		}

		for (mu_uint32 t = 0; t < numTexCoords; ++t)
		{
			auto &texCoord = mesh.TexCoords[t];
			texCoord.UV[0] = reader.Read<mu_float>();
			texCoord.UV[1] = reader.Read<mu_float>();
		}

		for (mu_uint32 t = 0; t < numTriangles; ++t)
		{
			auto &triangle = mesh.Triangles[t];

			const mu_uint8 polygon = reader.Read<mu_uint8>();
			reader.Skip(1);
			mu_assert(polygon == 3);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Vertices[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Normals[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.TexCoords[n] = reader.Read<mu_int16>();
			}
			reader.Skip(40);
		}

        reader.Skip(32); // Texture

		// TODO: Texture Script Parsing, really, didn't have a better idea?
	}

	std::vector<mu_uint32> AnimationKeys(numActions, 0);
	for (mu_uint32 a = 0; a < numActions; ++a)
	{
		auto &animation = Animations[a];
		animation.Loop = false;
		const mu_uint32 numKeys = static_cast<mu_uint32>(reader.Read<mu_int16>());
		AnimationKeys[a] = numKeys;
		animation.LockPositions = reader.Read<mu_boolean>();

		if (animation.LockPositions)
		{
			reader.Skip(sizeof(glm::vec3) * numKeys); // Useless data
		}
	}

    mu_char boneName[32 + 1] = {};
	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = BoneInfo[b];
		info.Dummy = reader.Read<mu_boolean>();
		if (info.Dummy) continue;

        reader.ReadLine(boneName, 32);
        BoneName[b] = boneName;

		info.Parent = reader.Read<mu_int16>();

		for (mu_uint32 a = 0; a < numActions; ++a)
		{
			auto &animation = Animations[a];

			const mu_uint32 numKeys = AnimationKeys[a];
			animation.Keys.resize(numKeys);

			/*
				Why I decided to structure it this way?
				to benefit from cache when processing animations, slower loading but faster performance later.
				Hopefully the bones are sorted.
			*/
			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				animationKey.Bones.resize(numBones);
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				reader.ReadLine(&animationKey.Bones[b].Position, sizeof(glm::vec3));
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				glm::vec3 rotation;
				reader.ReadLine(&rotation, sizeof(glm::vec3));
				animationKey.Bones[b].Rotation = glm::quat(rotation);
			}
		}
	}

	CalculateBoundingBoxes();

	return true;
}

void NModel::LoadBoundingBoxes(mu_utf8string path)
{
	const auto dotPos = path.find_last_of('.');
	if (dotPos != mu_utf8string::npos)
	{
		path = path.substr(0, dotPos);
	}
	path += ".bbox.json";

    std::unique_ptr<QFile> file;
    if (mu_rwfromfile_swt(file, path, QIODeviceBase::ReadOnly) == false)
    {
        return;
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
		return;
	}

	// Global
	{
		const auto &bbox = document["bbox"];
		const auto &min = bbox["min"];
		const auto &max = bbox["max"];

		NBoundingBox b;
		b.Min[0] = min[0].get<mu_float>();
		b.Min[1] = min[1].get<mu_float>();
		b.Min[2] = min[2].get<mu_float>();
		b.Max[0] = max[0].get<mu_float>();
		b.Max[1] = max[1].get<mu_float>();
		b.Max[2] = max[2].get<mu_float>();

		BBoxes.Global = NOrientedBoundingBox(b);
	}

	if (document.contains("animations"))
	{
		NBoundingBox box = {};
		const auto &animations = document["animations"];
		for (const auto &bbox : animations)
		{
			const auto &min = bbox["min"];
			const auto &max = bbox["max"];

			NBoundingBox b;
			b.Min[0] = min[0].get<mu_float>();
			b.Min[1] = min[1].get<mu_float>();
			b.Min[2] = min[2].get<mu_float>();
			b.Max[0] = max[0].get<mu_float>();
			b.Max[1] = max[1].get<mu_float>();
			b.Max[2] = max[2].get<mu_float>();

			BBoxes.PerAnimation.push_back(NOrientedBoundingBox(b));
		}
	}

	BBoxes.Valid = true;
}

void NModel::CalculateBoundingBoxes()
{
	const mu_uint32 numMeshes = static_cast<mu_uint32>(Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];

		const mu_uint32 numVertices = static_cast<mu_uint32>(mesh.Vertices.size());
		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			if (vertex.Node == NInvalidInt16) continue;

			auto &boundingBox = BoundingBoxes[vertex.Node];
			for (mu_uint32 n = 0; n < 3; ++n)
			{
				if (vertex.Position[n] < boundingBox.Min[n]) boundingBox.Min[n] = vertex.Position[n];
				if (vertex.Position[n] > boundingBox.Max[n]) boundingBox.Max[n] = vertex.Position[n];
			}

			boundingBox.Valid = true;
		}
	}
}

const mu_boolean NModel::PlayAnimation(
	mu_uint16 &CurrentAction,
	mu_uint16 &PriorAction,
	mu_float &CurrentFrame,
	mu_float &PriorFrame,
	const mu_float PlaySpeed
) const
{
	if (glm::abs(PlaySpeed) < glm::epsilon<mu_float>()) return true;
	const mu_uint32 numAnimations = static_cast<mu_uint32>(this->Animations.size());

	// Return true to keep original logic
	if (CurrentAction >= numAnimations) return true;

	const auto &currentAnimation = this->Animations[CurrentAction];
	const auto &currentFramesCount = static_cast<mu_uint32>(currentAnimation.Keys.size());
	if (currentFramesCount <= 1) return true;

	const mu_float tmpFrame = CurrentFrame;
	const mu_uint32 lastFrame = static_cast<mu_uint32>(glm::floor(CurrentFrame));
	CurrentFrame += PlaySpeed;
	const mu_uint32 newFrame = static_cast<mu_uint32>(glm::floor(CurrentFrame));

	if (CurrentAction == PriorAction || lastFrame != newFrame)
	{
		PriorAction = CurrentAction;
		PriorFrame = tmpFrame;
	}

	mu_boolean loop = true;
	if (currentAnimation.Loop)
	{
		if (newFrame >= currentFramesCount)
		{
			CurrentFrame = static_cast<mu_float>(currentFramesCount) - 0.01f;
			loop = false;
		}
	}
	else
	{
		const auto maxFrames = currentFramesCount - static_cast<mu_uint32>(!!currentAnimation.LockPositions);
		if (newFrame >= maxFrames)
		{
			CurrentFrame = glm::mod(CurrentFrame, static_cast<mu_float>(maxFrames));
			loop = false;
		}
	}

	return loop;
}
