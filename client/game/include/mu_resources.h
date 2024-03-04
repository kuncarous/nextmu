#ifndef __MU_RESOURCES_H__
#define __MU_RESOURCES_H__

#pragma once

enum class NResourceType
{
	Program,
	Texture,
	Model,
};

class NBaseResourceRequest
{
public:
	NBaseResourceRequest(const NResourceType type) : Type(type) {}
	virtual ~NBaseResourceRequest() {}

public:
	const NResourceType Type;
};

typedef std::unique_ptr<NBaseResourceRequest> NBaseResourceRequestPtr;

class NProgramResourceRequest : public NBaseResourceRequest
{
public:
	NProgramResourceRequest(
		const mu_utf8string id,
		const mu_utf8string vertex,
		const mu_utf8string fragment,
		const mu_utf8string resourceId,
		const nlohmann::json &macros
	) : ID(id),
		Vertex(vertex),
		Fragment(fragment),
		ResourceID(resourceId),
		Macros(macros),
		NBaseResourceRequest(NResourceType::Program)
	{}

public:
	const mu_utf8string ID;
	const mu_utf8string Vertex;
	const mu_utf8string Fragment;
	const mu_utf8string ResourceID;
	const nlohmann::json Macros;
};

class NTextureResourceRequest : public NBaseResourceRequest
{
public:
	NTextureResourceRequest(
		const mu_utf8string id,
		const mu_utf8string path,
		const mu_utf8string filter,
		const mu_utf8string wrap
	) : ID(id),
		Path(path),
		Filter(filter),
		Wrap(wrap),
		NBaseResourceRequest(NResourceType::Texture)
	{}

public:
	const mu_utf8string ID;
	const mu_utf8string Path;
	const mu_utf8string Filter;
	const mu_utf8string Wrap;
};

class NModelResourceRequest : public NBaseResourceRequest
{
public:
	NModelResourceRequest(
		const mu_utf8string id,
		const mu_utf8string path
	) : ID(id),
		Path(path),
		NBaseResourceRequest(NResourceType::Model)
	{}

public:
	const mu_utf8string ID;
	const mu_utf8string Path;
};

#endif