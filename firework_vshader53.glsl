/* 
File Name: "firework_vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 vPosition; // ths is the initial position
in vec3 vColor;
in vec3 velocity;


uniform float penetration_flag;

out float penetration_flag_for_frag;

out vec4 color;
out float current_y;


uniform mat4 ModelView;
uniform mat4 Projection;
uniform float time;


void main()
{



    float init_x = vPosition.x;
    float init_y = vPosition.y;
    float init_z = vPosition.z;

    float temp_x = 0.0 + time * velocity[0] * 0.001;  // translate to milli second
    float temp_y = 0.1 + 0.001 * velocity[1] * time +0.5 * (-0.00000049) * time * time;
    float temp_z = 0.0 + time * velocity[2] * 0.001;  // translate to milli second

    current_y = temp_y;

    vec4 vPosition4 = vec4(temp_x, temp_y, temp_z, 1.0);
    vec4 vColor4 = vec4(vColor.r, vColor.y, vColor.z, 1.0);

    // YJC: Original, incorrect below:
    //      gl_Position = projection*model_view*vPosition/vPosition.w;

    gl_Position = Projection * ModelView * vPosition4;


    if( penetration_flag == 3.0){
        penetration_flag_for_frag=1.0;
    }
    else if ( penetration_flag == 2.0){
        if (temp_y <0.0){
            vColor4 = vec4(0.529f, 0.807f, 0.92f, 0.0f);
        }
         
         penetration_flag_for_frag=0.0;

    }
    else if (penetration_flag == 1.0){
        penetration_flag_for_frag=0.0;
    }

    color = vColor4;



}
