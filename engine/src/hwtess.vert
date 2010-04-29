#extension GL_AMD_vertex_shader_tessellator : require

__samplerVertexAMD vertices;
__samplerVertexAMD texcoords;

//uniform sampler2D normals;
//uniform sampler2D xz_offsets;
//uniform sampler2D height_map;

uniform vec2 uv_lookup_table[4];

varying vec4 tmp;

void main(void)
{
	gl_Vertex = vec4(0.0);
	gl_MultiTexCoord0 = vec4(0.0);

	for (int i = 0; i < 3; ++i) {
		float weight = gl_BarycentricCoord[i];
		gl_Vertex += weight * vertexFetchAMD(vertices, gl_VertexTriangleIndex[i]);
		gl_MultiTexCoord0 += weight * vertexFetchAMD(texcoords, gl_VertexTriangleIndex[i]);
	}

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

	tmp = vec4(uv_lookup_table[gl_VertexTriangleIndex[0]], 0, 1);
}
