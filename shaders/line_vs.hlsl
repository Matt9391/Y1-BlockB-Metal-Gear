//--------------------------------------------------------------
// Debug line vertex shader
//
// Draws the lines of buz::debug_draw: each vertex has a world position
// and a color.
//--------------------------------------------------------------

struct vertex_input
{
    float3 position : POSITION;
    float4 color : COLOR0;
};

struct pixel_input
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

struct line_constants
{
    float4x4 view_projection;
};

[[vk::push_constant]] ConstantBuffer<line_constants> constants : register(b0);

pixel_input main(vertex_input input)
{
    pixel_input output;
    output.position = mul(constants.view_projection, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}
