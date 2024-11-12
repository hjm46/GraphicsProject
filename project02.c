/*
 * project02.c
 *
 *  Created on: September 10, 2024
 *      Author: Thumrongsak Kosiyatrakul
 */


#ifdef __APPLE__  // include Mac OS X verions of headers

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>

#else // non-Mac OS X operating systems

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#endif  // __APPLE__

#include "initShader.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "myLib.h"
#include "maze.h"

int num_vertices = 36;
GLuint ctm_location;
mat4 curr_trans_matrix = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint model_view_location;
mat4 model_view = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint projection_location;
mat4 projection = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
float x_coor = 0;
float y_coor = 0;
float z_coor = 0;
int length;
int width;
int len;
int wid;
int last_for_ground = 0;
int block_pos = 0;

mat4 look_at(GLfloat eye_x, GLfloat eye_y, GLfloat eye_z, 
            GLfloat at_x, GLfloat at_y, GLfloat at_z, 
            GLfloat up_x, GLfloat up_y, GLfloat up_z){
    vec4 n = {at_x, at_y, at_z, 0};
    vec4 v_up = {up_x, up_y, up_z, 0};

    vec4 v = vec_sub(v_up, scalar_vec_mult(dot_prod(v_up,n)/dot_prod(n,n), n));
    vec4 u = cross_prod(v, n);

    vec4 u_prime = normalize(magnitude(u), u);
    vec4 v_prime = normalize(magnitude(v), v);
    vec4 n_prime = normalize(magnitude(n), n);

    mat4 rotation = {{u_prime.x, v_prime.x, n_prime.x, 0}, {u_prime.y, v_prime.y, n_prime.y, 0}, {u_prime.z, v_prime.z, n_prime.z, 0}, {0,0,0,1}};
    return mat_mult(rotation, translate(-eye_x, -eye_y, -eye_z));
}

mat4 frustum(GLfloat left, GLfloat right, GLfloat bottom,
GLfloat top, GLfloat near, GLfloat far) {
    mat4 proj_mat = {{(-2*near)/(right-left),0,0,0}, {0,(-2*near)/(top-bottom),0,0}, {(left+right)/(right-left), (bottom+top)/(top-bottom), (near+far)/(far-near), -1}, {0,0, -(2*near*far)/(far-near),0}};
    return proj_mat;
}

void first_row_ground_tex(vec2* tex_coords, int vertices) {    
    // first row of blocks has grass (top), grass (side), and dirt
    int tex = 0;
    for(int i = 0; i < vertices; i += 36){
        // sides: dirt + grass
        tex_coords[tex] = (vec2) {0.75, 0.75};
        tex_coords[tex + 1] = (vec2) {0.50, 0.75};
        tex_coords[tex + 2] = (vec2) {0.75, 1.00};
        tex_coords[tex + 3] = (vec2) {0.75, 1.00};
        tex_coords[tex + 4] = (vec2) {0.50, 0.75};
        tex_coords[tex + 5] = (vec2) {0.50, 1.00};

        tex_coords[tex + 6] = (vec2) {0.75, 1.00};
        tex_coords[tex + 7] = (vec2) {0.50, 1.00};
        tex_coords[tex + 8] = (vec2) {0.75, 0.75};
        tex_coords[tex + 9] = (vec2) {0.75, 0.75};
        tex_coords[tex + 10] = (vec2) {0.50, 1.00};
        tex_coords[tex + 11] = (vec2) {0.50, 0.75};

        // bottom: dirt
        tex_coords[tex + 12] = (vec2) {0.75, 1.00};
        tex_coords[tex + 13] = (vec2) {0.75, 0.75};
        tex_coords[tex + 14] = (vec2) {1.00, 1.00};
        tex_coords[tex + 15] = (vec2) {1.00, 1.00};
        tex_coords[tex + 16] = (vec2){0.75, 0.75};
        tex_coords[tex + 17] = (vec2){0.75, 1.00};

        // dirt + grass
        tex_coords[tex + 18] = (vec2) {0.75, 0.75};
        tex_coords[tex + 19] = (vec2) {0.50, 0.75};
        tex_coords[tex + 20] = (vec2) {0.75, 1.00};
        tex_coords[tex + 21] = (vec2) {0.75, 1.00};
        tex_coords[tex + 22] = (vec2) {0.50, 0.75};
        tex_coords[tex + 23] = (vec2) {0.50, 1.00};

        // top: grass
        tex_coords[tex + 24] = (vec2) {0.0, 0.25};
        tex_coords[tex + 25] = (vec2) {0.0, 0.0};
        tex_coords[tex + 26] = (vec2) {0.25, 0.25};
        tex_coords[tex + 27] = (vec2) {0.25, 0.25};
        tex_coords[tex + 28] = (vec2) {0.0, 0.0};
        tex_coords[tex + 29] = (vec2) {0.0, 0.25};

        // dirt + grass
        tex_coords[tex + 30] = (vec2) {0.50, 1.00};
        tex_coords[tex + 31] = (vec2) {0.75, 1.00};
        tex_coords[tex + 32] = (vec2) {0.50, 0.75};
        tex_coords[tex + 33] = (vec2) {0.50, 0.75};
        tex_coords[tex + 34] = (vec2) {0.75, 1.00};
        tex_coords[tex + 35] = (vec2) {0.75, 0.75};
        tex += 36;
    }
}

