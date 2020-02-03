struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	float falloff;
};

struct SpotLight
{
	float3 position;
	float radius;
	float3 color;
	float falloff;
	float3 direction;
	float cosHalfAngle;
};

struct DirectionalLight
{
	float3 direction;
	float padding1;
	float3 color;
	float padding2;
};

float3 CalcPointLighting(in const PointLight light,
						 in const float3 position,
						 in const float3 view,
						 in const float3 normal,
						 in const float3 diffuse,
						 in const float specular,
						 in const float glossiness)
{
	float3 lightVector = light.position - position;
	const float distance = length(lightVector);
	lightVector /= distance;

	const float3 halfVector = normalize(view + lightVector);
	float4 lighting = lit(dot(normal, lightVector),
						  dot(normal, halfVector),
						  glossiness);

	lighting.xyz = (diffuse * lighting.y) + (lighting.z * specular);
	
	float attenuation = saturate(1.0f - (distance / light.radius));
	attenuation = pow(attenuation, light.falloff);

	return lighting.xyz * light.color * attenuation;
}

float3 CalcSpotLighting(in const SpotLight light,
						in const float3 position,
						in const float3 view,
						in const float3 normal,
						in const float3 diffuse,
						in const float specular,
						in const float glossiness)
{
	float3 lightVector = light.position - position;
	const float distance = length(lightVector);
	lightVector /= distance;

	const float3 halfVector = normalize(view + lightVector);
	float4 lighting = lit(dot(normal, lightVector),
						  dot(normal, halfVector),
						  glossiness);

	lighting.xyz = (diffuse * lighting.y) + (lighting.z * specular);
	
	float attenuation = saturate(1.0f - (distance / light.radius));
	
	const float angle = dot(light.direction, -lightVector);
	attenuation *= saturate((angle - light.cosHalfAngle) / (1.0f - light.cosHalfAngle));
	
	attenuation = pow(attenuation, light.falloff);

	return lighting.xyz * light.color * attenuation;
}

float3 CalcDirectionalLighting(in const DirectionalLight light,
							   in const float3 position,
							   in const float3 view,
							   in const float3 normal,
							   in const float3 diffuse,
							   in const float specular,
							   in const float glossiness)
{
	const float3 lightVector = -light.direction;
	const float3 halfVector = normalize(view + lightVector);
	float4 lighting = lit(dot(normal, lightVector),
						  dot(normal, halfVector),
						  glossiness);

	lighting.xyz = (diffuse * lighting.y) + (lighting.z * specular);
	return lighting.xyz * light.color;
}

float CalcViewDepth(in float sceneDepth, in matrix invProjectionMatrix)
{
	return 1.f / (sceneDepth * invProjectionMatrix._34 + invProjectionMatrix._44);
}

float4 InCodeNormalGlossiness(in const float3 normal, in const float glossiness)
{
	// NormalGlossinessBuffer Format [R10G10B10A2_UNORM]
	// NormalGlossinessBuffer.RG = Normal.XY;
	// NormalGlossinessBuffer.B = Specular power;
	// NormalGlossinessBuffer.A = Normal Z sign;
	return float4(
		normal.xy * 0.5f + 0.5f,			// RG
		glossiness,							// B
		(normal.z < 0.0f) ? 1.0f : 0.0f);	// A
}

void DecodeNormalGlossiness(in const float4 normalGlossiness, out float3 normal, out float glossiness)
{
	// NormalGlossinessBuffer Format [R10G10B10A2_UNORM]
	// NormalGlossinessBuffer.RG = Normal.XY;
	// NormalGlossinessBuffer.B = Specular power;
	// NormalGlossinessBuffer.A = Normal Z sign;
	normal.xy = normalGlossiness.xy * 2.0f - 1.0f;
	normal.z = sqrt(1.0f - saturate(dot(normal.xy, normal.xy))) * ((normalGlossiness.w == 0.0f) ? 1.0f : -1.0f);
	normal = normalize(normal);

	glossiness = normalGlossiness.z;
}