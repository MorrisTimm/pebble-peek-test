#include <peek_test.h>

#define SPEED 15
#define PEEK_HEIGHT 51
#ifdef PBL_PLATFORM_CHALK
#define PEEK_SIDE_WIDTH 51
#else
#define PEEK_SIDE_WIDTH 30
#endif

static GRect screen;
static int height;
static int interval;
static UnobstructedAreaHandlers handlers;
static void* context;
static AppTimer* peek_timer;
static AppTimer* animation_timer;
static TextLayer* text_layer;
static const char* text = "peek-a-boo";
static TextLayer* side_layer;
static TextLayer* top_layer;
static Layer* layer;
static bool peek;

static void animation_timer_callback(void* data) {
  GRect bounds = layer_get_frame((Layer*)layer);
  bool did_change = false;
  if(peek) {
    if(bounds.origin.y > screen.size.h-PEEK_HEIGHT) {
      bounds.origin.y -= SPEED;
      if(bounds.origin.y < screen.size.h-PEEK_HEIGHT) {
        bounds.origin.y = screen.size.h-PEEK_HEIGHT;
        did_change = true;
      } else {
        animation_timer = app_timer_register(33, animation_timer_callback, data);
      }
    }
  } else {
    if(bounds.origin.y < screen.size.h) {
      bounds.origin.y += SPEED;
      if(bounds.origin.y > screen.size.h) {
        bounds.origin.y = screen.size.h;
        did_change = true;
      } else {
        animation_timer = app_timer_register(33, animation_timer_callback, data);
      }
    }
  }
  layer_set_frame((Layer*)layer, bounds);
  layer_mark_dirty((Layer*)layer);
  if(handlers.change) {
    AnimationProgress percent;
    if(peek) {
      percent = (-(bounds.origin.y-screen.size.h)*100)/PEEK_HEIGHT;
    } else {
      percent = 100-((-(bounds.origin.y-screen.size.h)*100)/PEEK_HEIGHT);
    }
    handlers.change(percent, context);
  }
  if(handlers.did_change && did_change) {
    handlers.did_change(context);
  }
}

static void peek_timer_callback(void* data) {
  peek = !peek;
  peek_timer = app_timer_register(interval, peek_timer_callback, data);
  animation_timer = app_timer_register(33, animation_timer_callback, data);
  if(handlers.will_change) {
    GRect target = screen;
    if(peek) {
      target.size.h = screen.size.h-PEEK_HEIGHT;
    }
    handlers.will_change(target, context);
  }
}

Layer* peek_test_init(GRect screen_bounds, int peek_interval) {
  screen = screen_bounds;
  interval = peek_interval;
  
  text_layer = text_layer_create(GRect(0, 0, screen.size.w-PEEK_SIDE_WIDTH, PEEK_HEIGHT));
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  //text_layer_set_text(text_layer, text);
  side_layer = text_layer_create(GRect(screen.size.w-PEEK_SIDE_WIDTH, 0, PEEK_SIDE_WIDTH, PEEK_HEIGHT));
  text_layer_set_background_color(side_layer, PBL_IF_COLOR_ELSE(GColorPictonBlue, GColorLightGray));
  top_layer = text_layer_create(GRect(0, 0, screen.size.w, 2));
  text_layer_set_background_color(top_layer, GColorBlack);
  layer = layer_create(GRect(0, screen.size.h, screen.size.w, PEEK_HEIGHT));
  layer_add_child(layer, (Layer*)text_layer);
  layer_add_child(layer, (Layer*)side_layer);
  layer_add_child(layer, (Layer*)top_layer);

  peek = false;
  peek_timer = app_timer_register(interval, peek_timer_callback, NULL);
  return (Layer*)layer;
}

void peek_test_unobstructed_area_service_subscribe(UnobstructedAreaHandlers peek_handlers, void* peek_context) {
  handlers = peek_handlers;
  context = peek_context;
}

void peek_test_unobstructed_area_service_unsubscribe() {
  handlers.did_change = NULL;
  handlers.change = NULL;
  handlers.will_change = NULL;
}

GRect peek_test_get_unobstructed_bounds() {
  GRect bounds = layer_get_frame((Layer*)layer);
  GRect available = screen;
  available.size.h = bounds.origin.y;
  return available;
}
