#include "MorphosisSpray.h"

#include <algorithm>
#include <cmath>

using namespace ffglex;

static CFFGLPluginInfo PluginInfo(
	PluginFactory< MorphosisSpray >,
	"MSP1",
	"MorphosisSpray",
	2,
	1,
	1,
	001,
	FF_SOURCE,
	"Animated spray-paint pigment fields inspired by morph.jpeg.",
	"MorphosisSpray FFGL source for Resolume Arena"
);

static const char vertexShaderCode[] = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static const char fragmentShaderCode[] = R"(#version 410 core
uniform float uTime;
uniform vec2 uResolution;
uniform float uSpeed;
uniform float uFlow;
uniform float uGranularity;
uniform float uSprayDensity;
uniform float uBlobScale;
uniform float uCenterGlow;
uniform float uColorDrift;
uniform float uContrast;
uniform float uSeed;
uniform float uOpacity;
uniform vec4 uTopColor;
uniform vec4 uRightColor;
uniform vec4 uBottomColor;
uniform vec4 uLeftColor;
uniform vec4 uGlowColor;

in vec2 uv;
out vec4 fragColor;

float hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

vec2 hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.xx + p3.yz) * p3.zy);
}

float noise(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);

	float a = hash12(i + vec2(0.0, 0.0));
	float b = hash12(i + vec2(1.0, 0.0));
	float c = hash12(i + vec2(0.0, 1.0));
	float d = hash12(i + vec2(1.0, 1.0));

	return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
	float value = 0.0;
	float amp = 0.5;
	for(int i = 0; i < 5; ++i)
	{
		value += amp * noise(p);
		p = p * 2.03 + vec2(19.17, 7.31);
		amp *= 0.5;
	}
	return value;
}

float oval(vec2 p, vec2 c, vec2 r, float softness)
{
	vec2 q = (p - c) / r;
	return exp(-dot(q, q) * softness);
}

vec3 hueShift(vec3 color, float shift)
{
	const vec3 k = vec3(0.57735026919);
	float angle = shift * 6.28318530718;
	float c = cos(angle);
	float s = sin(angle);
	return color * c + cross(k, color) * s + k * dot(k, color) * (1.0 - c);
}

