#include "renderer.h"
#include "shapes.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define FRAME_DELAY 1000
#define SCREEN_WIDTH gfx_screenWidth()
#define SCREEN_HEIGHT gfx_screenHeight()

#define GAME_WIDTH 10
#define GAME_HEIGHT (20 + SHAPE_SIZE)
#define FALLING_SPEED 2

#define SQUARE_SIZE (SCREEN_HEIGHT / (GAME_HEIGHT + 1))
#define ORGIN_X (SCREEN_WIDTH / 2 - (GAME_WIDTH * SQUARE_SIZE / 2))
#define ORGIN_Y SQUARE_SIZE

#define BINDED 6
#define BINDED_KEYS \
    (int[BINDED]) { SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_SPACE, SDLK_ESCAPE, SDL_QUIT }
#define KEY_REINPUT_LIMITS \
    (float[BINDED]) { 0.7, 0.7, 5, 2, 0, 0 }

// Overengineered input function. It limits the repeated input frequency:
int get_key()
{
    static int previous_input_time[BINDED] = {0};
    float current_time = ((double)clock() / CLOCKS_PER_SEC) * 100;
    for (int key = 0; key < BINDED; key++)
    {
        if (gfx_isKeyDown(BINDED_KEYS[key]) && current_time - previous_input_time[key] > KEY_REINPUT_LIMITS[key])
        {
            previous_input_time[key] = current_time;
            return BINDED_KEYS[key];
        }
    }
    return -1;
}

// Tretromino definition and functions to handle it:
typedef struct
{
    int shape[SHAPE_SIZE][SHAPE_SIZE];
    int color;
    int type;
    int x;
    int y;
} tetromino;

void copy_shape(int from[SHAPE_SIZE][SHAPE_SIZE], int to[SHAPE_SIZE][SHAPE_SIZE])
{
    int x, y;
    for (y = 0; y < SHAPE_SIZE; y++)
    {
        for (x = 0; x < SHAPE_SIZE; x++)
        {
            to[x][y] = from[x][y];
        }
    }
}

void make_tetromino(tetromino *t, int type)
{
    copy_shape(SHAPES[type], t->shape);
    t->color = type + 1;
    t->type = type;
    t->x = GAME_WIDTH / 2 - SHAPE_SIZE / 2;
    t->y = 0;
}

void draw_square(float x, float y, int color)
{
    gfx_filledRect(x, y, x + SQUARE_SIZE, y + SQUARE_SIZE, color % 8);
    gfx_rect(x, y, x + SQUARE_SIZE + 1, y + SQUARE_SIZE + 1, BLACK);
}

void draw_tetromino(tetromino *t)
{
    int xt, yt;
    float x = ORGIN_X + t->x * SQUARE_SIZE;
    float y = ORGIN_Y + t->y * SQUARE_SIZE;
    for (yt = 0; yt < SHAPE_SIZE; yt++)
    {
        for (xt = 0; xt < SHAPE_SIZE; xt++)
        {
            if (t->shape[xt][yt])
            {
                if(xt==SHAPE_SIZE/2&&yt==SHAPE_SIZE/2)
                    draw_square(x + xt * SQUARE_SIZE, y + yt * SQUARE_SIZE, (t->color));
                else
                    draw_square(x + xt * SQUARE_SIZE, y + yt * SQUARE_SIZE, t->color);
            }
        }
    }
}

void place_tetromino(tetromino *t, int board[GAME_WIDTH][GAME_HEIGHT])
{
    int xt, yt;
    for (yt = 0; yt < SHAPE_SIZE; yt++)
    {
        for (xt = 0; xt < SHAPE_SIZE; xt++)
        {
            if (t->shape[xt][yt] != 0)
                board[t->x + xt][t->y + yt] = t->color;
        }
    }
}

bool is_collison(tetromino *t, int board[GAME_WIDTH][GAME_HEIGHT])
{
    int xt, yt;
    for (yt = 0; yt < SHAPE_SIZE; yt++)
    {
        for (xt = 0; xt < SHAPE_SIZE; xt++)
        {
            if (t->shape[xt][yt] != 0)
            {
                if (t->x + xt < 0 || t->x + xt >= GAME_WIDTH)
                    return true;
                if (t->y + yt < 0 || t->y + yt >= GAME_HEIGHT)
                    return true;
                if (board[t->x + xt][t->y + yt] != 0)
                    return true;
            }
        }
    }
    return false;
}

void rotate_tetromino_handle(tetromino *t, bool direction)
{
    if (t->type == 1)
        return;
    int new_shape[SHAPE_SIZE][SHAPE_SIZE] = {0};
    int x, y;
    for (y = 0; y < SHAPE_SIZE; y++)
    {
        for (x = 0; x < SHAPE_SIZE; x++)
        {
            if (direction)
                new_shape[y][SHAPE_SIZE - x - 1] = t->shape[x][y];
            else
                new_shape[SHAPE_SIZE - y - 1][x] = t->shape[x][y];
        }
    }
    copy_shape(new_shape, t->shape);
}