void ground_dirt(vec2* tex_coords, int start, int last) {
    for(int i = start; i < last; i += 6){
        tex_coords[i] = (vec2) {0.75, 1.00};
        tex_coords[i + 1] = (vec2){0.75, 0.75};
        tex_coords[i + 2] = (vec2){1.00, 1.00};
        tex_coords[i + 3] = (vec2){1.00, 1.00};
        tex_coords[i + 4] = (vec2){0.75, 0.75};
        tex_coords[i + 5] = (vec2){0.75, 1.00};
    }
}

void create_pyramid(vec4* positions, int index) {
    srand(time(NULL));
    int size = width * 4 + 6;
    int pyramid[size/2][size][size];

    // intialize to all 0s
    for (int i = 0; i < size / 2; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                pyramid[i][j][k] = 0;
            }
        }
    }

    // create the pyramid pattern
    for (int i = 0; i < size / 2; i++) {
        for (int j = i; j < size - i; j++) {
            for (int k = i; k < size - i; k++) {
                pyramid[i][j][k] = 1;
            }
        }
    }

    // add and remove random cubes
    for (int i = 0; i < size / 2; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
               if(rand() % 3 == 0) {
                if (pyramid[i][j][k] == 0)
                    pyramid[i][j][k] = 1;
                if (pyramid[i][j][k] == 1)
                    pyramid[i][j][k] = 0;
               }
            }
        }
    }

    // setting the cube positions
    int factor = 1;
    for (int i = 0; i < size / 2; i++) {
        int pos = 0;
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                if(pyramid[i][j][k] == 0) {
                    pos += 36;
                }
                if(pyramid[i][j][k] == 1) {
                    for(int x = 0; x < 36; x++) {
                        positions[index + x] = vec_sub(positions[pos], (vec4) {0, 2 * factor, 0, 0});
                        pos += 1;
                    }
                    index += 36;
                }
            }
        }
        factor += 1;
    }
    last_for_ground = index;
}

void grassy_stone(vec2* tex_coords, int start){
    for(int i = start; i < start + 36; i += 6){
        tex_coords[i] = (vec2) {1.00, 0.00};
        tex_coords[i + 1] = (vec2){0.75, 0.00};
        tex_coords[i + 2] = (vec2){1.00, 0.25};
        tex_coords[i + 3] = (vec2){1.00, 0.25};
        tex_coords[i + 4] = (vec2){0.75, 0.00};
        tex_coords[i + 5] = (vec2){0.75, 0.25};
    }
}

void stone(vec2* tex_coords, int start){
    for(int i = start; i < start + 36; i += 6){
        tex_coords[i] = (vec2) {0.25, 0.25};
        tex_coords[i + 1] = (vec2){0.00, 0.25};
        tex_coords[i + 2] = (vec2){0.25, 0.50};
        tex_coords[i + 3] = (vec2){0.25, 0.50};
        tex_coords[i + 4] = (vec2){0.00, 0.25};
        tex_coords[i + 5] = (vec2){0.00, 0.50};
    }
}

