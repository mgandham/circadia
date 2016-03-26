#include <pebble.h>

Window *window;
Window *event_window;
TextLayer *title_layer;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *event_title_layer;
BitmapLayer *draw_layer;
static AppTimer *timer;
static AppTimer *drawtimer;
int is_window_loaded = 0;
int event_counter = 0;
int yesterday_counter = 100;
char stringbuffer[64];
char debugbuffer[10];
int EVENT_TRACKER_KEY = 91;
int YESTERDAY_KEY = 34;
int pixel_multiplier = 2;
//InverterLayer *inverter_layer;
//InverterLayer *event_inverter;

int x = 0;
int y = 0;
int facing = 1; // 0 = left, 1 = right
int last_direction = 0;

GBitmap *gbfly_ld;
GBitmap *gbfly_lu;
GBitmap *gbfly_rd;
GBitmap *gbfly_ru;
GBitmap *selected_gbit;


void handle_timechanges(struct tm *tick_time, TimeUnits units_changed) {
	static char time_buffer[10];
	static char date_buffer[20];
  
  if (units_changed & DAY_UNIT) {
    yesterday_counter = event_counter;
    event_counter = 0;
  }
	
	strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
  
	text_layer_set_text(time_layer, time_buffer);
	
	strftime(date_buffer, sizeof(date_buffer), "%A%n%b %e. %Y", tick_time);
	text_layer_set_text(date_layer, date_buffer);
}

void event_timer_callback(void *data) { // destroy all event window layers
  window_stack_remove(event_window, true);
  
}

void draw_timer_callback(void *data) {
  int direction = rand()%5;
  
  while (((direction == 0) && (last_direction == 0))
         || ( ((direction == 2) || (direction == 5)) &&(facing == 1)&&(last_direction!= 0))
         || ( ((direction == 3) || (direction == 4)) &&(facing == 0)&&(last_direction!= 0))) {
    direction = rand()%4;
  }
  
  if (direction == 0) {         // up
    y = y-(6*pixel_multiplier);
    if (facing == 0) {
      selected_gbit = gbfly_lu;
    } else {
      selected_gbit = gbfly_ru;
    }
  } else if (direction == 1) {  // down
    y=y+(3*pixel_multiplier);
    if (facing == 0) {
      selected_gbit = gbfly_ld;
    } else {
      selected_gbit = gbfly_rd;
    }
  } else if ((direction == 2)||(direction == 5)) { // left
    x=x-(3*pixel_multiplier);
    selected_gbit = gbfly_ld;
    facing = 0;
  } else if ((direction == 3)||(direction == 4)) {  // right
    x=x+(3*pixel_multiplier);
    facing = 1;
    selected_gbit = gbfly_rd;
  }
  
  bitmap_layer_set_bitmap(draw_layer, selected_gbit);
  layer_set_bounds(bitmap_layer_get_layer(draw_layer), GRect(x, y, 144-x, 112-y));
  
  last_direction = direction;
  
  if (is_window_loaded == 1) {
  drawtimer = app_timer_register(100, draw_timer_callback, NULL);
  }
}

