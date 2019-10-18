#version 450
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

struct PostEffect {
	bool inversion;
	bool grayScale;
	bool sharpen;
	bool blur;
	bool edgeDetect;
};

uniform PostEffect postEffect;

vec3 samples[9];
void takeSamplesForKernel();

vec3 sharpen(vec3 color);
vec3 blur(vec3 color);
vec3 edgeDetect(vec3 color);

void main()
{
	takeSamplesForKernel();
	FragColor = texture(screenTexture, TexCoords);

	if(postEffect.sharpen)
	{
		FragColor = vec4(sharpen(FragColor.rgb), 1.0);
	}
	if(postEffect.blur) {
		FragColor = vec4(blur(FragColor.rgb), 1.0);
	}
	if(postEffect.edgeDetect) {
		FragColor = vec4(edgeDetect(FragColor.rgb), 1.0);
	}
	if(postEffect.inversion) {
		FragColor = vec4((1 - FragColor.rgb), 1.0);
	}
	if(postEffect.grayScale) 
	{
		float average = (FragColor.r + FragColor.g + FragColor.b) / 3;
		FragColor = vec4(average, average, average, 1.0);
	}
}

void takeSamplesForKernel() {
	const float offset = 1.0 / 300.0;  
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
	for(int i = 0; i < 9; i++) {
		samples[i] = texture(screenTexture, TexCoords.st + offsets[i]).rgb;
	}
}

vec3 sharpen(vec3 color) {
	float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

	vec3 result = vec3(0);
	for(int i = 0; i < 9; i++) {
		result += samples[i] * kernel[i];
		samples[i] = samples[i] * kernel[i];
	}
	return result;
}

vec3 blur(vec3 color) {
	float kernel[9] = float[](
		1.0 / 16, 2.0 / 16, 1.0 / 16,
		2.0 / 16, 4.0 / 16, 2.0 / 16,
		1.0 / 16, 2.0 / 16, 1.0 / 16  
	);

	vec3 result = vec3(0);
	for(int i = 0; i < 9; i++) {
		result += samples[i] * kernel[i];
		samples[i] = samples[i] * kernel[i];
	}
	return result;
}

vec3 edgeDetect(vec3 color) {
	float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

	vec3 result = vec3(0);
	for(int i = 0; i < 9; i++) {
		result += samples[i] * kernel[i];
		samples[i] = samples[i] * kernel[i];
	}
	return result;
}