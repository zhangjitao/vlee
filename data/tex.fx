float alpha = 1.f;
float4x4 transform;

texture tex;
sampler tex_sampler = sampler_state
{
	Texture = (tex);
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VS_OUTPUT
{
	float4 pos  : POSITION;
	float2 tex  : TEXCOORD1;
};

VS_OUTPUT vertex(float4 ipos : POSITION, float2 tex  : TEXCOORD0)
{
	VS_OUTPUT Out;
	Out.pos = mul(ipos, transform);
	Out.tex = tex;
	return Out;
}

float4 pixel(VS_OUTPUT In) : COLOR
{
	return tex2D(tex_sampler, In.tex) * alpha;
}

technique blur_ps_vs_2_0
{
	pass P0
	{
		VertexShader = compile vs_2_0 vertex();
		PixelShader  = compile ps_2_0 pixel();
	}
}