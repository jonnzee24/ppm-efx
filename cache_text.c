#define MAX_CACHED_TEXT 128

typedef struct {
    char text[64];
    SDL_Color color;
    SDL_Texture *texture;
    int w, h;
} CachedText;

static CachedText text_cache[MAX_CACHED_TEXT];
static int text_cache_count = 0;

SDL_Texture *get_cached_text(SDL_Renderer *renderer, const char *text, SDL_Color color, int *w, int *h) {
    // 1. look for an existing entry
    for (int i = 0; i < text_cache_count; i++) {
        if (strcmp(text_cache[i].text, text) == 0 &&
            memcmp(&text_cache[i].color, &color, sizeof(SDL_Color)) == 0) {
            *w = text_cache[i].w;
            *h = text_cache[i].h;
            return text_cache[i].texture;
        }
    }

    // 2. not found -> create it
    SDL_Surface *surface = TTF_RenderText_Blended(font_15, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int tw = surface->w, th = surface->h;
    SDL_FreeSurface(surface);

    // 3. store it if there's room
    if (text_cache_count < MAX_CACHED_TEXT) {
        CachedText *entry = &text_cache[text_cache_count++];
        strncpy(entry->text, text, sizeof(entry->text) - 1);
        entry->text[sizeof(entry->text) - 1] = '\0';
        entry->color = color;
        entry->texture = texture;
        entry->w = tw;
        entry->h = th;
    }

    *w = tw;
    *h = th;
    return texture;
}

void draw_text(SDL_Renderer *renderer, int x_pos, int y_pos, SDL_Color font_color, char *text) {
    int w, h;
    SDL_Texture *texture = get_cached_text(renderer, text, font_color, &w, &h);
    SDL_Rect rect = {x_pos, y_pos, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
