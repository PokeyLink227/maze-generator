#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bmp.h"
#include "color.h"

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
    byte visited, next_dir, walls[6];
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

typedef struct image_options {
    color_rgb fgcolor, bgcolor;
    char *output_file;
    int wall_width, passage_width;
} image_options;

typedef struct Stack {
    int max_size, top_element, *elements;
} Stack;

byte create_stack(Stack *s, int size) {
    s->elements = (int *)malloc(sizeof(int) * size);
    s->max_size = size;
    s->top_element = -1;
    return s->elements != 0;
}

void free_stack(Stack *s) {
    free(s->elements);
    return;
}

byte push(Stack *s, int val) {
    if (s->top_element == s->max_size - 1) return 0;
    s->elements[++s->top_element] = val;
    return 1;
}

int pop(Stack *s) {
    if (s->top_element == -1) return 0;
    return s->elements[s->top_element--];
}

int peek(Stack *s) {
    if (s->top_element == -1) return 0;
    return s->elements[s->top_element];
}

byte stack_is_empty(Stack *s) {
    return s->top_element == 0;
}

byte grid_contains(Vector3 dim, int pt, byte dir) {
    int x = pt % dim.x,
        y = pt / dim.x % dim.y,
        z = pt / dim.x / dim.y;

    switch (dir) {
        case NORTH: return y > 0;
        case EAST:  return x < dim.x - 1;
        case UP:    return z < dim.z - 1;
        case SOUTH: return y < dim.y - 1;
        case WEST:  return x > 0;
        case DOWN:  return z > 0;
        default:    return 0;
    }
}

/*
https://www.astrolog.org/labyrnth/algrithm.htm
growing tree
hunt and kill
*/

Node *maze_growingtree(Vector3 dim) {
    int num_nodes = dim.x * dim.y, visited_nodes = 0, current_node = 0;
    Node *nodes = malloc(sizeof(Node) * num_nodes);
    int direction_offsets[6] = {
        -dim.x,        /*NORTH*/
        1,             /*EAST*/
        dim.x * dim.y, /*UP*/
        dim.x,         /*SOUTH*/
        -1,            /*WEST*/
        -dim.x * dim.y /*DOWN*/
    };
    byte available_directions[6], num_available_dirs, selected_dir = 0;

    for (int i = 0; i < num_nodes; i++) nodes[i] = (Node){0, 0, {1, 1, 1, 1, 1, 1}};
    Stack visited;
    create_stack(&visited, num_nodes);

    while (visited_nodes < num_nodes) {

    }

    free_stack(&visited);
    return nodes;
}

Node *maze_backtrack(Vector3 dim) {
    int num_nodes = dim.x * dim.y * dim.z, visited_nodes = 0, current_node = 0;
    Node *nodes = malloc(sizeof(Node) * num_nodes);
    int direction_offsets[6] = {
        -dim.x,        /*NORTH*/
        1,             /*EAST*/
        dim.x * dim.y, /*UP*/
        dim.x,         /*SOUTH*/
        -1,            /*WEST*/
        -dim.x * dim.y /*DOWN*/
    };
    byte available_directions[6], num_available_dirs, selected_dir = 0;

    for (int i = 0; i < num_nodes; i++) nodes[i] = (Node){0, 0, {1, 1, 1, 1, 1, 1}};
    Stack visited;
    create_stack(&visited, num_nodes);

    while (visited_nodes < num_nodes) {
        if (!nodes[current_node].visited) {
            nodes[current_node].visited = VISITED;
            visited_nodes++;
            push(&visited, current_node);
        }

        num_available_dirs = 0;
        for (byte i = 0; i < 6; i++)
            if (grid_contains(dim, current_node, i) && !nodes[current_node + direction_offsets[i]].visited)
                available_directions[num_available_dirs++] = i;

        if (num_available_dirs > 0) {
            selected_dir = available_directions[rand() % num_available_dirs];
            nodes[current_node].walls[selected_dir] = 0;
            current_node += direction_offsets[selected_dir];
            nodes[current_node].walls[(selected_dir + 3) % 6] = 0;
        } else {
            pop(&visited);
            current_node = peek(&visited);
        }
    }

    free_stack(&visited);
    return nodes;
}