void brick(vec2* tex_coords, int start){
    for(int i = start; i < start + 36; i += 6){
        tex_coords[i] = (vec2) {1.00, 0.25};
        tex_coords[i + 1] = (vec2){0.75, 0.25};
        tex_coords[i + 2] = (vec2){1.00, 0.50};
        tex_coords[i + 3] = (vec2){1.00, 0.50};
        tex_coords[i + 4] = (vec2){0.75, 0.25};
        tex_coords[i + 5] = (vec2){0.75, 0.50};
    }
}

int create_block(vec4* positions, int index, int pos){
    for(int y = 0; y < 36; y++) {
        positions[index + y] = vec_add(positions[pos], (vec4) {0, 2, 0, 0});
        pos += 1;
    } 
    return pos;
}

void create_maze(post** maze, vec4* positions, vec2* tex_coords) {
    //top border
    int index = last_for_ground;
    int offset = 2;
    printf("+");
    int pos = 36 * (wid * 4 + 6) * offset + (72);
    pos = create_block(positions, index, pos);
    brick(tex_coords, index);
    index += 36;

    for(int j=0; j<width; j+=1)
    {
        printf("---+");
        for(int x = 0; x < 4; x++){
            pos = create_block(positions, index, pos);
            if(x == 3){
                brick(tex_coords, index);
            }
            else
                stone(tex_coords, index);
            index += 36;
        }
    }
    printf("   +\n|");
    for(int x = 0; x < 3; x++){
        pos += 36;
    }
    pos = create_block(positions, index, pos);
    brick(tex_coords, index);
    index += 36;
    // start of new row
    offset++;
    pos = 36 * (wid * 4 + 6) * offset + (72);
    pos = create_block(positions, index, pos);
    stone(tex_coords, index);
    index += 36;

    for(int j=0; j<width; j+=1)
    {
        printf("   ");
        for(int x = 0; x < 3; x++){
            pos += 36;
        }
        if(maze[0][j].north == 1) {
            printf("|");
            pos = create_block(positions, index, pos);
            stone(tex_coords, index);
            index += 36;
        }
        else{
            printf(" ");
            pos += 36;
        }       
    }
    printf("   |\n");
    for(int x = 0; x < 3; x++){
        pos += 36;
    }
    pos = create_block(positions, index, pos);
    stone(tex_coords, index);
    index += 36;
    offset++;
    pos = 36 * (wid * 4 + 6) * offset + (72);

    //interior
    for(int i=0; i<length; i+=1)
    {
        printf("+");
        pos = create_block(positions, index, pos);
        brick(tex_coords, index);
        index += 36;
        if(maze[i][0].west == 1){
            printf("---");
            for(int x = 0; x < 3; x++){
                pos = create_block(positions, index, pos);
                stone(tex_coords, index);
                index += 36;
            }
        }
        else{
            printf("   ");
            for(int x = 0; x < 3; x++){
                pos += 36;
            }
        }
        printf("+");
        pos = create_block(positions, index, pos);
        brick(tex_coords, index);
        index += 36;

        for(int j=1; j<width; j+=1)
        {
            if(maze[i][j].west == 1 && maze[i][j-1].east == 1){
                printf("---");
                for(int x = 0; x < 3; x++){
                    pos = create_block(positions, index, pos);
                    stone(tex_coords, index);
                    index += 36;
                }
            }
            else{
                printf("   ");
                for(int x = 0; x < 3; x++){
                    pos += 36;
                }
            }  
            printf("+");
            pos = create_block(positions, index, pos);
            brick(tex_coords, index);
            index += 36;
        }

        if(maze[i][width-1].east == 1){
            printf("---");
            for(int x = 0; x < 3; x++){
                pos = create_block(positions, index, pos);
                stone(tex_coords, index);
                index += 36;
            }
        }
        else{
            printf("   ");
            for(int x = 0; x < 3; x++){
                pos += 36;
            }
        }  
        printf("+");
        pos = create_block(positions, index, pos);
        brick(tex_coords, index);
        index += 36;

        printf("\n|");
        offset++;
        pos = 36 * (wid * 4 + 6) * offset + (72);
        pos = create_block(positions, index, pos);
        stone(tex_coords, index);
        index += 36;
        for(int j=0; j<width; j+=1)
        {
            printf("   ");
            for(int x = 0; x < 3; x++){
                pos += 36;
            }
            if(i == length-1 && maze[i][j].south == 1){
                printf("|");
                pos = create_block(positions, index, pos);
                stone(tex_coords, index);
                index += 36;
            }
            else if(maze[i][j].south == 1 && maze[i+1][j].north == 1){
                printf("|");
                pos = create_block(positions, index, pos);
                stone(tex_coords, index);
                index += 36;
            }
            else{
                printf(" ");
                pos += 36;
            }
        }
        printf("   |");
        for(int x = 0; x < 3; x++){
            pos += 36;
        }
        pos = create_block(positions, index, pos);
        stone(tex_coords, index);
        index += 36;
        printf("\n");
        offset++;
        pos = 36 * (wid * 4 + 6) * offset + (72);
    }

    // //bottom border
    printf("+   +");
    pos = create_block(positions, index, pos);
    brick(tex_coords, index);
    index += 72;
    pos += 72 + 36;
    pos = create_block(positions, index, pos);
    brick(tex_coords, index);
    index += 36;
    for(int j=1; j<width+1; j+=1)
    {
        printf("---+");
        for(int x = 0; x < 4; x++){
            pos = create_block(positions, index, pos);
            if(x == 3){
                brick(tex_coords, index);
            }
            else
                stone(tex_coords, index);
            index += 36;
        }
    }
    printf("\n\n");
}

