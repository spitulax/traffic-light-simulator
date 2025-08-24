#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 720

#define ROAD_WIDTH  160
#define LANE_WIDTH  ((float) ROAD_WIDTH / 2)
#define ROAD_COLOUR 0x404040FF
#define VERT_ROAD_X (SCREEN_WIDTH / 2 - ROAD_WIDTH / 2)
#define HORZ_ROAD_Y (SCREEN_HEIGHT / 2 - ROAD_WIDTH / 2)

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

#define G_TIMER      5
#define Y_TIMER      1
#define TIMER_BUFLEN 10

#define MAX_CARS     10
#define SPAWN_OFFSET 50
#define CAR_MARGIN   20
#define CAR_LEN      (CAR_WIDTH * 2)
#define CAR_WIDTH    (LANE_WIDTH - CAR_MARGIN * 2)
// TODO: randomise car colour
#define CAR_COLOUR    0xCCCC00FF
#define CAR_MIN_SPEED 200
#define CAR_MAX_SPEED 500

typedef enum {
    LANE_LEFT = 0,
    LANE_UP,
    LANE_RIGHT,
    LANE_DOWN,
    LANE_NUM,
} Lane;

typedef enum {
    COLOUR_RED = 0,
    COLOUR_YELLOW,
    COLOUR_GREEN,
} Colour;

typedef struct {
    Colour       colour;
    unsigned int timer;
} Light;

Light  lights[LANE_NUM];
size_t last_time = 0;
Lane   green_idx = 0;

typedef struct {
    bool used;
    Lane origin;
    int  speed;
    int  progress;
} Car;

Car cars[MAX_CARS];

unsigned int red_timer(const Lane lane) {
    const unsigned int i = (unsigned int) (lane - green_idx) % LANE_NUM;
    return G_TIMER * i + 1;
}

void lights_red(const Lane lane) {
    lights[lane] = (Light) {
        .colour = COLOUR_RED,
        .timer  = red_timer(lane),
    };
}

void lights_yellow(const Lane lane) {
    lights[lane] = (Light) {
        .colour = COLOUR_YELLOW,
        .timer  = Y_TIMER,
    };
}

void lights_green(const Lane lane) {
    lights[lane] = (Light) {
        .colour = COLOUR_GREEN,
        .timer  = G_TIMER,
    };
}

void lights_init(void) {
    for (Lane i = 0; i < LANE_NUM; ++i) {
        lights_red(i);
    }
    lights_green(green_idx);
}

void cars_init(void) {
    for (size_t i = 0; i < MAX_CARS; ++i) {
        cars[i].used = false;
    }
}

