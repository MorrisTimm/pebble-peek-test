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
static int s_interval;
static PeekType s_type;
static UnobstructedAreaHandlers s_handlers;
static void* s_context;
static AppTimer* s_peek_timer;
static AppTimer* s_animation_timer;
static Layer* s_layer;
static bool s_peek;

static void prv_animation_timer_callback(void* data) {
  GRect bounds = layer_get_frame((Layer*)s_layer);
  bool did_change = false;
  bool changing = false;
  if(s_peek) {
    switch(s_type) {
      case PEEK_TYPE_BOTTOM: {
        if(bounds.origin.y > s_screen.size.h-PEEK_HEIGHT) {
          bounds.origin.y -= SPEED;
          if(bounds.origin.y < s_screen.size.h-PEEK_HEIGHT) {
            bounds.origin.y = s_screen.size.h-PEEK_HEIGHT;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_TOP: {
        if(bounds.origin.y < 0) {
          bounds.origin.y += SPEED;
          if(bounds.origin.y > 0) {
            bounds.origin.y = 0;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_LEFT: {
        if(bounds.origin.x < 0) {
          bounds.origin.x += SPEED;
          if(bounds.origin.x > 0) {
            bounds.origin.x = 0;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_RIGHT: {
        if(bounds.origin.x > s_screen.size.w-PEEK_HEIGHT) {
          bounds.origin.x -= SPEED;
          if(bounds.origin.x < s_screen.size.w-PEEK_HEIGHT) {
            bounds.origin.x = s_screen.size.w-PEEK_HEIGHT;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
    }
  } else {
    switch(s_type) {
      case PEEK_TYPE_BOTTOM: {
        if(bounds.origin.y < s_screen.size.h) {
          bounds.origin.y += SPEED;
          if(bounds.origin.y > s_screen.size.h) {
            bounds.origin.y = s_screen.size.h;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_TOP: {
        if(bounds.origin.y > -PEEK_HEIGHT) {
          bounds.origin.y -= SPEED;
          if(bounds.origin.y < -PEEK_HEIGHT) {
            bounds.origin.y = -PEEK_HEIGHT;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_LEFT: {
        if(bounds.origin.x -PEEK_HEIGHT) {
          bounds.origin.x -= SPEED;
          if(bounds.origin.x < -PEEK_HEIGHT) {
            bounds.origin.x = -PEEK_HEIGHT;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
      case PEEK_TYPE_RIGHT: {
        if(bounds.origin.x < s_screen.size.w) {
          bounds.origin.x += SPEED;
          if(bounds.origin.x > s_screen.size.w) {
            bounds.origin.x = s_screen.size.w;
            did_change = true;
          } else {
            changing = true;
          }
        }
        break;
      }
    }
  }
  if(changing) {
    s_animation_timer = app_timer_register(33, prv_animation_timer_callback, data);
  }
  layer_set_frame((Layer*)s_layer, bounds);
  layer_mark_dirty((Layer*)s_layer);
  if(s_handlers.change) {
    AnimationProgress progress = 0;
    if(s_peek) {
      switch(s_type) {
        case PEEK_TYPE_BOTTOM: {
          progress = (-(bounds.origin.y-s_screen.size.h)*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT;
          break;
        }
        case PEEK_TYPE_TOP: {
          progress = ANIMATION_NORMALIZED_MAX-((-bounds.origin.y*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT);
          break;
        }
        case PEEK_TYPE_LEFT: {
          progress = ANIMATION_NORMALIZED_MAX-((-bounds.origin.x*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT);
          break;
        }
        case PEEK_TYPE_RIGHT: {
          progress = (-(bounds.origin.x-s_screen.size.w)*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT;
          break;
        }
      }
    } else {
      switch(s_type) {
        case PEEK_TYPE_BOTTOM: {
          progress = ANIMATION_NORMALIZED_MAX-((-(bounds.origin.y-s_screen.size.h)*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT);
          break;
        } case PEEK_TYPE_TOP: {
          progress = (-bounds.origin.y*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT;
          break;
        }
        case PEEK_TYPE_LEFT: {
          progress = (-bounds.origin.x*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT;
          break;
        }
        case PEEK_TYPE_RIGHT: {
          progress = ANIMATION_NORMALIZED_MAX-((-(bounds.origin.x-s_screen.size.w)*ANIMATION_NORMALIZED_MAX)/PEEK_HEIGHT);
          break;
        }
      }
    }
    s_handlers.change(progress, s_context);
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
      switch(s_type) {
        case PEEK_TYPE_BOTTOM: {
          target.size.h = s_screen.size.h-PEEK_HEIGHT;
          // no break
        }
        case PEEK_TYPE_TOP: {
          target.origin.y = PEEK_HEIGHT;
          break;
        }
        case PEEK_TYPE_RIGHT: {
          target.size.w = s_screen.size.w-PEEK_HEIGHT;
          // no break
        }
        case PEEK_TYPE_LEFT: {
          target.origin.x = PEEK_HEIGHT;
          break;
        }
      }
    }
    s_handlers.will_change(target, s_context);
  }
}

static void prv_layer_update_proc(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);
  switch(s_type) {
    case PEEK_TYPE_BOTTOM:
    case PEEK_TYPE_TOP: {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w-PEEK_SIDE_WIDTH, bounds.size.h), 0, GCornerNone);
      graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorPictonBlue, GColorLightGray));
      graphics_fill_rect(ctx, GRect(bounds.size.w-PEEK_SIDE_WIDTH, 0, PEEK_SIDE_WIDTH, bounds.size.h), 0, GCornerNone);
      graphics_context_set_fill_color(ctx, GColorBlack);
      if(PEEK_TYPE_BOTTOM == s_type) {
        graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, PEEK_BORDER), 0, GCornerNone);
      } else {
        graphics_fill_rect(ctx, GRect(0, bounds.size.h-PEEK_BORDER, bounds.size.w, PEEK_BORDER), 0, GCornerNone);
      }
      break;
    }
    case PEEK_TYPE_LEFT:
    case PEEK_TYPE_RIGHT: {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, bounds, 0, GCornerNone);
      graphics_context_set_fill_color(ctx, GColorBlack);
      if(PEEK_TYPE_LEFT == s_type) {
        graphics_fill_rect(ctx, GRect(bounds.size.w-PEEK_BORDER, 0, PEEK_BORDER, bounds.size.h), 0, GCornerNone);
      } else {
        graphics_fill_rect(ctx, GRect(0, 0, PEEK_BORDER, bounds.size.h), 0, GCornerNone);
      }
    }
  }
}

Layer* peek_test_init(GRect screen_bounds, int peek_interval, PeekType peek_type) {
  s_screen = screen_bounds;
  s_interval = peek_interval;
  s_type = peek_type;

  switch(s_type) {
    case PEEK_TYPE_BOTTOM: {
      s_layer = layer_create(GRect(0, s_screen.size.h, s_screen.size.w, PEEK_HEIGHT));
      break;
    }
    case PEEK_TYPE_TOP: {
      s_layer = layer_create(GRect(0, -PEEK_HEIGHT, s_screen.size.w, PEEK_HEIGHT));
      break;
    }
    case PEEK_TYPE_LEFT: {
      s_layer = layer_create(GRect(-PEEK_HEIGHT, 0, PEEK_HEIGHT, s_screen.size.h));
      break;
    }
    case PEEK_TYPE_RIGHT: {
      s_layer = layer_create(GRect(s_screen.size.w, 0, PEEK_HEIGHT, s_screen.size.h));
      break;
    }
  }
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
  switch(s_type) {
    case PEEK_TYPE_BOTTOM: {
      available.size.h = bounds.origin.y;
      break;
    }
    case PEEK_TYPE_TOP: {
      available.origin.y = PEEK_HEIGHT+bounds.origin.y;
      available.size.h -= available.origin.y;
      break;
    }
    case PEEK_TYPE_LEFT: {
      available.origin.x = PEEK_HEIGHT+bounds.origin.x;
      available.size.w -= available.origin.x;
      break;
    }
    case PEEK_TYPE_RIGHT: {
      available.size.w = bounds.origin.x;
      break;
    }
  }
  return available;
}
