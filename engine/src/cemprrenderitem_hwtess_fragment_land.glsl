uniform sampler2D sector_texture;

varying vec4 tmp;

void main()
{
	gl_FragColor = texture2D(sector_texture, gl_TexCoord[0].st);
	//gl_FragColor = tmp;
}
