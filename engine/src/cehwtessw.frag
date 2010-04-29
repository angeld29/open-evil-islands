//uniform sampler2D sector_texture;

void main()
{
	//vec4 color = texture2D(sector_texture, gl_TexCoord[0].st);
	//gl_FragColor = vec4(mix(gl_FrontMaterial.diffuse.rgb,
	//	color.rgb, color.a), gl_FrontMaterial.diffuse.a);
	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
