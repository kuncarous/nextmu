$input a_position, a_normal, a_texcoord0, a_texcoord1
$output v_color0, v_texcoord0

#include "../../common.sh"

SAMPLER2D(s_skeletonTexture, 1);
SAMPLER2D(s_lightTexture, 2);

uniform vec4 u_settings1; // BoneOffset, NormalScale, Dummy, Dummy

ivec2 GetBoneIndex(uint baseX, uint baseY, uint index)
{
	uint t = (baseX + index);
	baseX = t % SKELETON_TEXTURE_WIDTH;
	baseY += t / SKELETON_TEXTURE_WIDTH;

	return ivec2(int(baseX), int(baseY));
}

mat2x4 RequestBone(uint boneOffset, uint boneId)
{
	boneOffset += boneId;
	boneOffset *= 2u;
	
	uint boneX = boneOffset % SKELETON_TEXTURE_WIDTH;
	uint boneY = boneOffset / SKELETON_TEXTURE_WIDTH;
	
	return mat2x4(
		texelFetch(s_skeletonTexture, GetBoneIndex(boneX, boneY, 0u), 0).yzwx,
		texelFetch(s_skeletonTexture, GetBoneIndex(boneX, boneY, 1u), 0)
	);
}

vec3 TransformPosition(vec3 v, mat2x4 dq)
{
	v *= dq[1].w;
	return (2.0 * cross(dq[0].xyz, cross(dq[0].xyz, v) + dq[0].w * v) + v) + dq[1].xyz;
}

vec3 TransformNormal(vec3 v, mat2x4 dq)
{
	v *= dq[1].w;
	return (2.0 * cross(dq[0].xyz, cross(dq[0].xyz, v) + dq[0].w * v) + v);
}

void main()
{
	uint boneOffset = uint(u_settings1.x);
	mat2x4 vertexMatrix = RequestBone(boneOffset, a_texcoord1.x);
	vec3 position = TransformPosition(a_position, vertexMatrix);
	
	mat2x4 normalMatrix = RequestBone(boneOffset, a_texcoord1.y);
	vec3 normal = TransformNormal(a_normal, normalMatrix);
	
	position += normal * u_settings1.y;
	
	v_color0 = vec4(1.0, 1.0, 1.0, 1.0);
	v_texcoord0 = a_texcoord0;
	
	gl_Position = mul(u_modelViewProj, vec4(position, 1.0));
}