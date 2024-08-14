// --------------------------------------------------------------------------
// Handout: rotate-cube-shading.cpp (Rotating Cube with shading)
//
// * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
//          sample code "example3.cpp" of Chapter 5.
// * Extensively modified by Yi-Jen Chiang for the program structure, 
//   normal matrix, user interface, etc. 
//  (See keyboard() and mouse() functions for user interactions.)
// * Display a rotating cube with shading.
//
// - Light and material properties & Normal Matrix are sent to the shader as 
//   uniform variables.
// - Entire shading computation is done in the Eye Frame (in shader).
// --------------------------------------------------------------------------  
#include "Angel-yjc.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
GLuint program;       /* shader program object id */
GLuint firework_program; /* shader program for firework */

// each one is the object, they are vertex buffer object
GLuint cube_buffer;   /* vertex buffer object id for cube */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;  /* vertex buffer object id for x y z asixes */
GLuint sphere_buffer;  /* vertex buffer object id for sphere */
GLuint shadow_buffer;  /* vertex buffer object id for shadow */

// this is for shading
GLuint cube_buffer_shading;   /* vertex buffer object id for cube */
GLuint floor_shading_buffer;   /* vertex buffer object id for cube */
GLuint sphere_shading_buffer;  /* vertex buffer object id for sphere */
GLuint sphere_flat_shading_buffer;  /* vertex buffer object id for sphere */

// this is for fire_work
GLuint firework_buffer; /* vertex buffer object id for fire work */

// this is for texture
GLuint floor_texture_buffer;


// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 100.0;

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;



// the info for viewer
GLfloat angle = 0.0; // rotation angle in degrees
GLfloat angle_roation_speed = 0.030f; // the default is 0.03f, but i deliberately set the rotation speed to be really slow
//vec4 init_eye(3.0, 2.0, 0.0, 1.0); // initial viewer position
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position


int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'

// switch between solid and wireframe object
int cubeFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int sphere_face_Flag = 1; // 1: 8 faces; 2: 128 faces; 3: 256 faces; 4: 1024 faces
int accumulated_rotation_Flag = 0; // 1: correct; 0: non-correct. Toggled by key 'r' or 'R'
int right_mouse_no_effect = 0; //  0: no-effect; 1: have effect.
int draw_cube = 0; //  0: don't draw; 1: draw.
int filled = 0; //  0: don't fill sphere; 1: fill sphere.
int correct_shadow = 0;  //0: don't correct; 1: correct.
int enable_shadow = 0; // 1: enable shadow; 0: disable shadow. 
int enable_lighting = 0; // 1: enable ; 0: disable. 
int shading = 0; // 0: flat shading; 1: smooth shading.
int light_source = 0; // 0: point loght; 1: spot light; 2: only directional;
int fog = 0; // 0: no fog; 1: linear; 2: exponential; 3: exponential square
float blend_shadow = 0.0; // 0: dont blend; 1: blend
float fire_work = 0.0; // 0: no fire work; 1: fire work with penetrate; 2: firework with change color; 3: firework with discard


float floor_texture = 0.0; // 0: no floor texture; 1: yes floor texture
float sphere_texture = 0.0; // 0: no sphere texture; 1: yes line texture; 2: keyboard

int vertical_flag = 1;
int slant_flag = 0;
int eye_space_flag = 0;
int object_space_flag = 1;

int lattices = 1; // 0: no lattice; 1: lattic up rigtht; 2: latice tilted

int enable_lattices = 0;



//----------------------------------------
// the part to control the sphere animation

// ----------------------------------------------------------------------
// helper function for calculating cross product
point4 cross_product(point4 v, point4 w) {
    // this is v cross w
    point4 result;
    result.x = v.y * w.z - v.z * w.y;
    result.y = v.z * w.x - v.x * w.z;
    result.z = v.x * w.y - v.y * w.x;
    return result;
}

// ----------------------------------------------------------------------
// helper function for normalization
point4 normalized(point4 v) {

    float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    point4 result;
    result.x = v.x / mag;
    result.y = v.y / mag;
    result.z = v.z / mag;
    //printf("\n%f , %f, %f \n", v.x, v.y, v.z);
    return result;
}

//---------------------------- this part is for rolling

point4 A(3.0f, 1.0f, 5.0f, 1.0f);
point4 B(-1.0f, 1.0f, -4.0f, 1.0f);
point4 C(3.5f, 1.0f, -2.5f, 1.0f);

point4 AB = B - A;
point4 BC = C - B;
point4 CA = A - C;

point4 OY(0.0f, 1.0f, 0.0f, 0.0f); // the normal vector


//printf("AB vector %f, %f, %f\n", AB.x, AB.y, AB.z);
//printf("BC vector %f, %f, %f\n", BC.x, BC.y, BC.z);
//printf("CA vector %f, %f, %f\n", CA.x, CA.y, CA.z);

point4 A_to_B_rotation_axis = cross_product(OY, AB);
point4 B_to_C_rotation_axis = cross_product(OY, BC);
point4 C_to_A_rotation_axis = cross_product(OY, CA);

//printf("AB rotate vector %f, %f, %f\n", A_to_B_rotation_axis.x, A_to_B_rotation_axis.y, A_to_B_rotation_axis.z);
//printf("BC rotate vector %f, %f, %f\n", B_to_C_rotation_axis.x, B_to_C_rotation_axis.y, B_to_C_rotation_axis.z);
//printf("CA rotate vector %f, %f, %f\n", C_to_A_rotation_axis.x, C_to_A_rotation_axis.y, C_to_A_rotation_axis.z);




point4 AB_normalized = normalized(AB);
point4 BC_normalized = normalized(BC);
point4 CA_normalized = normalized(CA);


//printf("AB_normalized vector %f, %f, %f\n", AB_normalized.x, AB_normalized.y, AB_normalized.z);
//printf("BC_normalized vector %f, %f, %f\n", BC_normalized.x, BC_normalized.y, BC_normalized.z);
//printf("CA_normalized vector %f, %f, %f\n", CA_normalized.x, CA_normalized.y, CA_normalized.z);


// this is the initial starting point, which is A
point4 starting_point(3.0f, 1.0f, 5.0f, 1.0f);

// this is the initial starting rotation axis, which is A_to_B_rotation_axi
point4 starting_rotation_axis = cross_product(OY, AB);

// this is the initial starting translating direction, which is A-> B
point4 starting_rowing_direction_normalized = normalized(AB);


// this is for accumulated rotation
mat4 Acc_rotationMatrix(1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

// this is the point light for shadow
vec4 light(-14.0f, 12.0f, -3.0f, 1.0f);



mat4 accumulate_rotation(mat4 current, point4 axis, GLfloat angle) {
    // current 
    mat4 result;

    result = Rotate(angle, axis.x, axis.y, axis.z) * current;

    return result;
}


mat4 shadow_projection(vec4 point) {
    float x = point[0];
    float y = point[1];
    float z = point[2];
    float w = point[3];

    return    mat4
    (y, 0.0f, 0.0f, 0.0f,
        -x, 0.0f, -z, -1.0f,
        0.0f, 0.0f, y, 0.0f,
        0.0f, 0.0f, 0.0f, y);

}

//----------------------------------------------------the following is for Shading

/*----- Shader Lighting Parameters -----*/


// In World frame.
// Needs to transform it to Eye Frame
// before sending it to the shader(s).

//-----------------------this is point light
color4 light_ambient_point(0.0f, 0.0f, 0.0f, 1.0f);
color4 light_diffuse_point(1.0f, 1.0f, 1.0f, 1.0f);
color4 light_specular_point(1.0f, 1.0f, 1.0f, 1.0f);

float const_att = 2.0f;
float linear_att = 0.01f;
float quad_att = 0.001f;
point4 light_position_point(-14, 12.0, -3, 1.0);

//----------------------- this is spot light

color4 light_ambient_spot(0, 0, 0, 1.0);
color4 light_diffuse_spot(1.0, 1.0, 1.0, 1.0);
color4 light_specular_spot(1.0, 1.0, 1.0, 1.0);

point4 light_position_spot(-14, 12.0, -3, 1.0);
point4 spot_light_dir(-6.0, 0.0, -4.5, 1.0); // this is a point

float expo = 15.0;
float cut_angle = 20.0;


// this is for cube
color4 cube_material_ambient(0.8f, 0.3f, 0.2f, 1.0f);
color4 cube_material_diffuse(1.0f, 0.8f, 0.0f, 1.0f);
color4 cube_material_specular(1.0f, 0.8f, 0.0f, 1.0f);
float  cube_material_shininess = 4000.0f;


color4 ambient_product_point = light_ambient_point * cube_material_ambient;
color4 diffuse_product_point = light_diffuse_point * cube_material_diffuse;
color4 specular_product_point = light_specular_point * cube_material_specular;

color4 ambient_product_spot_cube_shading = light_ambient_spot * cube_material_ambient;
color4 diffuse_product_spot_cube_shading = light_diffuse_spot * cube_material_diffuse;
color4 specular_product_spot_cube_shading = light_specular_spot * cube_material_specular;


//this is for floor
color4 material_floor_shading_ambient(0.2f, 0.2f, 0.2f, 1.0f);
color4 material_floor_shading_diffuse(0.0f, 1.0f, 0.0f, 1.0f);
color4 material_floor_shading_specular(1.0f, 0.84f, 0.0f, 1.0f);
float  material_floor_shading_shininess = 125.0f;


color4 ambient_product_point_floor_shading = light_ambient_point * material_floor_shading_ambient;
color4 diffuse_product_point_floor_shading = light_diffuse_point * material_floor_shading_diffuse;
color4 specular_product_point_floor_shading = light_specular_point * material_floor_shading_specular;

color4 ambient_product_spot_floor_shading = light_ambient_spot * material_floor_shading_ambient;
color4 diffuse_product_spot_floor_shading = light_diffuse_spot * material_floor_shading_diffuse;
color4 specular_product_spot_floor_shading = light_specular_spot * material_floor_shading_specular;


// this is for sphere
color4 material_sphere_shading_ambient(0.2f, 0.2f, 0.2f, 1.0f);
color4 material_sphere_shading_diffuse(1.0f, 0.84f, 0.0f, 1.0f);
color4 material_sphere_shading_specular(1.0f, 0.84f, 0.0f, 1.0f);
float  material_sphere_shading_shininess = 125.0f;


color4 ambient_product_point_sphere_shading = light_ambient_point * material_sphere_shading_ambient;
color4 diffuse_product_point_sphere_shading = light_diffuse_point * material_sphere_shading_diffuse;
color4 specular_product_point_sphere_shading = light_specular_point * material_sphere_shading_specular;

color4 ambient_product_spot_sphere_shading = light_ambient_spot * material_sphere_shading_ambient;
color4 diffuse_product_spot_sphere_shading = light_diffuse_spot * material_sphere_shading_diffuse;
color4 specular_product_spot_sphere_shading = light_specular_spot * material_sphere_shading_specular;

void SetUp_Lighting_Uniform_Vars(color4 ambient_product, color4 diffuse_product, color4 specular_product,
    color4 input_light_ambient, color4 input_light_diffuse, color4 input_light_specular,
    color4 input_spot_ambient, color4 input_spot_diffuse, color4 input_spot_specular, point4 spot_light_dir, point4 light_position_spot, float spot_flag, float exponenet, float cut_angle,
    color4  material_ambient, color4  material_diffuse, color4 material_specular,
    point4 light_position, float point_flag, float const_att, float linear_att, float quad_att, float material_shininess, mat4 mv);



//---------------------------------------------- the part to read the files and  create triangles, and set all the cooredinate of the object

//------------------------ the following is for sphere
struct Triangles {
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;
    double x3;
    double y3;
    double z3;
};

struct Triangles triangles[1500]; // to store the information of the points
int nums_triangle_in_sphere = 0;
int triangle_count = 0;
int counter = 0; //0 to 2



/*** this is sphere ***/
const int sphere_NumVertices_for_8 = 24; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_128 = 384; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_256 = 768; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_1024 = 3072; //(1 face)*(2 triangles/face)*(3 vertices/triangle)


// points3 datatype --> means the x y z coordinate, three  real number
point4 sphere_points[4000]; // positions for all vertices
color4 sphere_colors[4000]; // colors for all vertices


//this is for shading
const int sphere_NumVertices_for_8_shading = 24; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_128_shading = 384; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_256_shading = 768; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
const int sphere_NumVertices_for_1024_shading = 3072; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 sphere_points_shading[4000]; // positions for all vertices
vec3 sphere_normals_shading[4000]; // colors for all vertices
point4 sphere_points_flat_shading[4000]; // positions for all vertices
vec3 sphere_normals_flat_shading[4000]; // colors for all vertices


//this is for sphere
point4 vertices_sphere[4000];
point4 vertices_sphere_shading[4000];
point4 vertices_sphere_flat_shading[4000];
color4 vertices_sphere_colors[1] = {
    // the x axis color
    color4(1.0, 0.84, 0.0, 1.0)
};




void sphere()
{
    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("the first point: %f %f %f\n", triangles[i].x1, triangles[i].y1, triangles[i].z1);
        //printf("the second point: %f %f %f\n", triangles[i].x2, triangles[i].y2, triangles[i].z2);
        //printf("the third point: %f %f %f\n\n", triangles[i].x3, triangles[i].y3, triangles[i].z3);

        //for storing vertices into sphere vertices

        point4 temp1(triangles[i].x1, triangles[i].y1, triangles[i].z1, 1.0);
        point4 temp2(triangles[i].x2, triangles[i].y2, triangles[i].z2, 1.0);
        point4 temp3(triangles[i].x3, triangles[i].y3, triangles[i].z3, 1.0);

        vertices_sphere[i * 3 + 0] = temp1;
        vertices_sphere[i * 3 + 1] = temp2;
        vertices_sphere[i * 3 + 2] = temp3;

        //printf("the point : % f\n\n", vertices_sphere[i * 3 + 0][1]);
    }

    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        sphere_colors[i * 3 + 0] = vertices_sphere_colors[0]; sphere_points[i * 3 + 0] = vertices_sphere[i * 3 + 0];
        sphere_colors[i * 3 + 1] = vertices_sphere_colors[0]; sphere_points[i * 3 + 1] = vertices_sphere[i * 3 + 1];
        sphere_colors[i * 3 + 2] = vertices_sphere_colors[0]; sphere_points[i * 3 + 2] = vertices_sphere[i * 3 + 2];

    }


}


