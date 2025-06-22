#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#include "hit.h"
#include "success.h"
#include "tema.h"
#include "FreeSans_ttf.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BALL_SIZE 10
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 80
#define PADDLE_SPEED 5
#define BALL_SPEED 5
#define MARGIN 20
#define COLOR_COUNT 7

SDL_Color rainbow[COLOR_COUNT] = {
    {255, 0, 0}, {255, 127, 0}, {255, 255, 0},
    {0, 255, 0}, {0, 0, 255}, {75, 0, 130}, {148, 0, 211}
};

void draw_border(SDL_Renderer* r, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(r, &(SDL_Rect){0, 0, WINDOW_WIDTH, 4});
    SDL_RenderFillRect(r, &(SDL_Rect){0, WINDOW_HEIGHT - 4, WINDOW_WIDTH, 4});
    SDL_RenderFillRect(r, &(SDL_Rect){0, 0, 4, WINDOW_HEIGHT});
    SDL_RenderFillRect(r, &(SDL_Rect){WINDOW_WIDTH - 4, 0, 4, WINDOW_HEIGHT});
}

void render_score(SDL_Renderer* r, TTF_Font* font, int score, int x, SDL_Color color) {
    char buf[10];
    snprintf(buf, sizeof(buf), "%d", score);
    SDL_Surface* surf = TTF_RenderText_Solid(font, buf, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {x, 20, surf->w, surf->h};
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void rainbow_flash(SDL_Renderer* r) {
    for (int i = 0; i < COLOR_COUNT; i++) {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        draw_border(r, rainbow[i]);
        //SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
        for (int y = MARGIN; y < WINDOW_HEIGHT - MARGIN; y += 10)
            SDL_RenderFillRect(r, &(SDL_Rect){WINDOW_WIDTH / 2 - 1, y, 2, 8});
        SDL_RenderPresent(r);
        SDL_Delay(200);
    }
}

void draw_filled_circle_from_rect(SDL_Renderer* renderer, const SDL_Rect* rect, SDL_Color color) {
    int cx = rect->x; //+ rect->w / 2 ;
    int cy = rect->y + rect->h / 2;
    int radius = rect->w * 2/3; // Assumiamo w == h

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius)
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        }
    }
}


int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() < 0 ||
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Errore inizializzazione SDL, TTF o audio\n");
        return 1;
    }

    SDL_Window* w = SDL_CreateWindow("Rainbow Pong",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);

    SDL_RWops* rw_font = SDL_RWFromConstMem(FreeSans_ttf, FreeSans_ttf_len);
    TTF_Font* font = TTF_OpenFontRW(rw_font, 1, 32); // 1 = libera SDL_RWops quando finito


    SDL_RWops* rw_hit = SDL_RWFromConstMem(hit_mp3, hit_mp3_len);
    Mix_Chunk* hit = Mix_LoadWAV_RW(rw_hit, 1);

    SDL_RWops* rw_bounce = SDL_RWFromConstMem(hit_mp3, hit_mp3_len);
    Mix_Chunk* bounce = Mix_LoadWAV_RW(rw_bounce, 1);
    
    SDL_RWops* rw_point = SDL_RWFromConstMem(success_mp3, success_mp3_len);
    Mix_Chunk* point = Mix_LoadWAV_RW(rw_point, 1);
    
    SDL_RWops* rw_music = SDL_RWFromConstMem(tema_mp3, tema_mp3_len);
    Mix_Music* music = Mix_LoadMUS_RW(rw_music, 1);// anche qui, 1 = libera l'RWops

    int music_on = 1;
    if (music) Mix_PlayMusic(music, -1);

    SDL_Rect ball = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2, BALL_SIZE, BALL_SIZE};
    SDL_Rect p1 = {MARGIN + 5, WINDOW_HEIGHT/2 - PADDLE_HEIGHT/2, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect p2 = {WINDOW_WIDTH - MARGIN - PADDLE_WIDTH - 5, p1.y, PADDLE_WIDTH, PADDLE_HEIGHT};

    int dx = BALL_SPEED, dy = BALL_SPEED;
    int score1 = 0, score2 = 0;
    int waiting = 0, quit = 0;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT ||
               (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q)) quit = 1;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m) {
                if (music_on) { Mix_PauseMusic(); music_on = 0; }
                else          { Mix_ResumeMusic(); music_on = 1; }
            }
        }

        SDL_PumpEvents();
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_A] && p1.y > 0) p1.y -= PADDLE_SPEED;
        if (keys[SDL_SCANCODE_Z] && p1.y + PADDLE_HEIGHT < WINDOW_HEIGHT) p1.y += PADDLE_SPEED;
        if (keys[SDL_SCANCODE_UP] && p2.y > 0) p2.y -= PADDLE_SPEED;
        if (keys[SDL_SCANCODE_DOWN] && p2.y + PADDLE_HEIGHT < WINDOW_HEIGHT) p2.y += PADDLE_SPEED;

        if (waiting == 0) {
            ball.x += dx;
            ball.y += dy;

            if (ball.y <= 0 || ball.y + BALL_SIZE >= WINDOW_HEIGHT) {
                dy *= -1;
                if (bounce) Mix_PlayChannel(-1, bounce, 0);
            }

            if (SDL_HasIntersection(&ball, &p1)) {
                dx = BALL_SPEED;
                if (hit) Mix_PlayChannel(-1, hit, 0);
            }

            if (SDL_HasIntersection(&ball, &p2)) {
                dx = -BALL_SPEED;
                if (hit) Mix_PlayChannel(-1, hit, 0);
            }

            if (ball.x + BALL_SIZE < 0) {
                score2++;
                if (point) Mix_PlayChannel(-1, point, 0);
                rainbow_flash(r);
                ball.x = WINDOW_WIDTH / 2;
                ball.y = WINDOW_HEIGHT / 2;
                waiting = SDL_GetTicks() + 2000;
            }

            if (ball.x > WINDOW_WIDTH) {
                score1++;
                if (point) Mix_PlayChannel(-1, point, 0);
                rainbow_flash(r);
                ball.x = WINDOW_WIDTH / 2;
                ball.y = WINDOW_HEIGHT / 2;
                waiting = SDL_GetTicks() + 2000;
            }
        } else if (SDL_GetTicks() > waiting) {
            waiting = 0;
            dx = (dx > 0) ? BALL_SPEED : -BALL_SPEED;
            dy = BALL_SPEED;
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        draw_border(r, (SDL_Color){255,255,255});
        SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
        for (int y = MARGIN; y < WINDOW_HEIGHT - MARGIN; y += 10)
        	SDL_RenderFillRect(r, &(SDL_Rect){WINDOW_WIDTH/2 - 1, y, 2, 8});
        SDL_SetRenderDrawColor(r, 255,255,255,255);
        SDL_RenderFillRect(r, &p1);
        SDL_RenderFillRect(r, &p2);
	draw_filled_circle_from_rect(r,&ball, (SDL_Color){255,255,255});
        //SDL_RenderFillRect(r, &ball);
        render_score(r, font, score1, 100, (SDL_Color){255,255,255});
        render_score(r, font, score2, WINDOW_WIDTH - 150, (SDL_Color){255,255,255});
        SDL_RenderPresent(r);
        SDL_Delay(16);
    }

    Mix_FreeChunk(hit);
    Mix_FreeChunk(bounce);
    Mix_FreeChunk(point);
    Mix_FreeMusic(music);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}