void main()
{
	vec2 st = uv;
	float aspect = max(uResolution.x / max(uResolution.y, 1.0), 0.0001);
	vec2 p = vec2((st.x - 0.5) * aspect + 0.5, st.y);

	float t = uTime * mix(0.05, 1.35, uSpeed);
	float seed = 10.0 + uSeed * 917.0;
	float flowAmount = uFlow * 0.12;
	float blobScale = mix(0.74, 1.42, uBlobScale);

	vec2 driftA = vec2(fbm(p * 2.0 + vec2(t * 0.18, seed)), fbm(p * 2.0 + vec2(seed, -t * 0.16))) - 0.5;
	vec2 driftB = vec2(fbm(p * 5.0 + vec2(-t * 0.12, seed * 0.3)), fbm(p * 5.0 + vec2(seed * 0.7, t * 0.14))) - 0.5;
	vec2 warped = p + (driftA * 1.1 + driftB * 0.45) * flowAmount;

	vec2 nudge1 = (hash22(vec2(seed, 1.0)) - 0.5) * 0.06;
	vec2 nudge2 = (hash22(vec2(seed, 2.0)) - 0.5) * 0.06;
	vec2 nudge3 = (hash22(vec2(seed, 3.0)) - 0.5) * 0.06;
	vec2 nudge4 = (hash22(vec2(seed, 4.0)) - 0.5) * 0.06;

	vec2 cTop = vec2(0.43, 0.90) + nudge1 + flowAmount * vec2(sin(t * 0.73 + seed), cos(t * 0.62));
	vec2 cRight = vec2(0.86, 0.50) + nudge2 + flowAmount * vec2(sin(t * 0.47 + 2.2), cos(t * 0.81 + seed));
	vec2 cBottom = vec2(0.66, 0.12) + nudge3 + flowAmount * vec2(sin(t * 0.57 + seed * 0.2), cos(t * 0.54 + 1.7));
	vec2 cLeft = vec2(0.23, 0.37) + nudge4 + flowAmount * vec2(sin(t * 0.65 + 4.0), cos(t * 0.46 + seed));
	vec2 cGlow = vec2(0.53, 0.51) + flowAmount * 0.55 * vec2(sin(t * 0.36 + seed), cos(t * 0.42 + 3.1));

	float top = oval(warped, cTop, vec2(0.54, 0.54) * blobScale, 1.20);
	float cyan = oval(warped, cRight, vec2(0.47, 0.47) * blobScale, 1.10);
	float blue = oval(warped, cBottom, vec2(0.52, 0.52) * blobScale, 1.08);
	float magenta = oval(warped, cLeft, vec2(0.43, 0.43) * blobScale, 1.16);
	float glow = oval(warped, cGlow, vec2(0.27, 0.27) * blobScale, 1.35);
	float halo = oval(warped, cGlow + vec2(-0.02, -0.02), vec2(0.39, 0.39) * blobScale, 1.25);

	vec3 greenColor = uTopColor.rgb * uTopColor.a;
	vec3 cyanColor = uRightColor.rgb * uRightColor.a;
	vec3 blueColor = uBottomColor.rgb * uBottomColor.a;
	vec3 magentaColor = uLeftColor.rgb * uLeftColor.a;
	vec3 warmColor = uGlowColor.rgb * uGlowColor.a;
	vec3 paperColor = vec3(0.17, 0.34, 0.31);

	float fieldNoise = fbm(warped * 3.3 + vec2(seed * 0.17, t * 0.06));
	vec3 color = paperColor;
	color += greenColor * (0.70 * top + 0.15);
	color += cyanColor * (0.86 * cyan + 0.08 * st.x);
	color += blueColor * (0.92 * blue + 0.23 * (1.0 - st.y));
	color += magentaColor * (0.88 * magenta);
	color /= 1.0 + 0.70 * top + 0.86 * cyan + 0.92 * blue + 0.88 * magenta;

	color = mix(color, warmColor, clamp(halo * 0.36 + glow * mix(0.35, 0.95, uCenterGlow), 0.0, 1.0));
	color += warmColor * glow * mix(0.05, 0.35, uCenterGlow);

	float driftHue = (fieldNoise - 0.5) * 0.20 * uColorDrift + sin(t * 0.19 + seed) * 0.035 * uColorDrift;
	color = clamp(hueShift(color, driftHue), 0.0, 1.25);

	float pigment = fbm(warped * mix(18.0, 52.0, uGranularity) + vec2(seed, -t * 0.18));
	color *= 0.92 + pigment * 0.15;

	float grainFrequency = mix(70.0, 1850.0, uGranularity);
	vec2 grainCell = floor((st + vec2(seed * 0.013, seed * 0.021)) * grainFrequency);
	float grain = hash12(grainCell);
	float fine = hash12(floor((st + vec2(t * 0.0007, -t * 0.0004)) * grainFrequency * 2.7));
	float spray = (grain - 0.5) * 0.46 + (fine - 0.5) * 0.18;
	float speck = smoothstep(0.80, 1.0, grain) - smoothstep(0.0, 0.13, fine) * 0.45;
	color += spray * uSprayDensity;
	color = mix(color, color + vec3(speck) * 0.14, uSprayDensity);

	float edge = smoothstep(0.88, 0.24, distance(st, vec2(0.5)));
	color *= mix(0.83, 1.04, edge);

	color = (color - 0.5) * mix(0.72, 1.72, uContrast) + 0.5;
	color = clamp(color, 0.0, 1.0);

	fragColor = vec4(color, clamp(uOpacity, 0.0, 1.0));
}
)";

