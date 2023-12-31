// ConvertObjects.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "shared_binaryreader.h"
#include "mu_crypt.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>

#include <array>
#include <vector>

struct NVertex
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Position = glm::vec3();
};

struct NNormal
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Normal = glm::vec3();
};

struct NTexCoord
{
	glm::vec2 UV = glm::vec2();
};

struct NTriangle
{
	std::array<mu_int16, 3> Vertices;
	std::array<mu_int16, 3> Normals;
	std::array<mu_int16, 3> TexCoords;
};

class NMesh
{
public:
	std::vector<NVertex> Vertices;
	std::vector<NNormal> Normals;
	std::vector<NTexCoord> TexCoords;
	std::vector<NTriangle> Triangles;
};

class NBone
{
public:
	glm::vec3 Position = glm::vec3();
	glm::quat Rotation = glm::quat();
};

class NAnimationKey
{
public:
	std::vector<NBone> Bones; // Per Bone
};

class NAnimation
{
public:
	mu_boolean Loop = false;
	mu_boolean LockPositions = false;
	mu_float PlaySpeed = 1.0f;
	std::vector<NAnimationKey> Keys; // Per Animation Frame
};

class NBoneInfo
{
public:
	mu_boolean Dummy = true;
	mu_int16 Parent = NInvalidInt16;
};

// Should change the bounding box to use a center and extent instead?, which one would benefit more at future?
constexpr mu_float InvalidBox = 50000.0f;
class NBoundingBox
{
public:
	glm::vec3 Min = glm::vec3(InvalidBox, InvalidBox, InvalidBox);
	glm::vec3 Max = glm::vec3(-InvalidBox, -InvalidBox, -InvalidBox);
};

class NBoundingBoxWithValidation : public NBoundingBox
{
public:
	mu_boolean Valid = false;
};

std::vector<NMesh> Meshes;
std::vector<mu_utf8string> BoneName; // Per Bone (separated for better cache ratio)
std::vector<NBoneInfo> BoneInfo; // Per Bone (separated for better cache ratio)
std::vector<NAnimation> Animations; // Per Animation (Action) (structured like this to get a better cache ratio)
std::vector<NBoundingBoxWithValidation> BoundingBoxes; // Per Bone

constexpr mu_uint32 MaxBones = 200;

void CalculateBoundingBoxes()
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

struct NCompressedMatrix
{
	glm::quat Rotation;
	glm::vec3 Position;
	mu_float Scale;
};

NEXTMU_INLINE const glm::vec3 Transform(const glm::vec3 v, const NCompressedMatrix &matrix)
{
	return matrix.Rotation * (v * matrix.Scale) + matrix.Position;
}

NEXTMU_INLINE const glm::vec3 TransformNormal(const glm::vec3 v, const NCompressedMatrix &matrix)
{
	return matrix.Rotation * (v * matrix.Scale);
}

NEXTMU_INLINE void MixBones(
	const NCompressedMatrix &parent,
	NCompressedMatrix &out
)
{
	out.Rotation = parent.Rotation * out.Rotation;
	out.Position = parent.Rotation * (out.Position * parent.Scale) + parent.Position;
	out.Scale = parent.Scale;
}

mu_float BodyHeight = 0.0f;
std::vector<NCompressedMatrix> Bones;
NCompressedMatrix Parent = {
	.Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
	.Position = glm::vec3(0.0f, 0.0f, 0.0f),
	.Scale = 1.05f, // Extra 5% to make it match
};
void Animate(
	const mu_uint32 Action,
	const mu_float Frame
)
{
	const mu_uint32 numAnimations = static_cast<mu_uint32>(Animations.size());
	const mu_uint32 numBones = static_cast<mu_uint32>(BoneInfo.size());

	mu_assert(numAnimations > 0);
	mu_assert(numBones > 0);

	if (Bones.size() < numBones) Bones.resize(numBones);

	mu_uint32 current = static_cast<mu_uint32>(Frame);

	const auto &currentAnimation = Animations[Action];
	const auto &currentFramesCount = static_cast<mu_uint32>(currentAnimation.Keys.size());

	if (currentAnimation.Loop)
	{
		if (current >= currentFramesCount) current = currentFramesCount - 1;
	}
	else
	{
		current = current % currentFramesCount;
	}

	/*
		Why I processed 4 frames instead of only 2 frames?
		To provide a correct animation blending in high framerates.
	*/
	const auto &currentFrame1 = currentAnimation.Keys[current];

	const mu_boolean lockPosition = currentAnimation.LockPositions;
	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = BoneInfo[b];
		if (info.Dummy) continue;

		const auto &currentBone1 = currentFrame1.Bones[b];

		const auto &currentRotation1 = currentBone1.Rotation;

		const glm::quat &currentRotation = currentRotation1;

		const auto &currentPosition1 = currentBone1.Position;

		const auto currentPosition = currentPosition1;

		auto &outBone = Bones[b];
		outBone.Rotation = currentRotation;
		outBone.Scale = 1.0f;

		if (b == 0 && lockPosition)
		{
			outBone.Position[0] = currentBone1.Position[0];
			outBone.Position[1] = currentBone1.Position[1];
			outBone.Position[2] = currentPosition[2] + BodyHeight;
		}
		else
		{
			outBone.Position = currentPosition;
		}

		mu_assert(info.Parent == NInvalidInt16 || (info.Parent >= 0 && info.Parent < static_cast<mu_int16>(numBones)));
		MixBones(
			info.Parent == NInvalidInt16
			? Parent
			: Bones[info.Parent],
			outBone
		);
	}
}

