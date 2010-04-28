//uniform sampler2D sector_texture;

varying vec4 tmp;

void main()
{
	//gl_FragColor = texture2D(sector_texture, gl_TexCoord[0].st);
	//gl_FragColor = vec4(gl_TexCoord[0].s, gl_TexCoord[0].t, 0.0, 1.0);
	gl_FragColor = vec4(tmp.x, tmp.y, 0.0, 1.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