void sphere_shading()
{
    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("the first point: %f %f %f\n", triangles[i].x1, triangles[i].y1, triangles[i].z1);
        //printf("the second point: %f %f %f\n", triangles[i].x2, triangles[i].y2, triangles[i].z2);
        //printf("the third point: %f %f %f\n\n", triangles[i].x3, triangles[i].y3, triangles[i].z3);

        //for storing vertices into sphere vertices

        point4 temp1(triangles[i].x1, triangles[i].y1, triangles[i].z1, 1.0);
        point4 temp2(triangles[i].x2, triangles[i].y2, triangles[i].z2, 1.0);
        point4 temp3(triangles[i].x3, triangles[i].y3, triangles[i].z3, 1.0);

        vertices_sphere_shading[i * 3 + 0] = temp1;
        vertices_sphere_shading[i * 3 + 1] = temp2;
        vertices_sphere_shading[i * 3 + 2] = temp3;



        //printf("the point : % f\n\n", vertices_sphere[i * 3 + 0][1]);
    }

    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("\n");
        //printf("the first point: %f %f %f\n", vertices_sphere_shading[i * 3 + 0][0], vertices_sphere_shading[i * 3 + 0][1], vertices_sphere_shading[i * 3 + 0][2]);
        //printf("the second point: %f %f %f\n", vertices_sphere_shading[i * 3 + 1][0], vertices_sphere_shading[i * 3 + 1][1], vertices_sphere_shading[i * 3 + 1][2]);
        //printf("the third point: %f %f %f\n\n", vertices_sphere_shading[i * 3 + 2][0], vertices_sphere_shading[i * 3 + 2][1], vertices_sphere_shading[i * 3 + 2][2]);

        vec4 u = vertices_sphere_shading[i * 3 + 1] - vertices_sphere_shading[i * 3 + 0];
        vec4 v = vertices_sphere_shading[i * 3 + 2] - vertices_sphere_shading[i * 3 + 0];

        vec3 normal = normalize(cross(u, v));
        //printf("the normal: %f %f %f\n\n", normal[0], normal[1], normal[2]);

        sphere_normals_shading[i * 3 + 0] = normal; sphere_points_shading[i * 3 + 0] = vertices_sphere_shading[i * 3 + 0];
        sphere_normals_shading[i * 3 + 1] = normal; sphere_points_shading[i * 3 + 1] = vertices_sphere_shading[i * 3 + 1];
        sphere_normals_shading[i * 3 + 2] = normal; sphere_points_shading[i * 3 + 2] = vertices_sphere_shading[i * 3 + 2];

    }


}


void sphere_flat_shading()
{
    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("the first point: %f %f %f\n", triangles[i].x1, triangles[i].y1, triangles[i].z1);
        //printf("the second point: %f %f %f\n", triangles[i].x2, triangles[i].y2, triangles[i].z2);
        //printf("the third point: %f %f %f\n\n", triangles[i].x3, triangles[i].y3, triangles[i].z3);

        //for storing vertices into sphere vertices

        point4 temp1(triangles[i].x1, triangles[i].y1, triangles[i].z1, 1.0);
        point4 temp2(triangles[i].x2, triangles[i].y2, triangles[i].z2, 1.0);
        point4 temp3(triangles[i].x3, triangles[i].y3, triangles[i].z3, 1.0);

        vertices_sphere_flat_shading[i * 3 + 0] = temp1;
        vertices_sphere_flat_shading[i * 3 + 1] = temp2;
        vertices_sphere_flat_shading[i * 3 + 2] = temp3;



        //printf("the point : % f\n\n", vertices_sphere[i * 3 + 0][1]);
    }

    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("\n");
        //printf("the first point: %f %f %f\n", vertices_sphere_flat_shading[i * 3 + 0][0], vertices_sphere_flat_shading[i * 3 + 0][1], vertices_sphere_flat_shading[i * 3 + 0][2]);
        //printf("the second point: %f %f %f\n", vertices_sphere_flat_shading[i * 3 + 1][0], vertices_sphere_flat_shading[i * 3 + 1][1], vertices_sphere_flat_shading[i * 3 + 1][2]);
        //printf("the third point: %f %f %f\n\n", vertices_sphere_flat_shading[i * 3 + 2][0], vertices_sphere_flat_shading[i * 3 + 2][1], vertices_sphere_flat_shading[i * 3 + 2][2]);


        vec3 normal1 = normalize(vec3(vertices_sphere_flat_shading[i * 3 + 0][0], vertices_sphere_flat_shading[i * 3 + 0][1], vertices_sphere_flat_shading[i * 3 + 0][2]) - (0.0, 0.0, 0.0));
        vec3 normal2 = normalize(vec3(vertices_sphere_flat_shading[i * 3 + 1][0], vertices_sphere_flat_shading[i * 3 + 1][1], vertices_sphere_flat_shading[i * 3 + 1][2]) - (0.0, 0.0, 0.0));
        vec3 normal3 = normalize(vec3(vertices_sphere_flat_shading[i * 3 + 2][0], vertices_sphere_flat_shading[i * 3 + 2][1], vertices_sphere_flat_shading[i * 3 + 2][2]) - (0.0, 0.0, 0.0));


        sphere_normals_flat_shading[i * 3 + 0] = normal1; sphere_points_flat_shading[i * 3 + 0] = vertices_sphere_flat_shading[i * 3 + 0];
        sphere_normals_flat_shading[i * 3 + 1] = normal2; sphere_points_flat_shading[i * 3 + 1] = vertices_sphere_flat_shading[i * 3 + 1];
        sphere_normals_flat_shading[i * 3 + 2] = normal3; sphere_points_flat_shading[i * 3 + 2] = vertices_sphere_flat_shading[i * 3 + 2];

    }


}

//-----------------------------------the following part is for floor and shadow
//-----------------------------------floor

/*** this is floor ***/
const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
// points3 datatype --> means the x y z coordinate, three  real number
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices

// this is for floor shading
const int NumVertices_floor_shading = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points_floor_shading[NumVertices_floor_shading];
vec3   normals_floor_shading[NumVertices_floor_shading];



// Vertices of the points used for the floor
point4 vertices_floor[4] = {
    point4(5.0, 0.0, 8.0, 1.0),
    point4(5.0, 0.0, -4.0, 1.0),
    point4(-5.0, 0.0, -4.0, 1.0),
    point4(-5.0, 0.0, 8.0, 1.0)

};
color4 vertex_floor_colors[1] = {
    color4(0.0, 1.0, 0.0, 1.0) // green

};

