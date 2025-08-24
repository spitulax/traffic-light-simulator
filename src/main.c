#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 720

#define ROAD_WIDTH  160
#define LANE_WIDTH  ((int) ROAD_WIDTH / 2)
#define ROAD_COLOUR 0x404040FF
#define VERT_ROAD_X ((int) SCREEN_WIDTH / 2 - (int) ROAD_WIDTH / 2)
#define HORZ_ROAD_Y ((int) SCREEN_HEIGHT / 2 - (int) ROAD_WIDTH / 2)

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

#define G_TIMER      10
#define Y_TIMER      2
#define TIMER_BUFLEN 10

#define MAX_CARS      10
#define SPAWN_OFFSET  50
#define CAR_MARGIN    20
#define CAR_LEN       (CAR_WIDTH * 2)
#define CAR_WIDTH     (LANE_WIDTH - CAR_MARGIN * 2)
#define CAR_MIN_SPEED 100
#define CAR_MAX_SPEED 300

#define CAR_COLOURS_LEN 26
unsigned int car_colours[CAR_COLOURS_LEN] = {
    0xDC8A78FF, 0xDD7878FF, 0xEA76CBFF, 0x8839EFFF, 0xD20F39FF, 0xE64553FF, 0xFE640BFF,
    0xDF8E1DFF, 0x40A02BFF, 0x179299FF, 0x04A5E5FF, 0x209FB5FF, 0x1E66F5FF, 0x7287FDFF,
    0x5C5F77FF, 0x6C6F85FF, 0x7C7F93FF, 0x8C8FA1FF, 0x9CA0B0FF, 0xACB0BEFF, 0xBCC0CCFF,
    0xCCD0DAFF, 0xEFF1F5FF, 0xE6E9EFFF, 0xDCE0E8FF,
};

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
    bool         used;
    Lane         origin;
    int          orig_speed;
    int          speed;
    int          progress;
    bool         crossed;
    unsigned int colour;
} Car;

Car cars[MAX_CARS];

