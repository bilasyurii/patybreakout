uniform sampler2D 	source;
uniform vec2 		offsetFactor;

void main() {
	vec2 textureCoordinates = gl_TexCoord[0].xy;
	vec4 color = vec4(0.0);
	
	color += texture2D(source, textureCoordinates - 10.0 * offsetFactor) * 0.0012;
    color += texture2D(source, textureCoordinates - 9.0 * offsetFactor) * 0.0015;
    color += texture2D(source, textureCoordinates - 8.0 * offsetFactor) * 0.0038;
    color += texture2D(source, textureCoordinates - 7.0 * offsetFactor) * 0.0087;
    color += texture2D(source, textureCoordinates - 6.0 * offsetFactor) * 0.0180;
    color += texture2D(source, textureCoordinates - 5.0 * offsetFactor) * 0.0332;
    color += texture2D(source, textureCoordinates - 4.0 * offsetFactor) * 0.0547;
    color += texture2D(source, textureCoordinates - 3.0 * offsetFactor) * 0.0807;
    color += texture2D(source, textureCoordinates - 2.0 * offsetFactor) * 0.1065;
    color += texture2D(source, textureCoordinates - offsetFactor) * 0.1258;
    color += texture2D(source, textureCoordinates) * 0.1330;
    color += texture2D(source, textureCoordinates + offsetFactor) * 0.1258;
    color += texture2D(source, textureCoordinates + 2.0 * offsetFactor) * 0.1065;
    color += texture2D(source, textureCoordinates + 3.0 * offsetFactor) * 0.0807;
    color += texture2D(source, textureCoordinates + 4.0 * offsetFactor) * 0.0547;
    color += texture2D(source, textureCoordinates + 5.0 * offsetFactor) * 0.0332;
    color += texture2D(source, textureCoordinates + 6.0 * offsetFactor) * 0.0180;
    color += texture2D(source, textureCoordinates - 7.0 * offsetFactor) * 0.0087;
    color += texture2D(source, textureCoordinates - 8.0 * offsetFactor) * 0.0038;
    color += texture2D(source, textureCoordinates - 9.0 * offsetFactor) * 0.0015;
    color += texture2D(source, textureCoordinates - 10.0 * offsetFactor) * 0.0012;
	
	gl_FragColor = color;
}