void init(void)
{
    num_vertices += (36 * (width * 4 + 6) * (length * 4 + 6) * (width * 2)) + (36 * width * 4 * length * 4);
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    vec4 *positions = (vec4 *) malloc(sizeof(vec4) * num_vertices);

    // creating one cube
    positions[0] = (vec4) { 1.0,  1.0, 1.0, 1.0};
    positions[1] = (vec4) {-1.0,  1.0, 1.0, 1.0};
    positions[2] = (vec4) { 1.0, -1.0, 1.0, 1.0};
    positions[3] = (vec4) { 1.0, -1.0, 1.0, 1.0};
    positions[4] = (vec4) {-1.0,  1.0, 1.0, 1.0};
    positions[5] = (vec4) {-1.0, -1.0, 1.0, 1.0};

    positions[6] = (vec4) { 1.0, -1.0, -1.0, 1.0};
    positions[7] = (vec4) {-1.0, -1.0, -1.0, 1.0};
    positions[8] = (vec4) { 1.0,  1.0, -1.0, 1.0};
    positions[9] = (vec4) { 1.0,  1.0, -1.0, 1.0};
    positions[10] = (vec4){-1.0, -1.0, -1.0, 1.0};
    positions[11] = (vec4){-1.0,  1.0, -1.0, 1.0};

    positions[12] = (vec4) {-1.0, -1.0,  1.0, 1.0};
    positions[13] = (vec4) {-1.0, -1.0, -1.0, 1.0};
    positions[14] = (vec4) { 1.0, -1.0,  1.0, 1.0};
    positions[15] = (vec4) { 1.0, -1.0,  1.0, 1.0};
    positions[16] = (vec4) {-1.0, -1.0, -1.0, 1.0};
    positions[17] = (vec4) { 1.0, -1.0, -1.0, 1.0};

    positions[18] = (vec4) {-1.0,  1.0,  1.0, 1.0};
    positions[19] = (vec4) {-1.0,  1.0, -1.0, 1.0};
    positions[20] = (vec4) {-1.0, -1.0,  1.0, 1.0};
    positions[21] = (vec4) {-1.0, -1.0,  1.0, 1.0};
    positions[22] = (vec4) {-1.0,  1.0, -1.0, 1.0};
    positions[23] = (vec4) {-1.0, -1.0, -1.0, 1.0};

    positions[24] = (vec4) { 1.0, 1.0,  1.0, 1.0};
    positions[25] = (vec4) { 1.0, 1.0, -1.0, 1.0};
    positions[26] = (vec4) {-1.0, 1.0,  1.0, 1.0};
    positions[27] = (vec4) {-1.0, 1.0,  1.0, 1.0};
    positions[28] = (vec4) { 1.0, 1.0, -1.0, 1.0};
    positions[29] = (vec4) {-1.0, 1.0, -1.0, 1.0};

    positions[30] = (vec4) { 1.0, -1.0,  1.0, 1.0};
    positions[31] = (vec4) { 1.0, -1.0, -1.0, 1.0};
    positions[32] = (vec4) { 1.0,  1.0,  1.0, 1.0};
    positions[33] = (vec4) { 1.0,  1.0,  1.0, 1.0};
    positions[34] = (vec4) { 1.0, -1.0, -1.0, 1.0};
    positions[35] = (vec4) { 1.0,  1.0, -1.0, 1.0};

    // create upside-down pyramid w/ textures --> ground should be larger than l & w of maze
    // first layer
    int index = 36;
    int pos = 0;
    for(int i = 0; i < width * 4 + 5; i++) {
        for(int j = 0; j < 36; j++) {
            positions[index + j] = vec_add(positions[pos], (vec4) {2, 0, 0, 0});
            pos += 1;
        }
        index += 36;
    }

    pos = 0;
    for(int k = 0; k < width * 4 + 6; k++) {
        for(int i = 0; i < length * 4 + 5; i++) {
            for(int j = 0; j < 36; j++) {
                positions[index + j] = vec_sub(positions[pos], (vec4) {0, 0, 2, 0});
                pos += 1;
            }
            index += 36;
        }
    }

    // creating the dirt layers
    int start = index;
    create_pyramid(positions, index);

    // define texture coords
    vec2 *tex_coords = (vec2 *) malloc(sizeof(vec2) * num_vertices);
    first_row_ground_tex(tex_coords, start);
    ground_dirt(tex_coords, start, last_for_ground);

    // generating the maze on the platform
    len = length;
    wid = width;
    post** maze = gen_maze(width, length);
    create_maze(maze, positions, tex_coords);

    // TO-DO:
    // scale and center
    // float scale_by = width * 4 + 12;
    // if(length < width) {
    //     scale_by = length * 4 + 12;
    // }
    float scale_by = width * 20;
    for(int i = 0; i < num_vertices; i++) {
        //positions[i] = mat_vec_mult(scale(1/scale_by, 1/scale_by, 1/scale_by), mat_vec_mult(translate(-(width * 4 + 9), 0, -(width * 4 + 9)), positions[i]));
        positions[i] = mat_vec_mult(translate(-(width * 4 + 9), 0, -(width * 4 + 9)), positions[i]);
    }
    // emma: commented out repositioning using scale();
    // translate island to center and set model_view and projection
    model_view = look_at(0,0,width, 0,0,1, 0,1,0);
    projection = frustum(-1,1,-1,1, -1, -100);
    

    // create array of texels, open texture file, and fill array with data
    int tex_width = 64;
    int tex_height = 64;
    GLubyte my_texels[tex_width][tex_height][3];

    FILE *fp = fopen("textures02.raw", "r");
    fread(my_texels, tex_width * tex_height * 3, 1, fp);
    fclose(fp);

    // parameters for texture mapping
    GLuint mytex[1];
    glGenTextures(1, mytex);
    glBindTexture(GL_TEXTURE_2D, mytex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    int param;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);

    GLuint vao;
    #ifdef __APPLE__
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    #else
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    #endif

    // allocating and transfering data to graphic card
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec2) * num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec2) * num_vertices, tex_coords);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (0));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4) * num_vertices));
    
    ctm_location = glGetUniformLocation(program, "ctm");
    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glPolygonMode(GL_FRONT, GL_FILL);
    // glPolygonMode(GL_BACK, GL_LINE);

    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &curr_trans_matrix);
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);
    
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    //printf("%c %i %i ", key, mousex, mousey);
    if(key == 'q')
    	exit(0);
    // foward
    if(key == 'w') {

    }
    // slide left
    if(key == 'a') {

    }
    // backward
    if(key == 's') {

    }
    // slide right
    if(key == 'd') {

    }
    // look right
    if(key == 'k') {

    }
    // look left
    if(key == 'j') {

    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        x_coor = (x * 2 / 511.0) - 1;
        y_coor = 1 - (y * 2 / 511.0);
        if (x_coor * x_coor + y_coor * y_coor <= 1)
            z_coor = sqrt(1.0 - (x_coor * x_coor + y_coor * y_coor));
        else
            return;
    }

    glutPostRedisplay();
}

