//--------------------------------------------------------------
// Debug line pixel shader
//--------------------------------------------------------------

struct pixel_input
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

float4 main(pixel_input input) : SV_TARGET
{
    return input.color;
}
