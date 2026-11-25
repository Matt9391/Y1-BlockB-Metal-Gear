//--------------------------------------------------------------
// Diffuse vertex shader
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
    // Object space to world space: where the mesh is
    float4x4 world;

    // Rows of the matrix that turns object space normals into world space
    // (the inverse transpose of the world matrix). Constants are read in
    // rows of four floats, so each float3 shares its row with one setting.
    float3 normal_row_0;
    float lit;          // 1 when the scene's light shades the mesh, 0 when not
    float3 normal_row_1;
    float alpha_cutoff; // pixels with less alpha are not drawn (material::alpha_cutoff)
    float3 normal_row_2;
    float padding;      // fills the last row

    float4 base_color;
};

[[vk::push_constant]] ConstantBuffer<mesh_constants> constants : register(b0);

// Set by renderer_3d once a frame, and again when the camera changes (must
// match frame_data in renderer_3d.cpp)
[[vk::binding(0, 3)]] cbuffer frame_data : register(b2)
{
    float4x4 view_projection; // world space to the screen: the camera
    float4 camera_position;   // where the camera is in world space; w is 1
};

pixel_input main(vertex_input input)
{
    const float3x3 normal_matrix = float3x3(constants.normal_row_0, constants.normal_row_1, constants.normal_row_2);
    const float4 world_position = mul(constants.world, float4(input.position, 1.0f));

    pixel_input output;
    output.position = mul(view_projection, world_position);
    output.normal = mul(normal_matrix, input.normal);
    output.uv = input.uv;

    return output;
}
