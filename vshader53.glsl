/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in vec3 vColor;
out vec4 color;

// this is for passing into fragment shader 
out float fog_distance;
out float fog_f_for_frag;

// this is for texture
in  vec2 vTexCoord;
out vec2 texCoord;

out  float float_texture;

uniform vec4 uColor; // obj color (as a uniform variable)

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform float Shininess;
uniform float Shading;

//the following is for blending shadow
uniform float blending_shadow;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;

uniform vec4 input_light_ambient;
uniform	vec4 input_light_diffuse;
uniform	vec4 input_light_specular;

uniform float point_flag;

// this is fo spot light
uniform vec4 input_spot_ambient;
uniform vec4 input_spot_diffuse;
uniform vec4 input_spot_specular;
uniform vec4 spot_light_dir;
uniform vec4 light_position_spot;
uniform float spot_flag;
uniform float spot_exponent;
uniform float spot_cut_angle;

// this is for flog
uniform float fog_flag;



uniform int floor_texture;

uniform int sphere_texture;

uniform int vertical_flag;
uniform int slant_flag;
uniform int eye_space_flag;
uniform int object_space_flag;
uniform int sphere_line_flag;

uniform int lattice_option;
out vec2 lattice_Coord;




void main()
{
    



    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;

    vec3 pos1 = (vPosition).xyz;



    





    // this is for flog
    fog_f_for_frag = fog_flag;
    fog_distance = length(pos-(ModelView*vec4(7.0, 3.0, -10.0, 1.0)).xyz);

	
    vec3 L = normalize( LightPosition.xyz - pos ); // this is point P to light source
    vec3 E = normalize( -pos ); // this is the viewer at origin 0.0.0 to the P --> 0 - p
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
      // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz; 
    vec3 N = normalize(Normal_Matrix * vNormal); // here you get the nromal of the object and you put them into eye frame, and you normalize it.

// YJC Note: N must use the one pointing *toward* the viewer
//     ==> If (N dot E) < 0 then N must be changed to -N
//
   if ( dot(N, E) < 0 ) N = -N;


/*--- To Do: Compute attenuation ---*/
//float attenuation = 1.0; 

// Compute distance between light source and point being shaded
float distance = distance(LightPosition.xyz, pos);
float attenuation = 1.0 / (ConstAtt + LinearAtt *distance   + QuadAtt * distance * distance );

 // Compute terms in the illumination equation

    vec4 ambient = input_light_ambient*material_ambient;

    float d = max( dot(L, N), 0.0 );
    vec4  diffuse = d * input_light_diffuse*material_diffuse;

    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * input_light_specular*material_specular;

    if( dot(L, N) < 0.0 ) {
	    specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 



    vec4 initial_color = 1.0 * (ambient + diffuse + specular);
    vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, 1.0); 

    vec4 light_ambient_global_ambient= vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // this is the directional light
    vec3 dir_l_distant=vec3(0.1,0.0,-1.0); // this is the vector from P to the light source (because it does not have a point light source)
	vec4 dir_light_ambient=vec4( 0.0, 0.0, 0.0, 1.0 );
	vec4 dir_light_diffuse=vec4( 0.8, 0.8, 0.8, 1.0 );
	vec4 dir_light_specular=vec4( 0.2, 0.2, 0.2, 1.0 );

    vec4 dir_ambient_product = dir_light_ambient * material_ambient;
	vec4 dir_diffuse_product = dir_light_diffuse * material_diffuse;
	vec4 dir_specular_product = dir_light_specular * material_specular;

    vec3 L_dir = normalize(-dir_l_distant); //  also it is already in the eye coordinat system
	vec3 E_dir = normalize(-pos);
	vec3 H_dir = normalize ( L_dir + E_dir);

    vec4 dir_ambient = dir_ambient_product;
	
	float dir_d = max( dot(L_dir, N), 0 );
	vec4  dir_diffuse = dir_d * dir_diffuse_product;
	
	float dir_s = pow( max(dot(N, H_dir), 0.0), Shininess );
	vec4  dir_specular = dir_s * dir_specular_product;
	if( dot(L_dir, N) < 0.0 ) {
	    dir_specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	vec4 dir_color =1.0 * (dir_ambient + dir_diffuse + dir_specular);

    vec4 color_add_point_light = attenuation * (ambient + diffuse + specular);


    // calculation for spot light
   
	
    vec4 ambient_spot = input_spot_ambient*material_ambient;

    float d_spot = max( dot(L, N), 0.0 );
    vec4  diffuse_spot = d_spot * input_spot_diffuse*material_diffuse;

    float s_spot = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular_spot = s_spot * input_spot_specular*material_specular;

    if( dot(L, N) < 0.0 ) {
	    specular_spot = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    vec4 color_add_spot_light_original = attenuation * (ambient_spot + diffuse_spot + specular_spot);

    vec3 spot_l= -L; // the L is the position P to light source, but we need to calculate from the light source, --> need to negate
    vec3 spot_l_f= normalize(spot_light_dir.xyz- LightPosition.xyz ); // these are all in eye frame and the spot_light_dir is a point
    // and on the description, the direction of the spot lught is from LightPosition to spot_light_dir

    float spot_att=pow(dot(spot_l,spot_l_f),spot_exponent);


	vec4 color_add_spot_light = spot_att* color_add_spot_light_original;

    
    if ( dot(spot_l,spot_l_f) < cos(spot_cut_angle*3.1415926535/180)){ // need to change the number into degree
        color_add_spot_light=vec4(0,0,0,1);
    }



    gl_Position = Projection * ModelView * vPosition;

/*--- attenuation below must be computed properly ---*/

    if(Shading == 1.0 ){
        if (spot_flag == 1.0){

            color = dir_color+ light_ambient_global_ambient *material_ambient+color_add_spot_light;
        }else{
            if  (point_flag == 1.0){
            
                color = dir_color+ light_ambient_global_ambient *material_ambient+color_add_point_light;
            }else{
                color = dir_color+ light_ambient_global_ambient *material_ambient;
            }
        }
    }
    else if  (Shading == 0.0){
        if ( floor_texture == 1) {
            vColor4 = vec4 (0.0, 1.0, 0.0, 1.0);
            

        }
        if (sphere_texture == 1){
            vColor4 = vec4 (1.0, 0.84, 0.0, 1.0);



        }

        if ( blending_shadow == 1.0){
            color = vec4(vColor4.r, vColor4.g, vColor4.b, 0.65); 
        }else{
            color = vColor4;
        }


    }

    if ( floor_texture == 1) {

        texCoord = vTexCoord;
    }

    if (sphere_texture == 1){ // 1D texture
        if (object_space_flag == 1){
            if (vertical_flag == 1) {
                float_texture = pos1 [0]*2.5;
            }
            if (slant_flag ==1){
                float_texture = 1.5 * (pos1[0] + pos1[1] + pos1[2]);
            }
            
        }

        if ( eye_space_flag ==1) {

            
            if (vertical_flag == 1) {
                float_texture = pos [0]*2.5;
            }
            if (slant_flag ==1){
                float_texture = 1.5 * (pos[0] + pos[1] + pos[2]);
            }
        }
    }
    
    else if(sphere_texture == 2){ // checker board
        if (object_space_flag == 1){
            if (vertical_flag == 1) {


                texCoord[0] = (pos1[0] + 1) * 0.5;
                texCoord[1] = (pos1[1] + 1) * 0.5;
                
            }
            if (slant_flag ==1){

                texCoord[0] = (pos1[0] + pos1[1] + pos1[2]) * 0.3;
                texCoord[1] = (pos1[0] - pos1[1] + pos1[2]) * 0.3;
                
            }
            
        }

        if ( eye_space_flag ==1) {

            
            if (vertical_flag == 1) {


                texCoord[0] = (pos[0] + 1) * 0.5;
                texCoord[1] = (pos[1] + 1) * 0.5;
                
            }
            if (slant_flag ==1){

                texCoord[0] = (pos[0] + pos[1] + pos[2]) * 0.3;
                texCoord[1] = (pos[0] - pos[1] + pos[2]) * 0.3;
                
            }
        }
    }

    // calculation of lattices Coordinate
    if (lattice_option == 1){// upright
        // this is in object frame
        lattice_Coord[0] = (pos1[0] + 1) * 0.5;
        lattice_Coord[1] = (pos1[1] + 1) * 0.5;
    }else if (lattice_option == 2){
        // this is in object frame
        lattice_Coord[0] = (pos1[0] + pos1[1] + pos1[2]) * 0.3;
        lattice_Coord[1] = (pos1[0] - pos1[1] + pos1[2]) * 0.3;
    }

}
 