void GenerateBoundingBox(const mu_utf8string filename)
{
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		std::cout << "model missing" << std::endl;
		return;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

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
		mu_int64 decryptedSize = CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, nullptr);
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[decryptedSize]);
		CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, decryptedBuffer.get());
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), static_cast<mu_uint32>(decryptedSize));
	}

	reader.Skip(32); // Name bytes

	const mu_uint32 numMeshes = static_cast<mu_uint32>(reader.Read<mu_int16>());
	if (numMeshes == 0)
	{
		std::cout << "Model doesn't contains meshes" << std::endl;
		return;
	}

	const mu_uint32 numBones = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numActions = static_cast<mu_uint32>(reader.Read<mu_int16>());
	if (numBones >= static_cast<mu_int16>(MaxBones))
	{
		std::cout << "Model has more bones than allowed" << std::endl;
		return;
	}

	Meshes.resize(numMeshes);
	BoneName.resize(numBones);
	BoneInfo.resize(numBones);
	Animations.resize(numActions);
	BoundingBoxes.resize(numBones);

	mu_char name[32 + 1] = {};
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

		reader.ReadLine(name, 32);
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

	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = BoneInfo[b];
		info.Dummy = reader.Read<mu_boolean>();
		if (info.Dummy) continue;

		reader.ReadLine(name, 32);
		BoneName[b] = name;

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

	nlohmann::ordered_json janimations = nlohmann::ordered_json::array();
	NBoundingBox globalBBox;
	for (mu_uint32 a = 0; a < numActions; ++a)
	{
		const mu_uint32 numKeys = AnimationKeys[a];
		auto &animation = Animations[a];
		NBoundingBox BBox;
		for (mu_uint32 frame = 0; frame < numKeys; ++frame)
		{
			Animate(a, static_cast<mu_float>(frame));

			for (mu_uint32 b = 0; b < numBones; ++b)
			{
				const auto &boundingBox = BoundingBoxes[b];
				if (!boundingBox.Valid) continue;

				const auto &bone = Bones[b];
				glm::vec3 min = Transform(boundingBox.Min, bone);
				for (mu_uint32 k = 0; k < 3; ++k)
				{
					if (min[k] < globalBBox.Min[k]) globalBBox.Min[k] = min[k];
					if (min[k] > globalBBox.Max[k]) globalBBox.Max[k] = min[k];
					if (min[k] < BBox.Min[k]) BBox.Min[k] = min[k];
					if (min[k] > BBox.Max[k]) BBox.Max[k] = min[k];
				}

				glm::vec3 max = Transform(boundingBox.Max, bone);
				for (mu_uint32 k = 0; k < 3; ++k)
				{
					if (max[k] < globalBBox.Min[k]) globalBBox.Min[k] = max[k];
					if (max[k] > globalBBox.Max[k]) globalBBox.Max[k] = max[k];
					if (max[k] < BBox.Min[k]) BBox.Min[k] = max[k];
					if (max[k] > BBox.Max[k]) BBox.Max[k] = max[k];
				}
			}
		}

		nlohmann::ordered_json jbbox;

		nlohmann::ordered_json jbboxMin = nlohmann::ordered_json::array();
		jbboxMin.push_back(BBox.Min.x);
		jbboxMin.push_back(BBox.Min.y);
		jbboxMin.push_back(BBox.Min.z);
		jbbox["min"] = jbboxMin;

		nlohmann::ordered_json jbboxMax = nlohmann::ordered_json::array();
		jbboxMax.push_back(BBox.Max.x);
		jbboxMax.push_back(BBox.Max.y);
		jbboxMax.push_back(BBox.Max.z);
		jbbox["max"] = jbboxMax;

		janimations.push_back(jbbox);
	}

	nlohmann::ordered_json jbbox;

	nlohmann::ordered_json jbboxMin = nlohmann::ordered_json::array();
	jbboxMin.push_back(globalBBox.Min.x);
	jbboxMin.push_back(globalBBox.Min.y);
	jbboxMin.push_back(globalBBox.Min.z);
	jbbox["min"] = jbboxMin;

	nlohmann::ordered_json jbboxMax = nlohmann::ordered_json::array();
	jbboxMax.push_back(globalBBox.Max.x);
	jbboxMax.push_back(globalBBox.Max.y);
	jbboxMax.push_back(globalBBox.Max.z);
	jbbox["max"] = jbboxMax;

	nlohmann::ordered_json jdocument;
	jdocument["bbox"] = jbbox;
	jdocument["animations"] = janimations;

	auto dotPos = filename.find_last_of('.');
	const mu_utf8string outfilename = (
		(
			dotPos == mu_utf8string::npos
			? filename
			: filename.substr(0, dotPos)
		) + ".bbox.json"
	);

	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, outfilename, "wb") == false)
	{
		std::cout << "failed to save file" << std::endl;
		return;
	}

	const auto output = jdocument.dump(1, '\t');
	SDL_RWwrite(fp, output.c_str(), output.size(), 1);
	SDL_RWclose(fp);
}

int main(int argc, char *argv[])
{
	if (SDL_Init(0) < 0)
	{
		std::cout << "failed to initialize SDL" << std::endl;
		return 0;
	}

    if (argc != 2)
    {
        std::cout << "model missing" << std::endl;
        return 0;
    }

	const mu_utf8string filename = argv[1];
	GenerateBoundingBox(filename);

	SDL_Quit();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
