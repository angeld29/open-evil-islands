uniform sampler2D sector_texture;

void main()
{
    // just only GL_DECAL
    vec4 color = texture2D(sector_texture, gl_TexCoord[0].st);
    gl_FragColor = vec4(mix(gl_FrontMaterial.diffuse.rgb,
        color.rgb, color.a), gl_FrontMaterial.diffuse.a);
}
