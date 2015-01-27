#extension GL_AMD_vertex_shader_tessellator : require

__samplerVertexAMD vertices;

uniform samplerBuffer normals;
uniform samplerBuffer xz_offsets;
uniform samplerBuffer height_map;

uniform float vertex_side;
uniform float vertex_count;

uniform float sector_x;
uniform float sector_z;

uniform float vertex_side_offset_inv;

uniform float sector_x_offset;
uniform float sector_z_offset;

void main(void)
{
    // as is from AMD specification
    gl_Vertex = vec4(0.0);

    for (int i = 0; i < 3; ++i) {
        float weight = gl_BarycentricCoord[i];
        gl_Vertex += weight * vertexFetchAMD(vertices, gl_VertexTriangleIndex[i]);
    }

    // find height value for this vertex in height map
    // map vertex coords to sector
    float x = gl_Vertex.x - sector_x_offset;
    float z = abs(gl_Vertex.z) - sector_z_offset;

    int index = int(round(z) * vertex_side + round(x));

    // note that buffer data is stored as RGBA32F in sampler
    gl_Vertex.y = texelFetchBuffer(height_map, index / 4)[index % 4];

    // restore texture coordinates from vertex position
    // it's simple because we have a regular terrain
    gl_MultiTexCoord0 = vec4(x * vertex_side_offset_inv,
        1.0 - z * vertex_side_offset_inv, 0.0, 0.0);

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