Node *maze_wilson(Vector3 dim) {
    int num_nodes = dim.x * dim.y * dim.z, visited_nodes = 0, start_node = 0, current_node;
    Node *nodes = malloc(sizeof(Node) * num_nodes);
    int direction_offsets[6] = {
        -dim.x,        /*NORTH*/
        1,             /*EAST*/
        dim.x * dim.y, /*UP*/
        dim.x,         /*SOUTH*/
        -1,            /*WEST*/
        -dim.x * dim.y /*DOWN*/
    };
    byte available_directions[6], num_available_dirs, selected_dir = 0;

    for (int i = 0; i < num_nodes; i++) nodes[i] = (Node){0, 0, {1, 1, 1, 1, 1, 1}};
    nodes[0].visited = INMAZE;
    visited_nodes++;

    while (visited_nodes < num_nodes) {
        while (nodes[start_node].visited == INMAZE) start_node++;
        current_node = start_node;
        while (nodes[current_node].visited != INMAZE) {
            num_available_dirs = 0;
            for (byte i = 0; i < 6; i++) if (grid_contains(dim, current_node, i)) available_directions[num_available_dirs++] = i;
            selected_dir = available_directions[rand() % num_available_dirs];
            nodes[current_node].next_dir = selected_dir;
            current_node += direction_offsets[selected_dir];
        }
        current_node = start_node;
        while (nodes[current_node].visited != INMAZE) {
            nodes[current_node].visited = INMAZE;
            selected_dir = nodes[current_node].next_dir;
            nodes[current_node].walls[selected_dir] = 0;
            current_node += direction_offsets[nodes[current_node].next_dir];
            nodes[current_node].walls[(selected_dir + 3) % 6] = 0;
            visited_nodes++;
        }
    }
    return nodes;
}

