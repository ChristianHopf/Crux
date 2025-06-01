#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse;
//uniform sampler2D texture1;
//uniform sampler2D texture2;

void main(){
	//FragColor = vec4(ourColor, 1.0);
	//FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
  FragColor = texture(texture_diffuse, TexCoord);
}
