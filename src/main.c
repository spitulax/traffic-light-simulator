#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 720

#define ROAD_WIDTH  160
#define ROAD_COLOUR 0x404040FF

#define LIGHT_RADIUS   16
#define LIGHT_DIAMETER (LIGHT_RADIUS * 2)
#define LIGHT_MARGIN   2

#define FRAME_COLOUR 0x000000FF
#define FRAME_W      (LIGHT_DIAMETER + LIGHT_MARGIN * 2)
#define FRAME_H      (LIGHT_DIAMETER * 3 + LIGHT_MARGIN * 4)

#define TEXT_COLOUR 0x202020FF
#define FONT_SIZE   20

#define RED_OFF    0x400000FF
#define YELLOW_OFF 0x404000FF
#define GREEN_OFF  0x004000FF
#define RED_ON     0xFF0000FF
#define YELLOW_ON  0xFFFF00FF
#define GREEN_ON   0x00FF00FF

#define NUM          4
#define G_TIMER      5
#define Y_TIMER      1
#define TIMER_BUFLEN 10

typedef enum {
    COLOUR_RED = 0,
    COLOUR_YELLOW,
    COLOUR_GREEN,
} Colour;

typedef struct {
    Colour       colour;
    unsigned int timer;
} State;

State  states[NUM];
size_t last_time = 0;
size_t green_idx = 0;

unsigned int red_timer(const size_t index) {
    const unsigned int i = (unsigned int) (index - green_idx) % NUM;
    return G_TIMER * i + 1;
}

void state_red(const size_t i) {
    states[i] = (State) {
        .colour = COLOUR_RED,
        .timer  = red_timer(i),
    };
}

void state_yellow(const size_t i) {
    states[i] = (State) {
        .colour = COLOUR_YELLOW,
        .timer  = Y_TIMER,
    };
}

void state_green(const size_t i) {
    states[i] = (State) {
        .colour = COLOUR_GREEN,
        .timer  = G_TIMER,
    };
}

void states_init(void) {
    for (size_t i = 0; i < NUM; ++i) {
        state_red(i);
    }
    state_green(green_idx);
}

unsigned int colour_get_colour(const Colour col, const bool on) {
    if (on) {
        switch (col) {
            case COLOUR_RED: {
                return RED_ON;
            } break;
            case COLOUR_YELLOW: {
                return YELLOW_ON;
            } break;
            case COLOUR_GREEN: {
                return GREEN_ON;
            } break;
        }
    } else {
        switch (col) {
            case COLOUR_RED: {
                return RED_OFF;
            } break;
            case COLOUR_YELLOW: {
                return YELLOW_OFF;
            } break;
            case COLOUR_GREEN: {
                return GREEN_OFF;
            } break;
        }
    }
    assert(0 && "unreachable");
}

bool colour_is_on(const Colour col, const size_t index) {
    const State *state = &states[index];
    return col == state->colour;
}

void draw_timer(const int pos_x, const int pos_y, const size_t index) {
    const State *state = &states[index];
    char         buf[TIMER_BUFLEN];
    const int    n = sprintf(buf, "%u", state->timer);
    buf[n]         = '\0';
    DrawText(buf, pos_x + FRAME_W + LIGHT_MARGIN, pos_y, FONT_SIZE, GetColor(TEXT_COLOUR));
}

void draw_light(int x, int y, const int w, const int h, const bool vert, const size_t index) {
    DrawRectangle(x, y, w, h, GetColor(FRAME_COLOUR));

    for (int i = 0; i < 3; ++i) {
        const int   offset1  = FRAME_W / 2;
        const int   offset2  = LIGHT_MARGIN + LIGHT_RADIUS + i * (LIGHT_MARGIN + LIGHT_DIAMETER);
        const int   x_offset = (vert) ? offset1 : offset2;
        const int   y_offset = (vert) ? offset2 : offset1;
        const int   circ_x   = x + x_offset;
        const int   circ_y   = y + y_offset;
        const bool  is_on    = colour_is_on((Colour) i, index);
        const Color col      = GetColor(colour_get_colour((Colour) i, is_on));
        DrawCircle(circ_x, circ_y, LIGHT_RADIUS, col);

        if (is_on) {
            const State *state = &states[index];
            char         buf[TIMER_BUFLEN];
            const int    n   = sprintf(buf, "%u", state->timer);
            buf[n]           = '\0';
            const int font_w = MeasureText(buf, FONT_SIZE);
            DrawText(buf,
                     circ_x - (font_w / 2),
                     circ_y - (LIGHT_RADIUS / 2),
                     FONT_SIZE,
                     GetColor(TEXT_COLOUR));
        }
    }
}

void draw_light_v(const int pos_x, const int pos_y, const size_t index) {
    draw_light(pos_x, pos_y, FRAME_W, FRAME_H, true, index);
}

void draw_light_h(const int pos_x, const int pos_y, const size_t index) {
    draw_light(pos_x, pos_y, FRAME_H, FRAME_W, false, index);
}

void draw(void) {
    const int pos_x = SCREEN_WIDTH / 2 - ROAD_WIDTH / 2;
    const int pos_y = SCREEN_HEIGHT / 2 - ROAD_WIDTH / 2;

    // Horizontal
    DrawRectangle(0, pos_y, SCREEN_WIDTH * 2, ROAD_WIDTH, GetColor(ROAD_COLOUR));
    // Vertical
    DrawRectangle(pos_x, 0, ROAD_WIDTH, SCREEN_HEIGHT * 2, GetColor(ROAD_COLOUR));

    // Left
    draw_light_v(pos_x - FRAME_W, pos_y + LIGHT_MARGIN, 0);
    // Up
    draw_light_h(pos_x + ROAD_WIDTH - FRAME_H - LIGHT_MARGIN, pos_y - FRAME_W, 1);
    // Right
    draw_light_v(pos_x + ROAD_WIDTH, pos_y + ROAD_WIDTH - FRAME_H - LIGHT_MARGIN, 2);
    // Down
    draw_light_h(pos_x + LIGHT_MARGIN, pos_y + ROAD_WIDTH, 3);
}

void update(void) {
    const size_t time = (size_t) GetTime() + 1;
    if (last_time != time) {
        for (size_t i = 0; i < NUM; ++i) {
            State *state = &states[i];
            if (state->timer > 0) {
                --state->timer;
            }
            if (state->timer <= 0 && state->colour == COLOUR_YELLOW) {
                state_red(i);
                state_green(green_idx);
            }
        }
    }

    for (size_t i = 0; i < NUM; ++i) {
        State *state = &states[i];
        if (i == green_idx && state->colour == COLOUR_GREEN && state->timer <= 0) {
            state_yellow(i);
            green_idx = (green_idx + 1) % NUM;
            break;
        }
    }

    last_time = time;
}

int main(void) {
    states_init();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Traffic Light Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