void generate_image(Vector3 dimensions, Node *data, image_options opt) {
    //int passage_width = 40, wall_width = 10;
    int cell_width = opt.passage_width + opt.wall_width;

    color_rgb up_arrow[1600], down_arrow[1600], multi_arrow[1600];
    for (int i = 0; i < 1600; i++) { up_arrow[i] = opt.fgcolor; down_arrow[i] = opt.fgcolor; multi_arrow[i] = opt.fgcolor; }
    for (int i = 0; i < 40; i++) for (int j = i; j <= i + 40; j += 40) { up_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; down_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; multi_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; }
    for (int i = 1560; i < 1600; i++) for (int j = i; j >= i - 40; j -= 40) { up_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; down_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; multi_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; }
    for (int i = 0; i < 1560; i += 40) for (int j = i; j <= i + 1; j++) { up_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; down_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; multi_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; }
    for (int i = 39; i < 1600; i += 40) for (int j = i; j >= i - 1; j--) { up_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; down_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; multi_arrow[j] = (color_rgb){0x80, 0x80, 0x80}; }

    for (int i = 320; i < 1280; i += 40) for (int j = i + 18; j < i + 22; j++) { up_arrow[j] = opt.bgcolor; down_arrow[j] = opt.bgcolor; multi_arrow[j] = opt.bgcolor; }

    for (int len = 4; len >= 0; len--) for (int i = 0; i < (5 - len) * 2; i++) {
        up_arrow[280 + (4 - len) * 40 + i + 15 + len] = opt.bgcolor;
        down_arrow[1120 + (len) * 40 + i + 15 + len] = opt.bgcolor;
        multi_arrow[280 + (4 - len) * 40 + i + 15 + len] = opt.bgcolor;
        multi_arrow[1120 + (len) * 40 + i + 15 + len] = opt.bgcolor;
    }

    Vector3 image_dimensions = (Vector3){(dimensions.x * cell_width + opt.wall_width), (dimensions.y * cell_width + opt.wall_width), (dimensions.x * cell_width + opt.wall_width) * dimensions.z};
    color_rgb *pixels = (color_rgb *)malloc(sizeof(color_rgb) * image_dimensions.z * image_dimensions.y);

    for (int i = 0; i < image_dimensions.z * image_dimensions.y; i++) pixels[i] = opt.bgcolor;

    for (int z = 0; z < dimensions.z; z++) for (int y = 0; y < dimensions.y; y++) for (int x = 0; x < dimensions.x; x++) {
        if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[UP] && !data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[DOWN])for (int off_y = 0; off_y < opt.passage_width; off_y++) for (int off_x = 0; off_x < opt.passage_width; off_x++) pixels[(y * cell_width + opt.wall_width + off_y) * (image_dimensions.z) + (x * cell_width + opt.wall_width + off_x) + (z * image_dimensions.x)] = multi_arrow[off_x + off_y * 40];
        else if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[UP]) for (int off_y = 0; off_y < opt.passage_width; off_y++) for (int off_x = 0; off_x < opt.passage_width; off_x++) pixels[(y * cell_width + opt.wall_width + off_y) * (image_dimensions.z) + (x * cell_width + opt.wall_width + off_x) + (z * image_dimensions.x)] = up_arrow[off_x + off_y * 40];
        else if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[DOWN]) for (int off_y = 0; off_y < opt.passage_width; off_y++) for (int off_x = 0; off_x < opt.passage_width; off_x++) pixels[(y * cell_width + opt.wall_width + off_y) * (image_dimensions.z) + (x * cell_width + opt.wall_width + off_x) + (z * image_dimensions.x)] = down_arrow[off_x + off_y * 40];
        else for (int off_y = 0; off_y < opt.passage_width; off_y++) for (int off_x = 0; off_x < opt.passage_width; off_x++) pixels[(y * cell_width + opt.wall_width + off_y) * (image_dimensions.z) + (x * cell_width + opt.wall_width + off_x) + (z * image_dimensions.x)] = opt.fgcolor;

        if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[SOUTH]) for (int off_y = 0; off_y < opt.wall_width; off_y++) for (int off_x = 0; off_x < opt.passage_width; off_x++) pixels[((y + 1) * cell_width + off_y) * (image_dimensions.z) + (x * cell_width + opt.wall_width + off_x) + (z * image_dimensions.x)] = opt.fgcolor;
        if (!data[z * dimensions.x * dimensions.y + y * dimensions.x + x].walls[EAST]) for (int off_y = 0; off_y < opt.passage_width; off_y++) for (int off_x = 0; off_x < opt.wall_width; off_x++) pixels[(y * cell_width + opt.wall_width + off_y) * (image_dimensions.z) + ((x + 1) * cell_width + off_x) + (z * image_dimensions.x)] = opt.fgcolor;
    }

    save_image((bmp_image){image_dimensions.z, image_dimensions.y, pixels}, opt.output_file);
    free(pixels);
}

int matchcmd(char *str, char **cmds, int len) {
    char matched;
    for (int i = 0; i < len; i++) {
        matched = 1;
        for (int c = 0; str[c] || cmds[i][c]; c++) {
            if (cmds[i][c] == '#' && ((str[c] >= '0' && str[c] <= '9') || !str[c])) return i;
            if (cmds[i][c] == '*') return i;
            if (str[c] != cmds[i][c]) { matched = 0; break; }
        }
        if (matched) return i;
    }
    return -1;
}

