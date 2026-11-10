//--------------------------------------------------------------
// Mesh pixel shader
//
// A texture (white when the material has none) times the material color,
// lit by one directional light.
//--------------------------------------------------------------

struct pixel_input
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// Must match mesh_vs.hlsl
struct mesh_constants
{
    float4x4 world_view_projection;
    float4 normal_rows[3];
    float4 base_color;
};

[[vk::push_constant]] ConstantBuffer<mesh_constants> constants : register(b0);

[[vk::binding(0, 0)]] Texture2D mesh_texture : register(t0);
[[vk::binding(0, 1)]] SamplerState mesh_sampler : register(s0);

// One light, shining down at an angle, plus light from everywhere so the
// side facing away is not black
static const float3 k_light_direction = normalize(float3(-0.4f, -1.0f, -0.3f));
static const float k_ambient = 0.35f;

float4 main(pixel_input input) : SV_TARGET
{
    const bool lit = constants.normal_rows[0].w > 0.5f;

    float4 color = mesh_texture.Sample(mesh_sampler, input.uv) * constants.base_color;

    if (lit)
    {
        const float diffuse = max(0.0f, dot(normalize(input.normal), -k_light_direction));
        color.rgb *= k_ambient + (1.0f - k_ambient) * diffuse;
    }

    return color;
}
