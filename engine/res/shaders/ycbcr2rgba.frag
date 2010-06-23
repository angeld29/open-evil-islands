uniform sampler2D texture;

void main()
{
	vec4 ycbcr = texture2D(texture, gl_TexCoord[0].st);
	float y = 1.1643 * (ycbcr[0] - 0.0625);
	float cb = ycbcr[1] - 0.5;
	float cr = ycbcr[2] - 0.5;
	gl_FragColor.r = y + 1.5958 * cr;
	gl_FragColor.g = y - 0.39173 * cb - 0.81290 * cr;
	gl_FragColor.b = y + 2.017 * cb;
	gl_FragColor.a = 1.0;
}
