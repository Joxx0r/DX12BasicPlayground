#pragma once

enum class RevLightType : uint8_t
{
	PointLight,
	SpotLight,
	DirectionalLight,
	MAX
};

enum class RevResourceType : uint8_t
{
	RenderTarget = 1,
	ShaderResource = 2,
	UAV = 4,
	MAX
};;

enum class RevModelType : uint8_t
{
	Normal,
	Animated,
	Max
};

