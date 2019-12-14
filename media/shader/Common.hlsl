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