// Vertices of the points used for the floor shaading
point4 vertices_floor_shading[8] = {
    point4(5.0, 0.0, 8.0, 1.0),
    point4(5.0, 0.0, -4.0, 1.0),
    point4(-5.0, 0.0, -4.0, 1.0),
    point4(-5.0, 0.0, 8.0, 1.0)
};

// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    // the vertice it used is the 0 3 4 7 plane
    // use the counter clock-wise direction
    floor_colors[0] = vertex_floor_colors[0]; floor_points[0] = vertices_floor[0];
    floor_colors[1] = vertex_floor_colors[0]; floor_points[1] = vertices_floor[1];
    floor_colors[2] = vertex_floor_colors[0]; floor_points[2] = vertices_floor[2];

    floor_colors[3] = vertex_floor_colors[0]; floor_points[3] = vertices_floor[0];
    floor_colors[4] = vertex_floor_colors[0]; floor_points[4] = vertices_floor[2];
    floor_colors[5] = vertex_floor_colors[0]; floor_points[5] = vertices_floor[3];
}

void floor_shader()
{


    vec4 u = vertices_floor_shading[0] - vertices_floor_shading[1];
    vec4 v = vertices_floor_shading[2] - vertices_floor_shading[1];

    vec3 normal = normalize(cross(u, v));

    normals_floor_shading[0] = normal; points_floor_shading[0] = vertices_floor_shading[0];
    normals_floor_shading[1] = normal; points_floor_shading[1] = vertices_floor_shading[1];
    normals_floor_shading[2] = normal; points_floor_shading[2] = vertices_floor_shading[2];

    normals_floor_shading[3] = normal; points_floor_shading[3] = vertices_floor_shading[0];
    normals_floor_shading[4] = normal; points_floor_shading[4] = vertices_floor_shading[2];
    normals_floor_shading[5] = normal; points_floor_shading[5] = vertices_floor_shading[3];


}

//---------------------------------------shadow

point4 shadow_points[4000]; // positions for all vertices
color4 shadow_colors[4000]; // colors for all vertices



//this is for shadow of the sphere
point4 vertices_shadow[4000];
color4 vertices_shadow_colors[1] = {
    // the x axis color
    color4(0.25, 0.25, 0.25, 0.65)
};


void shadow()
{
    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        //printf("the first point: %f %f %f\n", triangles[i].x1, triangles[i].y1, triangles[i].z1);
        //printf("the second point: %f %f %f\n", triangles[i].x2, triangles[i].y2, triangles[i].z2);
        //printf("the third point: %f %f %f\n\n", triangles[i].x3, triangles[i].y3, triangles[i].z3);

        //for storing vertices into sphere vertices


        point4 temp1(triangles[i].x1, triangles[i].y1, triangles[i].z1, 1.0);
        point4 temp2(triangles[i].x2, triangles[i].y2, triangles[i].z2, 1.0);
        point4 temp3(triangles[i].x3, triangles[i].y3, triangles[i].z3, 1.0);

        vertices_shadow[i * 3 + 0] = temp1;
        vertices_shadow[i * 3 + 1] = temp2;
        vertices_shadow[i * 3 + 2] = temp3;

        //printf("the point : % f\n\n", vertices_sphere[i * 3 + 0][1]);
    }

    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        shadow_colors[i * 3 + 0] = vertices_shadow_colors[0]; shadow_points[i * 3 + 0] = vertices_shadow[i * 3 + 0];
        shadow_colors[i * 3 + 1] = vertices_shadow_colors[0]; shadow_points[i * 3 + 1] = vertices_shadow[i * 3 + 1];
        shadow_colors[i * 3 + 2] = vertices_shadow_colors[0]; shadow_points[i * 3 + 2] = vertices_shadow[i * 3 + 2];
        //printf("the shadow color: %f %f %f %f\n\n", shadow_colors[i * 3 + 2][0], shadow_colors[i * 3 + 2][1], shadow_colors[i * 3 + 2][2], shadow_colors[i * 3 + 2][3]);

    }


}

// -------------------------------------------------the following is for axis

/*** this is axises ***/
const int axis_NumVertices = 9; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
// points3 datatype --> means the x y z coordinate, three  real number
point4 axis_points[axis_NumVertices]; // positions for all vertices
color4 axis_colors[axis_NumVertices]; // colors for all vertices


// this is for axises
point4 vertices_axis[9] = {
    // the x axis
    point4(10.0, 0.0, 0.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0),

    // the y axis
    point4(0.0, 10.0, 0.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0),

    // the z axis
    point4(0.0, 0.0, 10.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0),
    point4(0.0, 0.0, 0.0, 1.0)


};

color4 vertices_axis_colors[3] = {
    // the x axis color
    color4(1.0, 0.0, 0.0, 1.0),

    // the y axis color
    color4(1.0, 0.0, 1.0, 1.0),

    // the z axis color
    color4(0.0, 0.0, 1.0, 1.0)
};


void axis()
{
    // the x axis
    axis_colors[0] = vertices_axis_colors[0]; axis_points[0] = vertices_axis[0];
    axis_colors[1] = vertices_axis_colors[0]; axis_points[1] = vertices_axis[1];
    axis_colors[2] = vertices_axis_colors[0]; axis_points[2] = vertices_axis[2];

    // the y axis
    axis_colors[3] = vertices_axis_colors[1]; axis_points[3] = vertices_axis[3];
    axis_colors[4] = vertices_axis_colors[1]; axis_points[4] = vertices_axis[4];
    axis_colors[5] = vertices_axis_colors[1]; axis_points[5] = vertices_axis[5];

    // the z axis
    axis_colors[6] = vertices_axis_colors[2]; axis_points[6] = vertices_axis[6];
    axis_colors[7] = vertices_axis_colors[2]; axis_points[7] = vertices_axis[7];
    axis_colors[8] = vertices_axis_colors[2]; axis_points[8] = vertices_axis[8];
}


// ------------------------------------------------the following is for color cube

/*** this is color cube***/
const int cube_NumVertices = 36; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)
#if 0
point3 cube_points[cube_NumVertices]; // positions for all vertices
color3 cube_colors[cube_NumVertices]; // colors for all vertices
#endif
#if 1
point4 cube_points[100];
color4 cube_colors[100];
#endif

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
};
// RGBA colors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 1.0, 1.0, 1.0),  // white
    color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

// this is for color cube

int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// quad(): generate two triangles for each face and assign colors to the vertices
void quad(int a, int b, int c, int d)
{
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
    cube_colors[Index] = vertex_colors[b]; cube_points[Index] = vertices[b]; Index++;
    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;

    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;
    cube_colors[Index] = vertex_colors[d]; cube_points[Index] = vertices[d]; Index++;
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}


//---------------------------------------------------the following is for shading cube

/*** this is shading cube***/
const int NumVertices_shading = 36; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)
point4 points_shading[NumVertices_shading];
vec3   normals_shading[NumVertices_shading];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices_shading[8] = {
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
};

int Index_shading = 0;


// quad() generates two triangles for each face and assigns normals
//        to the vertices
void quad_shading(int a, int b, int c, int d)
{
    // Initialize temporary vectors along the quad's edges to
    //   compute its face normal 
    vec4 u = vertices_shading[b] - vertices_shading[a];
    vec4 v = vertices_shading[d] - vertices_shading[a];

    vec3 normal = normalize(cross(u, v));

    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[a]; Index_shading++;
    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[b]; Index_shading++;
    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[c]; Index_shading++;
    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[a]; Index_shading++;
    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[c]; Index_shading++;
    normals_shading[Index_shading] = normal; points_shading[Index_shading] = vertices_shading[d]; Index_shading++;
}

// colorcube() generates 6 quad faces (12 triangles): 36 vertices & 36 normals
void colorcube_shading()
{
    quad_shading(1, 0, 3, 2);
    quad_shading(2, 3, 7, 6);
    quad_shading(3, 0, 4, 7);
    quad_shading(6, 5, 1, 2);
    quad_shading(4, 5, 6, 7);
    quad_shading(5, 4, 0, 1);
}



//-----------------------------------------------------the following part is for firework

float t = 0.0;

const int firework_NumVertices = 300;
point4 firework_points[firework_NumVertices];
color4 firework_colors[firework_NumVertices];

void firework()
{



    for (int i = 0; i < 300; i++)
    {
        vec4 temp_v = vec4( 2.0 * ((rand() % 256) / 256.0 - 0.5), 
                            2.0 * 1.2 * (rand() % 256) / 256.0, 
                            2.0 * ((rand() % 256) / 256.0 - 0.5), 
                            1.0);


        firework_points[i] = temp_v;
        vec4 temp_c = vec4( (rand() % 256) / 256.0, 
                            (rand() % 256) / 256.0, 
                            (rand() % 256) / 256.0, 
                            1.0);

        firework_colors[i] = temp_c;
    }
}

// ------------------------------------the following is for texture

static GLuint texName;
static GLuint line_texName;

/*--- Quad arrays: 6 vertices of 2 triangles, for the quad (a b c d).
      Triangles are abc, cda. --*/
point4 floor_texture_vertices[6] = {

    point4(-5.0,  0,  -4.0, 1.0),
    point4(-5.0,  0,  8.0, 1.0),
    point4(5.0, 0,  8.0, 1.0),

    point4(5.0, 0,  8.0, 1.0),
    point4(5.0, 0,  -4.0, 1.0),
    point4(-5,  0,  -4.0, 1.0),
};

vec3 floor_texture_normals[6] = {
    vec3(0.0,  1.0,  0.0),
    vec3(0.0,  1.0,  0.0),
    vec3(0.0,  1.0,  0.0),
    vec3(0.0,  1.0,  0.0),
    vec3(0.0,  1.0,  0.0),
    vec3(0.0,  1.0,  0.0),
};

vec2 floor_texture_coord[6] = {
    vec2(0.0, 0.0), //a
    vec2(0.0, 1.5), //b
    vec2(1.25, 1.5),//c

    vec2(1.25, 1.5),//c
    vec2(1.25, 0.0),//d
    vec2(0.0, 0.0), //a
};
// here, thit just means that the length is the texture map's 1.5 times --> repear by 1.5 times 

// the following code is from the cource website


/* global definitions for constants and global image arrays */

#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

