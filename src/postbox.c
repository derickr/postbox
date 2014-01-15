#include <pebble.h>

static Window *window;
//static TextLayer *lat_layer;
//static TextLayer *lon_layer;
static TextLayer *ref_layer;
static TextLayer *desc_layer;
static TextLayer *distance_layer;
static TextLayer *timeLayer;

static char ref[15];
static char desc[250];
static char distance[32];

enum {
  QUOTE_KEY_REF = 0x0,
  QUOTE_KEY_DESC = 0x1,
  QUOTE_KEY_DISTANCE = 0x2,
  QUOTE_KEY_VIB = 0x3,
};

static void send_ref(char *ref) {
  Tuplet ref_tuple = TupletCString(QUOTE_KEY_REF, ref);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &ref_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	send_ref(ref);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
//  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void in_received_handler(DictionaryIterator *iter, void *context)
{
	Tuple *ref_tuple = dict_find(iter, QUOTE_KEY_REF);
	Tuple *desc_tuple = dict_find(iter, QUOTE_KEY_DESC);
	Tuple *distance_tuple = dict_find(iter, QUOTE_KEY_DISTANCE);
	Tuple *vib_tuple = dict_find(iter, QUOTE_KEY_VIB);
/*
	Tuple *lat_tuple = dict_find(iter, QUOTE_KEY_LAT);
	Tuple *lon_tuple = dict_find(iter, QUOTE_KEY_LON);
	Tuple *road_tuple = dict_find(iter, QUOTE_KEY_ROAD);
	Tuple *city_tuple = dict_find(iter, QUOTE_KEY_CITY);
	Tuple *village_tuple = dict_find(iter, QUOTE_KEY_VILLAGE);
	Tuple *country_tuple = dict_find(iter, QUOTE_KEY_COUNTRY);

	if (lat_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, lat_tuple->value->cstring);
		strncpy(lat, lat_tuple->value->cstring, 12);
		text_layer_set_text(lat_layer, lat);
	}
	if (lon_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, lon_tuple->value->cstring);
		strncpy(lon, lon_tuple->value->cstring, 12);
		text_layer_set_text(lon_layer, lon);
	}
*/
	if (ref_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, ref_tuple->value->cstring);
		strncpy(ref, ref_tuple->value->cstring, 14);
		text_layer_set_text(ref_layer, ref);
	}
	if (desc_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, desc_tuple->value->cstring);
		strncpy(desc, desc_tuple->value->cstring, 249);
		text_layer_set_text(desc_layer, desc);
	}
	if (distance_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, distance_tuple->value->cstring);
		strncpy(distance, distance_tuple->value->cstring, 31);
		text_layer_set_text(distance_layer, distance);
	}
	if (vib_tuple) {
		static const uint32_t const segments[] = { 100, 100, 200 };
		vibes_cancel();
		VibePattern pat = {
			.durations = segments,
			.num_segments = ARRAY_LENGTH(segments),
		};
		vibes_enqueue_custom_pattern(pat);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrating");
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void app_message_init(void) {
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(200, 200);
  //fetch_msg();
}

// Called once per minute
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  static char timeText[] = "00:00"; // Needs to be static because it's used by the system later.

  time_t now = time(NULL);
  struct tm * currentTime = localtime(&now);
/*
  if (currentTime->tm_min == 0) {
    vibes_enqueue_custom_pattern(HOUR_VIBE_PATTERN);
  } else if ((currentTime->tm_min % VIBE_INTERVAL_IN_MINUTES) == 0) {
    vibes_enqueue_custom_pattern(PART_HOUR_INTERVAL_VIBE_PATTERN);
  }
*/
  strftime(timeText, sizeof(timeText), "%R", currentTime);

APP_LOG(APP_LOG_LEVEL_DEBUG, timeText);
  text_layer_set_text(timeLayer, timeText);

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28);

/*
  lat_layer = text_layer_create( (GRect) { .origin = { 0, 50 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(lat_layer, "lat");
  text_layer_set_text_alignment(lat_layer, GTextAlignmentCenter);
  text_layer_set_font(lat_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(lat_layer));

  lon_layer = text_layer_create( (GRect) { .origin = { 0, 65 }, .size = { bounds.size.w, 50 } });
  text_layer_set_text(lon_layer, "lon");
  text_layer_set_text_alignment(lon_layer, GTextAlignmentCenter);
  text_layer_set_font(lon_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(lon_layer));
*/
  timeLayer = text_layer_create(GRect(0, 0, 144, 54 /* height */));
  text_layer_set_font(timeLayer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(timeLayer, GTextAlignmentCenter);

  ref_layer = text_layer_create( (GRect) { .origin = { 0, 50 }, .size = { bounds.size.w, 24 } });
  text_layer_set_text(ref_layer, "");
  text_layer_set_text_alignment(ref_layer, GTextAlignmentCenter);
  text_layer_set_font(ref_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(ref_layer));

  desc_layer = text_layer_create( (GRect) { .origin = { 2, 80 }, .size = { bounds.size.w - 4, 60 } });
  text_layer_set_text(desc_layer, "Hello there. I am locating the nearest postbox. Stay with me for a bit.");
  text_layer_set_text_alignment(desc_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(desc_layer, GTextOverflowModeFill);
  text_layer_set_font(desc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(desc_layer));

  distance_layer = text_layer_create( (GRect) { .origin = { 0, 140 }, .size = { bounds.size.w, 24 } });
  text_layer_set_text(distance_layer, "");
  text_layer_set_text_alignment(distance_layer, GTextAlignmentCenter);
  text_layer_set_font(distance_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(distance_layer));

  handle_minute_tick(NULL, 0);

  layer_add_child(window_layer, text_layer_get_layer(timeLayer));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);


//  fetch_msg();
}

static void window_unload(Window *window) {
  text_layer_destroy(timeLayer);
  text_layer_destroy(ref_layer);
  text_layer_destroy(desc_layer);
  text_layer_destroy(distance_layer);
}

static void init(void) {
  window = window_create();
  app_message_init();
  /*
  char entry_title[] = "Enter Symbol";
  entry_init(entry_title);
*/
  window_set_click_config_provider(window, click_config_provider);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