void motion(int x, int y)
{
    float x_coordinate = (x * 2 / 511.0) - 1;
    float y_coordinate = 1 - (y * 2 / 511.0);
    float z_coordinate;
    if (x_coordinate * x_coordinate + y_coordinate * y_coordinate <= 1)
        z_coordinate = sqrt(1.0 - (x_coordinate * x_coordinate + y_coordinate * y_coordinate));
    else
        return;
    if (x_coordinate > 1)
        x_coordinate = 1;
    if (x_coordinate < -1)
        x_coordinate = -1;
    if (y_coordinate > 1)
        y_coordinate = 1;
    if (y_coordinate < -1)
        y_coordinate = -1;

    // printf("%f\n", z_coordinate);
    vec4 fixed_point = {0.0, 0.0, 0.0, 1.0};
    vec4 p1 = {x_coor, y_coor, z_coor, 1.0};
    vec4 p2 = {x_coordinate, y_coordinate, z_coordinate, 1.0};

    vec4 u = vec_sub(p1, fixed_point);
    vec4 v = vec_sub(p2, fixed_point);

    // vector u x v is the about vector (cross product to get vector)
    vec4 about_vec = cross_prod(u, v);
    about_vec = normalize(magnitude(about_vec), about_vec);

    // θ = cos-1 [ (a * b) / (|a| * |b|) ]
    float dot_product = dot_prod(u, v);
    float u_magnitude = magnitude(u);
    float v_magnitude = magnitude(v);
    // make sure that there is no division by 0!
    if (u_magnitude == 0 || v_magnitude == 0)
        return;
    float m = u_magnitude * v_magnitude;
    float theta;
    // need to check that acosf is valid and will not be nan
    if ((dot_product / m) > 1.0) 
        theta = acosf(1.0) * 180.0 / M_PI;
    else if ((dot_product / m) < -1.0)
        theta = acosf(-1.0) * 180.0 / M_PI;
    else
        theta = acosf(dot_product / m) * 180.0 / M_PI;
    mat4 r_z = z_rotate(theta);

    // R = R𝑥 (−𝜃𝑥 )R𝑦 (𝜃𝑦 )R𝑧 (45)R𝑦 (−𝜃𝑦 )R𝑥 (𝜃𝑥 )
    float d = sqrt((about_vec.y * about_vec.y) + (about_vec.z * about_vec.z));
    if ((about_vec.y * about_vec.y) + (about_vec.z * about_vec.z) < 0) 
        return;
    if (d == 0)
        return;
    mat4 r_x = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, (about_vec.z / d), (about_vec.y / d), 0.0},
        {0.0, -(about_vec.y / d), (about_vec.z / d), 0.0},
        {0.0, 0.0, 0.0, 1.0}};
    mat4 transpose_r_x = transpose(r_x);

    mat4 r_y = {
        {(d), 0.0, (about_vec.x), 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {-(about_vec.x), 0.0, (d), 0.0},
        {0.0, 0.0, 0.0, 1.0}};
    mat4 transpose_r_y = transpose(r_y);

    mat4 rotation_matrix = mat_mult(transpose_r_x, mat_mult(transpose_r_y, mat_mult(r_z, mat_mult(r_y, r_x))));
    curr_trans_matrix = mat_mult(rotation_matrix, curr_trans_matrix);

   //print_m4(curr_trans_matrix);

    x_coor = x_coordinate;
    y_coor = y_coordinate;
    z_coor = z_coordinate;
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    printf("Enter length: ");
    scanf("%d", &length);
    printf("Enter width: ");
    scanf("%d", &width);
    //printf("%d", width);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Project 2/3");
    #ifndef __APPLE__
    glewInit();
    #endif
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();

    return 0;
}