MorphosisSpray::MorphosisSpray() :
	params{ {
		0.32f,
		0.45f,
		0.62f,
		0.56f,
		0.55f,
		0.78f,
		0.20f,
		0.56f,
		0.18f,
		1.00f,
		0.387f,
		0.651f,
		0.430f,
		1.000f,
		0.525f,
		0.971f,
		0.680f,
		1.000f,
		0.618f,
		0.810f,
		0.420f,
		1.000f,
		0.912f,
		0.879f,
		0.580f,
		1.000f,
		0.074f,
		0.450f,
		1.000f,
		1.000f,
	} },
	startedAt( std::chrono::steady_clock::now() ),
	hostTimeSeconds( 0.0f ),
	hasHostTime( false ),
	timeLocation( -1 ),
	resolutionLocation( -1 ),
	speedLocation( -1 ),
	flowLocation( -1 ),
	granularityLocation( -1 ),
	sprayDensityLocation( -1 ),
	blobScaleLocation( -1 ),
	centerGlowLocation( -1 ),
	colorDriftLocation( -1 ),
	contrastLocation( -1 ),
	seedLocation( -1 ),
	opacityLocation( -1 ),
	topColorLocation( -1 ),
	rightColorLocation( -1 ),
	bottomColorLocation( -1 ),
	leftColorLocation( -1 ),
	glowColorLocation( -1 )
{
	SetMinInputs( 0 );
	SetMaxInputs( 0 );
	SetTimeSupported( true );

	SetParamInfof( Speed, "Speed", FF_TYPE_STANDARD );
	SetParamInfof( Flow, "Flow", FF_TYPE_STANDARD );
	SetParamInfof( Granularity, "Granularity", FF_TYPE_STANDARD );
	SetParamInfof( SprayDensity, "Spray Density", FF_TYPE_STANDARD );
	SetParamInfof( BlobScale, "Blob Scale", FF_TYPE_STANDARD );
	SetParamInfof( CenterGlow, "Center Glow", FF_TYPE_STANDARD );
	SetParamInfof( ColorDrift, "Color Drift", FF_TYPE_STANDARD );
	SetParamInfof( Contrast, "Contrast", FF_TYPE_STANDARD );
	SetParamInfof( Seed, "Seed", FF_TYPE_STANDARD );
	SetParamInfof( Opacity, "Opacity", FF_TYPE_ALPHA );
	SetParamInfof( TopHue, "Top", FF_TYPE_HUE );
	SetParamInfof( TopSat, "Top_Sat", FF_TYPE_SATURATION );
	SetParamInfof( TopBri, "Top_Bri", FF_TYPE_BRIGHTNESS );
	SetParamInfof( TopAlpha, "Top_Alpha", FF_TYPE_ALPHA );
	SetParamInfof( RightHue, "Right", FF_TYPE_HUE );
	SetParamInfof( RightSat, "Right_Sat", FF_TYPE_SATURATION );
	SetParamInfof( RightBri, "Right_Bri", FF_TYPE_BRIGHTNESS );
	SetParamInfof( RightAlpha, "Right_Alpha", FF_TYPE_ALPHA );
	SetParamInfof( BottomHue, "Bottom", FF_TYPE_HUE );
	SetParamInfof( BottomSat, "Bottom_Sat", FF_TYPE_SATURATION );
	SetParamInfof( BottomBri, "Bottom_Bri", FF_TYPE_BRIGHTNESS );
	SetParamInfof( BottomAlpha, "Bottom_Alpha", FF_TYPE_ALPHA );
	SetParamInfof( LeftHue, "Left", FF_TYPE_HUE );
	SetParamInfof( LeftSat, "Left_Sat", FF_TYPE_SATURATION );
	SetParamInfof( LeftBri, "Left_Bri", FF_TYPE_BRIGHTNESS );
	SetParamInfof( LeftAlpha, "Left_Alpha", FF_TYPE_ALPHA );
	SetParamInfof( GlowHue, "Glow", FF_TYPE_HUE );
	SetParamInfof( GlowSat, "Glow_Sat", FF_TYPE_SATURATION );
	SetParamInfof( GlowBri, "Glow_Bri", FF_TYPE_BRIGHTNESS );
	SetParamInfof( GlowAlpha, "Glow_Alpha", FF_TYPE_ALPHA );

	SetParamGroup( Speed, "Motion" );
	SetParamGroup( Flow, "Motion" );
	SetParamGroup( Granularity, "Spray" );
	SetParamGroup( SprayDensity, "Spray" );
	SetParamGroup( BlobScale, "Pigment" );
	SetParamGroup( CenterGlow, "Pigment" );
	SetParamGroup( ColorDrift, "Color" );
	SetParamGroup( Contrast, "Color" );
	SetParamGroup( Seed, "Variation" );
	SetParamGroup( Opacity, "Output" );
	SetParamGroup( TopHue, "Blob Colors" );
	SetParamGroup( TopSat, "Blob Colors" );
	SetParamGroup( TopBri, "Blob Colors" );
	SetParamGroup( TopAlpha, "Blob Colors" );
	SetParamGroup( RightHue, "Blob Colors" );
	SetParamGroup( RightSat, "Blob Colors" );
	SetParamGroup( RightBri, "Blob Colors" );
	SetParamGroup( RightAlpha, "Blob Colors" );
	SetParamGroup( BottomHue, "Blob Colors" );
	SetParamGroup( BottomSat, "Blob Colors" );
	SetParamGroup( BottomBri, "Blob Colors" );
	SetParamGroup( BottomAlpha, "Blob Colors" );
	SetParamGroup( LeftHue, "Blob Colors" );
	SetParamGroup( LeftSat, "Blob Colors" );
	SetParamGroup( LeftBri, "Blob Colors" );
	SetParamGroup( LeftAlpha, "Blob Colors" );
	SetParamGroup( GlowHue, "Blob Colors" );
	SetParamGroup( GlowSat, "Blob Colors" );
	SetParamGroup( GlowBri, "Blob Colors" );
	SetParamGroup( GlowAlpha, "Blob Colors" );

	SetParamDisplayName( TopHue, "Top Color", false );
	SetParamDisplayName( RightHue, "Right Color", false );
	SetParamDisplayName( BottomHue, "Bottom Color", false );
	SetParamDisplayName( LeftHue, "Left Color", false );
	SetParamDisplayName( GlowHue, "Glow Color", false );
}