/*************************************************************
void image_set_up(void):
  generate checkerboard and stripe images.

* Inside init(), call this function and set up texture objects
  for texture mapping.
  (init() is called from main() before calling glutMainLoop().)
***************************************************************/
void image_set_up(void)
{
    int i, j, c;

    /* --- Generate checkerboard image to the image array ---*/
    for (i = 0; i < ImageHeight; i++)
        for (j = 0; j < ImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

            if (c == 1) /* white */
            {
                c = 255;
                Image[i][j][0] = (GLubyte)c;
                Image[i][j][1] = (GLubyte)c;
                Image[i][j][2] = (GLubyte)c;
            }
            else  /* green */
            {
                Image[i][j][0] = (GLubyte)0;
                Image[i][j][1] = (GLubyte)150;
                Image[i][j][2] = (GLubyte)0;
            }

            Image[i][j][3] = (GLubyte)255;
        }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Generate 1D stripe image to array stripeImage[] ---*/
    for (j = 0; j < stripeImageWidth; j++) {
        /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
           When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
         */
        stripeImage[4 * j] = (GLubyte)255;
        stripeImage[4 * j + 1] = (GLubyte)((j > 4) ? 255 : 0);
        stripeImage[4 * j + 2] = (GLubyte)0;
        stripeImage[4 * j + 3] = (GLubyte)255;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    /*----------- End 1D stripe image ----------------*/

    /*--- texture mapping set-up is to be done in
          init() (set up texture objects),
          display() (activate the texture object to be used, etc.)
          and in shaders.
     ---*/

} /* end function */


void init_floor_texture() {
    image_set_up();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &texName);      // Generate texture obj name(s)

    glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
    glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

    /** Note: If using multiple textures, repeat the above process starting from
              glActiveTexture(), but each time use a *different texture unit*,
              so that each texture is bound to a *different texture unit*.    **/

              /*--- Create and initialize vertex buffer object for quad ---*/

    glGenBuffers(1, &floor_texture_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_texture_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_texture_vertices) + sizeof(floor_texture_normals) + sizeof(floor_texture_coord),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_texture_vertices), floor_texture_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_texture_vertices), sizeof(floor_texture_normals), floor_texture_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_texture_vertices) + sizeof(floor_texture_normals),
        sizeof(floor_texture_coord), floor_texture_coord);

}

void init_ID_texture()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &line_texName);      // Generate texture obj name(s)

    glActiveTexture(GL_TEXTURE1);  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_1D, line_texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 32,
        0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
}




void init()
{
  
    //-----------------------------the following  is for sphere

    sphere();

    if (sphere_face_Flag == 1) {

        glGenBuffers(1, &sphere_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8 + sizeof(color4) * sphere_NumVertices_for_8,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_8, sphere_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8,
            sizeof(color4) * sphere_NumVertices_for_8,
            sphere_colors);
    }

    else if (sphere_face_Flag == 2) {

        glGenBuffers(1, &sphere_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128 + sizeof(color4) * sphere_NumVertices_for_128,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_128, sphere_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128,
            sizeof(color4) * sphere_NumVertices_for_128,
            sphere_colors);
    }

    else if (sphere_face_Flag == 3) {

        glGenBuffers(1, &sphere_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256 + sizeof(color4) * sphere_NumVertices_for_256,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_256, sphere_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256,
            sizeof(color4) * sphere_NumVertices_for_256,
            sphere_colors);
    }


    else if (sphere_face_Flag == 4) {

        glGenBuffers(1, &sphere_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024 + sizeof(color4) * sphere_NumVertices_for_1024,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_1024, sphere_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024,
            sizeof(color4) * sphere_NumVertices_for_1024,
            sphere_colors);
    }


    sphere_shading();

    if (sphere_face_Flag == 1) {

        glGenBuffers(1, &sphere_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8_shading + sizeof(vec3) * sphere_NumVertices_for_8_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_8_shading, sphere_points_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8_shading,
            sizeof(vec3) * sphere_NumVertices_for_8_shading,
            sphere_normals_shading);



    }
    else if (sphere_face_Flag == 2) {

        glGenBuffers(1, &sphere_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128_shading + sizeof(vec3) * sphere_NumVertices_for_128_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_128_shading, sphere_points_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128_shading,
            sizeof(vec3) * sphere_NumVertices_for_128_shading,
            sphere_normals_shading);



    }
    else if (sphere_face_Flag == 3) {

        glGenBuffers(1, &sphere_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256_shading + sizeof(vec3) * sphere_NumVertices_for_256_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_256_shading, sphere_points_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256_shading,
            sizeof(vec3) * sphere_NumVertices_for_256_shading,
            sphere_normals_shading);
    }
    else if (sphere_face_Flag == 4) {

        glGenBuffers(1, &sphere_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024_shading + sizeof(vec3) * sphere_NumVertices_for_1024_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_1024_shading, sphere_points_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024_shading,
            sizeof(vec3) * sphere_NumVertices_for_1024_shading,
            sphere_normals_shading);
    }

    sphere_flat_shading();

    if (sphere_face_Flag == 1) {

        glGenBuffers(1, &sphere_flat_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8_shading + sizeof(vec3) * sphere_NumVertices_for_8_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_8_shading, sphere_points_flat_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8_shading,
            sizeof(vec3) * sphere_NumVertices_for_8_shading,
            sphere_normals_flat_shading);



    }
    else if (sphere_face_Flag == 2) {

        glGenBuffers(1, &sphere_flat_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128_shading + sizeof(vec3) * sphere_NumVertices_for_128_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_128_shading, sphere_points_flat_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128_shading,
            sizeof(vec3) * sphere_NumVertices_for_128_shading,
            sphere_normals_flat_shading);



    }
    else if (sphere_face_Flag == 3) {

        glGenBuffers(1, &sphere_flat_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256_shading + sizeof(vec3) * sphere_NumVertices_for_256_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_256_shading, sphere_points_flat_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256_shading,
            sizeof(vec3) * sphere_NumVertices_for_256_shading,
            sphere_normals_flat_shading);
    }
    else if (sphere_face_Flag == 4) {

        glGenBuffers(1, &sphere_flat_shading_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_shading_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024_shading + sizeof(vec3) * sphere_NumVertices_for_1024_shading,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_1024_shading, sphere_points_flat_shading);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024_shading,
            sizeof(vec3) * sphere_NumVertices_for_1024_shading,
            sphere_normals_flat_shading);
    }


    //--------------------------------------the following is for floor and shadow
    //--------------------------------------floor
    floor();
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer); // to generate the name of the object buffer. generate 1 name, and the floor_buffer --> the ID of the buffer of the floor
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    //starting point // size of the data // the data given stored
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
        floor_colors);

    floor_shader();

    // Create and initialize a vertex buffer object
    glGenBuffers(1, &floor_shading_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_shading_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_floor_shading) + sizeof(normals_floor_shading),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_floor_shading), points_floor_shading);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_floor_shading),
        sizeof(normals_floor_shading), normals_floor_shading);



    //----------------------------shadow
    shadow();

    if (sphere_face_Flag == 1) {

        glGenBuffers(1, &shadow_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8 + sizeof(color4) * sphere_NumVertices_for_8,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_8, shadow_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_8,
            sizeof(color4) * sphere_NumVertices_for_8,
            shadow_colors);
    }

    else if (sphere_face_Flag == 2) {

        glGenBuffers(1, &shadow_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128 + sizeof(color4) * sphere_NumVertices_for_128,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_128, shadow_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_128,
            sizeof(color4) * sphere_NumVertices_for_128,
            shadow_colors);
    }

    else if (sphere_face_Flag == 3) {

        glGenBuffers(1, &shadow_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256 + sizeof(color4) * sphere_NumVertices_for_256,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_256, shadow_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_256,
            sizeof(color4) * sphere_NumVertices_for_256,
            shadow_colors);
    }


    else if (sphere_face_Flag == 4) {

        glGenBuffers(1, &shadow_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024 + sizeof(color4) * sphere_NumVertices_for_1024,
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(point4) * sphere_NumVertices_for_1024, shadow_points);
        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(point4) * sphere_NumVertices_for_1024,
            sizeof(color4) * sphere_NumVertices_for_1024,
            shadow_colors);
    }


    //------------------------------------------the following part is for axis
    axis();
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &axis_buffer); // to generate the name of the object buffer. generate 1 name, and the floor_buffer --> the ID of the buffer of the floor
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    //starting point // size of the data // the data given stored
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
        axis_colors);


    //---------------------------------------------the following is for color cube
    colorcube();

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#endif

    // Create and initialize a vertex buffer object for cube, to be used in display()
    glGenBuffers(1, &cube_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);

#if 0
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(cube_colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_colors),
        cube_colors);
#endif
#if 1 // this is the different way when setting the number to be 100
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(point4) * cube_NumVertices + sizeof(color4) * cube_NumVertices,
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        sizeof(point4) * cube_NumVertices, cube_points);
    glBufferSubData(GL_ARRAY_BUFFER,
        sizeof(point4) * cube_NumVertices,
        sizeof(color4) * cube_NumVertices,
        cube_colors);
