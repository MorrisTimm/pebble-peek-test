#include <pebble.h>
#include "pebble-peek-test.h"

#define SPEED 15
#define PEEK_HEIGHT 51
#define PEEK_BORDER 2
#ifdef PBL_PLATFORM_CHALK
#define PEEK_SIDE_WIDTH 51
#else
#define PEEK_SIDE_WIDTH 30
#endif

static GRect s_screen;
static int s_height;
static int s_interval;
static UnobstructedAreaHandlers s_handlers;
static void* s_context;
static AppTimer* s_peek_timer;
static AppTimer* s_animation_timer;
static TextLayer* s_text_layer;
static TextLayer* s_side_layer;
static TextLayer* s_top_layer;
static Layer* s_layer;
static bool s_peek;

static void prv_animation_timer_callback(void* data) {
  GRect bounds = layer_get_frame((Layer*)s_layer);
  bool did_change = false;
  if(s_peek) {
    if(bounds.origin.y > s_screen.size.h-PEEK_HEIGHT) {
      bounds.origin.y -= SPEED;
      if(bounds.origin.y < s_screen.size.h-PEEK_HEIGHT) {
        bounds.origin.y = s_screen.size.h-PEEK_HEIGHT;
        did_change = true;
      } else {
        s_animation_timer = app_timer_register(33, prv_animation_timer_callback, data);
      }
    }
  } else {
    if(bounds.origin.y < s_screen.size.h) {
      bounds.origin.y += SPEED;
      if(bounds.origin.y > s_screen.size.h) {
        bounds.origin.y = s_screen.size.h;
        did_change = true;
      } else {
        s_animation_timer = app_timer_register(33, prv_animation_timer_callback, data);
      }
    }
  }
  layer_set_frame((Layer*)s_layer, bounds);
  layer_mark_dirty((Layer*)s_layer);
  if(s_handlers.change) {
    AnimationProgress percent;
    if(s_peek) {
      percent = (-(bounds.origin.y-s_screen.size.h)*100)/PEEK_HEIGHT;
    } else {
      percent = 100-((-(bounds.origin.y-s_screen.size.h)*100)/PEEK_HEIGHT);
    }
    s_handlers.change(percent, s_context);
  }
  if(s_handlers.did_change && did_change) {
    s_handlers.did_change(s_context);
  }
}

static void prv_peek_timer_callback(void* data) {
  s_peek = !s_peek;
  s_peek_timer = app_timer_register(s_interval, prv_peek_timer_callback, data);
  s_animation_timer = app_timer_register(33, prv_animation_timer_callback, data);
  if(s_handlers.will_change) {
    GRect target = s_screen;
    if(s_peek) {
      target.size.h = s_screen.size.h-PEEK_HEIGHT;
    }
    s_handlers.will_change(target, s_context);
  }
}

static void prv_layer_update_proc(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, PEEK_BORDER, s_screen.size.w-PEEK_SIDE_WIDTH, PEEK_HEIGHT), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorPictonBlue, GColorLightGray));
  graphics_fill_rect(ctx, GRect(s_screen.size.w-PEEK_SIDE_WIDTH, PEEK_BORDER, PEEK_SIDE_WIDTH, PEEK_HEIGHT), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, s_screen.size.w, PEEK_BORDER), 0, GCornerNone);
}

Layer* peek_test_init(GRect screen_bounds, int peek_interval) {
  s_screen = screen_bounds;
  s_interval = peek_interval;
  
  s_layer = layer_create(GRect(0, s_screen.size.h, s_screen.size.w, PEEK_HEIGHT));
  layer_set_update_proc(s_layer, prv_layer_update_proc);

  s_peek = false;
  s_peek_timer = app_timer_register(s_interval, prv_peek_timer_callback, NULL);
  return (Layer*)s_layer;
}

void peek_test_unobstructed_area_service_subscribe(UnobstructedAreaHandlers peek_handlers, void* peek_context) {
  s_handlers = peek_handlers;
  s_context = peek_context;
}

void peek_test_unobstructed_area_service_unsubscribe() {
  s_handlers.did_change = NULL;
  s_handlers.change = NULL;
  s_handlers.will_change = NULL;
}

GRect peek_test_get_unobstructed_bounds() {
  GRect bounds = layer_get_frame((Layer*)s_layer);
  GRect available = s_screen;
  available.size.h = bounds.origin.y;
  return available;
}