FFResult MorphosisSpray::InitGL( const FFGLViewportStruct* vp )
{
	if( !shader.Compile( vertexShaderCode, fragmentShaderCode ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}

	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	timeLocation = shader.FindUniform( "uTime" );
	resolutionLocation = shader.FindUniform( "uResolution" );
	speedLocation = shader.FindUniform( "uSpeed" );
	flowLocation = shader.FindUniform( "uFlow" );
	granularityLocation = shader.FindUniform( "uGranularity" );
	sprayDensityLocation = shader.FindUniform( "uSprayDensity" );
	blobScaleLocation = shader.FindUniform( "uBlobScale" );
	centerGlowLocation = shader.FindUniform( "uCenterGlow" );
	colorDriftLocation = shader.FindUniform( "uColorDrift" );
	contrastLocation = shader.FindUniform( "uContrast" );
	seedLocation = shader.FindUniform( "uSeed" );
	opacityLocation = shader.FindUniform( "uOpacity" );
	topColorLocation = shader.FindUniform( "uTopColor" );
	rightColorLocation = shader.FindUniform( "uRightColor" );
	bottomColorLocation = shader.FindUniform( "uBottomColor" );
	leftColorLocation = shader.FindUniform( "uLeftColor" );
	glowColorLocation = shader.FindUniform( "uGlowColor" );

	return CFFGLPlugin::InitGL( vp );
}

FFResult MorphosisSpray::ProcessOpenGL( ProcessOpenGLStruct* pGL )
{
	(void)pGL;

	ScopedShaderBinding shaderBinding( shader.GetGLID() );
	glUniform1f( timeLocation, CurrentTimeSeconds() );
	glUniform2f( resolutionLocation, static_cast< float >( currentViewport.width ), static_cast< float >( currentViewport.height ) );
	glUniform1f( speedLocation, params[ Speed ] );
	glUniform1f( flowLocation, params[ Flow ] );
	glUniform1f( granularityLocation, params[ Granularity ] );
	glUniform1f( sprayDensityLocation, params[ SprayDensity ] );
	glUniform1f( blobScaleLocation, params[ BlobScale ] );
	glUniform1f( centerGlowLocation, params[ CenterGlow ] );
	glUniform1f( colorDriftLocation, params[ ColorDrift ] );
	glUniform1f( contrastLocation, params[ Contrast ] );
	glUniform1f( seedLocation, params[ Seed ] );
	glUniform1f( opacityLocation, params[ Opacity ] );
	SetColorUniform( topColorLocation, TopHue );
	SetColorUniform( rightColorLocation, RightHue );
	SetColorUniform( bottomColorLocation, BottomHue );
	SetColorUniform( leftColorLocation, LeftHue );
	SetColorUniform( glowColorLocation, GlowHue );

	quad.Draw();

	return FF_SUCCESS;
}

FFResult MorphosisSpray::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	timeLocation = -1;
	resolutionLocation = -1;
	speedLocation = -1;
	flowLocation = -1;
	granularityLocation = -1;
	sprayDensityLocation = -1;
	blobScaleLocation = -1;
	centerGlowLocation = -1;
	colorDriftLocation = -1;
	contrastLocation = -1;
	seedLocation = -1;
	opacityLocation = -1;
	topColorLocation = -1;
	rightColorLocation = -1;
	bottomColorLocation = -1;
	leftColorLocation = -1;
	glowColorLocation = -1;

	return FF_SUCCESS;
}

FFResult MorphosisSpray::SetFloatParameter( unsigned int index, float value )
{
	if( index >= ParamCount )
		return FF_FAIL;

	params[ index ] = std::clamp( value, 0.0f, 1.0f );
	return FF_SUCCESS;
}

float MorphosisSpray::GetFloatParameter( unsigned int index )
{
	if( index >= ParamCount )
		return 0.0f;

	return params[ index ];
}

FFResult MorphosisSpray::SetTime( double time )
{
	hostTimeSeconds = static_cast< float >( time * 0.001 );
	hasHostTime = true;
	return FF_SUCCESS;
}

unsigned int MorphosisSpray::Resize( const FFGLViewportStruct* vp )
{
	currentViewport = *vp;
	return FF_SUCCESS;
}

const char* MorphosisSpray::GetShortName()
{
	return "MorphSpray";
}

float MorphosisSpray::CurrentTimeSeconds() const
{
	if( hasHostTime )
		return hostTimeSeconds;

	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration< float >( now - startedAt ).count();
}

void MorphosisSpray::SetColorUniform( GLint location, Param hueParam )
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float hue = params[ hueParam ];
	if( hue == 1.0f )
		hue = 0.0f;

	HSVtoRGB( hue, params[ hueParam + 1 ], params[ hueParam + 2 ], r, g, b );
	glUniform4f( location, r, g, b, params[ hueParam + 3 ] );
}
