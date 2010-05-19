uniform sampler2D sector_texture;

void main()
{
	gl_FragColor = texture2D(sector_texture, gl_TexCoord[0].st);
}
