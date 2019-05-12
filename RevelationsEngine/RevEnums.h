#pragma once

enum class RevLightType : UINT8
{
	PointLight,
	SpotLight,
	DirectionalLight,
	MAX
};

enum class RevResourceType : UINT8
{
	RenderTarget = 1,
	ShaderResource = 2,
	UAV = 4,
	MAX
};;

enum class RevModelType : UINT8
{
	Normal,
	Animated,
	Max
};

