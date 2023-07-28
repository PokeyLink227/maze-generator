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

typedef struct Node {
    byte visited, walls[6];
} Node;

typedef struct Vector3 {
    int x, y, z;
} Vector3;

typedef struct Vector2 {
    int x, y;
} Vector2;

typedef struct box {
    Vector3 position, dimensions;
} box;

Vector3 vecadd(Vector3 lhs, Vector3 rhs) {
    return (Vector3){lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

typedef struct V3Stack {
    int max_size, top_element;
    Vector3 elements[];
} V3Stack;

V3Stack *create_stack(int size) {
    V3Stack *s = (V3Stack *)malloc(sizeof(V3Stack) + sizeof(Vector3) * size);
    s->max_size = size;
    s->top_element = -1;
    return s;
}

byte Push(V3Stack *s, Vector3 data) {
    if (s->top_element == s->max_size - 1) return 0;
    s->elements[++s->top_element] = data;
    return 1;
}

Vector3 Pop(V3Stack *s) {
    if (s->top_element == -1) return (Vector3){0, 0, 0};
    return s->elements[s->top_element--];
}

Vector3 Peek(V3Stack *s) {
    if (s->top_element == -1) return (Vector3){0, 0, 0};
    return s->elements[s->top_element];
}

byte is_empty(V3Stack *s) {
    return s->top_element == 0;
}

int pos_in_array(Vector3 pos, Vector3 dimensions) {
    return (pos.x + pos.y * dimensions.x + pos.z * dimensions.x * dimensions.y);
}

byte contains(box box, Vector3 point) {
    if (
        point.x >= box.dimensions.x || point.x < box.position.x ||
        point.y >= box.dimensions.y || point.y < box.position.y ||
        point.z >= box.dimensions.z || point.z < box.position.z
    ) return 0;
    return 1;
}

Node *create_maze(Vector3 dimensions, Vector3 *exclusions) {
    int num_nodes = dimensions.x * dimensions.y * dimensions.z;

    box bounding_box = (box){(Vector3){0, 0, 0}, dimensions};

    Vector3 directions_v[6] = {
        {0, -1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, -1},
    };
    char c[6] = {'N', 'E', 'U', 'S', 'W', 'D'};

    Node *nodes = (Node *)malloc(sizeof(Node) * num_nodes);
    // set visited to 0 and all walls to active (1)
    for (int i = 0; i < num_nodes; i++) nodes[i] = (Node){0, {1, 1, 1, 1, 1, 1}};

    V3Stack *visited = create_stack(num_nodes);

    int num_visited = 0;
    Vector3 pos = {0, 0, 0};
    byte dirs[6];
    byte num_dirs, selected_dir;

    while (num_visited < num_nodes) {
        //mark current node as visited

        if (nodes[pos_in_array(pos, dimensions)].visited == 0) {
            num_visited++;
            nodes[pos_in_array(pos, dimensions)].visited = 1;
            Push(visited, pos);
        }

        // find next node to travel to
        // get list of all nodes that can be traveled to and pick a random one
        num_dirs = 0;
        for (byte i = 0; i < 6; i++) if (contains(bounding_box, vecadd(pos, directions_v[i])) && !nodes[pos_in_array(vecadd(pos, directions_v[i]), dimensions)].visited) dirs[num_dirs++] = i;

        if (num_dirs > 0) { // a new path is available
            selected_dir = dirs[rand() % num_dirs];

            // set walls to 0 bewteen 2 nodes
            //printf("connecting %c from (%i, %i, %i) to (%i, %i, %i)\n", c[selected_dir], pos.x, pos.y, pos.z, vecadd(pos, directions_v[selected_dir]).x, vecadd(pos, directions_v[selected_dir]).y, vecadd(pos, directions_v[selected_dir]).z);
            nodes[pos_in_array(pos, dimensions)].walls[selected_dir] = 0;
            pos = vecadd(pos, directions_v[selected_dir]);
            nodes[pos_in_array(pos, dimensions)].walls[(selected_dir + 3) % 6] = 0;
        }
        else {
            Pop(visited);
            pos = Peek(visited);
            //printf("backtracking to (%i, %i, %i)\n", pos.x, pos.y, pos.z);
        }
    }
    return nodes;
}


void generate_image(Vector3 dimensions, Node *data) {
    int passage_width = 40, wall_width = 10;
    int cell_width = passage_width + wall_width;
    //create a grid of size (x * 4 + 1) * (y * 4 + 1) * (z + y * 3)
    int num_nudes = dimensions.x * dimensions.y * dimensions.z;
    Vector2 image_dimensions = (Vector2){(dimensions.x * cell_width + wall_width), (dimensions.y * cell_width + wall_width)};
    color_rgb *pixels = (color_rgb *)malloc(sizeof(color_rgb) * image_dimensions.x * image_dimensions.y);

    for (int i = 0; i < image_dimensions.x * image_dimensions.y; i++) pixels[i] = (color_rgb){0x00, 0x00, 0x00};


    for (int y = 0; y < dimensions.y; y++) for (int x = 0; x < dimensions.x; x++) {
        for (int off_y = 0; off_y < passage_width; off_y++) for (int off_x = 0; off_x < passage_width; off_x++) pixels[(y * cell_width + wall_width + off_y) * (image_dimensions.x) + (x * cell_width + wall_width + off_x)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[y * dimensions.x + x].walls[SOUTH]) for (int off_y = 0; off_y < wall_width; off_y++) for (int off_x = 0; off_x < passage_width; off_x++) pixels[((y + 1) * cell_width + off_y) * (image_dimensions.x) + (x * cell_width + wall_width + off_x)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[y * dimensions.x + x].walls[EAST]) for (int off_y = 0; off_y < passage_width; off_y++) for (int off_x = 0; off_x < wall_width; off_x++) pixels[(y * cell_width + wall_width + off_y) * (image_dimensions.x) + ((x + 1) * cell_width + off_x)] = (color_rgb){0xff, 0xff, 0xff};
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
    Vector3 dimensions = {20, 20, 1};

    Node *nodes = create_maze(dimensions, 0);

    /*
    for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) {
        printf("node %i is %s connected [%c, %c, %c, %c, %c, %c]\n", i, (nodes[i].visited ? "visited" : "not visited"), (nodes[i].walls[NORTH] ? '-' : 'N'), (nodes[i].walls[SOUTH] ? '-' : 'S'), (nodes[i].walls[EAST] ? '-' : 'E'), (nodes[i].walls[WEST] ? '-' : 'W'), (nodes[i].walls[UP] ? '-' : 'U'), (nodes[i].walls[DOWN] ? '-' : 'D'));
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