void rotate_tetromino(tetromino *t, int board[GAME_WIDTH][GAME_HEIGHT])
{
    rotate_tetromino_handle(t, 1);
    if (is_collison(t, board))
        rotate_tetromino_handle(t, 0);
}

bool move_tetromino(tetromino *t, int board[GAME_WIDTH][GAME_HEIGHT], int dx, int dy)
{
    t->x += dx;
    t->y += dy;
    if (is_collison(t, board))
    {
        t->x -= dx;
        t->y -= dy;
        return 0;
    }
    return 1;
}

int handle_tetromino_movement(tetromino *t, int board[GAME_WIDTH][GAME_HEIGHT], int key)
{
    static int previous_fall_time = 0;
    float current_time = ((double)clock() / CLOCKS_PER_SEC) * 100;

    if (key == SDLK_SPACE)
        rotate_tetromino(t, board);
    if (key == SDLK_RIGHT)
        move_tetromino(t, board, 1, 0);
    if (key == SDLK_LEFT)
        move_tetromino(t, board, -1, 0);

    if (key == SDLK_DOWN)
    {
        while (move_tetromino(t, board, 0, 1))
            ;
        return 0;
    }
    if (current_time - previous_fall_time > FALLING_SPEED)
    {
        previous_fall_time = current_time;
        return !move_tetromino(t, board, 0, 1);
    }
    return 0;
}

// Board drawing and handling:
bool clear_row(int row, int board[GAME_WIDTH][GAME_HEIGHT])
{
    int x, y;
    for (y = row - 1; y >= 0; y--)
    {
        for (x = 0; x < GAME_WIDTH; x++)
        {
            board[x][y + 1] = board[x][y];
        }
    }
    for (x = 0; x < GAME_WIDTH; x++)
    {
        board[x][0] = 0;
    }
    return true;
}

void clear_full_rows(int board[GAME_WIDTH][GAME_HEIGHT])
{
    int x, y;
    for (y = GAME_HEIGHT - 1; y >= 0; y--)
    {
        for (x = 0; x < GAME_WIDTH; x++)
        {
            if (board[x][y] == 0)
                break;
        }
        if (x == GAME_WIDTH)
        {
            clear_row(y, board);
            y++;
        }
    }
}

void draw_board(int board[GAME_WIDTH][GAME_HEIGHT])
{
    int top = ORGIN_Y + SHAPE_SIZE * SQUARE_SIZE;
    int bottom = ORGIN_Y + GAME_HEIGHT * SQUARE_SIZE;
    int left = ORGIN_X;
    int right = ORGIN_X + GAME_WIDTH * SQUARE_SIZE;

    for (int y = 0; y < GAME_HEIGHT; y++)
    {
        for (int x = 0; x < GAME_WIDTH; x++)
        {
            if (board[x][y] != 0)
                draw_square(ORGIN_X + x * SQUARE_SIZE, ORGIN_Y + y * SQUARE_SIZE, board[x][y]);
        }
    }
    gfx_rect(right + 2 * SQUARE_SIZE, top, right + (SHAPE_SIZE + 4) * SQUARE_SIZE, top + (SHAPE_SIZE + 2) * SQUARE_SIZE, WHITE);
    gfx_line(left, top, left, bottom, WHITE);
    gfx_line(right, top, right, bottom, WHITE);
    gfx_line(left, bottom, right, bottom, WHITE);
}

void draw_view(tetromino *current, tetromino *next, int board[GAME_WIDTH][GAME_HEIGHT])
{
    gfx_updateScreen();
    gfx_filledRect(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLACK);
    draw_tetromino(current);
    draw_board(board);
    draw_tetromino(next);
}

int main(int argc, char *argv[])
{
    if (gfx_init())
    {
        exit(3);
    }
    srand(time(NULL));

    tetromino current;
    tetromino next;
    make_tetromino(&next, rand() % SHAPES_COUNT);

    int board[GAME_WIDTH][GAME_HEIGHT] = {0};
    bool lost = false;
    bool exit = false;
    int key;
    while (!exit && !lost)
    {
        make_tetromino(&current, next.type);
        make_tetromino(&next, rand() % SHAPES_COUNT);
        next.x = GAME_WIDTH + 3;
        next.y = SHAPE_SIZE + 1;

        bool placed = false;
        while (!placed && !exit)
        {

            key = get_key();
            exit = (key == SDLK_ESCAPE) || (key == SDL_QUIT);

            placed = handle_tetromino_movement(&current, board, key);
            draw_view(&current, &next, board);
            SDL_Delay(25);
        }
        place_tetromino(&current, board);
        clear_full_rows(board);
        lost = current.y <= 1;
    }
    gfx_filledRect(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BLACK);
    if (lost)
    {
        gfx_textout(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "YOU FAILED!", RED);
    }
    else
    {
        gfx_textout(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "EXITING...", YELLOW);
    }
    gfx_updateScreen();
    SDL_Delay(FRAME_DELAY);
    return 0;
}