void cars_add(const Lane origin) {
    for (size_t i = 0; i < MAX_CARS; ++i) {
        Car *car = &cars[i];
        if (!car->used) {
            *car = (Car) {
                .used     = true,
                .origin   = origin,
                .progress = 0 - (int) CAR_LEN,
                // TODO: randomise speed
                .speed = CAR_MAX_SPEED,
            };
            break;
        }
    }
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

bool colour_is_on(const Colour col, const Lane lane) {
    const Light *state = &lights[lane];
    return col == state->colour;
}

void draw_timer(const int pos_x, const int pos_y, const Lane lane) {
    const Light *state = &lights[lane];
    char         buf[TIMER_BUFLEN];
    const int    n = sprintf(buf, "%u", state->timer);
    buf[n]         = '\0';
    DrawText(buf, pos_x + FRAME_W + LIGHT_MARGIN, pos_y, FONT_SIZE, GetColor(TEXT_COLOUR));
}

void draw_light(int x, int y, const int w, const int h, const bool vert, const Lane lane) {
    DrawRectangle(x, y, w, h, GetColor(FRAME_COLOUR));

    for (int i = 0; i < 3; ++i) {
        const int   offset1  = FRAME_W / 2;
        const int   offset2  = LIGHT_MARGIN + LIGHT_RADIUS + i * (LIGHT_MARGIN + LIGHT_DIAMETER);
        const int   x_offset = (vert) ? offset1 : offset2;
        const int   y_offset = (vert) ? offset2 : offset1;
        const int   circ_x   = x + x_offset;
        const int   circ_y   = y + y_offset;
        const bool  is_on    = colour_is_on((Colour) i, lane);
        const Color col      = GetColor(colour_get_colour((Colour) i, is_on));
        DrawCircle(circ_x, circ_y, LIGHT_RADIUS, col);

        if (is_on) {
            const Light *state = &lights[lane];
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

void draw_light_v(const int pos_x, const int pos_y, const Lane lane) {
    draw_light(pos_x, pos_y, FRAME_W, FRAME_H, true, lane);
}

void draw_light_h(const int pos_x, const int pos_y, const Lane lane) {
    draw_light(pos_x, pos_y, FRAME_H, FRAME_W, false, lane);
}

void draw_car(const size_t i) {
    const Car *car = &cars[i];
    switch (car->origin) {
        case LANE_LEFT: {
            DrawRectangle(car->progress - (int) CAR_LEN,
                          HORZ_ROAD_Y + CAR_MARGIN,
                          (int) CAR_LEN,
                          (int) CAR_WIDTH,
                          GetColor(CAR_COLOUR));
        } break;
        case LANE_RIGHT: {
            DrawRectangle(SCREEN_WIDTH - car->progress,
                          HORZ_ROAD_Y + (int) LANE_WIDTH + CAR_MARGIN,
                          (int) CAR_LEN,
                          (int) CAR_WIDTH,
                          GetColor(CAR_COLOUR));
        } break;
        case LANE_UP: {
            DrawRectangle(VERT_ROAD_X + (int) LANE_WIDTH + CAR_MARGIN,
                          car->progress - (int) CAR_LEN,
                          (int) CAR_WIDTH,
                          (int) CAR_LEN,
                          GetColor(CAR_COLOUR));
        } break;
        case LANE_DOWN: {
            DrawRectangle(VERT_ROAD_X + CAR_MARGIN,
                          SCREEN_HEIGHT - car->progress,
                          (int) CAR_WIDTH,
                          (int) CAR_LEN,
                          GetColor(CAR_COLOUR));
        } break;
        default: {
            assert(0 && "unimplemented");
        };
    }
}

void draw(void) {
    DrawRectangle(0, HORZ_ROAD_Y, SCREEN_WIDTH * 2, ROAD_WIDTH, GetColor(ROAD_COLOUR));
    DrawRectangle(VERT_ROAD_X, 0, ROAD_WIDTH, SCREEN_HEIGHT * 2, GetColor(ROAD_COLOUR));

    for (size_t i = 0; i < MAX_CARS; ++i) {
        if (cars[i].used) {
            draw_car(i);
        }
    }

    const int gap = ROAD_WIDTH - FRAME_H - LIGHT_MARGIN;
    draw_light_v(VERT_ROAD_X - FRAME_W, HORZ_ROAD_Y + LIGHT_MARGIN, LANE_LEFT);
    draw_light_h(VERT_ROAD_X + gap, HORZ_ROAD_Y - FRAME_W, LANE_UP);
    draw_light_v(VERT_ROAD_X + ROAD_WIDTH, HORZ_ROAD_Y + gap, LANE_RIGHT);
    draw_light_h(VERT_ROAD_X + LIGHT_MARGIN, HORZ_ROAD_Y + ROAD_WIDTH, LANE_DOWN);
}

void update(void) {
    const size_t time = (size_t) GetTime() + 1;
    if (last_time != time) {
        for (Lane i = 0; i < LANE_NUM; ++i) {
            Light *state = &lights[i];
            if (state->timer > 0) {
                --state->timer;
            }
            if (state->timer <= 0 && state->colour == COLOUR_YELLOW) {
                lights_red(i);
                lights_green(green_idx);
            }
        }
    }

    for (Lane i = 0; i < LANE_NUM; ++i) {
        Light *state = &lights[i];
        if (i == green_idx && state->colour == COLOUR_GREEN && state->timer <= 0) {
            lights_yellow(i);
            green_idx = (green_idx + 1) % LANE_NUM;
            break;
        }
    }

    for (size_t i = 0; i < MAX_CARS; ++i) {
        Car *car = &cars[i];
        if (car->used) {
            car->progress += (int) ((float) car->speed * GetFrameTime());

            int upper_bound = 0;
            switch (car->origin) {
                case LANE_LEFT:
                case LANE_RIGHT: {
                    upper_bound = SCREEN_WIDTH;
                } break;
                case LANE_UP:
                case LANE_DOWN: {
                    upper_bound = SCREEN_HEIGHT;
                } break;
                case LANE_NUM: {
                    assert(0 && "unreachable");
                }
            }

            if (car->progress > upper_bound + (int) CAR_LEN) {
                car->used = false;
            }
        }
    }

    last_time = time;
}

int main(void) {
    lights_init();
    cars_init();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Traffic Light Simulator");
    SetTargetFPS(60);

    cars_add(LANE_LEFT);
    cars_add(LANE_UP);
    cars_add(LANE_RIGHT);
    cars_add(LANE_DOWN);

    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