#endif


    //-------------------------------------------------this is for shading cube

    colorcube_shading();

    // Create and initialize a vertex buffer object
    glGenBuffers(1, &cube_buffer_shading);
    glBindBuffer(GL_ARRAY_BUFFER, cube_buffer_shading);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_shading) + sizeof(normals_shading),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_shading), points_shading);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_shading),
        sizeof(normals_shading), normals_shading);


    //-------------------------------------------------this is for fire work

    firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firework_points) + sizeof(firework_colors),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(firework_points), firework_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(firework_points),
        sizeof(firework_colors), firework_colors);


    //------------------------------------------------------this is for texture for floor

    init_floor_texture();

    //------------------------------------------------------this is texture for sphere
    init_ID_texture();
    



    // Load shaders and create a shader program (to be used in display())
    program = InitShader( "vshader53.glsl", "fshader53.glsl" );


    // Load shaders and create a shader program (to be used in display()) ------> this one is for the fire work
    firework_program = InitShader("firework_vshader53.glsl", "firework_fshader53.glsl");

    glEnable(GL_DEPTH_TEST);
    //glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
    glClearColor(0.529f, 0.807f, 0.92f, 0.0f); // Sky blue background color

    glLineWidth(1.2);
}
void SetUp_Lighting_Uniform_Vars(color4 ambient_product, color4 diffuse_product, color4 specular_product,
    color4 input_light_ambient, color4 input_light_diffuse, color4 input_light_specular,
    color4 input_spot_ambient, color4 input_spot_diffuse, color4 input_spot_specular, point4 spot_light_dir, point4 light_position_spot, float spot_flag, float exponenet, float cut_angle,
    color4  material_ambient, color4  material_diffuse, color4 material_specular,
    point4 light_position, float point_flag, float const_att, float linear_att, float quad_att, float material_shininess, mat4 mv)
{
    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
        1, ambient_product);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
        1, diffuse_product);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
        1, specular_product);



    // The Light Position in Eye Frame
    vec4 light_position_eyeFrame = mv * light_position;
    glUniform4fv(glGetUniformLocation(program, "LightPosition"),
        1, light_position_eyeFrame);

    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
        const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
        linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
        quad_att);

    glUniform4fv(glGetUniformLocation(program, "input_light_ambient"),
        1, input_light_ambient);

    glUniform4fv(glGetUniformLocation(program, "input_light_diffuse"),
        1, input_light_diffuse);

    glUniform4fv(glGetUniformLocation(program, "input_light_specular"),
        1, input_light_specular);

    glUniform1f(glGetUniformLocation(program, "point_flag"),
        point_flag);

    glUniform4fv(glGetUniformLocation(program, "input_spot_ambient"),
        1, input_spot_ambient);

    glUniform4fv(glGetUniformLocation(program, "input_spot_diffuse"),
        1, input_spot_diffuse);

    glUniform4fv(glGetUniformLocation(program, "input_spot_specular"),
        1, input_spot_specular);

    vec4 spot_dir_eye = mv* spot_light_dir;
    glUniform4fv(glGetUniformLocation(program, "spot_light_dir"),
        1, spot_dir_eye);

    vec4 spot_pos_eye = mv * light_position_spot;
    glUniform4fv(glGetUniformLocation(program, "light_position_spot"),
        1, spot_pos_eye);

    glUniform1f(glGetUniformLocation(program, "spot_flag"),
        spot_flag);

    glUniform1f(glGetUniformLocation(program, "spot_exponent"),
        exponenet);

    glUniform1f(glGetUniformLocation(program, "spot_cut_angle"),
        cut_angle);

    glUniform4fv(glGetUniformLocation(program, "material_ambient"),
        1, material_ambient);

    glUniform4fv(glGetUniformLocation(program, "material_diffuse"),
        1, material_diffuse);

    glUniform4fv(glGetUniformLocation(program, "material_specular"),
        1, material_specular);

    glUniform1f(glGetUniformLocation(program, "Shininess"),
        material_shininess);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, float shading_option)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);


    glUniform1f(glGetUniformLocation(program, "Shading"),
        shading_option);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(color4) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point4) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vNormal);
}

//----------------------------------------this draw object is for blending shadow, or else the original drawObject will be too much
void drawObj_shadow(GLuint buffer, int num_vertices, float shading_option, float blending_shadow_flag)
{
   
    glBindBuffer(GL_ARRAY_BUFFER, buffer);


    glUniform1f(glGetUniformLocation(program, "Shading"),
        shading_option);

    glUniform1f(glGetUniformLocation(program, "blending_shadow"),
        blending_shadow_flag);

  
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(color4) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(point4) * num_vertices));
   
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

   
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vNormal);
}


//----------------------------------------this draw object is for drawing firework, or else the original drawObject will be too much
void drawObj_firework(GLuint buffer, int num_vertices, float penetration_flag)
{

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glUniform1f(glGetUniformLocation(firework_program, "penetration_flag"),
        penetration_flag);

    GLuint vPosition = glGetAttribLocation(firework_program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(firework_program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(sizeof(color4) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    GLuint velocity = glGetAttribLocation(firework_program, "velocity");
    glEnableVertexAttribArray(velocity);
    glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);


    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(velocity);

}


//----------------------------------------the following drawObject is for drawing texture 
void drawObj_texture(GLuint buffer, int num_vertices, float shading_option)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);


    glUniform1f(glGetUniformLocation(program, "Shading"),
        shading_option);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(0));

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(num_vertices * sizeof(point4)));

 

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(num_vertices * (sizeof(vec3) + sizeof(point4))));



    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);


    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);

    glDisableVertexAttribArray(vNormal);
    glDisableVertexAttribArray(vTexCoord);
}



