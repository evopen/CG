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
	bool paint;
};

uniform PostEffect postEffect;

uniform int sample_radius;

vec3 samples[9];
vec3 kuwaharaSamples[25];
void takeSamplesForKernel();

vec3 sharpen(vec3 color);
vec3 blur(vec3 color);
vec3 edgeDetect(vec3 color);
vec3 paint(vec3 color);

void main()
{
	takeSamplesForKernel();
	FragColor = texture(screenTexture, TexCoords);

	if(postEffect.paint) {
		FragColor = vec4(paint(FragColor.rgb), 1.0);
	}
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
	ivec2 sizes = textureSize(screenTexture, 0);
	float x_offset = 1.0 / sizes[0] * sample_radius;  
	float y_offset = 1.0 / sizes[1] * sample_radius;  
	vec2 offsets[9] = vec2[](
        vec2(-x_offset,  y_offset), // top-left
        vec2( 0.0f,    y_offset), // top-center
        vec2( x_offset,  y_offset), // top-right
        vec2(-x_offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( x_offset,  0.0f),   // center-right
        vec2(-x_offset, -y_offset), // bottom-left
        vec2( 0.0f,   -y_offset), // bottom-center
        vec2( x_offset, -y_offset)  // bottom-right    
    );

	vec2 kuwaharaOffsets[25] = {
		vec2(-2*x_offset, 2*y_offset), vec2(-1*x_offset, 2*y_offset), vec2(0.0f, 2*y_offset), vec2(1*x_offset, 2*y_offset), vec2(2*x_offset, 2*y_offset),
		vec2(-2*x_offset, 1*y_offset), vec2(-1*x_offset, 1*y_offset), vec2(0.0f, 1*y_offset), vec2(1*x_offset, 1*y_offset), vec2(2*x_offset, 1*y_offset),
		vec2(-2*x_offset, 0*y_offset), vec2(-1*x_offset, 0*y_offset), vec2(0.0f, 0*y_offset), vec2(1*x_offset, 0*y_offset), vec2(2*x_offset, 0*y_offset),
		vec2(-2*x_offset, -1*y_offset), vec2(-1*x_offset, -1*y_offset), vec2(0.0f, -1*y_offset), vec2(1*x_offset, -1*y_offset), vec2(2*x_offset, -1*y_offset),
		vec2(-2*x_offset, -2*y_offset), vec2(-1*x_offset,-2*y_offset), vec2(0.0f, -2*y_offset), vec2(1*x_offset, -2*y_offset), vec2(2*x_offset, -2*y_offset),
	};

	for(int i = 0; i < 9; i++) {
		samples[i] = texture(screenTexture, TexCoords.st + offsets[i]).rgb;
	}

	for(int i = 0; i < 25; i++) {
		kuwaharaSamples[i] = texture(screenTexture, TexCoords.st + kuwaharaOffsets[i]).rgb;
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

vec3 paint(vec3 color) {
	vec3 mean[4] = vec3[](vec3(0.0), vec3(0), vec3(0), vec3(0));
	vec3 variance[4] = vec3[](vec3(0.0), vec3(0), vec3(0), vec3(0));
	float totalVarience[4] = float[](0,0,0,0);

	for(int kernel = 0; kernel < 4; kernel++) {
		int i_begin, i_end, j_begin, j_end;

		if(kernel % 2 ==0) {
			i_begin = 0;
		} else {
			i_begin = 2;
		}
		i_end = i_begin + 3;

		if(kernel < 2) {
			j_begin = 0;
		} else {
			j_begin = 2;
		}
		j_end = j_begin + 3;
		
		for(int i = i_begin; i < i_end; i++) {
			for(int j = j_begin; j < j_end; j++) {
				mean[kernel] += kuwaharaSamples[j * 5 + i];
				variance[kernel] += kuwaharaSamples[j * 5 + i] * kuwaharaSamples[j * 5 + i];
			}
		}
		mean[kernel] /= 9;
		variance[kernel] = variance[kernel] / 9 - mean[kernel] * mean[kernel];
		totalVarience[kernel] = variance[kernel].r + variance[kernel].g + variance[kernel].b;
	}

	int minVarIndex = 0;
	for(int i = 1;i<4;i++) {
		if(totalVarience[i] < totalVarience[minVarIndex]) {
			minVarIndex = i;
		}
	}

	return mean[minVarIndex];
}