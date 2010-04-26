#extension GL_AMD_vertex_shader_tessellator : require

__samplerVertexAMD vertex;

uniform vec2 uv_lookup_table[4];

//uniform sampler2D xz_offset_texture;
//uniform sampler2D height_map_texture;

void main(void)
{
	gl_Vertex = vec4(0.0);
	gl_Normal = vec3(0.0);
	gl_MultiTexCoord0 = vec4(0.0);

	float u = gl_UVCoord[0];
	float v = gl_UVCoord[1];

	// redo the UVs
	// patch vertex index is the index number modulo 4
	int index0 = gl_VertexQuadIndex[0] % 4;
	int index1 = gl_VertexQuadIndex[1] % 4;
	int index2 = gl_VertexQuadIndex[2] % 4;
	int index3 = gl_VertexQuadIndex[3] % 4;

	// base weights to interpolate depend on index value
	vec2 base_weight0 = uv_lookup_table[index0];
	vec2 base_weight1 = uv_lookup_table[index1];
	vec2 base_weight2 = uv_lookup_table[index2];
	vec2 base_weight3 = uv_lookup_table[index3];

	// use cartesian interpolation to calculate our parametric coordinates
	vec2 uv1 = (base_weight0 * u + base_weight1 * (1.0 - u));
	vec2 uv2 = (base_weight3 * u + base_weight2 * (1.0 - u));
	vec2 uv = uv1 * v + uv2 * (1.0 - v);

	u = uv.x;
	v = uv.y;

	// now the UVs obtained from the HW tessellation unit
	// have been mapped into [0,1] range

	// make sure that our 4 corner points remain in the same order even though
	// they are getting reordered by the HW
	vec4 v0, v1, v2, v3;

	if ((index0 == 0) && (index1 == 1) &&
			(index2 == 2) && (index3 == 3)) {
		v0 = vertexFetchAMD(vertex, gl_VertexQuadIndex[0]);
		v1 = vertexFetchAMD(vertex, gl_VertexQuadIndex[1]);
		v2 = vertexFetchAMD(vertex, gl_VertexQuadIndex[2]);
		v3 = vertexFetchAMD(vertex, gl_VertexQuadIndex[3]);
	} else if ((index0 == 1) && (index1 == 0) &&
				(index2 == 3) && (index3 == 2)) {
		v0 = vertexFetchAMD(vertex, gl_VertexQuadIndex[1]);
		v1 = vertexFetchAMD(vertex, gl_VertexQuadIndex[0]);
		v2 = vertexFetchAMD(vertex, gl_VertexQuadIndex[3]);
		v3 = vertexFetchAMD(vertex, gl_VertexQuadIndex[2]);
	} else if ((index0 == 2) && (index1 == 3) &&
				(index2 == 0) && (index3 == 1)) {
		v0 = vertexFetchAMD(vertex, gl_VertexQuadIndex[2]);
		v1 = vertexFetchAMD(vertex, gl_VertexQuadIndex[3]);
		v2 = vertexFetchAMD(vertex, gl_VertexQuadIndex[0]);
		v3 = vertexFetchAMD(vertex, gl_VertexQuadIndex[1]);
	} else if ((index0 == 3) && (index1 == 2) &&
				(index2 == 1) && (index3 == 0)) {
		v0 = vertexFetchAMD(vertex, gl_VertexQuadIndex[3]);
		v1 = vertexFetchAMD(vertex, gl_VertexQuadIndex[2]);
		v2 = vertexFetchAMD(vertex, gl_VertexQuadIndex[1]);
		v3 = vertexFetchAMD(vertex, gl_VertexQuadIndex[0]);
	} else {
		v0 = vertexFetchAMD(vertex, gl_VertexQuadIndex[0]);
		v1 = vertexFetchAMD(vertex, gl_VertexQuadIndex[1]);
		v2 = vertexFetchAMD(vertex, gl_VertexQuadIndex[2]);
		v3 = vertexFetchAMD(vertex, gl_VertexQuadIndex[3]);
	}

	/*// define the basis using parameter u
	float bu0 = (1-u) * (1-u) * (1-u);
	float bu1 = 3 * u * (1-u) * (1-u);
	float bu2 = 3 * u * u * (1-u);
	float bu3 = u * u * u;

	// define the basis using parameter v
	float bv0 = (1-v) * (1-v) * (1-v);
	float bv1 = 3 * v * (1-v) * (1-v);
	float bv2 = 3 * v * v * (1-v);
	float bv3 = v * v * v;

	gl_Vertex += bu0 * bv0 * v0;
	gl_Vertex += bu3 * bv0 * v1;
	gl_Vertex += bu0 * bv3 * v3;
	gl_Vertex += bu3 * bv3 * v2;*/

	//gl_Vertex.z = v0.x + u * (v1.x - v0.x);
	//gl_Vertex.x = v2.z + v * (v3.z - v2.z);

	gl_Vertex += u * v0;
	gl_Vertex += u * v1;
	gl_Vertex += u * v3;
	gl_Vertex += u * v2;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
