//--------------------------------------------------------------
// Sprite pixel shader
//
// Samples the sprite texture and multiplies it with the tint color.
//--------------------------------------------------------------

struct pixel_input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

// DirectX 12 finds these by register (t0, s0). Vulkan groups resources in
// descriptor sets instead: the texture is binding 0 of set 0, the sampler
// binding 0 of set 1 (see the Vulkan backend in source/engine/rhi).
[[vk::binding(0, 0)]] Texture2D sprite_texture : register(t0);
[[vk::binding(0, 1)]] SamplerState sprite_sampler : register(s0);

float4 main(pixel_input input) : SV_TARGET
{
    return sprite_texture.Sample(sprite_sampler, input.uv) * input.tint;
}