void display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//** Important: glUseProgram() must be called *before* any shader variable
//              locations can be retrieved. This is needed to pass on values to
//              uniform/attribute variables in shader ("variable binding" in 
//              shader).
    glUseProgram( program );
	 
    // Retrieve transformation uniform variable locations
    // ** Must be called *after* glUseProgram().
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

   /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

    // Generate the model-view matrix

	  vec4 at(0.0, 0.0, 0.0, 1.0);
          vec4 up(0.0, 1.0, 0.0, 0.0);
          mat4 mv = LookAt(eye, at, up); // model-view matrix using Correct LookAt()
               // model-view matrix for the light position.


          //---------------------------this part is for flog

          glUniform1f(glGetUniformLocation(program, "fog_flag"), fog * 1.0);




          //------------------------the following is for sphere

          float d = angle * (2 * M_PI * 1.0f / 360.0f);



          // get the current location of the center of the sphere

          point4 sphere_center(0.0f, 0.0f, 0.0f, 0.0f); // the sphere is always at the origin
          point4 transformed_center;
          transformed_center.x += starting_point.x + d * starting_rowing_direction_normalized.x;
          transformed_center.y += starting_point.y + d * starting_rowing_direction_normalized.y;
          transformed_center.z += starting_point.z + d * starting_rowing_direction_normalized.z;

          //printf("current location %f, %f, %f\n", transformed_center.x, transformed_center.y, transformed_center.z);


          // if right now i am at or passing A(3, 1, 5):
          //     i will go to B
          // if right now i am at or passing B(-1, 1, 4):
          //     i will go to C
          // if right now i am at or passing C(3.5, 1, -2.5):
          //     i will go to A



          if (transformed_center.x <= -1 && transformed_center.z <= -4) {
              //printf("I pass B\n");
              //start point change to B and i go to C

              starting_point = B;

              //printf(" %f, %f, %f \n", starting_point.x, starting_point.y, starting_point.z);
              starting_rowing_direction_normalized = normalized(BC);
              starting_rotation_axis = cross_product(OY, BC);

              if (accumulated_rotation_Flag == 1) {
                  Acc_rotationMatrix = accumulate_rotation(Acc_rotationMatrix, cross_product(OY, AB), angle);
              }
              //printf("current angle: %f\n", angle);
              angle = 0;


          }
          else if (transformed_center.x <= 3 && transformed_center.z >= 5) {
              //printf("I pass A\n");
              //start point change to B and i go to C

              starting_point = A;
              starting_rowing_direction_normalized = normalized(AB);
              starting_rotation_axis = cross_product(OY, AB);

              if (accumulated_rotation_Flag == 1) {

                  Acc_rotationMatrix = accumulate_rotation(Acc_rotationMatrix, cross_product(OY, CA), angle);
              }
              //printf("current angle: %f\n", angle);
              angle = 0;

          }
          else if (transformed_center.x >= 3.5 && transformed_center.z >= -2.5) {
              //printf("I pass C\n");
              //start point change to B and i go to C

              starting_point = C;
              starting_rowing_direction_normalized = normalized(CA);
              starting_rotation_axis = cross_product(OY, CA);

              if (accumulated_rotation_Flag == 1) {
                  Acc_rotationMatrix = accumulate_rotation(Acc_rotationMatrix, cross_product(OY, BC), angle);
              }
              //printf("current angle: %f\n", angle);
              angle = 0;

          }



          if (light_source == 0) { // point
              SetUp_Lighting_Uniform_Vars(ambient_product_point_sphere_shading, diffuse_product_point_sphere_shading, specular_product_point_sphere_shading,
                  light_ambient_point, light_diffuse_point, light_specular_point,
                  light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                  material_sphere_shading_ambient, material_sphere_shading_diffuse, material_sphere_shading_specular,
                  light_position_point, 1.0, const_att, linear_att, quad_att, material_sphere_shading_shininess, mv);
          }
          else  if (light_source == 1) { // spot 

              SetUp_Lighting_Uniform_Vars(ambient_product_point_sphere_shading, diffuse_product_point_sphere_shading, specular_product_point_sphere_shading,
                  light_ambient_point, light_diffuse_point, light_specular_point,
                  light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 1.0, expo, cut_angle,
                  material_sphere_shading_ambient, material_sphere_shading_diffuse, material_sphere_shading_specular,
                  light_position_point, 0.0, const_att, linear_att, quad_att, material_sphere_shading_shininess, mv);

          }
          else  if (light_source == 2) { // dir

              SetUp_Lighting_Uniform_Vars(ambient_product_point_sphere_shading, diffuse_product_point_sphere_shading, specular_product_point_sphere_shading,
                  light_ambient_point, light_diffuse_point, light_specular_point,
                  light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                  material_sphere_shading_ambient, material_sphere_shading_diffuse, material_sphere_shading_specular,
                  light_position_point, 0.0, const_att, linear_att, quad_att, material_sphere_shading_shininess, mv);

          }


    // The model-view matrix with all transformations for the cube


    mat4 model_view = LookAt(eye, at, up) * Translate(starting_point.x + d * starting_rowing_direction_normalized.x,
        starting_point.y + d * starting_rowing_direction_normalized.y,
        starting_point.z + d * starting_rowing_direction_normalized.z)
        * Rotate(angle, starting_rotation_axis.x, starting_rotation_axis.y, starting_rotation_axis.z)  *Acc_rotationMatrix;



    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view );

    // Set up the Normal Matrix from the model-view matrix
    mat3 normal_matrix = NormalMatrix(model_view, 1);
         // Flag in NormalMatrix(): 
         //    1: model_view involves non-uniform scaling
	 //    0: otherwise.
	 // Using 1 is always correct.  
	 // But if no non-uniform scaling, 
         //     using 0 is faster (avoids matrix inverse computation).

    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 
		       1, GL_TRUE, normal_matrix );

    if (filled == 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (filled == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (sphere_face_Flag == 1) {

        if (sphere_texture == 1) {



            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 1);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

        }
        else if (sphere_texture == 2)
        {
            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 2);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
        else {


            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }

            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_8, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_8, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }

    }
    else if (sphere_face_Flag == 2) {

        if (sphere_texture == 1) {



            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 1);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

        }
        else if (sphere_texture == 2)
        {
            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 2);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
        else {


            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }

            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_128, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_128, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
    }
    else if (sphere_face_Flag == 3) {


        if (sphere_texture == 1) {



            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 1);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

        }
        else if (sphere_texture == 2)
        {
            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 2);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
        else {


            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }

            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_256, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_256, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
    }
    else if (sphere_face_Flag == 4) {

        if (sphere_texture == 1) {



            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag ==1 ) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag ==1 )glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 1);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

        }
        else if (sphere_texture == 2) 
        {
            if (object_space_flag == 1) glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);
            if (eye_space_flag == 1) glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 1);
            if (vertical_flag == 1) glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
            if (slant_flag == 1)glUniform1i(glGetUniformLocation(program, "slant_flag"), 1);

            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }


            glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
            glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 2);


            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "sphere_texture"), 0);
            glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "vertical_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
        else {


            if (enable_lattices == 1) {
                if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
            }

            if (enable_lighting == 0) {
                drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
            }
            else if (enable_lighting == 1) {

                if (filled == 0) {
                    drawObj(sphere_buffer, sphere_NumVertices_for_1024, 0.0);  // draw the sphere
                }
                else if (filled == 1) {
                    if (shading == 1) {
                        drawObj(sphere_flat_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                    else {
                        drawObj(sphere_shading_buffer, sphere_NumVertices_for_1024, 1.0);  // draw the sphere
                    }
                }

            }

            glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
        }
    }

    // ------------------------------the following is for floor and shadow

    mat4 shadowProjection = shadow_projection(light);


    if (correct_shadow == 0) {

        if (enable_lighting == 0) {
            /*----- Set up the Mode-View matrix for the floor -----*/
// The set-up below gives a new scene (scene 2), using Correct LookAt() function
//      mv = LookAt(eye, at, up) * Translate(0.3f, 0.0f, 0.0f) * Scale (1.6f, 1.5f, 3.3f);
            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 0.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_buffer, floor_NumVertices, 0.0);  // draw the floor
            }



           
        }
        else if (enable_lighting == 1) {

            //           //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


            if (light_source == 0) { // point

                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 1.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);

            }
            else if (light_source == 1) { // spot
                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 1.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);
            }
            else if (light_source == 2) { // directional
                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);
            }

            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

            // Set up the Normal Matrix from the model-view matrix
            mat3 normal_matrix = NormalMatrix(model_view, 1);
            // Flag in NormalMatrix(): 
            //    1: model_view involves non-uniform scaling
        //    0: otherwise.
        // Using 1 is always correct.  
        // But if no non-uniform scaling, 
            //     using 0 is faster (avoids matrix inverse computation).

            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                1, GL_TRUE, normal_matrix);
            //           //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 1.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_shading_buffer, NumVertices_floor_shading, 1.0);  // draw the floor
            }


            
        }



        // this is for shadow projection

        model_view = LookAt(eye, at, up) * shadowProjection * Translate(starting_point.x + d * starting_rowing_direction_normalized.x,
            starting_point.y + d * starting_rowing_direction_normalized.y,
            starting_point.z + d * starting_rowing_direction_normalized.z)
            * Rotate(angle, starting_rotation_axis.x, starting_rotation_axis.y, starting_rotation_axis.z) * Acc_rotationMatrix;



        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

        if (filled == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else if (filled == 1) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }


        if (enable_shadow == 1 && eye[1] > 0.0) {

            if (blend_shadow == 1.0) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }



            if (sphere_face_Flag == 1) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_8, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }
            else if (sphere_face_Flag == 2) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_128, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }
            else if (sphere_face_Flag == 3) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_256, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

            }
            else if (sphere_face_Flag == 4) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_1024, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }

            if (blend_shadow == 1.0) {
                glDisable(GL_BLEND);

            }


        }

    }
    else if (correct_shadow == 1) {

        glDepthMask(GL_FALSE);

        if (enable_lighting == 0) {
            /*----- Set up the Mode-View matrix for the floor -----*/
// The set-up below gives a new scene (scene 2), using Correct LookAt() function
//      mv = LookAt(eye, at, up) * Translate(0.3f, 0.0f, 0.0f) * Scale (1.6f, 1.5f, 3.3f);
            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 0.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_buffer, floor_NumVertices, 0.0);  // draw the floor
            }

            
        }
        else if (enable_lighting == 1) {



            if (light_source == 0) { // point

                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 1.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);

            }
            else if (light_source == 1) { // spot
                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 1.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);
            }
            else if (light_source == 2) { // directional

                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);
            }



            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

            // Set up the Normal Matrix from the model-view matrix
            mat3 normal_matrix = NormalMatrix(model_view, 1);
            // Flag in NormalMatrix(): 
            //    1: model_view involves non-uniform scaling
        //    0: otherwise.
        // Using 1 is always correct.  
        // But if no non-uniform scaling, 
            //     using 0 is faster (avoids matrix inverse computation).

            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                1, GL_TRUE, normal_matrix);





            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 1.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_shading_buffer, NumVertices_floor_shading, 1.0);  // draw the floor
            }


        }


        if (blend_shadow == 0.0) {

            glDepthMask(GL_TRUE);
        }
        // this is for shadow projection

        model_view = LookAt(eye, at, up) * shadowProjection * Translate(starting_point.x + d * starting_rowing_direction_normalized.x,
            starting_point.y + d * starting_rowing_direction_normalized.y,
            starting_point.z + d * starting_rowing_direction_normalized.z)
            * Rotate(angle, starting_rotation_axis.x, starting_rotation_axis.y, starting_rotation_axis.z) * Acc_rotationMatrix;



        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

        if (filled == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else if (filled == 1) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }


        if (enable_shadow == 1 && eye[1] > 0.0) {

            if (blend_shadow == 1.0) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }




            if (sphere_face_Flag == 1) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_8, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }
            else if (sphere_face_Flag == 2) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_128, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }
            else if (sphere_face_Flag == 3) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_256, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);

            }
            else if (sphere_face_Flag == 4) {

                if (enable_lattices == 1) {
                    if (lattices == 1) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 1); }
                    else if (lattices == 2) { glUniform1i(glGetUniformLocation(program, "lattice_option"), 2); }
                }

                drawObj_shadow(shadow_buffer, sphere_NumVertices_for_1024, 0.0, blend_shadow);

                glUniform1i(glGetUniformLocation(program, "lattice_option"), 0);
            }

            if (blend_shadow == 1.0) {
                glDisable(GL_BLEND);

            }



        }

        if (blend_shadow == 1.0) {

            glDepthMask(GL_TRUE);
        }

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        if (enable_lighting == 0) {
            /*----- Set up the Mode-View matrix for the floor -----*/
// The set-up below gives a new scene (scene 2), using Correct LookAt() function
//      mv = LookAt(eye, at, up) * Translate(0.3f, 0.0f, 0.0f) * Scale (1.6f, 1.5f, 3.3f);
            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 0.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_buffer, floor_NumVertices, 0.0);  // draw the floor
            }

           
        }
        else if (enable_lighting == 1) {



            if (light_source == 0) { // point

                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 1.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);

            }
            else if (light_source == 1) { // spot
                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 1.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);
            }
            else if (light_source == 2) {
                SetUp_Lighting_Uniform_Vars(ambient_product_point_floor_shading, diffuse_product_point_floor_shading, specular_product_point_floor_shading,
                    light_ambient_point, light_diffuse_point, light_specular_point,
                    light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 0.0, expo, cut_angle,
                    material_floor_shading_ambient, material_floor_shading_diffuse, material_floor_shading_specular,
                    light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);

            }


            model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);
            //
            // The set-up below gives the original scene (scene 1), using Correct LookAt()
            //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            // The set-up below gives the original scene (scene 1), when using previously 
            //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
            //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
            //
            glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

            // Set up the Normal Matrix from the model-view matrix
            mat3 normal_matrix = NormalMatrix(model_view, 1);
            // Flag in NormalMatrix(): 
            //    1: model_view involves non-uniform scaling
        //    0: otherwise.
        // Using 1 is always correct.  
        // But if no non-uniform scaling, 
            //     using 0 is faster (avoids matrix inverse computation).

            glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                1, GL_TRUE, normal_matrix);





            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (floor_texture == 1.0) {

                glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
                //glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);

                   // Pass on the quad_color to the uniform var "uColor" in vertex shader

                glUniform1i(glGetUniformLocation(program, "floor_texture"), 1);



                drawObj_texture(floor_texture_buffer, 6, 1.0);
                glUniform1i(glGetUniformLocation(program, "floor_texture"), 0);
            }
            else {
                drawObj(floor_shading_buffer, NumVertices_floor_shading, 1.0);  // draw the floor
            }

        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


    }



    //------------------------- the following is for axis


        /*----- Set up the Mode-View matrix for the axis -----*/
 // The set-up below gives a new scene (scene 2), using Correct LookAt() function
 //      mv = LookAt(eye, at, up) * Translate(0.3f, 0.0f, 0.0f) * Scale (1.6f, 1.5f, 3.3f);
    model_view = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, axis_NumVertices, 0.0);  // draw axis


    //-------------------------------the following is for the color cube

    /*----- Set Up the Model-View matrix for the cube -----*/
#if 0 // The following is to verify the correctness of the function NormalMatrix():
      // Commenting out Rotate() and un-commenting mat4WithUpperLeftMat3() 
      // gives the same result.
    mv = mv * Translate(0.0f, 0.5f, 0.0f) * Scale(1.4f, 1.4f, 1.4f)
        * Rotate(angle, 0.0f, 0.0f, 2.0f);
    // * mat4WithUpperLeftMat3(NormalMatrix(Rotate(angle, 0.0f, 0.0f, 2.0f), 1));
#endif
#if 1 // The following is to verify that Rotate() about (0,2,0) is RotateY():
      // Commenting out Rotate() and un-commenting RotateY()
      // gives the same result.
  //
  // The set-up below gives a new scene (scene 2), using Correct LookAt().
    model_view = LookAt(eye, at, up) * Translate(0.0f, 4.3f, 0.0f) * Scale(1.2f, 1.2f, 1.2f)
        * Rotate(angle, 0.0f, 2.0f, 0.0f) * Acc_rotationMatrix;
    // * RotateY(angle);
//
// The set-up below gives the original scene (scene 1), using Correct LookAt().
//  mv = Translate(0.0f, 0.5f, 0.0f) * mv * Scale (1.4f, 1.4f, 1.4f) 
//               * Rotate(angle, 0.0f, 2.0f, 0.0f);
          // * RotateY(angle); 
//
// The set-up below gives the original scene (scene 1), when using previously 
//     Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() )
//  mv = Translate(-1.0f, -0.5f, 0.0f) * mv * Scale (1.4f, 1.4f, 1.4f) 
//               * Rotate(angle, 0.0f, 2.0f, 0.0f);
          // * RotateY(angle);