int main(int argc, char **argv) {
    init_color();

    byte option_timed = 0;
    Vector3 dimensions = {10, 10, 1};
    time_t rand_seed = time(0);
    image_options opt = {
        (color_rgb){0xff, 0xff, 0xff},
        (color_rgb){0x00, 0x00, 0x00},
        "out.bmp",
        1,
        4
    };
    char opt_save_image = 1;

    Node *(* generation_methods[3])(Vector3) = {
        maze_backtrack,
        maze_wilson,
        maze_growingtree
    };


    char *commands[20] = {
        "h",
        "help",
        "d",
        "dim",
        "t",
        "timer",
        "s#",
        "seed#",
        "o*",
        "passagecolor#",
        "pc#",
        "wallcolor#",
        "wc#",
        "wallwidth#",
        "ww#",
        "passagewidth#",
        "pw#",
        "n",
        "method#",
        "m#"
    };

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') switch (matchcmd(argv[i] + 1, commands, 20)) {
        case 0: /*h*/
        case 1: /*help*/
            printf("Usage: maze {options}\nOptions: [] - Required, {} - Optional\n  -help          -h                 Shows this page\n  -dim           -d  [x] [y] {z}    Set custom dimensions for maze\n  -timer         -t                 Enable timer during maze generation\n  -seed          -s  [number]       Set the rng seed\n                 -o  [name]         Set output file name\n  -passagecolor  -pc [r] [g] [b]    Set rgb color of maze passages\n  -wallcolor     -wc [r] [g] [b]    Set rgb color of maze walls\n  -wallwidth     -ww [number]       Set width of walls in pixels\n  -passagewidth  -pw [number]       Set width of passages in pixels\n");
            return 1;
            break;
        case 2: /*d*/
        case 3: /*dim*/
            if (i + 2 < argc && (argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9') && (argv[i + 2][0] >= '0' && argv[i + 2][0] <= '9')) { // two more arguments that are also numbers exist
                dimensions.x = atoi(argv[i + 1]);
                dimensions.y = atoi(argv[i + 2]);
                i += 2;
                if (i + 1 < argc && (argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9')) dimensions.z = atoi(argv[++i]);
            } else {
                printf("\x1b[91mError\x1b[0m: flag -d requires 2-3 integers\n");
                return 1;
            }
            break;
        case 4: /*t*/
        case 5: /*timer*/
            option_timed = 1;
            break;
        case 6: /*s*/
            if (argv[i][2]) rand_seed = atoi(argv[i] + 2);
            else if (i + 1 < argc) rand_seed = atoi(argv[++i]);
            else {
                printf("\x1b[91mError\x1b[0m: flag -s requires an integer\n");
                return 1;
            }
            break;
        case 7: /*seed*/
            if (argv[i][5]) rand_seed = atoi(argv[i] + 5);
            else if (i + 1 < argc) rand_seed = atoi(argv[++i]);
            else {
                printf("\x1b[91mError\x1b[0m: flag -seed requires an integer\n");
                return 1;
            }
            break;
        case 8: /*o*/
            if (argv[i][2]) opt.output_file = argv[i] + 2;
            else if (i + 1 < argc) opt.output_file = argv[++i];
            else {
                printf("\x1b[91mError\x1b[0m: flag -o requires a file name\n");
                return 1;
            }
            break;
        case 9: /*passagecolor*/
        case 10: /*pc*/
            if (i + 3 < argc) {
                opt.fgcolor.red = atoi(argv[i + 1]);
                opt.fgcolor.green = atoi(argv[i + 2]);
                opt.fgcolor.blue = atoi(argv[i + 3]);
                i += 3;
            } else {
                printf("\x1b[91mError\x1b[0m: flag -pc requires 3 integers\n");
                return 1;
            }
            break;
        case 11: /**/
        case 12: /**/
            if (i + 3 < argc) {
                opt.bgcolor.red = atoi(argv[i + 1]);
                opt.bgcolor.green = atoi(argv[i + 2]);
                opt.bgcolor.blue = atoi(argv[i + 3]);
                i += 3;
            } else {
                printf("\x1b[91mError\x1b[0m: flag -wc requires 3 integers\n");
                return 1;
            }
            break;
        case 13: /**/
            if (i + 1 < argc) {
                opt.wall_width = atoi(argv[i + 1]);
            } else {
                printf("\x1b[91mError\x1b[0m: flag -wallwidth requires an integer\n");
                return 1;
            }
            break;
        case 14: /**/
            if (argv[i][3]) opt.wall_width = atoi(argv[i] + 3);
            else if (i + 1 < argc) opt.wall_width = atoi(argv[i + 1]);
            else {
                printf("\x1b[91mError\x1b[0m: flag -ww requires an integer\n");
                return 1;
            }
            break;
        case 15: /**/
            if (i + 1 < argc) {
                opt.passage_width = atoi(argv[i + 1]);
            } else {
                printf("\x1b[91mError\x1b[0m: flag -pw requires an integer\n");
                return 1;
            }
            break;
        case 16: /**/
            if (argv[i][3]) opt.passage_width = atoi(argv[i] + 3);
            else if (i + 1 < argc) opt.passage_width = atoi(argv[i + 1]);
            else {
                printf("\x1b[91mError\x1b[0m: flag -pw requires an integer\n");
                return 1;
            }
            break;
        case 17: /**/
            opt_save_image = 0;
            break;
        case 18: /**/
        case 19: /**/
        default:
            printf("\x1b[91mError\x1b[0m: unknown flag %s, use -h for help\n", argv[i]);
        }
    }

    if (!dimensions.x || !dimensions.y || !dimensions.z) {
        printf("\x1b[91mError\x1b[0m: Invalid maze dimensions\n");
        return 1;
    }

    if (argc == 1) printf("No options provided, using defaults. Use -h to see help menu\n");
    if (dimensions.z == 1) printf("Generating 2D maze of size {x: %i, y: %i} with seed: %li\n", dimensions.x, dimensions.y, rand_seed);
    else {
        opt.wall_width = 10;
        opt.passage_width = 40;
        printf("Generating 3D maze of size {x: %i, y: %i, z: %i} with seed: %li.  Warning 3D mazes not fully supported yet\n", dimensions.x, dimensions.y, dimensions.z, rand_seed);
    }

    srand(rand_seed);

    struct timespec start, finish, elapse;

    if (option_timed) clock_gettime(CLOCK_REALTIME, &start);

    Node *nodes = maze_backtrack(dimensions);

    if (option_timed) {
        clock_gettime(CLOCK_REALTIME, &finish);
        elapse.tv_sec = finish.tv_sec - start.tv_sec;
        elapse.tv_nsec = finish.tv_nsec - start.tv_nsec;
        if (elapse.tv_sec > 0 && elapse.tv_nsec < 0) elapse.tv_nsec = 1000000000 - elapse.tv_nsec;
        printf("time: %d.%.9ld seconds\n", (int)elapse.tv_sec, elapse.tv_nsec);
    }

    if (opt_save_image) generate_image(dimensions, nodes, opt);

    //for (int i = 0; i < dimensions.x * dimensions.y * dimensions.z; i++) printf("node %i is %s connected [%c, %c, %c, %c, %c, %c]\n", i, (nodes[i].visited ? "visited" : "not visited"), (nodes[i].walls[NORTH] ? '-' : 'N'), (nodes[i].walls[SOUTH] ? '-' : 'S'), (nodes[i].walls[EAST] ? '-' : 'E'), (nodes[i].walls[WEST] ? '-' : 'W'), (nodes[i].walls[UP] ? '-' : 'U'), (nodes[i].walls[DOWN] ? '-' : 'D'));

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
-add option to suppress output

east = index + 1
west = index - 1
south = index + x
north = index - x
up = index + x * y
down = index - x * y

printf("%c, %c, %c, %c, %c, %c, %c, %c, %c, %c, %c\n", 179, 180, 191, 192, 193, 194, 195, 196, 197, 217, 218);

*/