unsigned int red_timer(const Lane lane) {
    const unsigned int i = (unsigned int) (lane - green_idx) % LANE_NUM;
    return G_TIMER * i + Y_TIMER * i;
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

void cars_add(const Lane origin) {
    int progress_min = 0 - CAR_LEN;
    for (size_t i = 0; i < MAX_CARS; ++i) {
        Car *car = &cars[i];
        if (car->origin == origin && car->progress < progress_min) {
            progress_min = car->progress - CAR_LEN - CAR_MARGIN;
        }
    }

    for (size_t i = 0; i < MAX_CARS; ++i) {
        Car *car = &cars[i];
        if (!car->used) {
            *car = (Car) {
                .used       = true,
                .origin     = origin,
                .progress   = (progress_min > 0) ? 0 : progress_min,
                .orig_speed = GetRandomValue(CAR_MIN_SPEED, CAR_MAX_SPEED),
                .crossed    = false,
                .colour     = car_colours[GetRandomValue(0, CAR_COLOURS_LEN - 1)],
            };
            car->speed = car->orig_speed;
            break;
        }
    }
}

void cars_init(void) {
    for (size_t i = 0; i < MAX_CARS; ++i) {
        cars[i].used = false;
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
    }

    else {
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

int crossing_start(const Lane lane) {
    switch (lane) {
        case LANE_LEFT: {
            return VERT_ROAD_X - FRAME_W - CAR_MARGIN;
        } break;
        case LANE_UP: {
            return HORZ_ROAD_Y - FRAME_W - CAR_MARGIN;
        } break;
        case LANE_RIGHT: {
            return SCREEN_WIDTH - (VERT_ROAD_X + ROAD_WIDTH + FRAME_W + CAR_MARGIN);
        } break;
        case LANE_DOWN: {
            return SCREEN_HEIGHT - (HORZ_ROAD_Y + ROAD_WIDTH + FRAME_W + CAR_MARGIN);
        } break;
        case LANE_NUM: {
        };
    }
    assert(0 && "unreachable");
}

int frame_start(const Lane lane) {
    switch (lane) {
        case LANE_LEFT: {
            return VERT_ROAD_X - FRAME_W;
        } break;
        case LANE_UP: {
            return HORZ_ROAD_Y - FRAME_W;
        } break;
        case LANE_RIGHT: {
            return VERT_ROAD_X + ROAD_WIDTH;
        } break;
        case LANE_DOWN: {
            return HORZ_ROAD_Y + ROAD_WIDTH;
        } break;
        case LANE_NUM: {
        };
    }
    assert(0 && "unreachable");
}

bool colour_is_on(const Colour col, const Lane lane) {
    const Light *light = &lights[lane];
    return col == light->colour;
}

void draw_timer(const int pos_x, const int pos_y, const Lane lane) {
    const Light *light = &lights[lane];
    char         buf[TIMER_BUFLEN];
    const int    n = sprintf(buf, "%u", light->timer);
    buf[n]         = '\0';
    DrawText(buf, pos_x + FRAME_W + LIGHT_MARGIN, pos_y, FONT_SIZE, GetColor(TEXT_COLOUR));
}

void draw_light(const Lane lane) {
    const int gap = ROAD_WIDTH - FRAME_H - LIGHT_MARGIN;

    int  x = 0, y = 0, w = 0, h = 0;
    bool vert = 0;
    switch (lane) {
        case LANE_LEFT: {
            x    = frame_start(lane);
            y    = HORZ_ROAD_Y + LIGHT_MARGIN;
            w    = FRAME_W;
            h    = FRAME_H;
            vert = true;
        } break;
        case LANE_UP: {
            x    = VERT_ROAD_X + gap;
            y    = frame_start(lane);
            w    = FRAME_H;
            h    = FRAME_W;
            vert = false;
        } break;
        case LANE_RIGHT: {
            x    = frame_start(lane);
            y    = HORZ_ROAD_Y + gap;
            w    = FRAME_W;
            h    = FRAME_H;
            vert = true;
        } break;
        case LANE_DOWN: {
            x    = VERT_ROAD_X + LIGHT_MARGIN;
            y    = frame_start(lane);
            w    = FRAME_H;
            h    = FRAME_W;
            vert = false;
        } break;
        case LANE_NUM: {
            assert(0 && "unreachable");
        };
    }

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
            const Light *light = &lights[lane];
            char         buf[TIMER_BUFLEN];
            const int    n   = sprintf(buf, "%u", light->timer);
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

void draw_car(const size_t i) {
    const Car *car = &cars[i];
    switch (car->origin) {
        case LANE_LEFT: {
            DrawRectangle(car->progress - CAR_LEN,
                          HORZ_ROAD_Y + CAR_MARGIN,
                          CAR_LEN,
                          CAR_WIDTH,
                          GetColor(car->colour));
        } break;
        case LANE_RIGHT: {
            DrawRectangle(SCREEN_WIDTH - car->progress,
                          HORZ_ROAD_Y + LANE_WIDTH + CAR_MARGIN,
                          CAR_LEN,
                          CAR_WIDTH,
                          GetColor(car->colour));
        } break;
        case LANE_UP: {
            DrawRectangle(VERT_ROAD_X + LANE_WIDTH + CAR_MARGIN,
                          car->progress - CAR_LEN,
                          CAR_WIDTH,
                          CAR_LEN,
                          GetColor(car->colour));
        } break;
        case LANE_DOWN: {
            DrawRectangle(VERT_ROAD_X + CAR_MARGIN,
                          SCREEN_HEIGHT - car->progress,
                          CAR_WIDTH,
                          CAR_LEN,
                          GetColor(car->colour));
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

    draw_light(LANE_LEFT);
    draw_light(LANE_UP);
    draw_light(LANE_RIGHT);
    draw_light(LANE_DOWN);
}

void update_lights(void) {
    bool switched = false;

    for (Lane i = 0; i < LANE_NUM; ++i) {
        if (switched && i == green_idx) continue;
        Light *light = &lights[i];
        if (light->timer > 0) {
            --light->timer;
        }
        if (light->timer <= 0 && light->colour == COLOUR_YELLOW) {
            lights_red(i);
            lights_green(green_idx);
            switched = true;
        }
    }

    for (Lane i = 0; i < LANE_NUM; ++i) {
        Light *light = &lights[i];
        if (i == green_idx && light->colour == COLOUR_GREEN && light->timer <= 0) {
            lights_yellow(i);
            green_idx = (green_idx + 1) % LANE_NUM;
            break;
        }
    }
}

void spawn_cars(void) {
    const Lane lane = (Lane) GetRandomValue(0, LANE_NUM - 1);
    cars_add(lane);
}

void update_cars(void) {
    for (size_t i = 0; i < MAX_CARS; ++i) {
        Car *car = &cars[i];
        if (!car->used) continue;
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

        for (size_t j = 0; j < MAX_CARS; ++j) {
            Car *other_car = &cars[j];
            if (j != i && other_car->used && other_car->origin == car->origin &&
                other_car->progress > car->progress) {
                if (car->speed == 0 ||
                    (car->progress >= other_car->progress - CAR_LEN - CAR_MARGIN)) {
                    car->speed = other_car->speed;
                    break;
                }
            }
        }

        if (!car->crossed && car->progress >= crossing_start(car->origin)) {
            if (lights[car->origin].colour != COLOUR_GREEN) {
                car->speed = 0;
            } else {
                car->speed   = car->orig_speed;
                car->crossed = true;
            }
        }

        if (car->progress > upper_bound + CAR_LEN * 2) {
            car->used = false;
        }

        car->progress += (int) ((float) car->speed * GetFrameTime());
    }
}

void update(void) {
    const size_t time = (size_t) GetTime();

    if (last_time != time) {
        update_lights();
        spawn_cars();
    }
    update_cars();

    last_time = time;
}

int main(void) {
    SetRandomSeed((unsigned int) time(NULL));

    lights_init();
    cars_init();

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
