#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bmp.h"

enum direction {
    NORTH,
    EAST,
    UP,
    SOUTH,
    WEST,
    DOWN
};

typedef unsigned char byte;

typedef struct vector3 {
    int x, y, z;
} vector3;

typedef struct vector2 {
    int x, y;
} vector2;

typedef struct box {
    vector3 position, dimensions;
} box;

vector3 vecadd(vector3 lhs, vector3 rhs) {
    return (vector3){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

typedef struct stack {
    int max_size, top_element;
    vector3 elements[];
} stack;

stack *create_stack(int size) {
    stack *s = (stack *)malloc(sizeof(stack) + sizeof(vector3) * size);
    s->max_size = size;
    s->top_element = -1;
    return s;
}

byte push(stack *s, vector3 data) {
    if (s->top_element == s->max_size - 1) return 0;
    s->elements[++s->top_element] = data;
    return 1;
}

vector3 pop(stack *s) {
    if (s->top_element == -1) return (vector3){0, 0, 0};
    return s->elements[s->top_element--];
}

vector3 peek(stack *s) {
    if (s->top_element == -1) return (vector3){0, 0, 0};
    return s->elements[s->top_element];
}

byte is_empty(stack *s) {
    return s->top_element == 0;
}

int pos_in_array(vector3 pos, vector3 dimensions) {
    return (pos.x + pos.y * dimensions.x + pos.z * dimensions.x * dimensions.y);
}

byte contains(box box, vector3 point) {
    if (
        point.x >= box.dimensions.x || point.x < box.position.x ||
        point.y >= box.dimensions.y || point.y < box.position.y ||
        point.z >= box.dimensions.z || point.z < box.position.z
    ) return 0;
    return 1;
}

byte *create_maze(vector3 dimensions, vector3 *exclusions) {
    int num_nodes = dimensions.x * dimensions.y * dimensions.z;

    box bounding_box = (box){(vector3){0, 0, 0}, dimensions};

    vector3 directions_v[6] = {
        {0, -1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, -1},
    };
    char c[6] = {'N', 'E', 'U', 'S', 'W', 'D'};

    byte *nodes = (byte *)malloc(sizeof(byte) * 7 * num_nodes);
    // set visited to 0 and all walls to active (1)
    for (int i = 0; i < num_nodes * 7; i++) {
        if (i % 7 == 0) nodes[i] = 0;
        else nodes[i] = 1;
    }

    stack *visited = create_stack(num_nodes);

    int num_visited = 0;
    vector3 pos = {0, 0, 0};
    byte dirs[6];
    byte num_dirs, selected_dir;

    while (num_visited < num_nodes) {
        //mark current node as visited
        //printf("%i nodes visited\n", num_visited);

        if (nodes[pos_in_array(pos, dimensions) * 7] == 0) {
            num_visited++;
            nodes[pos_in_array(pos, dimensions) * 7] = 1;
            push(visited, pos);
        }

        // find next node to travel to
        // get list of all nodes that can be traveled to and pick a random one
        num_dirs = 0;
        for (byte i = 0; i < 6; i++)
            if (contains(bounding_box, vecadd(pos, directions_v[i])) && !nodes[pos_in_array(vecadd(pos, directions_v[i]), dimensions) * 7]) {
                dirs[num_dirs++] = i;
            }

        //printf("%i\n", num_dirs);
        if (num_dirs > 0) { // a new path is available

            selected_dir = dirs[rand() % num_dirs];

            // set walls to 0 bewteen 2 nodes
            //printf("connecting %c from (%i, %i, %i) to (%i, %i, %i)\n", c[selected_dir], pos.x, pos.y, pos.z, vecadd(pos, directions_v[selected_dir]).x, vecadd(pos, directions_v[selected_dir]).y, vecadd(pos, directions_v[selected_dir]).z);
            nodes[pos_in_array(pos, dimensions) * 7 + selected_dir + 1] = 0;
            pos = vecadd(pos, directions_v[selected_dir]);
            nodes[pos_in_array(pos, dimensions) * 7 + (selected_dir + 3) % 6 + 1] = 0;
        }
        else {
            pop(visited);
            pos = peek(visited);
            //printf("backtracking to (%i, %i, %i)\n", pos.x, pos.y, pos.z);
        }
    }

    return nodes;
}

void generate_image(vector3 dimensions, byte *data) {

    //create a grid of size (x * 4 + 1) * (y * 4 + 1) * (z + y * 3)
    int num_nudes = dimensions.x * dimensions.y * dimensions.z;
    vector2 image_dimensions = (vector2){(dimensions.x * 2 + 1), (dimensions.y * 2 + 1)};
    color_rgb *pixels = (color_rgb *)malloc(sizeof(color_rgb) * image_dimensions.x * image_dimensions.y);

    for (int i = 0; i < image_dimensions.x * image_dimensions.y; i++) pixels[i] = (color_rgb){0x00, 0x00, 0x00};


    for (int y = 0; y < dimensions.y; y++) for (int x = 0; x < dimensions.x; x++) {
        pixels[(y * 2 + 1) * (image_dimensions.x) + (x * 2 + 1)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[(y * dimensions.x + x) * 7 + 1 + SOUTH]) pixels[(y * 2 + 2) * (image_dimensions.x) + (x * 2 + 1)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[(y * dimensions.x + x) * 7 + 1 + EAST]) pixels[(y * 2 + 1) * (image_dimensions.x) + (x * 2 + 2)] = (color_rgb){0xff, 0xff, 0xff};
    }

    byte *header, *pixel_array;

    header = generate_header(image_dimensions.x, image_dimensions.y);
    pixel_array = generate_pixel_array(header, pixels);
    export_image(header, pixel_array);

}



int main(int argc, char **argv) {

    printf("%c, %c, %c, %c, %c, %c, %c, %c, %c, %c, %c\n", 179, 180, 191, 192, 193, 194, 195, 196, 197, 217, 218);

    int t = time(0);
    //printf("seed: %i\n", t);
    srand(1982739873);
    vector3 dimensions = {100, 100, 1};

    byte *nodes = create_maze(dimensions, 0);
    /*
    for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) {
        printf("node %i is %s connected [%c, %c, %c, %c, %c, %c]\n", i, (nodes[i * 7] ? "visited" : "not visited"), (nodes[i * 7 + 1 + NORTH] ? '-' : 'N'), (nodes[i * 7 + 1 + SOUTH] ? '-' : 'S'), (nodes[i * 7 + 1 + EAST] ? '-' : 'E'), (nodes[i * 7 + 1 + WEST] ? '-' : 'W'), (nodes[i * 7 + 1 + UP] ? '-' : 'U'), (nodes[i * 7 + 1 + DOWN] ? '-' : 'D'));
    }
    */
    generate_image(dimensions, nodes);

}

/*

store walls in adjency matrix
initalize all nodes to be unconnected
x - 6, y - 3, z - 2
x x x x x x
x x x x x x
x x x x x x

x x x x x x
x x x x x x
x x x x x x

east = index + 1
west = index - 1
south = index + x
north = index - x
up = index + x * y
down = index - x * y

length of array will be x * y * z

entrance defind

*/