//
#endif
#if 0  // The following is to verify that Rotate() about (3,0,0) is RotateX():
       // Commenting out Rotate() and un-commenting RotateX()
       // gives the same result.
    mv = mv * Translate(0.0f, 0.5f, 0.0f) * Scale(1.4f, 1.4f, 1.4f)
        * Rotate(angle, 3.0f, 0.0f, 0.0f);
    // * RotateX(angle);
#endif
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
    if (cubeFlag == 1) // Filled cube
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (draw_cube == 1) // draw cube
        drawObj(cube_buffer, cube_NumVertices, 0.0);  // draw the cube

    //-------------------------------------------------this is for shading cube

    SetUp_Lighting_Uniform_Vars(ambient_product_point, diffuse_product_point, specular_product_point,
        light_ambient_point, light_diffuse_point, light_specular_point,
        light_ambient_spot, light_diffuse_spot, light_specular_spot, spot_light_dir, light_position_spot, 1.0, expo, cut_angle,
        cube_material_ambient, cube_material_diffuse, cube_material_specular,
        light_position_point, 0.0, const_att, linear_att, quad_att, material_floor_shading_shininess, mv);



    model_view = LookAt(eye, at, up) * Translate(0.0f, 2.7f, 0.0f) * Scale(1.2f, 1.2f, 1.2f)* Rotate(angle, 0.0f, 2.0f, 0.0f);
    //
    // The set-up below gives the original scene (scene 1), using Correct LookAt()
    //    mv = Translate(0.0f, 0.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
    //
    // The set-up below gives the original scene (scene 1), when using previously 
    //       Incorrect LookAt() (= Translate(1.0f, 1.0f, 0.0f) * correct LookAt() ) 
    //    mv = Translate(-1.0f, -1.0f, 0.3f) * LookAt(eye, at, up) * Scale (1.6f, 1.5f, 3.3f);
    //
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major

    // Set up the Normal Matrix from the model-view matrix
    normal_matrix = NormalMatrix(model_view, 1);
    // Flag in NormalMatrix(): 
    //    1: model_view involves non-uniform scaling
//    0: otherwise.
// Using 1 is always correct.  
// But if no non-uniform scaling, 
    //     using 0 is faster (avoids matrix inverse computation).

  
    if (cubeFlag == 1) // Filled cube
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (draw_cube == 1) // draw cube
        drawObj(cube_buffer_shading, NumVertices_shading, 1.0);  // draw the cube


    //--------------------------------------------the following is for fire work

    model_view = LookAt(eye, at, up);




    glUseProgram(firework_program);
    ModelView = glGetUniformLocation(firework_program, "ModelView");
    Projection = glGetUniformLocation(firework_program, "Projection");
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
    glPointSize(3.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);


    if (right_mouse_no_effect == 1.0) {
        float t_sub = glutGet(GLUT_ELAPSED_TIME);
        int time = int((t_sub-t)) % 5000;
        glUniform1f(glGetUniformLocation(firework_program, "time"), time);
    }
    else {
        glUniform1f(glGetUniformLocation(firework_program, "time"), 0);
    }


    model_view = LookAt(eye, at, up);

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
    if (fire_work == 1.0) { // in correct 
        drawObj_firework(firework_buffer, firework_NumVertices, 1.0);
    }
    else if (fire_work == 2.0) { // in correct with color
        drawObj_firework(firework_buffer, firework_NumVertices, 2.0);
    }
    else if (fire_work == 3.0) { // correct
        drawObj_firework(firework_buffer, firework_NumVertices, 3.0);
    }




    else if (fire_work == 0.0){

        t = glutGet(GLUT_ELAPSED_TIME);
    }




    glutSwapBuffers();
}

/*---------------------------------------------------------------------
timer(): Timer function for animation updates
---------------------------------------------------------------------*/
void timer(int value)
{


    angle += angle_roation_speed;

    glutPostRedisplay(); // Request a redraw to update the display
    glutTimerFunc(100 / 60, timer, 0); // Call the timer function again after 1/60th of a second
}

//---------------------------------------------------------------------------
void idle(void)
{
    angle += angle_roation_speed;
    // angle += 1.0f;    //YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}
//------------------------------------------------------------------------------- code for controlling mouse event

// this is a mouse event
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Handle left mouse button click
        //printf("Left mouse click. \n");
        // Open the menu at the mouse cursor position
        glutPostRedisplay(); // Ensure a display event is triggered to show the menu
        glutAttachMenu(GLUT_RIGHT_BUTTON); // Attach the menu to the right mouse button
        glutDetachMenu(GLUT_RIGHT_BUTTON); // Detach the menu after selection
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        // Handle left mouse button click
        //printf("Right mouse click. \n");

        if (right_mouse_no_effect == 1) {
            animationFlag = 1 - animationFlag;
            if (animationFlag == 1) glutIdleFunc(idle);
            else                    glutIdleFunc(NULL);

        }


    }
}
//--------------------------------------------------------------------------- code for controlling menus

