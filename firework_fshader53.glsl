/* 
File Name: "firework_fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 color;
out vec4 fColor;

in float penetration_flag_for_frag;

in float current_y;


void main() 
{ 
    if (penetration_flag_for_frag == 1.0){
        if (current_y < 0.1) discard;
    } 

    fColor = color;

} 

