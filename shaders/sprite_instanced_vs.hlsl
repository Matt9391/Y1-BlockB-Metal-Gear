//--------------------------------------------------------------
// Instanced sprite vertex shader
//
// Draws many sprites of one texture in a single draw call. The unit quad
// comes from stream 0 (per vertex), and every sprite's position, size,
// rotation, tint and texture rectangle from stream 1 (per instance). The
// sprite transformation that draw_sprite builds on the cpu for every sprite
// is built here instead, once per vertex.
//
// Uses sprite_ps.hlsl as its pixel shader.
//--------------------------------------------------------------

struct vertex_input
{
    // Per vertex: a corner of the unit quad
    float2 position : POSITION;
    float2 uv : TEXCOORD0;

    // Per instance: one sprite (must match gpu_sprite_instance in renderer_2d.cpp)
    float4 position_size : TEXCOORD1; // center in xy, size in zw
    float4 uv_rect : TEXCOORD2;       // offset in xy, scale in zw (0..1 texture space)
    float4 tint : TEXCOORD3;
    float4 rotation : TEXCOORD4;      // clockwise radians in x, yzw unused
};

struct pixel_input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

struct instanced_constants
{
    // The camera: maps world positions to the screen
    float4x4 view_projection;
};

// Root constants (DirectX 12) / push constants (Vulkan), see sprite_vs.hlsl
[[vk::push_constant]] ConstantBuffer<instanced_constants> constants : register(b0);

pixel_input main(vertex_input input)
{
    // Scale the unit quad to the sprite size, rotate it around its center,
    // then move it into place: the same order as make_sprite_transform.
    const float2 scaled = input.position * input.position_size.zw;

    const float c = cos(input.rotation.x);
    const float s = sin(input.rotation.x);
    const float2 rotated = float2(scaled.x * c - scaled.y * s, scaled.x * s + scaled.y * c);

    const float2 world_position = rotated + input.position_size.xy;

    pixel_input output;
    output.position = mul(constants.view_projection, float4(world_position, 0.0f, 1.0f));
    output.uv = input.uv_rect.xy + input.uv * input.uv_rect.zw;
    output.tint = input.tint;

    return output;
}
