#pragma once

#include <FFGLSDK.h>
#include <array>
#include <chrono>

class MorphosisSpray : public CFFGLPlugin
{
public:
	MorphosisSpray();

	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;
	FFResult SetFloatParameter( unsigned int index, float value ) override;
	float GetFloatParameter( unsigned int index ) override;
	FFResult SetTime( double time ) override;
	unsigned int Resize( const FFGLViewportStruct* vp ) override;
	const char* GetShortName() override;

private:
	enum Param : unsigned int
	{
		Speed,
		Flow,
		Granularity,
		SprayDensity,
		BlobScale,
		CenterGlow,
		ColorDrift,
		Contrast,
		Seed,
		Opacity,
		TopHue,
		TopSat,
		TopBri,
		TopAlpha,
		RightHue,
		RightSat,
		RightBri,
		RightAlpha,
		BottomHue,
		BottomSat,
		BottomBri,
		BottomAlpha,
		LeftHue,
		LeftSat,
		LeftBri,
		LeftAlpha,
		GlowHue,
		GlowSat,
		GlowBri,
		GlowAlpha,
		ParamCount
	};

	float CurrentTimeSeconds() const;
	void SetColorUniform( GLint location, Param hueParam );

	std::array< float, ParamCount > params;
	ffglex::FFGLShader shader;
	ffglex::FFGLScreenQuad quad;
	std::chrono::steady_clock::time_point startedAt;
	float hostTimeSeconds;
	bool hasHostTime;

	GLint timeLocation;
	GLint resolutionLocation;
	GLint speedLocation;
	GLint flowLocation;
	GLint granularityLocation;
	GLint sprayDensityLocation;
	GLint blobScaleLocation;
	GLint centerGlowLocation;
	GLint colorDriftLocation;
	GLint contrastLocation;
	GLint seedLocation;
	GLint opacityLocation;
	GLint topColorLocation;
	GLint rightColorLocation;
	GLint bottomColorLocation;
	GLint leftColorLocation;
	GLint glowColorLocation;
};
