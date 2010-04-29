uniform sampler2D sector_texture;

varying vec4 test;

void main()
{
	gl_FragColor = texture2D(sector_texture, gl_TexCoord[0].st);
	//gl_FragColor = vec4(test.st, 0.0, 1.0);
}
