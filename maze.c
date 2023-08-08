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

enum status {
    UNVISITED,
    VISITED,
    INMAZE
};

typedef unsigned char byte;

typedef struct Node {
    byte visited, parent_dir, walls[6];
    int parent;
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

Vector3 vecsub(Vector3 lhs, Vector3 rhs) {
    return (Vector3){lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
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

Node *create_maze_basic(Vector3 dimensions, Vector3 *exclusions) {
    int num_nodes = dimensions.x * dimensions.y * dimensions.z;
    Node *nodes = (Node *)malloc(sizeof(Node) * num_nodes);
    // set visited to 0 and all walls to active (1)
    for (int i = 0; i < num_nodes; i++) nodes[i] = (Node){0, 0, {1, 1, 1, 1, 1, 1}, -1};

    box bounding_box = (box){(Vector3){0, 0, 0}, dimensions};

    Vector3 directions_v[6] = {
        {0, -1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, -1},
    };

    // in order to add rooms, set nodes to visited and increment starting num_visited value. to save memory you can also create a smaller stack and decrement num_nodes

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

/*
create a maze using wilson's algorithm
*/
Node *create_maze_wilson(Vector3 dimensions, Vector3 *exclusions) {
    int num_nodes = dimensions.x * dimensions.y * dimensions.z;
    Node *nodes = (Node *)malloc(sizeof(struct Node) * num_nodes);
    Vector3 *open = (Vector3 *)malloc(sizeof(struct Vector3) * num_nodes); // stores node positions
    int *open_pos = (int *)malloc(sizeof(int) * num_nodes); // stores index of node position in open list
    for (int i = 0; i < num_nodes; i++) {
        nodes[i] = (Node){0, 0, {1, 1, 1, 1, 1, 1}, -1};
        open[i] = (Vector3){i % dimensions.x, i / dimensions.x % dimensions.y, i / (dimensions.x * dimensions.y)};
        open_pos[i] = i;
    }

    box bounding_box = (box){(Vector3){0, 0, 0}, dimensions};
    Vector3 directions_v[6] = {
        {0, -1, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, -1},
    };

    int open_len = num_nodes, current_node_index, next_node_index;

    int n = rand() % open_len;
    open[n] = open[--open_len]; // delete node n from list found at index open_pos[n]
    open_pos[open_len] = n;
    nodes[n].visited = INMAZE;

    byte dirs[6];
    byte num_dirs, selected_dir;

    Vector3 current_node;

    while (open_len > 0) {
        // select an unvisited node

        do {
            n = rand() % open_len;
            current_node = open[n];
            open[n] = open[--open_len];
            open_pos[pos_in_array(open[open_len], dimensions)] = n;
            current_node_index = pos_in_array(current_node, dimensions);
        } while (nodes[current_node_index].visited == INMAZE);
        nodes[current_node_index].parent = -1;
        // make a loop-erased random walk to the first node in the maze
        while (1) {
            nodes[current_node_index].visited = VISITED;
            // select valid direction
            num_dirs = 0;
            for (byte i = 0; i < 6; i++) if (contains(bounding_box, vecadd(current_node, directions_v[i]))) dirs[num_dirs++] = i;
            selected_dir = dirs[rand() % num_dirs];
            next_node_index = pos_in_array(vecadd(current_node, directions_v[selected_dir]), dimensions);

            if (nodes[next_node_index].visited == VISITED) {
                int fallback = next_node_index;
                // set all nodes to unvisited until reaching the next node index again
                while (current_node_index != fallback) {
                    nodes[current_node_index].visited = UNVISITED;
                    current_node_index = nodes[current_node_index].parent;
                }
                current_node = (Vector3){current_node_index % dimensions.x, current_node_index / dimensions.x % dimensions.y, current_node_index / (dimensions.x * dimensions.y)};
            } else {
                nodes[next_node_index].parent = current_node_index;
                nodes[next_node_index].parent_dir = (selected_dir + 3) % 6;
                current_node = vecadd(current_node, directions_v[selected_dir]);
                current_node_index = next_node_index;
            }

            if (nodes[next_node_index].visited == INMAZE) break;
        }

        // trace and add visited nodes to maze
        int length = 0;
        while (1) {
            length++;
            nodes[current_node_index].visited = INMAZE;
            nodes[current_node_index].walls[nodes[current_node_index].parent_dir] = 0;
            nodes[nodes[current_node_index].parent].walls[(nodes[current_node_index].parent_dir + 3) % 6] = 0;
            current_node_index = nodes[current_node_index].parent;

            if (nodes[current_node_index].parent == -1) {
                nodes[current_node_index].visited = INMAZE;
                break;
            }

            open[open_pos[current_node_index]] = open[--open_len];
            open_pos[pos_in_array(open[open_len], dimensions)] = open_pos[current_node_index];
        }
    }
    return nodes;
}

void generate_image(Vector3 dimensions, Node *data, char *file_name) {
    int passage_width = 40, wall_width = 10;
    int cell_width = passage_width + wall_width;

    Vector3 image_dimensions = (Vector3){(dimensions.x * cell_width + wall_width), (dimensions.y * cell_width + wall_width), (dimensions.x * cell_width + wall_width) * dimensions.z};
    color_rgb *pixels = (color_rgb *)malloc(sizeof(color_rgb) * image_dimensions.z * image_dimensions.y);

    for (int i = 0; i < image_dimensions.z * image_dimensions.y; i++) pixels[i] = (color_rgb){0x00, 0x00, 0x00};


    for (int z = 0; z < dimensions.z; z++) for (int y = 0; y < dimensions.y; y++) for (int x = 0; x < dimensions.x; x++) {
        for (int off_y = 0; off_y < passage_width; off_y++) for (int off_x = 0; off_x < passage_width; off_x++) pixels[(y * cell_width + wall_width + off_y) * (image_dimensions.z) + (x * cell_width + wall_width + off_x) + (z * image_dimensions.x)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[SOUTH]) for (int off_y = 0; off_y < wall_width; off_y++) for (int off_x = 0; off_x < passage_width; off_x++) pixels[((y + 1) * cell_width + off_y) * (image_dimensions.z) + (x * cell_width + wall_width + off_x) + (z * image_dimensions.x)] = (color_rgb){0xff, 0xff, 0xff};
        if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[EAST]) for (int off_y = 0; off_y < passage_width; off_y++) for (int off_x = 0; off_x < wall_width; off_x++) pixels[(y * cell_width + wall_width + off_y) * (image_dimensions.z) + ((x + 1) * cell_width + off_x) + (z * image_dimensions.x)] = (color_rgb){0xff, 0xff, 0xff};
    }

    byte *header, *pixel_array;

    header = generate_header(image_dimensions.z, image_dimensions.y);
    pixel_array = generate_pixel_array(header, pixels);
    export_image(header, pixel_array, file_name);

}

int main(int argc, char **argv) {

    byte option_timed = 0;
    Vector3 dimensions = {10, 10, 1};
    char *output_file_name = "out.bmp";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != 0 && argv[i][2] == 0) { // argument is valid flag - [char] \0
            switch (argv[i][1]) {
                case 'h':
                    printf("Usage: maze {options}\nOptions: [] - Required, {} - Optional\n  -h                Shows this page\n  -d [x] [y] {z}    Set custom dimensions for maze\n  -t                Enable timer during maze generation\n  -o [name]         Set output file name\n  -f [format]       Set output image format\n  -m [name]         Set method for maze generation");
                    return 0;
                    break;
                case 'd': // set dimensions
                    if (i + 2 < argc && (argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9') && (argv[i + 2][0] >= '0' && argv[i + 2][0] <= '9')) { // two more arguments that are also numbers exist
                        dimensions.x = atoi(argv[i + 1]);
                        dimensions.y = atoi(argv[i + 2]);
                        i += 2;
                        if (i + 1 < argc && (argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9')) dimensions.z = atoi(argv[++i]);
                    } else {
                        printf("Invalid argument list for flag -d\n");
                        return 1;
                    }
                    break;
                case 't': // enable timer
                    option_timed = 1;
                    break;
                case 'o': // set output file
                    if (i + 1 < argc) {
                        output_file_name = argv[++i];
                    } else {
                        printf("Missing output file name\n");
                        return 1;
                    }
                    break;
                case 'f': // set image format
                // add options for setting fg bg color and changing cell width
                default:
                    printf("Unknown flag %c. use -h for help\n", argv[i][1]);
            }
        } else {
            printf("invalid flag %s\n", argv[i]);
            return 1;
        }
    }


    if (dimensions.z == 1) printf("Generating 2D maze of size {x: %i, y: %i}\n", dimensions.x, dimensions.y);
    else printf("Generating 3D maze of size {x: %i, y: %i, z: %i}.  Warning 3D mazes not fully supported yet\n", dimensions.x, dimensions.y, dimensions.z);

    srand(time(0));

    clock_t start, stop;

    if (option_timed) start = clock();

    Node *nodes = create_maze_basic(dimensions, 0);

    if (option_timed) {
        stop = clock();
        printf("time: %ld\n", stop - start);
    }


    generate_image(dimensions, nodes, output_file_name);


    //for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) printf("node %i is %s connected [%c, %c, %c, %c, %c, %c]\n", i, (nodes[i].visited ? "visited" : "not visited"), (nodes[i].walls[NORTH] ? '-' : 'N'), (nodes[i].walls[SOUTH] ? '-' : 'S'), (nodes[i].walls[EAST] ? '-' : 'E'), (nodes[i].walls[WEST] ? '-' : 'W'), (nodes[i].walls[UP] ? '-' : 'U'), (nodes[i].walls[DOWN] ? '-' : 'D'));

    //generate_image(dimensions, nodes, "mz2.bmp");

    return 0;
}

/*
TODO:
-try to maybe get rid of the vectors during generation
-add more command line options
-improve command parsing
-add indication for 3d level change
-add indication for start and end of mazes
-add support for maze exclusions/rooms

east = index + 1
west = index - 1
south = index + x
north = index - x
up = index + x * y
down = index - x * y

printf("%c, %c, %c, %c, %c, %c, %c, %c, %c, %c, %c\n", 179, 180, 191, 192, 193, 194, 195, 196, 197, 217, 218);

*/
