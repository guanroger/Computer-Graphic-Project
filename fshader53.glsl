/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 color;
out vec4 fColor;

// this is for flog
in float fog_distance;
in float fog_f_for_frag;

in float float_texture;

// this is for texture
in  vec2 texCoord;
uniform sampler2D texture_2D; /* Note: If using multiple textures,
                                       each texture must be bound to a
                                       *different texture unit*, with the
                                       sampler uniform var set accordingly.
                                 The (fragment) shader can access *all texture units*
                                 simultaneously.
                              */

uniform sampler1D texture_1D;



uniform int floor_texture;
uniform int sphere_texture;

// for lattices
in vec2 lattice_Coord;
uniform int lattice_option;

void main() 
{ 
    vec4 original_color= color;

    // this is for texture
    vec4 tex_color=color;

    if  (floor_texture == 1) tex_color = color * texture( texture_2D, texCoord ) ;

    if (sphere_texture ==1) {
        tex_color = color * texture( texture_1D, float_texture ) ;
    }
    else if (sphere_texture ==2){

        vec4 temp_color = texture(texture_2D, texCoord);
        //if (temp_color[0] == 0 && temp_color[1] == 1 && temp_color[2] == 0 ) {
        if (temp_color[0] == 0){
          temp_color = vec4(0.9, 0.1, 0.1, 1.0); // change color to red
        }
        tex_color = color * temp_color;
    }

    // this is for lattice

    if (lattice_option == 1 || lattice_option == 2){
        float s = lattice_Coord[0];
        float t = lattice_Coord[1];
        if (fract(4 * s) < 0.35 && fract(4 * s) > 0 && fract(4 * t) > 0 &&  fract(4 * t) < 0.35)   discard;
    }


    // this is for fog
    vec4 fog_color=vec4(0.7,0.7,0.7,0.5);

    if(fog_f_for_frag==0.0)
    { 
        fColor =tex_color;
    }
    else if(fog_f_for_frag==1.0)
    { 
        float x=clamp(fog_distance,0.0,18.0); 
        fColor=mix(tex_color,fog_color,x/18);
    }
    else if(fog_f_for_frag==2.0)
    { 
        float x=1/exp(0.09*fog_distance);
        fColor=mix(tex_color,fog_color,1-x);
    }
    else if(fog_f_for_frag==3.0)
    { 
        float x=1/exp(0.09*0.09*fog_distance*fog_distance);  
        fColor=mix(tex_color ,fog_color,1-x);
    }



} 

