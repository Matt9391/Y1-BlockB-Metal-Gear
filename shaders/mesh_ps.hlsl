//--------------------------------------------------------------
// Mesh pixel shader
//
// A texture (white when the material has none) times the material color,
// lit by one directional light. Pixels with less alpha than the material's
// cutoff are not drawn.
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
    float3 normal_row_0;
    float lit;
    float3 normal_row_1;
    float alpha_cutoff;
    float3 normal_row_2;
    float padding;
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
    const bool lit = constants.lit > 0.5f;

    float4 color = mesh_texture.Sample(mesh_sampler, input.uv) * constants.base_color;

    // A see-through part of the texture, like the gaps in a railing: skip
    // the pixel, so whatever is behind it stays visible
    if (color.a < constants.alpha_cutoff)
    {
        discard;
    }

    if (lit)
    {
        const float diffuse = max(0.0f, dot(normalize(input.normal), -k_light_direction));
        color.rgb *= k_ambient + (1.0f - k_ambient) * diffuse;
    }

    return color;
}
