#version 330

// This vertex shader should be used to render a triangle whose normalized device coordinates are:
// (-0.5, -0.5, 0.0), ( 0.5, -0.5, 0.0), ( 0.0,  0.5, 0.0)
// And it also should send the vertex color as a varying to the fragment shader where the colors are (in order):
// (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)

out Varyings {
    vec3 color;
} vs_out;

// in vec3 position;

uniform vec2 translation = vec2(0.0, 0.0);
uniform vec2 scale = vec2(1.0, 1.0);

// Currently, the triangle is always in the same position, but we don't want that.
// So two uniforms should be added: translation (vec2) and scale (vec2).
// Each vertex "v" should be transformed to be "scale * v + translation".
// The default value for "translation" is (0.0, 0.0) and for "scale" is (1.0, 1.0).

//TODO: (Req 1) Finish this shader

void main(){

    const vec3 position[3] = vec3[3] (
        vec3(-0.5,-0.5,0.0),
        vec3(0.5,-0.5,0.0),
        vec3(0.0,0.5,0.0)
    );
    vec3 colors[3] = vec3[3] (
        vec3(1.0,0.0,0.0),
        vec3(0.0,1.0,0.0),
        vec3(0.0,0.0,1.0)
    );
     // get the position by index of vertex 
     //vertex "v" should be transformed to be "scale * v + translation".
     // the default for scale is 1 so I pad it with 1 and and translation is 0 so I pad it with 0
    gl_Position = vec4(vec3(scale,1.0)*position[gl_VertexID]+vec3(translation,0.0) ,1.0);
    // get the color by index of vertex 
    vs_out.color = colors[gl_VertexID];
}