//--------------------------------------------------------------
// Sprite vertex shader
//
// Transforms the unit quad to its position on screen and passes
// the texture coordinates and tint color on to the pixel shader.
//--------------------------------------------------------------

struct vertex_input
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct pixel_input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

// Set by renderer_2d for every sprite (must match sprite_constants in renderer_2d/private/sprite_layout.h)
struct sprite_constants
{
    // Sprite transformation and screen projection combined into one matrix.
    // Stored column-major (hlsl's default), the same layout glm uses for
    // its matrices on the cpu, so the engine can upload them unchanged.
    float4x4 transform;
    float4 tint_color;

    // Part of the texture this sprite shows: offset in xy, scale in zw
    // (0..1 texture space). (0, 0, 1, 1) shows the whole texture; sprite
    // sheets select a single frame here.
    float4 uv_rect;
};

// DirectX 12 receives these as root constants in register b0; Vulkan as
// push constants, which the vk::push_constant attribute selects. Each of
// the two apis ignores the other one's annotation. For OpenGL,
// shader_compiler turns them into the uniform block buz_constants.
[[vk::push_constant]] ConstantBuffer<sprite_constants> constants : register(b0);

pixel_input main(vertex_input input)
{
    pixel_input output;
    output.position = mul(constants.transform, float4(input.position, 0.0f, 1.0f));
    output.uv = constants.uv_rect.xy + input.uv * constants.uv_rect.zw;
    output.tint = constants.tint_color;

    return output;
}