void event_window_load(Window *window) {
  
  // generate random int for selecting text message
  
  int text_selector = rand()%4;
  
  if (text_selector == 0) {
    snprintf(stringbuffer, 64, "Daily progress: %d",event_counter);
  } else if (text_selector == 1) {
    snprintf(stringbuffer, 64, "Percent complete: %d",(int)(((float)event_counter/(float)yesterday_counter)*100));
  } else if (text_selector == 2) {
    snprintf(stringbuffer, 64, "Remaining: %d\nGreat work!",yesterday_counter-event_counter);
  } else if (text_selector == 3) {
    snprintf(stringbuffer, 64, "Today: %d\nGoal: %d",event_counter, yesterday_counter);
  }
  
  gbfly_ld = gbitmap_create_with_resource(RESOURCE_ID_BFLY_LD);
  gbfly_lu = gbitmap_create_with_resource(RESOURCE_ID_BFLY_LU);
  gbfly_rd = gbitmap_create_with_resource(RESOURCE_ID_BFLY_RD);
  gbfly_ru = gbitmap_create_with_resource(RESOURCE_ID_BFLY_RU);
  
  draw_layer = bitmap_layer_create(GRect(0, 0, 144, 112));
  layer_set_bounds(bitmap_layer_get_layer(draw_layer),GRect(x, y, 144-x, 112-y));
  bitmap_layer_set_bitmap(draw_layer, gbfly_rd);
  
  drawtimer = app_timer_register(100, draw_timer_callback, NULL);
  
  
  event_title_layer = text_layer_create(GRect(0, 112, 144, 168));
  
  text_layer_set_font(event_title_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(event_title_layer, stringbuffer);
  
  //event_inverter = inverter_layer_create(GRect(0, 112, 144, 168));
  
  
  // add child layers
  Layer *event_root_layer = window_get_root_layer(window);
  layer_add_child(event_root_layer, text_layer_get_layer(event_title_layer));
  layer_add_child(event_root_layer, bitmap_layer_get_layer(draw_layer));
  //layer_add_child(event_root_layer, inverter_layer_get_layer(event_inverter));
  
  timer = app_timer_register(5000, event_timer_callback, NULL);
}

void event_window_unload(Window *window) {
  
  
  
  gbitmap_destroy(gbfly_ld);
  gbitmap_destroy(gbfly_lu);
  gbitmap_destroy(gbfly_rd);
  gbitmap_destroy(gbfly_ru);
  
  bitmap_layer_destroy(draw_layer);
  text_layer_destroy(event_title_layer);
  //inverter_layer_destroy(event_inverter);
  
  window_destroy(window);
  is_window_loaded = 0;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "HIVECTRL: Butterfly unloaded");
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  
  if (is_window_loaded == 0){
    is_window_loaded = 1;
    
    event_counter++;
    
    x = -15;
    y = 5;
    
    //srand(time(NULL));
    //snprintf(debugbuffer, 10, "%d",rand()%4);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, debugbuffer);
    
    
     // create window and graphics layers
    event_window = window_create();
    
    window_set_window_handlers(event_window, (WindowHandlers) {
    .load = event_window_load,
    .unload = event_window_unload
    });
    
    window_stack_push(event_window, true /* Animated */);
    
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "HIVECTRL: Butterfly active");
  }
   
}

void root_window_unload(Window *window) {
  // Destroy the root window layers
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
  text_layer_destroy(title_layer);
	//inverter_layer_destroy(inverter_layer);
}

void root_window_load(Window *window) {
  title_layer = text_layer_create(GRect(0, 0, 144, 56));
  date_layer = text_layer_create(GRect(0, 56, 144, 112));
	time_layer = text_layer_create(GRect(0, 112, 144, 168));

  // title text properties
  text_layer_set_text(title_layer, "Butterfly");
  text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(title_layer, GTextAlignmentLeft);
  layer_set_bounds(text_layer_get_layer(title_layer), GRect(0,10,144,46));
  
  // date layer properties
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
	                  
	// time text properties
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
  
  // inverter layer
	//inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  
	// Add root layers to the window
  Layer *window_root_layer = window_get_root_layer(window);
	layer_add_child(window_root_layer, text_layer_get_layer(title_layer));
  layer_add_child(window_root_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_root_layer, text_layer_get_layer(time_layer));
	//layer_add_child(window_root_layer, inverter_layer_get_layer(inverter_layer));
	
	time_t now = time(NULL);
	handle_timechanges(localtime(&now), MINUTE_UNIT); // immediately updates time on init
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_timechanges);
  accel_tap_service_subscribe(accel_tap_handler);
}

void handle_init(void) {
  // initialize persistent variables
  if (persist_exists(EVENT_TRACKER_KEY)) {
    event_counter = persist_read_int(EVENT_TRACKER_KEY);
  }
  
  if (persist_exists(YESTERDAY_KEY)) {
    yesterday_counter = persist_read_int(YESTERDAY_KEY);
  }
  
	// Create a window and text layers
	window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = root_window_load,
    .unload = root_window_unload
  });
  	
	// Push the window
	window_stack_push(window, true);

}

void handle_deinit(void) {
	// write persistent variables
  persist_write_int(EVENT_TRACKER_KEY, event_counter);
  persist_write_int(YESTERDAY_KEY, yesterday_counter);
  
	// Destroy the window
	window_destroy(window);
  
  // unsubscribe
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}