//--------------------------------------------------------------
// Mesh vertex shader
//
// Transforms a vertex of a 3D mesh to the screen and its normal to world
// space for the lighting.
//--------------------------------------------------------------

struct vertex_input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct pixel_input
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// Set by renderer_3d for every draw (must match mesh_constants in renderer_3d.cpp)
struct mesh_constants
{
    // Object to screen: world, view and projection combined
    float4x4 world_view_projection;

    // Rows of the matrix that turns object space normals into world space
    // (the inverse transpose of the world matrix). normal_rows[0].w is 1
    // when the mesh is lit.
    float4 normal_rows[3];

    float4 base_color;
};

[[vk::push_constant]] ConstantBuffer<mesh_constants> constants : register(b0);

pixel_input main(vertex_input input)
{
    const float3x3 normal_matrix = float3x3(constants.normal_rows[0].xyz, constants.normal_rows[1].xyz, constants.normal_rows[2].xyz);

    pixel_input output;
    output.position = mul(constants.world_view_projection, float4(input.position, 1.0f));
    output.normal = mul(normal_matrix, input.normal);
    output.uv = input.uv;

    return output;
}