// this is the menu
void top_menu(int option) {
    switch (option) {
    case 1: // Quit
        //printf("Case 1.");
        exit(EXIT_SUCCESS);
        break;
    case 2: // Default View Point
        //printf("Case 2.");
        eye = init_eye;
        break;
    case 3:
        // Handle Option 3
        //printf("Case 3.");
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void rotation_menu(int option) {
    switch (option) {
    case 31: // correct rotation
        //printf("Case 31.");
        accumulated_rotation_Flag = 1;

        break;
    case 32: // incorrect rotation
        //printf("Case 32.");
        accumulated_rotation_Flag = 0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void cube_menu(int option) {
    switch (option) {
    case 41: // draw the cube
        draw_cube = 1;

        break;
    case 42: // dont draw the cube

        draw_cube = 0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void fill_menu(int option) {
    switch (option) {
    case 51: // fill the sphere
        filled = 1;

        break;
    case 52: // dont fill the sphere

        filled = 0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void shadow_menu(int option) {
    switch (option) {
    case 61: // enable shadow
        enable_shadow = 1;
        correct_shadow = 1;

        break;
    case 62: // disable shadow

        enable_shadow = 0;
        correct_shadow = 0;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void correct_shadow_menu(int option) {
    switch (option) {
    case 621: // correct
        correct_shadow = 1;

        break;
    case 622: // incorrect
        correct_shadow = 0;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void blend_shadow_menu(int option) {
    switch (option) {
    case 111: // blend
        blend_shadow=1.0;

        break;
    case 112: // dont 
        blend_shadow = 0.0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void lighting_menu(int option) {
    switch (option) {
    case 71: // enable 
        enable_lighting = 1;


        break;
    case 72: // disable 

        enable_lighting = 0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void shading_menu(int option) {
    switch (option) {
    case 81: // flat
        shading = 0;
        filled = 1;


        break;
    case 82: // disable 

        shading = 1;
        filled = 1;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}
void light_menu(int option) {
    switch (option) {
    case 91: // point
        light_source = 0;


        break;
    case 92: // spot

        light_source = 1;

        break;

    case 93: // only directional

        light_source = 2;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void fog_menu(int option) {
    switch (option) {
    case 101: // point
        fog=0;


        break;
    case 102: // spot

        fog=1;

        break;

    case 103: // only directional

        fog=2;

        break;
    case 104: // only directional

        fog = 3;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void firework_menu(int option) {
    switch (option) {
    case 121: // no fire work
        fire_work= 0.0;


        break;
    case 122: // incorrect, no color change

        fire_work = 1.0;

        break;

    case 123: //incorrect, no color change

        fire_work = 2.0;

        break;    
    case 124: // correct

        fire_work = 3.0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void texture_floor_menu(int option) {
    switch (option) {
    case 131: // flat
        floor_texture = 0.0;


        break;
    case 132: // disable 
        floor_texture = 1.0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void sphere_sphere_menu(int option) {
    switch (option) {
    case 141: // flat
        sphere_texture = 0.0;


        break;
    case 142: // disable 
        sphere_texture = 1.0;

        break;
    case 143: // disable 
        sphere_texture = 2.0;

        break;
    default:
        break;
    }
    glutPostRedisplay();
}


void menu() {
    // Create a menu and attach it to the right mouse button

    int sub_menu_for_sphere_texture = glutCreateMenu(sphere_sphere_menu);
    glutSetMenuFont(sub_menu_for_sphere_texture, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" No Sphere Texture ", 141);
    glutAddMenuEntry(" Contour Line Texture ", 142);
    glutAddMenuEntry(" Checkerboard Texture ", 143);

    int sub_menu_for_floor_texture = glutCreateMenu(texture_floor_menu);
    glutSetMenuFont(sub_menu_for_floor_texture, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" No Texture Floor ", 131);
    glutAddMenuEntry(" Texture Floor ", 132);


    int sub_menu_for_firework = glutCreateMenu(firework_menu);
    glutSetMenuFont(sub_menu_for_firework, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" No Firework ", 121);
    glutAddMenuEntry(" FireWork (Incorrect With Penetration) ", 122);
    glutAddMenuEntry(" FireWork (Incorrect With Change Color) ", 123);
    glutAddMenuEntry(" FireWork (Correct with Discard) ", 124);

    int blend_shadow_menu_for_sphere = glutCreateMenu(blend_shadow_menu);
    glutSetMenuFont(blend_shadow_menu_for_sphere, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Blend Shadow ", 111);
    glutAddMenuEntry(" Don't Blend Shadow ", 112);

    int sub_menu_for_fog = glutCreateMenu(fog_menu);
    glutSetMenuFont(sub_menu_for_fog, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" No Fog ", 101);
    glutAddMenuEntry(" Linear ", 102);
    glutAddMenuEntry(" Exponential ", 103);
    glutAddMenuEntry(" Exponential Square", 104);

    int sub_menu_for_light_source = glutCreateMenu(light_menu);
    glutSetMenuFont(sub_menu_for_light_source, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Point Light ", 91);
    glutAddMenuEntry(" Spot Light ", 92);
    glutAddMenuEntry(" Only Directional ", 93);

    int sub_menu_for_shading = glutCreateMenu(shading_menu);
    glutSetMenuFont(sub_menu_for_shading, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Flat Shading ", 81);
    glutAddMenuEntry(" Smooth Shading ", 82);

    int sub_menu_for_lighting = glutCreateMenu(lighting_menu);
    glutSetMenuFont(sub_menu_for_lighting, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Enable Lighting (Yes) ", 71);
    glutAddMenuEntry(" Disbale Lighting (No) ", 72);

    int correct_shadow_menu_for_sphere = glutCreateMenu(correct_shadow_menu);
    glutSetMenuFont(correct_shadow_menu_for_sphere, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Correct Shadow ", 621);
    glutAddMenuEntry(" Incorrect Shadow ", 622);

    int shadow_menu_for_sphere = glutCreateMenu(shadow_menu);
    glutSetMenuFont(shadow_menu_for_sphere, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Enable Shadow (Yes) ", 61);
    glutAddMenuEntry(" Disbale Shadow (No) ", 62);

    int fill_menu_for_sphere = glutCreateMenu(fill_menu);
    glutSetMenuFont(fill_menu_for_sphere, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Fill the sphere (Solid Sphere) ", 51);
    glutAddMenuEntry(" Don't fill the sphere (Wireframe Sphere) ", 52);

    int sub_menu_for_drawing_cube = glutCreateMenu(cube_menu);
    glutSetMenuFont(sub_menu_for_drawing_cube, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Draw the cube ", 41);
    glutAddMenuEntry(" Don't draw the cube ", 42);

    int sub_menu_for_correct_rowtation = glutCreateMenu(rotation_menu);
    glutSetMenuFont(sub_menu_for_correct_rowtation, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Correct Rotation ", 31);
    glutAddMenuEntry(" Incorrect Rotation ", 32);

    // Add menu 
    int menu = glutCreateMenu(top_menu);
    glutSetMenuFont(menu, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Quit", 1);
    glutAddMenuEntry(" Default View Point ", 2);

    int empty = 0;
    glutAddSubMenu(" Rotation Correctness ", sub_menu_for_correct_rowtation);
    glutAddSubMenu(" Cube Options ", sub_menu_for_drawing_cube);
    glutAddSubMenu(" Wireframe Sphere Options ", fill_menu_for_sphere);
    glutAddSubMenu(" Shadow Options ", shadow_menu_for_sphere);
    glutAddSubMenu(" Correct Shadow? ", correct_shadow_menu_for_sphere);
    glutAddSubMenu(" Blend Shadow? ", blend_shadow_menu_for_sphere);
    glutAddSubMenu(" Enable Lighting ", sub_menu_for_lighting);
    glutAddSubMenu(" Light Source ", sub_menu_for_light_source);
    glutAddSubMenu(" Shading ", sub_menu_for_shading);
    glutAddSubMenu(" Fog Options ", sub_menu_for_fog);
    glutAddSubMenu(" Firework Options ", sub_menu_for_firework);
    glutAddSubMenu(" Texture Floor Option ", sub_menu_for_floor_texture);
    glutAddSubMenu(" Texture Sphere Option ", sub_menu_for_sphere_texture);
    glutAddMenuEntry(" Lattice - Use l/L, u/U, t/T ",empty);
    glutAddMenuEntry(" Sphere Texture - Use v/V, s/S, o/O, e/E ", empty);

    //glutAddMenuEntry("Option 3", 3);



    // Attach the menu to the left mouse button
    glutAttachMenu(GLUT_LEFT_BUTTON);
}


//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {

    case 'r': case 'R': // Toggle between correct or non-correct rotation
        accumulated_rotation_Flag = 1 - accumulated_rotation_Flag;
        break;


    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;

    case 'X': eye[0] += 1.0; break;
    case 'x': eye[0] -= 1.0; break;
    case 'Y': eye[1] += 1.0; break;
    case 'y': eye[1] -= 1.0; break;
    case 'Z': eye[2] += 1.0; break;
    case 'z': eye[2] -= 1.0; break;

    case 'b': case 'B': // Toggle between animation and non-animation
        t = glutGet(GLUT_ELAPSED_TIME);
        animationFlag = 1 - animationFlag;
        if (animationFlag == 1) glutIdleFunc(idle);
        else                    glutIdleFunc(NULL);

        if (right_mouse_no_effect == 0) right_mouse_no_effect = 1;

        break;

    case 'c': case 'C': // Toggle between filled and wireframe cube
        cubeFlag = 1 - cubeFlag;
        break;

    case 'f': case 'F': // Toggle between filled and wireframe floor
        floorFlag = 1 - floorFlag;
        break;

    case 'v': case'V':
        vertical_flag = 1;
        slant_flag = 0;
        break;
    case 's': case'S':
        vertical_flag = 0;
        slant_flag = 1;
        break;
    case 'o': case'O':
        object_space_flag = 1;
        eye_space_flag = 0;
        break;
    case 'e': case'E':
        object_space_flag = 0;
        eye_space_flag = 1;
        break;

    case 'u': case'U':
        enable_lattices = 1;
        lattices = 1;

        break;
    case 't': case'T':
        enable_lattices = 1;
        lattices = 2;

        break;
    case 'l': case'L':
        enable_lattices = 1- enable_lattices;

        break;


    case ' ':  // reset to initial viewer/eye position
        eye = init_eye;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat)width / (GLfloat)height;
    glutPostRedisplay();
}

//----------------------------------------------------- code for reading the sphere

// fucntion to read the vertices of the sphere

// Code for Part a prompting user to choose the sphere, and read and store the points of the triangles on the sphere
void read_sphere(FILE* fp) {

    // Read the number of triangles
    fscanf_s(fp, "%d", &nums_triangle_in_sphere);
    printf("The sphere has %d triangles. \n", nums_triangle_in_sphere);

    // Read and print each line from the file
    char line[256];  // Assuming each line has at most 255 characters
    while (fgets(line, sizeof(line), fp) != NULL) {
        //printf("%s", line);
        // Remove newline character from the end of the line
        line[strcspn(line, "\n")] = '\0';

        // Check if the line equals "3"
        if (strcmp(line, "3") == 0) {
            // Perform your specific action here
            //printf("Line equals '3'.\n");
            continue;

        }
        else {
            // Print the line
            //printf("%s\n", line);
            // store the points of the triangle

            if (strcmp(line, "") == 0) {
                // Perform your specific action here
                //printf("Line equals 'n'.\n");
                continue;

            }

            // split the line usng " " 
            char* token = strtok(line, " ");
            double values[3]; // Assuming there are three float values in the line

            int index = 0;
            while (token != nullptr && index < 3) {
                // Convert token to float and store in values array
                values[index] = atof(token);
                token = strtok(nullptr, " "); // Get next token
                index++;
            }

            //printf("%f %f %f \n", values[0], values[1], values[2]);


            if (counter == 0) { // it is the triangle's first point
                //fscanf_s(, "%d %d %d", &triangles[triangle_count].x1, &triangles[triangle_count].y1, &triangles[triangle_count].z1)
                triangles[triangle_count].x1 = values[0];
                triangles[triangle_count].y1 = values[1];
                triangles[triangle_count].z1 = values[2];
                counter++;


            }
            else if (counter == 1) { // it is the triangle's second point
                //fscanf_s(fp, "%d %d %d", &triangles[triangle_count].x2, &triangles[triangle_count].y2, &triangles[triangle_count].z2);
                triangles[triangle_count].x2 = values[0];
                triangles[triangle_count].y2 = values[1];
                triangles[triangle_count].z2 = values[2];
                counter++;


            }
            else if (counter == 2) { // it is the triangle's third point
                //fscanf_s(fp, "%d %d %d", &triangles[triangle_count].x3, &triangles[triangle_count].y3, &triangles[triangle_count].z3);
                triangles[triangle_count].x3 = values[0];
                triangles[triangle_count].y3 = values[1];
                triangles[triangle_count].z3 = values[2];
                counter = 0;
                triangle_count++;

            }

        }
    }


    /***
    // for checking the storing of the point
    for (int i = 0; i < nums_triangle_in_sphere; i++) {
        printf("The points in the %d Triangle: \n", i);
        printf(" %f %f %f \n", triangles[i].x1, triangles[i].y1, triangles[i].z1);
        printf(" %f %f %f\n", triangles[i].x2, triangles[i].y2, triangles[i].z2);
        printf(" %f %f %f\n", triangles[i].x3, triangles[i].y3, triangles[i].z3);


    }
    **/



}

void read_sphere_8() {

    FILE* fp;

    // Open the file for reading
    if (fopen_s(&fp, "sphere.8", "r") != 0) {
        printf("Error opening file.\n");
        return;
    }
    else {
        printf("File Opened.\n");
    }
    read_sphere(fp);

}

void read_sphere_128() {

    FILE* fp;

    // Open the file for reading
    if (fopen_s(&fp, "sphere.128", "r") != 0) {
        printf("Error opening file.\n");
        return;
    }
    else {
        printf("File Opened.\n");
    }
    read_sphere(fp);

}

void read_sphere_256() {

    FILE* fp;

    // Open the file for reading
    if (fopen_s(&fp, "sphere.256", "r") != 0) {
        printf("Error opening file.\n");
        return;
    }
    else {
        printf("File Opened.\n");
    }
    read_sphere(fp);

}

void read_sphere_1024() {

    FILE* fp;

    // Open the file for reading
    if (fopen_s(&fp, "sphere.1024", "r") != 0) {
        printf("Error opening file.\n");
        return;
    }
    else {
        printf("File Opened.\n");
    }
    read_sphere(fp);

}


//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    // this is for enabling the z buffer
    glEnable(GL_DEPTH_TEST);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Color Cube");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
        exit(1);
    }
#endif

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));


    int sphere_type = 0;

    do {
        printf("which sphere do you want to see?\n");
        printf("(Type 1 for 8-face sphere, 2 for 128-face sphere, 3 for 256-face sphere, 4 for 1024-face sphere):");
        scanf_s(" %d", &sphere_type, sizeof(sphere_type)); // Note the space before %c to consume any leading whitespace
    } while (sphere_type != 1 && sphere_type != 2 && sphere_type != 3 && sphere_type != 4);



    if (sphere_type == 1) {
        sphere_face_Flag = 1;
        read_sphere_8();
    }
    else if (sphere_type == 2) {
        sphere_face_Flag = 2;
        read_sphere_128();
    }
    else if (sphere_type == 3) {
        sphere_face_Flag = 3;
        read_sphere_256();
    }
    else if (sphere_type == 4) {
        sphere_face_Flag = 4;
        read_sphere_1024();
    }





    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    // Register the timer function
    //glutTimerFunc(1000 / 60, timer, 0); // Calls timer every 1/60th of a second (60 FPS)
    glutMouseFunc(mouse);
    menu();

    init();

    glutMainLoop();
    return 0;
}
