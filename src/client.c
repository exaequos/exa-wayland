#include <wayland-util.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-client-protocol-core.h>
#include <xdg-shell.h>
#include <xdg-decoration-unstable-v1.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <emscripten.h>

#include <xkbcommon/xkbcommon-compose.h>

#define printf(...)
#define emscripten_log(...)

#define NB_SURFACE_MAX 64
#define EVENT_QUEUE_SIZE 64
#define NB_CALLBACK_MAX 64

#define KEYBOARD_RATE 20
#define KEYBOARD_DELAY  500

#define MOD_SHIFT_INDEX 1
#define MOD_ALT_INDEX   2
#define MOD_CTRL_INDEX  3

struct wl_proxy {

  uint32_t version;
  struct wl_display * wl_display;
  void * data;
  void (**listeners)(void);
  const struct wl_interface * interface;
};

struct event {

  struct wl_proxy * proxy;
  char name[128];
  void * args;
};

struct wl_display {

  struct wl_proxy proxy;
  struct event event_queue[EVENT_QUEUE_SIZE];
  int head;
  int tail;
};

struct wl_registry {

  struct wl_proxy proxy;
};

struct wl_compositor {

  struct wl_proxy proxy;
};

struct wl_shm {

  struct wl_proxy proxy;
};

struct wl_shm_pool {

  struct wl_proxy proxy;
  int fd;
  int size;
};

struct wl_buffer {

  struct wl_proxy proxy;
  int width;
  int height;
  int stride;
  int format;
  int fd;
};

struct xdg_wm_base {

  struct wl_proxy proxy;
};

struct wl_seat {

  struct wl_proxy proxy;
};

struct wl_keyboard {

  struct wl_proxy proxy;
  uint32_t serial;
};

struct wl_pointer {

  struct wl_proxy proxy;
  uint32_t serial;
};

struct wl_surface {

  struct wl_proxy proxy;
  int id;
  struct wl_buffer * buffer;
};

struct xdg_surface {

  struct wl_proxy proxy;
  struct wl_surface * wl_surface;
};

struct zxdg_decoration_manager_v1 {

  struct wl_proxy proxy;
};

struct zxdg_toplevel_decoration_v1 {

  struct wl_proxy proxy;
};

struct xdg_toplevel {

  struct wl_proxy proxy;
  struct xdg_surface * xdg_surface;
  struct zxdg_toplevel_decoration_v1 decoration;
  char title[128];
};

struct wl_callback {

  struct wl_proxy proxy;
  struct wl_surface * wl_surface;
};

struct xkb_keymap {

};

struct xkb_state {

  uint32_t depressed_mods;
  uint32_t latched_mods;
  uint32_t locked_mods;
};


static struct wl_display display = {

  .proxy = {

    .version = 0,
    .wl_display = NULL,
    .interface = &wl_display_interface,
  },
};

static struct wl_surface surfaces[NB_SURFACE_MAX];
static struct xdg_surface xdg_surfaces[NB_SURFACE_MAX];
static struct xdg_toplevel xdg_toplevels[NB_SURFACE_MAX];

static struct wl_shm_pool wl_shm_pools[16];
static struct wl_buffer wl_buffers[16];

static struct wl_callback frame_callbacks[NB_CALLBACK_MAX];

struct xkb_keymap keymap;

struct xkb_state kbd_state;

struct args_usu {

  uint32_t arg1;
  char arg2[512];
  uint32_t arg3;
};

struct args_uhu {

  uint32_t arg1;
  int32_t arg2;
  uint32_t arg3;
};

struct args_u {

  uint32_t arg1;
};

struct args_iia {
  
  int32_t arg1;
  int32_t arg2;
  void * arg3;
};

struct args_uuuu {

  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
  uint32_t arg4;
};

struct args_ii {

  int32_t arg1;
  int32_t arg2;
};

struct args_uuuuu {

  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
  uint32_t arg4;
  uint32_t arg5;
};

struct args_uuf {

  uint32_t arg1;
  uint32_t arg2;
  int32_t arg3;
};

struct args_uff {

  uint32_t arg1;
  int32_t arg2;
  int32_t arg3;
};

void send_event(struct wl_proxy * proxy, const char * name, ...) {

  printf("send_event: %s\n", name);

  display.event_queue[display.head].proxy = proxy;
  strcpy(display.event_queue[display.head].name, name);
  
  va_list ap;

  va_start(ap, name);

  struct wl_interface * interface = proxy->interface;

  int i = 0;

  for (;i < interface->event_count; ++i) {

    if (strcmp(interface->events[i].name, name) == 0) {

      int offset = 0;

	//Signature starts by the minimum version, so we skip it
	
	while ( (interface->events[i].signature[offset] >= '0') && (interface->events[i].signature[offset] <= '9') )
	  ++offset;

      if (strcmp(interface->events[i].signature+offset, "usu") == 0) {

	struct args_usu * args = (struct args_usu *)malloc(sizeof(struct args_usu));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	strcpy(args->arg2, va_arg(ap, const char *));
	args->arg3 = va_arg(ap, uint32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "u") == 0) {

	struct args_u * args = (struct args_u *)malloc(sizeof(struct args_u));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "iia") == 0) {

	struct args_iia * args = (struct args_iia *)malloc(sizeof(struct args_iia));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, int32_t);
	args->arg2 = va_arg(ap, int32_t);
	args->arg3 = va_arg(ap, void *);
      }
      else if (strcmp(interface->events[i].signature+offset, "uhu") == 0) {

	struct args_uhu * args = (struct args_uhu *)malloc(sizeof(struct args_uhu));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	args->arg2 = va_arg(ap, int32_t);
	args->arg3 = va_arg(ap, uint32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "uuuu") == 0) {

	struct args_uuuu * args = (struct args_uuuu *)malloc(sizeof(struct args_uuuu));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	args->arg2 = va_arg(ap, uint32_t);
	args->arg3 = va_arg(ap, uint32_t);
	args->arg4 = va_arg(ap, uint32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "ii") == 0) {

	struct args_ii * args = (struct args_ii *)malloc(sizeof(struct args_ii));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, int32_t);
	args->arg2 = va_arg(ap, int32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "uuuuu") == 0) {

	struct args_uuuuu * args = (struct args_uuuuu *)malloc(sizeof(struct args_uuuu));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	args->arg2 = va_arg(ap, uint32_t);
	args->arg3 = va_arg(ap, uint32_t);
	args->arg4 = va_arg(ap, uint32_t);
	args->arg5 = va_arg(ap, uint32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "uuf") == 0) {

	struct args_uuf * args = (struct args_uuf *)malloc(sizeof(struct args_uuf));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	args->arg2 = va_arg(ap, uint32_t);
	args->arg3 = va_arg(ap, int32_t);
      }
      else if (strcmp(interface->events[i].signature+offset, "uff") == 0) {

	struct args_uff * args = (struct args_uff *)malloc(sizeof(struct args_uff));

	display.event_queue[display.head].args = args;

	args->arg1 = va_arg(ap, uint32_t);
	args->arg2 = va_arg(ap, int32_t);
	args->arg3 = va_arg(ap, int32_t);
      }
      else if (interface->events[i].signature[offset] == 0) {

	display.event_queue[display.head].args = NULL;
      }

      break;
    }
  }
  
  va_end(ap);

  display.head = (display.head+1) % EVENT_QUEUE_SIZE;

  EM_ASM({

      Module['wayland'].queueNotEmpty = 1;
      
      setTimeout(() => {

	  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].queueNotEmpty) ) {

	    // TODO check rw
		      
	    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
	  }
		    
	}, 0);
    });
}

struct wl_display * wl_display_connect(const char *name) {

  EM_ASM_({

      const fd = 0x7e000000;

      let desc = {};

      desc['fd'] = fd;
      desc['remote_fd'] = -1;
      desc['select'] = (fd, rw, start, notif_select) => {

	if (start) {

	  //console.log("select wayland display: fd="+fd+", rw="+rw+", start="+start);

	  if ( (Module['wayland'].events.length > 0) || (Module['wayland'].queueNotEmpty) ) {

	    Module['fd_table'][fd].notif_select = null;
	    notif_select(fd, rw);
	  }
	  else {

	    Module['fd_table'][fd].notif_select = notif_select;
	  }
	}
	else {

	  Module['fd_table'][fd].notif_select = null;
	}
	
      };

      Module['fd_table'][fd] = desc;

      if (!('wayland' in Module)) {

	Module['wayland'] = {};
	Module['wayland'].requests = new Array();
	Module['wayland'].events = new Array();

	Module['wayland'].render = function() {

	  for (const request of Module['wayland'].requests) {

	    //console.log("Wayland client: render -> "+request.type);

	    if (request.type == 'damage_buffer') {

	      // TODO
	    }
	    else if (request.type == 'commit') {

	      const canvas = Module['surfaces'][request.surface_id-1];

	      const ctx = canvas.getContext('2d');

	      if ( (canvas.width != request.width) || (canvas.height = request.height) ) {
	      canvas.width = request.width;
	      canvas.height = request.height;

	      canvas.style.width = request.width / window.devicePixelRatio + "px";
	      canvas.style.height = request.height / window.devicePixelRatio + "px";
	      canvas.parentElement.style.width = request.width / window.devicePixelRatio + "px";
	      }
	
	      const pixels = new Uint8ClampedArray(Module.HEAPU8.buffer, Module['shm'].fds[request.shm_fd-0x7f000000].mem, Module['shm'].fds[request.shm_fd-0x7f000000].len);

	      const imageData = new ImageData(pixels, request.width, request.height);
	
	      ctx.putImageData(imageData, 0, 0);

	      Module['wayland'].events.push({

		'type': 1, // buffer released
		'surface_id': request.surface_id,
		'shm_fd': request.shm_fd
		});

	      Module['wayland'].events.push({

		'type': 2, // frame done
		'surface_id': request.surface_id,
		'timestamp': new Date().getTime()
		});
	    }
	  }

	  if (Module['wayland'].events.length > 0) {

	    setTimeout(() => {

		  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

		    // TODO check rw
		      
		    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
		  }
		    
		}, 0);
	  }
	  
	  Module['wayland'].requests = new Array();

	  window.requestAnimationFrame(Module['wayland'].render);
	};

	Module['wayland'].queueNotEmpty = 0;
      }
    });
  
  for (int i = 0; i < NB_SURFACE_MAX; ++i) {

    surfaces[i].id = -1;
    xdg_surfaces[i].wl_surface = NULL;
    xdg_toplevels[i].xdg_surface = NULL;
  }

  for (int i = 0; i < NB_CALLBACK_MAX; ++i) {

    frame_callbacks[i].wl_surface = NULL;
  }

  display.head = 0;
  display.tail = 0;
  
  return &display;
}

void wl_display_disconnect(struct wl_display *display) {

  emscripten_log(EM_LOG_CONSOLE, "--> wl_display_disconnect");

  EM_ASM({
      Module.iframeShown = true;

      let m = new Object();
	
      m.type = 6; // hide iframe
      m.pid = Module.getpid() & 0x0000ffff;

      window.parent.postMessage(m);
    });
}

  /*interface: 'wl_compositor', version: 5, name: 1
interface: 'wl_drm', version: 2, name: 2
interface: 'wl_shm', version: 1, name: 3
interface: 'wl_output', version: 3, name: 4
interface: 'zxdg_output_manager_v1', version: 3, name: 5
interface: 'wl_data_device_manager', version: 3, name: 6
interface: 'zwp_primary_selection_device_manager_v1', version: 1, name: 7
interface: 'wl_subcompositor', version: 1, name: 8
interface: 'xdg_wm_base', version: 4, name: 9
interface: 'gtk_shell1', version: 5, name: 10
interface: 'wp_viewporter', version: 1, name: 11
interface: 'zwp_pointer_gestures_v1', version: 3, name: 12
interface: 'zwp_tablet_manager_v2', version: 1, name: 13
interface: 'wl_seat', version: 8, name: 14
interface: 'zwp_relative_pointer_manager_v1', version: 1, name: 15
interface: 'zwp_pointer_constraints_v1', version: 1, name: 16
interface: 'zxdg_exporter_v1', version: 1, name: 17
interface: 'zxdg_importer_v1', version: 1, name: 18
interface: 'zwp_linux_dmabuf_v1', version: 4, name: 19
interface: 'wp_single_pixel_buffer_manager_v1', version: 1, name: 20
interface: 'zwp_keyboard_shortcuts_inhibit_manager_v1', version: 1, name: 21
interface: 'zwp_text_input_manager_v3', version: 1, name: 22
interface: 'wp_presentation', version: 1, name: 23
interface: 'xdg_activation_v1', version: 1, name: 24*/

extern const struct wl_interface wl_registry_interface;

static struct wl_registry registry = {

  .proxy = {

    .version = 0,
    .wl_display = &display,
    .interface = &wl_registry_interface,
  },
};


void wl_proxy_destroy(struct wl_proxy *proxy) {

}

uint32_t wl_proxy_get_version(struct wl_proxy * proxy)
{
	return proxy->version;
}

int wl_display_roundtrip(struct wl_display * display) {

  while (display->head != display->tail) {

    struct wl_interface * interface = display->event_queue[display->tail].proxy->interface;

    int i = 0;

    for (;i < interface->event_count; ++i) {

      if (strcmp(interface->events[i].name, display->event_queue[display->tail].name) == 0) {

	printf("found event: %d %s\n", i, interface->events[i].signature);

	int offset = 0;

	//Signature starts by the minimum version, so we skip it
	
	while ( (interface->events[i].signature[offset] >= '0') && (interface->events[i].signature[offset] <= '9') )
	  ++offset;

	if (strcmp(interface->events[i].signature+offset, "usu") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, const char *, uint32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_usu * args = (struct args_usu * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3);
	}
	else if (strcmp(interface->events[i].signature+offset, "u") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_u * args = (struct args_u * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1);
	}
	else if (strcmp(interface->events[i].signature+offset, "iia") == 0) {

	  void (*listener)(void *, struct wl_proxy *, int32_t, int32_t, struct wl_array *) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_iia * args = (struct args_iia * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3);
	}
	else if (strcmp(interface->events[i].signature+offset, "uhu") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, int32_t, uint32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_uhu * args = (struct args_uhu * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3);
	}
	else if (strcmp(interface->events[i].signature+offset, "uuuu") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, uint32_t, uint32_t, uint32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_uuuu * args = (struct args_uuuu * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3, args->arg4);
	}
	else if (strcmp(interface->events[i].signature+offset, "ii") == 0) {

	  void (*listener)(void *, struct wl_proxy *, int32_t, int32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_ii * args = (struct args_ii * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2);
	}
	else if (strcmp(interface->events[i].signature+offset, "uuuuu") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_uuuuu * args = (struct args_uuuu * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);
	}
	else if (strcmp(interface->events[i].signature+offset, "uuf") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, uint32_t, int32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_uuf * args = (struct args_uuf * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3);
	}
	else if (strcmp(interface->events[i].signature+offset, "uff") == 0) {

	  void (*listener)(void *, struct wl_proxy *, uint32_t, int32_t, int32_t) = display->event_queue[display->tail].proxy->listeners[i];

	  struct args_uff * args = (struct args_uff * )display->event_queue[display->tail].args;

	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy, args->arg1, args->arg2, args->arg3);
	}
	else if (interface->events[i].signature[offset] == 0) {

	  void (*listener)(void *, struct wl_proxy *) = display->event_queue[display->tail].proxy->listeners[i];
	  
	  if (listener)
	    (*listener)(display->event_queue[display->tail].proxy->data, display->event_queue[display->tail].proxy);
	}
	
	break;
      }
    }

    if (display->event_queue[display->tail].args)
      free(display->event_queue[display->tail].args);

    display->tail = (display->tail +1) % EVENT_QUEUE_SIZE;
  }

  EM_ASM({

      Module['wayland'].queueNotEmpty = 0;
    });
  
  return 0;
}


static const struct wl_interface *wayland_types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	/*&wl_callback_interface*/NULL,
	/*&wl_registry_interface*/NULL,
	/*&wl_surface_interface*/NULL,
	/*&wl_region_interface*/NULL,
	/*&wl_buffer_interface*/NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	/*&wl_shm_pool_interface*/NULL,
	NULL,
	NULL,
	#if 0
	&wl_data_source_interface,
	&wl_surface_interface,
	&wl_surface_interface,
	NULL,
	&wl_data_source_interface,
	NULL,
	&wl_data_offer_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	&wl_data_offer_interface,
	&wl_data_offer_interface,
	&wl_data_source_interface,
	&wl_data_device_interface,
	&wl_seat_interface,
	&wl_shell_surface_interface,
	&wl_surface_interface,
	&wl_seat_interface,
	NULL,
	&wl_seat_interface,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&wl_output_interface,
	&wl_seat_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	&wl_output_interface,
	&wl_buffer_interface,
	NULL,
	NULL,
	&wl_callback_interface,
	&wl_region_interface,
	&wl_region_interface,
	&wl_output_interface,
	&wl_output_interface,
	&wl_pointer_interface,
	&wl_keyboard_interface,
	&wl_touch_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	&wl_subsurface_interface,
	&wl_surface_interface,
	&wl_surface_interface,
	&wl_surface_interface,
	&wl_surface_interface,
	#endif
};

static const struct wl_message wl_display_requests[] = {
	{ "sync", "n", wayland_types + 8 },
	{ "get_registry", "n", wayland_types + 9 },
};

static const struct wl_message wl_display_events[] = {
	{ "error", "ous", wayland_types + 0 },
	{ "delete_id", "u", wayland_types + 0 },
};

const struct wl_interface wl_display_interface = {
	"wl_display", 1,
	2, wl_display_requests,
	2, wl_display_events,
};

static const struct wl_message wl_registry_requests[] = {
	{ "bind", "usun", wayland_types + 0 },
};

static const struct wl_message wl_registry_events[] = {
	{ "global", "usu", wayland_types + 0 },
	{ "global_remove", "u", wayland_types + 0 },
};

const struct wl_interface wl_registry_interface = {
	"wl_registry", 1,
	1, wl_registry_requests,
	2, wl_registry_events,
};

static const struct wl_message wl_callback_events[] = {
	{ "done", "u", wayland_types + 0 },
};

const struct wl_interface wl_callback_interface = {
	"wl_callback", 1,
	0, NULL,
	1, wl_callback_events,
};

static const struct wl_message wl_compositor_requests[] = {
	{ "create_surface", "n", wayland_types + 10 },
	{ "create_region", "n", wayland_types + 11 },
};

const struct wl_interface wl_compositor_interface = {
	"wl_compositor", 6,
	2, wl_compositor_requests,
	0, NULL,
};

static const struct wl_message wl_shm_pool_requests[] = {
	{ "create_buffer", "niiiiu", wayland_types + 12 },
	{ "destroy", "", wayland_types + 0 },
	{ "resize", "i", wayland_types + 0 },
};

const struct wl_interface wl_shm_pool_interface = {
	"wl_shm_pool", 1,
	3, wl_shm_pool_requests,
	0, NULL,
};

static const struct wl_message wl_shm_requests[] = {
	{ "create_pool", "nhi", wayland_types + 18 },
};

static const struct wl_message wl_shm_events[] = {
	{ "format", "u", wayland_types + 0 },
};

const struct wl_interface wl_shm_interface = {
	"wl_shm", 1,
	1, wl_shm_requests,
	1, wl_shm_events,
};


static const struct wl_message wl_surface_requests[] = {
	{ "destroy", "", wayland_types + 0 },
	{ "attach", "?oii", wayland_types + 58 },
	{ "damage", "iiii", wayland_types + 0 },
	{ "frame", "n", wayland_types + 61 },
	{ "set_opaque_region", "?o", wayland_types + 62 },
	{ "set_input_region", "?o", wayland_types + 63 },
	{ "commit", "", wayland_types + 0 },
	{ "set_buffer_transform", "2i", wayland_types + 0 },
	{ "set_buffer_scale", "3i", wayland_types + 0 },
	{ "damage_buffer", "4iiii", wayland_types + 0 },
	{ "offset", "5ii", wayland_types + 0 },
};

static const struct wl_message wl_surface_events[] = {
	{ "enter", "o", wayland_types + 64 },
	{ "leave", "o", wayland_types + 65 },
	{ "preferred_buffer_scale", "6i", wayland_types + 0 },
	{ "preferred_buffer_transform", "6u", wayland_types + 0 },
};

const struct wl_interface wl_surface_interface = {
	"wl_surface", 6,
	11, wl_surface_requests,
	4, wl_surface_events,
};

static const struct wl_message wl_buffer_requests[] = {
	{ "destroy", "", wayland_types + 0 },
};

static const struct wl_message wl_buffer_events[] = {
	{ "release", "", wayland_types + 0 },
};

const struct wl_interface wl_buffer_interface = {
	"wl_buffer", 1,
	1, wl_buffer_requests,
	1, wl_buffer_events,
};

static const struct wl_interface *xdg_shell_types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	/*&xdg_positioner_interface*/NULL,
	/*&xdg_surface_interface*/NULL,
	/*&wl_surface_interface*/NULL,
	/*&xdg_toplevel_interface*/NULL,
	#if 0
	&xdg_popup_interface,
	&xdg_surface_interface,
	&xdg_positioner_interface,
	&xdg_toplevel_interface,
	&wl_seat_interface,
	NULL,
	NULL,
	NULL,
	&wl_seat_interface,
	NULL,
	&wl_seat_interface,
	NULL,
	NULL,
	&wl_output_interface,
	&wl_seat_interface,
	NULL,
	&xdg_positioner_interface,
	NULL,
	#endif
};

static const struct wl_message xdg_wm_base_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "create_positioner", "n", xdg_shell_types + 4 },
	{ "get_xdg_surface", "no", xdg_shell_types + 5 },
	{ "pong", "u", xdg_shell_types + 0 },
};

static const struct wl_message xdg_wm_base_events[] = {
	{ "ping", "u", xdg_shell_types + 0 },
};

const struct wl_interface xdg_wm_base_interface = {
	"xdg_wm_base", 6,
	4, xdg_wm_base_requests,
	1, xdg_wm_base_events,
};

static const struct wl_message xdg_surface_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "get_toplevel", "n", xdg_shell_types + 7 },
	{ "get_popup", "n?oo", xdg_shell_types + 8 },
	{ "set_window_geometry", "iiii", xdg_shell_types + 0 },
	{ "ack_configure", "u", xdg_shell_types + 0 },
};

static const struct wl_message xdg_surface_events[] = {
	{ "configure", "u", xdg_shell_types + 0 },
};

const struct wl_interface xdg_surface_interface = {
	"xdg_surface", 6,
	5, xdg_surface_requests,
	1, xdg_surface_events,
};

static const struct wl_message xdg_toplevel_requests[] = {
	{ "destroy", "", xdg_shell_types + 0 },
	{ "set_parent", "?o", xdg_shell_types + 11 },
	{ "set_title", "s", xdg_shell_types + 0 },
	{ "set_app_id", "s", xdg_shell_types + 0 },
	{ "show_window_menu", "ouii", xdg_shell_types + 12 },
	{ "move", "ou", xdg_shell_types + 16 },
	{ "resize", "ouu", xdg_shell_types + 18 },
	{ "set_max_size", "ii", xdg_shell_types + 0 },
	{ "set_min_size", "ii", xdg_shell_types + 0 },
	{ "set_maximized", "", xdg_shell_types + 0 },
	{ "unset_maximized", "", xdg_shell_types + 0 },
	{ "set_fullscreen", "?o", xdg_shell_types + 21 },
	{ "unset_fullscreen", "", xdg_shell_types + 0 },
	{ "set_minimized", "", xdg_shell_types + 0 },
};

static const struct wl_message xdg_toplevel_events[] = {
	{ "configure", "iia", xdg_shell_types + 0 },
	{ "close", "", xdg_shell_types + 0 },
	{ "configure_bounds", "4ii", xdg_shell_types + 0 },
	{ "wm_capabilities", "5a", xdg_shell_types + 0 },
};

const struct wl_interface xdg_toplevel_interface = {
	"xdg_toplevel", 6,
	14, xdg_toplevel_requests,
	4, xdg_toplevel_events,
};

static const struct wl_message wl_data_offer_requests[] = {
	{ "accept", "u?s", wayland_types + 0 },
	{ "receive", "sh", wayland_types + 0 },
	{ "destroy", "", wayland_types + 0 },
	{ "finish", "3", wayland_types + 0 },
	{ "set_actions", "3uu", wayland_types + 0 },
};

static const struct wl_message wl_data_offer_events[] = {
	{ "offer", "s", wayland_types + 0 },
	{ "source_actions", "3u", wayland_types + 0 },
	{ "action", "3u", wayland_types + 0 },
};

const struct wl_interface wl_data_offer_interface = {
	"wl_data_offer", 3,
	5, wl_data_offer_requests,
	3, wl_data_offer_events,
};

static const struct wl_message wl_data_source_requests[] = {
	{ "offer", "s", wayland_types + 0 },
	{ "destroy", "", wayland_types + 0 },
	{ "set_actions", "3u", wayland_types + 0 },
};

static const struct wl_message wl_data_source_events[] = {
	{ "target", "?s", wayland_types + 0 },
	{ "send", "sh", wayland_types + 0 },
	{ "cancelled", "", wayland_types + 0 },
	{ "dnd_drop_performed", "3", wayland_types + 0 },
	{ "dnd_finished", "3", wayland_types + 0 },
	{ "action", "3u", wayland_types + 0 },
};

const struct wl_interface wl_data_source_interface = {
	"wl_data_source", 3,
	3, wl_data_source_requests,
	6, wl_data_source_events,
};

static const struct wl_message wl_data_device_requests[] = {
	{ "start_drag", "?oo?ou", wayland_types + 21 },
	{ "set_selection", "?ou", wayland_types + 25 },
	{ "release", "2", wayland_types + 0 },
};

static const struct wl_message wl_data_device_events[] = {
	{ "data_offer", "n", wayland_types + 27 },
	{ "enter", "uoff?o", wayland_types + 28 },
	{ "leave", "", wayland_types + 0 },
	{ "motion", "uff", wayland_types + 0 },
	{ "drop", "", wayland_types + 0 },
	{ "selection", "?o", wayland_types + 33 },
};

const struct wl_interface wl_data_device_interface = {
	"wl_data_device", 3,
	3, wl_data_device_requests,
	6, wl_data_device_events,
};

static const struct wl_message wl_data_device_manager_requests[] = {
	{ "create_data_source", "n", wayland_types + 34 },
	{ "get_data_device", "no", wayland_types + 35 },
};

const struct wl_interface wl_data_device_manager_interface = {
	"wl_data_device_manager", 3,
	2, wl_data_device_manager_requests,
	0, NULL,
};

static const struct wl_message wl_seat_requests[] = {
	{ "get_pointer", "n", wayland_types + 66 },
	{ "get_keyboard", "n", wayland_types + 67 },
	{ "get_touch", "n", wayland_types + 68 },
	{ "release", "5", wayland_types + 0 },
};

static const struct wl_message wl_seat_events[] = {
	{ "capabilities", "u", wayland_types + 0 },
	{ "name", "2s", wayland_types + 0 },
};

const struct wl_interface wl_seat_interface = {
	"wl_seat", 6,
	4, wl_seat_requests,
	2, wl_seat_events,
};

static const struct wl_message wl_pointer_requests[] = {
	{ "set_cursor", "u?oii", wayland_types + 69 },
	{ "release", "3", wayland_types + 0 },
};

static const struct wl_message wl_pointer_events[] = {
	{ "enter", "uoff", wayland_types + 73 },
	{ "leave", "uo", wayland_types + 77 },
	{ "motion", "uff", wayland_types + 0 },
	{ "button", "uuuu", wayland_types + 0 },
	{ "axis", "uuf", wayland_types + 0 },
	{ "frame", "5", wayland_types + 0 },
	{ "axis_source", "5u", wayland_types + 0 },
	{ "axis_stop", "5uu", wayland_types + 0 },
	{ "axis_discrete", "5ui", wayland_types + 0 },
};

const struct wl_interface wl_pointer_interface = {
	"wl_pointer", 6,
	2, wl_pointer_requests,
	9, wl_pointer_events,
};

static const struct wl_message wl_keyboard_requests[] = {
	{ "release", "3", wayland_types + 0 },
};

static const struct wl_message wl_keyboard_events[] = {
	{ "keymap", "uhu", wayland_types + 0 },
	{ "enter", "uoa", wayland_types + 79 },
	{ "leave", "uo", wayland_types + 82 },
	{ "key", "uuuu", wayland_types + 0 },
	{ "modifiers", "uuuuu", wayland_types + 0 },
	{ "repeat_info", "4ii", wayland_types + 0 },
};

const struct wl_interface wl_keyboard_interface = {
	"wl_keyboard", 6,
	1, wl_keyboard_requests,
	6, wl_keyboard_events,
};

static const struct wl_message wl_touch_requests[] = {
	{ "release", "3", wayland_types + 0 },
};

static const struct wl_message wl_touch_events[] = {
	{ "down", "uuoiff", wayland_types + 84 },
	{ "up", "uui", wayland_types + 0 },
	{ "motion", "uiff", wayland_types + 0 },
	{ "frame", "", wayland_types + 0 },
	{ "cancel", "", wayland_types + 0 },
	{ "shape", "6iff", wayland_types + 0 },
	{ "orientation", "6if", wayland_types + 0 },
};

const struct wl_interface wl_touch_interface = {
	"wl_touch", 6,
	1, wl_touch_requests,
	7, wl_touch_events,
};

extern const struct wl_interface zwp_primary_selection_device_v1_interface;
extern const struct wl_interface zwp_primary_selection_offer_v1_interface;
extern const struct wl_interface zwp_primary_selection_source_v1_interface;

static const struct wl_interface *wp_primary_selection_unstable_v1_types[] = {
	NULL,
	NULL,
	&zwp_primary_selection_source_v1_interface,
	&zwp_primary_selection_device_v1_interface,
	&wl_seat_interface,
	&zwp_primary_selection_source_v1_interface,
	NULL,
	&zwp_primary_selection_offer_v1_interface,
	&zwp_primary_selection_offer_v1_interface,
};

static const struct wl_message zwp_primary_selection_device_manager_v1_requests[] = {
	{ "create_source", "n", wp_primary_selection_unstable_v1_types + 2 },
	{ "get_device", "no", wp_primary_selection_unstable_v1_types + 3 },
	{ "destroy", "", wp_primary_selection_unstable_v1_types + 0 },
};

const struct wl_interface zwp_primary_selection_device_manager_v1_interface = {
	"zwp_primary_selection_device_manager_v1", 1,
	3, zwp_primary_selection_device_manager_v1_requests,
	0, NULL,
};

static const struct wl_message zwp_primary_selection_device_v1_requests[] = {
	{ "set_selection", "?ou", wp_primary_selection_unstable_v1_types + 5 },
	{ "destroy", "", wp_primary_selection_unstable_v1_types + 0 },
};

static const struct wl_message zwp_primary_selection_device_v1_events[] = {
	{ "data_offer", "n", wp_primary_selection_unstable_v1_types + 7 },
	{ "selection", "?o", wp_primary_selection_unstable_v1_types + 8 },
};

const struct wl_interface zwp_primary_selection_device_v1_interface = {
	"zwp_primary_selection_device_v1", 1,
	2, zwp_primary_selection_device_v1_requests,
	2, zwp_primary_selection_device_v1_events,
};

static const struct wl_message zwp_primary_selection_offer_v1_requests[] = {
	{ "receive", "sh", wp_primary_selection_unstable_v1_types + 0 },
	{ "destroy", "", wp_primary_selection_unstable_v1_types + 0 },
};

static const struct wl_message zwp_primary_selection_offer_v1_events[] = {
	{ "offer", "s", wp_primary_selection_unstable_v1_types + 0 },
};

const struct wl_interface zwp_primary_selection_offer_v1_interface = {
	"zwp_primary_selection_offer_v1", 1,
	2, zwp_primary_selection_offer_v1_requests,
	1, zwp_primary_selection_offer_v1_events,
};

static const struct wl_message zwp_primary_selection_source_v1_requests[] = {
	{ "offer", "s", wp_primary_selection_unstable_v1_types + 0 },
	{ "destroy", "", wp_primary_selection_unstable_v1_types + 0 },
};

static const struct wl_message zwp_primary_selection_source_v1_events[] = {
	{ "send", "sh", wp_primary_selection_unstable_v1_types + 0 },
	{ "cancelled", "", wp_primary_selection_unstable_v1_types + 0 },
};

const struct wl_interface zwp_primary_selection_source_v1_interface = {
	"zwp_primary_selection_source_v1", 1,
	2, zwp_primary_selection_source_v1_requests,
	2, zwp_primary_selection_source_v1_events,
};

extern const struct wl_interface xdg_toplevel_interface;
extern const struct wl_interface zxdg_toplevel_decoration_v1_interface;

static const struct wl_interface *xdg_decoration_unstable_v1_types[] = {
	NULL,
	&zxdg_toplevel_decoration_v1_interface,
	&xdg_toplevel_interface,
};

static const struct wl_message zxdg_decoration_manager_v1_requests[] = {
	{ "destroy", "", xdg_decoration_unstable_v1_types + 0 },
	{ "get_toplevel_decoration", "no", xdg_decoration_unstable_v1_types + 1 },
};

const struct wl_interface zxdg_decoration_manager_v1_interface = {
	"zxdg_decoration_manager_v1", 1,
	2, zxdg_decoration_manager_v1_requests,
	0, NULL,
};

static const struct wl_message zxdg_toplevel_decoration_v1_requests[] = {
	{ "destroy", "", xdg_decoration_unstable_v1_types + 0 },
	{ "set_mode", "u", xdg_decoration_unstable_v1_types + 0 },
	{ "unset_mode", "", xdg_decoration_unstable_v1_types + 0 },
};

static const struct wl_message zxdg_toplevel_decoration_v1_events[] = {
	{ "configure", "u", xdg_decoration_unstable_v1_types + 0 },
};

const struct wl_interface zxdg_toplevel_decoration_v1_interface = {
	"zxdg_toplevel_decoration_v1", 1,
	3, zxdg_toplevel_decoration_v1_requests,
	1, zxdg_toplevel_decoration_v1_events,
};




static struct wl_compositor compositor = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &wl_compositor_interface,
  },
};

static struct wl_shm shm = {

  .proxy = {

    .version = 0,
    .wl_display = &display,
    .interface = &wl_shm_interface,
  },
};

static struct xdg_wm_base xdg_wm_base = {

  .proxy = {

    .version = 0,
    .wl_display = &display,
    .interface = &xdg_wm_base_interface,
  },
};

static struct wl_seat seat = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &wl_seat_interface,
  },
};

static struct wl_keyboard keyboard = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &wl_keyboard_interface,
  },
  .serial = 0,
};

static struct wl_pointer pointer = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &wl_pointer_interface,
  },
  .serial = 0,
};

static struct zxdg_decoration_manager_v1 decoration_manager = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &zxdg_decoration_manager_v1_interface,
  },
};

/*static struct zxdg_toplevel_decoration_v1 toplevel_decoration = {

  .proxy = {
    
    .version = 0,
    .wl_display = &display,
    .interface = &zxdg_toplevel_decoration_v1_interface,
  },
  };*/

struct wl_proxy *
wl_proxy_marshal_flags(struct wl_proxy *proxy, uint32_t opcode,
		       const struct wl_interface *interface, uint32_t version,
		       uint32_t flags, ...) {

  //printf("wl_proxy_marshal_flags: %p %p %p %d\n", proxy, proxy->interface, interface, opcode);

  if (!proxy || !proxy->interface || !proxy->interface->name)
    return NULL;

  //printf("wl_proxy_marshal_flags: %s %d\n", proxy->interface->name, opcode);

  if ( (strcmp(proxy->interface->name, "wl_display") == 0) &&
       (opcode == WL_DISPLAY_GET_REGISTRY) ) {
    
    send_event((struct wl_proxy *) &registry, "global", 1, "wl_compositor", 5);
    send_event((struct wl_proxy *) &registry, "global", 2, "wl_shm", 1);
    send_event((struct wl_proxy *) &registry, "global", 3, "xdg_wm_base", 4);
    send_event((struct wl_proxy *) &registry, "global", 4, "wl_seat", 8);
    send_event((struct wl_proxy *) &registry, "global", 5, "zxdg_decoration_manager_v1", 1);
    
    return (struct wl_proxy *)&registry;
  }
  else if ( (strcmp(proxy->interface->name, "wl_registry") == 0) &&
       (opcode == WL_REGISTRY_BIND) ) {

    if (!interface)
      return NULL;
    
    if (strcmp(interface->name, "wl_compositor") == 0) {
      
      return (struct wl_proxy *)&compositor;
    }
    else if (strcmp(interface->name, "wl_shm") == 0) {

      return (struct wl_proxy *)&shm;
    }
    else if (strcmp(interface->name, "xdg_wm_base") == 0) {

      return (struct wl_proxy *)&xdg_wm_base;
    }
    else if (strcmp(interface->name, "wl_seat") == 0) {

      return (struct wl_proxy *)&seat;
    }
    else if (strcmp(interface->name, "zxdg_decoration_manager_v1") == 0) {

      return (struct wl_proxy *)&decoration_manager;
    }
  }
  else if ( (strcmp(proxy->interface->name, "wl_compositor") == 0) &&
       (opcode == WL_COMPOSITOR_CREATE_SURFACE) ) {

    printf("WL_COMPOSITOR_CREATE_SURFACE\n");

    int id = EM_ASM_INT({

	//console.log("degas client: Create surface");

	const newCanvas = document.createElement("canvas");

	if (!Module['surfaces'])
	  Module['surfaces'] = new Array();

	Module['surfaces'].push(newCanvas);

	const id = Module['surfaces'].length;

	newCanvas.addEventListener("mouseenter", (event) => {

	    console.log("mouseenter");
	    
	  });

	newCanvas.addEventListener("mouseleave", (event) => {

	    console.log("mouseleave");
	    
	  });

	newCanvas.addEventListener("mousemove", (event) => {

	    if (Module.selected_toplevel)
	      return;

	    if (Module.pointerListener) {

	      event.preventDefault();
	      event.stopPropagation();

	      //console.log(event);
	      
	      Module['wayland'].events.push({

		'type': 9, // mousemove
		'id': id,
		'x': event.offsetX * window.devicePixelRatio,
		'y': event.offsetY * window.devicePixelRatio
		});

	      setTimeout(() => {

		  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

		    // TODO check rw
		      
		    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
		  }
		    
		}, 0);
	    }
	    
	  });
	
	newCanvas.addEventListener("mousedown", (event) => {

	    if (Module.selected_toplevel)
	      return;
	    
	    if (Module.pointerListener) {

	      event.preventDefault();
	      event.stopPropagation();

	      //console.log(event);
	      
	      Module['wayland'].events.push({

		'type': 8, // button
		'id': id,
		'state': 1, // pressed
		'button': event.button
		});

	      setTimeout(() => {

		  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

		    // TODO check rw
		      
		    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
		  }
		    
		}, 0);
	    }
	    
	  });

	newCanvas.addEventListener("mouseup", (event) => {

	    if (Module.selected_toplevel)
	      return;
	    
	    if (Module.pointerListener) {

	      event.preventDefault();
	      event.stopPropagation();
	      
	      //console.log(event);

	      Module['wayland'].events.push({

		'type': 8, // button
		'id': id,
		'state': 0, // released
		'button': event.button
		});

	      setTimeout(() => {

		  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

		    // TODO check rw
		      
		    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
		  }
		    
		}, 0);
	    }
	    
	  });

	newCanvas.addEventListener("wheel", (event) => {

	    if (Module.selected_toplevel)
	      return;
	    
	    if (Module.pointerListener) {

	      //console.log("wheel");
	      //console.log(event);

	      Module['wayland'].events.push({

		'type': 7, // wheel
		'id': id,
		'deltaX': event.deltaX * window.devicePixelRatio,
		'deltaY': event.deltaY * window.devicePixelRatio,
		'deltaMode': event.deltaMode
		});

	      setTimeout(() => {

		  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

		    // TODO check rw
		      
		    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
		  }
		    
		}, 0);
	      
	    }
	    
	  });
	
	return id;
      });
    
    printf("WL_COMPOSITOR_CREATE_SURFACE: %d\n", id);

    for (int i = 0; i < NB_SURFACE_MAX; ++i) {

      if (surfaces[i].id < 0) {

	surfaces[i].id = id;
	surfaces[i].buffer = NULL;
	surfaces[i].proxy.version = 0;
	surfaces[i].proxy.wl_display = &display;
	surfaces[i].proxy.interface = &wl_surface_interface;

	printf("WL_COMPOSITOR_CREATE_SURFACE: wl_surface=%p\n", &surfaces[i]);

	return (struct wl_proxy *)&surfaces[i];
      }
    }
  }
  else if ( (strcmp(proxy->interface->name, "xdg_wm_base") == 0) &&
       (opcode == XDG_WM_BASE_GET_XDG_SURFACE) ) {

    va_list ap;

    va_start(ap, flags);

    void * dummy = va_arg(ap, void*);

    struct wl_surface * wl_surface = va_arg(ap, struct wl_surface*);
    
    va_end(ap);

    for (int i = 0; i < NB_SURFACE_MAX; ++i) {

      if (xdg_surfaces[i].wl_surface == wl_surface) {

	printf("XDG_WM_BASE_GET_XDG_SURFACE: %p\n", &xdg_surfaces[i]);

	return (struct wl_proxy *)&xdg_surfaces[i];
      }
      else if (xdg_surfaces[i].wl_surface == NULL) {

	xdg_surfaces[i].wl_surface = wl_surface;
	xdg_surfaces[i].proxy.version = 0;
	xdg_surfaces[i].proxy.wl_display = &display;
	xdg_surfaces[i].proxy.interface = &xdg_surface_interface;

	printf("XDG_WM_BASE_GET_XDG_SURFACE: %p (wl_surface=%p)\n", &xdg_surfaces[i], wl_surface);

	return (struct wl_proxy *)&xdg_surfaces[i];
      }
    }
  }
  else if ( (strcmp(proxy->interface->name, "xdg_surface") == 0) &&
       (opcode == XDG_SURFACE_GET_TOPLEVEL) ) {

    printf("XDG_SURFACE_GET_TOPLEVEL: xdg_surface=%p wl_surface=%p id=%d\n", proxy, ((struct xdg_surface *)proxy)->wl_surface, ((struct xdg_surface *)proxy)->wl_surface->id);

    EM_ASM({

	let div = document.createElement("div");

	div.style.position = "absolute";

	div.style.left = Math.floor(Math.random() * 100) + "px";
	div.style.top = Math.floor(Math.random() * 100) + "px";

	let deco = document.createElement("div");
	deco.id = "deco";

	div.appendChild(deco);

	div.appendChild(Module['surfaces'][$0-1]);
	  
	document.body.appendChild(div);

      }, ((struct xdg_surface *)proxy)->wl_surface->id);

    for (int i = 0; i < NB_SURFACE_MAX; ++i) {

      if (xdg_toplevels[i].xdg_surface == NULL) {

	xdg_toplevels[i].xdg_surface = (struct xdg_surface *)proxy;
	xdg_toplevels[i].proxy.version = 0;
	xdg_toplevels[i].proxy.wl_display = &display;
	xdg_toplevels[i].proxy.interface = &xdg_toplevel_interface;
	
	printf("XDG_SURFACE_GET_TOPLEVEL: %p\n", &xdg_toplevels[i]);

	return (struct wl_proxy *)&xdg_toplevels[i];
      }
    }
  }
  else if ( (strcmp(proxy->interface->name, "xdg_surface") == 0) &&
       (opcode == XDG_SURFACE_ACK_CONFIGURE) ) {

    printf("XDG_SURFACE_ACK_CONFIGURE\n");

    EM_ASM({

	window.requestAnimationFrame(Module['wayland'].render);
      });
  }
  else if ( (strcmp(proxy->interface->name, "xdg_toplevel") == 0) &&
       (opcode == XDG_TOPLEVEL_SET_TITLE) ) {

    va_list ap;

    va_start(ap, flags);

    char * title = (char *)va_arg(ap, char *);
    
    va_end(ap);

    if (title)
      strcpy(((struct xdg_toplevel *)proxy)->title, title);
    else
      ((struct xdg_toplevel *)proxy)->title[0] = 0;
  }
  else if ( (strcmp(proxy->interface->name, "xdg_toplevel") == 0) &&
       (opcode == XDG_TOPLEVEL_DESTROY) ) {

    emscripten_log(EM_LOG_CONSOLE, "XDG_TOPLEVEL_DESTROY");
  }
  else if ( (strcmp(proxy->interface->name, "wl_surface") == 0) &&
       (opcode == WL_SURFACE_COMMIT) ) {

    printf("WL_SURFACE_COMMIT: %p\n", proxy);

    if (((struct wl_surface *)proxy)->buffer) {

      EM_ASM({

	  Module['wayland'].requests.push({

	    'type': 'commit',
	    'surface_id': $0,
	    'shm_fd': $1,
	    'width': $2,
	    'height': $3
	    });

	  if (!Module.iframeShown) {

	    Module.iframeShown = true;

	    let m = new Object();
	
	    m.type = 7; // show iframe and hide body
	    m.pid = Module.getpid() & 0x0000ffff;

	    window.parent.postMessage(m);
	  }
	
	}, ((struct wl_surface *)proxy)->id, ((struct wl_surface *)proxy)->buffer->fd, ((struct wl_surface *)proxy)->buffer->width, ((struct wl_surface *)proxy)->buffer->height);
    }
    else {

      for (int i = 0; i < NB_SURFACE_MAX; ++i) {

	if (xdg_surfaces[i].wl_surface == (struct wl_surface *)proxy) {

	  printf("WL_SURFACE_COMMIT: xdg_surface found\n");

	  if (((struct wl_proxy *)(&xdg_surfaces[i]))->listeners) {

	    send_event(&xdg_surfaces[i], "configure", 0);
	  }

	  break;
	}
      }

    }

  }
  else if ( (strcmp(proxy->interface->name, "wl_surface") == 0) &&
       (opcode == WL_SURFACE_ATTACH) ) {

    va_list ap;

    va_start(ap, flags);

    struct wl_buffer * buffer = (struct wl_buffer *)va_arg(ap, struct wl_buffer*);

    int x = va_arg(ap, int);
    int y = va_arg(ap, int);
    
    va_end(ap);

    printf("WL_SURFACE_ATTACH: %p %d %d\n", buffer, x, y);

    ((struct wl_surface *)proxy)->buffer = buffer;
  }
  else if ( (strcmp(proxy->interface->name, "wl_surface") == 0) &&
       (opcode == WL_SURFACE_FRAME) ) {

    printf("WL_SURFACE_FRAME\n");

    for (int i = 0; i < NB_CALLBACK_MAX; ++i) {

      if (frame_callbacks[i].wl_surface == NULL) {

	frame_callbacks[i].wl_surface = (struct wl_surface *)proxy;
	frame_callbacks[i].proxy.version = 0;
	frame_callbacks[i].proxy.wl_display = &display;
	frame_callbacks[i].proxy.interface = &wl_callback_interface;

	return (struct wl_proxy *)&frame_callbacks[i];
      }
    }
  }
  else if ( (strcmp(proxy->interface->name, "wl_surface") == 0) &&
       (opcode == WL_SURFACE_DAMAGE_BUFFER) ) {

    va_list ap;

    va_start(ap, flags);

    int x = va_arg(ap, int);
    int y = va_arg(ap, int);
    int width = va_arg(ap, int);
    int height = va_arg(ap, int);

    va_end(ap);

    printf("WL_SURFACE_DAMAGE_BUFFER: %d %d %d %d\n", x, y, width, height);

    EM_ASM({

	  Module['wayland'].requests.push({

	    'type': 'damage_buffer',
	    'surface_id': $0,
	    'x': $1,
	    'y': $2,
	    'width': $3,
	    'height': $4
	    });
	
      }, ((struct wl_surface *)proxy)->id, x, y, width, height);
  } 
  else if ( (strcmp(proxy->interface->name, "wl_shm") == 0) &&
       (opcode == WL_SHM_CREATE_POOL) ) {

    va_list ap;

    va_start(ap, flags);

    struct wl_shm * wl_shm = va_arg(ap, struct wl_shm*);
    int fd = va_arg(ap, int);
    int size = va_arg(ap, int);
    
    va_end(ap);

    wl_shm_pools[0].fd = fd;
    wl_shm_pools[0].size = size;
    wl_shm_pools[0].proxy.version = 0;
    wl_shm_pools[0].proxy.wl_display = &display;
    wl_shm_pools[0].proxy.interface = &wl_shm_pool_interface;

    printf("WL_SHM_CREATE_POOL: fd=%d size=%d -> %p\n", fd, size, &wl_shm_pools[0]);

    return (struct wl_proxy *)&wl_shm_pools[0];
  }
  else if ( (strcmp(proxy->interface->name, "wl_shm_pool") == 0) &&
       (opcode == WL_SHM_POOL_CREATE_BUFFER) ) {

    printf("WL_SHM_POOL_CREATE_BUFFER: %p\n", proxy);

    va_list ap;

    va_start(ap, flags);

    struct wl_shm_pool* dummy1 = va_arg(ap, struct wl_shm_pool*); // it is NULL !!
    int dummy2 = va_arg(ap, int);
    int width = va_arg(ap, int);
    int height = va_arg(ap, int);
    int stride = va_arg(ap, int);
    int format = va_arg(ap, int);
    
    va_end(ap);

    printf("WL_SHM_POOL_CREATE_BUFFER: %d %d %d %d\n", width, height, stride, format);

    wl_buffers[0].width = width;
    wl_buffers[0].height = height;
    wl_buffers[0].stride = stride;
    wl_buffers[0].format = format;
    wl_buffers[0].fd = ((struct wl_shm_pool *)proxy)->fd;
    wl_buffers[0].proxy.version = 0;
    wl_buffers[0].proxy.wl_display = &display;
    wl_buffers[0].proxy.interface = &wl_buffer_interface;

    return (struct wl_proxy *)&wl_buffers[0];
  }
  else if ( (strcmp(proxy->interface->name, "wl_shm_pool") == 0) &&
       (opcode == WL_SHM_POOL_DESTROY) ) {

    
  }
  else if ( (strcmp(proxy->interface->name, "wl_seat") == 0) &&
       (opcode == WL_SEAT_GET_KEYBOARD) ) {

    return (struct wl_proxy *)&keyboard;
  }
  else if ( (strcmp(proxy->interface->name, "wl_seat") == 0) &&
       (opcode == WL_SEAT_GET_POINTER) ) {

    return (struct wl_proxy *)&pointer;
  }
  else if ( (strcmp(proxy->interface->name, "zxdg_decoration_manager_v1") == 0) &&
       (opcode == ZXDG_DECORATION_MANAGER_V1_GET_TOPLEVEL_DECORATION) ) {

    va_list ap;

    va_start(ap, flags);

    void * dummy = va_arg(ap, void *);
    struct xdg_toplevel * toplevel = (struct xdg_toplevel *)va_arg(ap, struct xdg_toplevel *);

    printf("ZXDG_DECORATION_MANAGER_V1_GET_TOPLEVEL_DECORATION: toplevel=%p decoration=%p\n", toplevel, &(toplevel->decoration));
    
    va_end(ap);
    
    toplevel->decoration.proxy.version = 0;
    toplevel->decoration.proxy.wl_display = &display;
    toplevel->decoration.proxy.interface = &zxdg_toplevel_decoration_v1_interface;

    return (struct wl_proxy *)&(toplevel->decoration);
  }
  else if ( (strcmp(proxy->interface->name, "zxdg_toplevel_decoration_v1") == 0) &&
       (opcode == ZXDG_TOPLEVEL_DECORATION_V1_SET_MODE) ) {

    va_list ap;

    va_start(ap, flags);

    uint32_t mode = va_arg(ap, uint32_t);
    
    va_end(ap);

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {

      char * ptr = ((char *)proxy)-sizeof(struct xdg_surface *)-sizeof(struct wl_proxy);

      struct xdg_toplevel * toplevel = (struct xdg_toplevel *)ptr;
      
      printf("ZXDG_TOPLEVEL_DECORATION_V1_SET_MODE: decoration=%p toplevel=%p xdg_surface=%p wl_surface=%p\n", proxy, toplevel, toplevel->xdg_surface, toplevel->xdg_surface->wl_surface);

      char defaultInnerHTMLDeco[] = "<div id='innerDeco' style='height:25px;background-color:#ddfffb;display:flex;align-items:center'><img id='close' src='/netfs/usr/share/close_icon.png' style='width:15px;height:13px;margin-left:5px;user-select:none'></img><img id='min' src='/netfs/usr/share/min_icon.png' style='width:15px;height:15px;margin-left:5px;user-select:none'></img><span id='title' style='margin:auto; font-family:sans-serif; user-select:none'>[TITLE]</span></div>";

      char * innerHTMLDeco = &defaultInnerHTMLDeco[0];

      FILE * f = fopen("/home/.config/xdg/deco.html", "r");

      if (!f) {

	f = fopen("/etc/xdg/system/deco.html", "r");
      }

      if (f) {

	fseek(f, 0, SEEK_END);

	long size = ftell(f);

	fseek(f, 0, SEEK_SET);

	innerHTMLDeco = (char *)malloc(size);

	if (innerHTMLDeco) {

	  fread(innerHTMLDeco, 1, size, f);
	}
	else {

	  innerHTMLDeco = &defaultInnerHTMLDeco[0];
	}
	
	fclose(f);
      }

      EM_ASM({

	  //console.log($0);
	  //console.log(Module['surfaces'][$0-1]);

	  let w = Module['surfaces'][$0-1].parentElement;

	  let deco = w.firstChild;

	  let decoHTML = UTF8ToString($2);

	  deco.innerHTML += decoHTML.replace("[TITLE]", UTF8ToString($1)).trim();

	  deco.addEventListener("mousedown", (event) => {

	      //console.log("Decoration mouse down: "+event.target.id);

		  if (event.target.id == 'close') {

		    Module['wayland'].events.push({

		      'type': 5, // close button pressed
		      'surface_id': $0
		      });

		    setTimeout(() => {

			if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

			  Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
			}
		    
		      }, 0);

		    event.stopPropagation();
		  }
		  else if (event.target.id == 'min') {

		    let m = new Object();
	
		    m.type = 12; // minimise
		    m.pid = Module.getpid() & 0x0000ffff;
		    
		    window.parent.postMessage(m);
		
		    event.stopPropagation();
		  }
		  else if (event.target.id == 'axel') {

		    let m = new Object();
	
		    m.type = 14; // axel
		    m.pid = Module.getpid() & 0x0000ffff;
		    
		    window.parent.postMessage(m);
		
		    event.stopPropagation();
		  }
		  else {
		  
		    if (!Module.selected_toplevel) {

		      Module.selected_toplevel = deco.parentElement;
		
		      Module.selected_x = event.clientX;
		      Module.selected_y = event.clientY;
		      Module.start_x = parseInt(Module.selected_toplevel.style.left);
		      Module.start_y = parseInt(Module.selected_toplevel.style.top);
		    }
		  }
	      
		}, false);

	  w.addEventListener("mousedown", (event) => {

	      //console.log("Window mouse down");
	      //console.log(event);
		
	      event.stopPropagation();
	      
	    }, false);

	  window.addEventListener('message', (event) => {

	      if (event.data.type == 8) { // mouse down

		for (const div of document.getElementsByTagName("div")) {

		  const rect = div.getBoundingClientRect();

		  //console.log("Bingo ? ");
		  //console.log("x="+event.data.x+", y="+event.data.y);
		  //console.log(rect);
		  
		  if ( (event.data.x >= rect.left) && (event.data.x <= rect.right) && (event.data.y >= rect.top) && (event.data.y <= rect.bottom) ) {

		    let m = new Object();
	
		    m.type = 10; // ask focus
		    m.pid = Module.getpid() & 0x0000ffff;
		    
		    window.parent.postMessage(m);

		    //console.log("Bingo !!");
		    //console.log(rect);

		    return;
		  }
		}

		let m = new Object();
		
		m.type = 9; // continue searching clicked window
		m.pid = Module.getpid() & 0x0000ffff;
		m.x = event.data.x;
		m.y = event.data.y;
		    
		window.parent.postMessage(m);
	      }
	    });

	  document.body.addEventListener("mousemove", (event) => {

	      //console.log("Body mouse move");

	      if (Module.selected_toplevel) {

		Module.selected_toplevel.style.left = (Module.start_x+event.clientX-Module.selected_x)+"px";
		Module.selected_toplevel.style.top = (Module.start_y+event.clientY-Module.selected_y)+"px";
	      }
	      
	    }, false);

	  document.body.addEventListener("mousedown", (event) => {

	      //console.log("Body mouse down: click outside window !");
	      //console.log(event);

	      let m = new Object();
	
	      m.type = 8; // mouse down
	      m.pid = Module.getpid() & 0x0000ffff;
	      m.x = event.clientX;
	      m.y = event.clientY;

	      window.parent.postMessage(m);
	      
	    }, false);

	  document.body.addEventListener("mouseup", (event) => {

	      //console.log("Body mouse up");

	      Module.selected_toplevel = null;
	      
	    }, false);
	
	}, toplevel->xdg_surface->wl_surface->id, toplevel->title, innerHTMLDeco);

      if (innerHTMLDeco != &defaultInnerHTMLDeco[0])
	free(innerHTMLDeco);
    }
  }

  return NULL;
}

int wl_proxy_add_listener(struct wl_proxy * proxy,
		      void (**implementation)(void), void *data) {
  
  if (proxy && proxy->interface && proxy->interface->name) {
    printf("wl_proxy_add_listener: %s\n", proxy->interface->name);
    
    if (strcmp(proxy->interface->name, "wl_shm") == 0) {

      send_event(proxy, "format", WL_SHM_FORMAT_ARGB8888);
    }
    else if (strcmp(proxy->interface->name, "xdg_toplevel") == 0) {

      int width, height;
      
      EM_ASM({

	  const w = window.devicePixelRatio*window.parent.innerWidth; // window.innerWidth return 0
	  const h = window.devicePixelRatio*window.parent.innerHeight;

	  Module.HEAPU8[$0] =  w & 0xff;
	  Module.HEAPU8[$0+1] = (w >> 8) & 0xff;
	  Module.HEAPU8[$0+2] = (w >> 16) & 0xff;
	  Module.HEAPU8[$0+3] = (w >> 24) & 0xff;

	  Module.HEAPU8[$1] =  h & 0xff;
	  Module.HEAPU8[$1+1] = (h >> 8) & 0xff;
	  Module.HEAPU8[$1+2] = (h >> 16) & 0xff;
	  Module.HEAPU8[$1+3] = (h >> 24) & 0xff;

	}, &width, &height);

      width = (4*width/5);
      height = (4*height/5);

      send_event(proxy, "configure", width, height, data);
    }
    else if (strcmp(proxy->interface->name, "wl_seat") == 0) {

      send_event(proxy, "capabilities", WL_SEAT_CAPABILITY_KEYBOARD);
      send_event(proxy, "capabilities", WL_SEAT_CAPABILITY_POINTER);
    }
    else if (strcmp(proxy->interface->name, "wl_keyboard") == 0) {

      EM_ASM({

	  Module.mods = 0;

	  Module.computeMods = (shift, alt, ctrl) => {

	    let mods = 0;

	    if (shift)
	      mods = mods|1;

	    if (alt)
	      mods = mods|2;

	    if (ctrl)
	      mods = mods|4;
	    
	    return mods;
	  };

	  Module.computeKey = (event) => {

	    //( (event.keyCode >= 37) && (event.keyCode <= 40) )?event.keyCode+1000:event.keyCode

	    if (event.key === "Shift")
	      return 0xffe1;
	    else if (event.key === "Control")
	      return 0xffe3;
	    else if (event.key === "Alt")
	      return 0xffe9;
	    else if (event.key === "ArrowUp")
	      return 0xff52;
	    else if (event.key === "ArrowRight")
	      return 0xff53;
	    else if (event.key === "ArrowDown")
	      return 0xff54;
	    else if (event.key === "ArrowLeft")
	      return 0xff51;
	    else if (event.key === "Tab")
	      return 0xff09;
	    else if (event.key === "Backspace")
	      return 0xff08;
	    else if (event.key === "Enter")
	      return 0xff0d;
	    else if (event.key === "Escape")
	      return 0xff1b;
	    else if (event.key === "PageUp")
	      return 0xff55;
	    else if (event.key === "PageDown")
	      return 0xff56;
	    else if (event.key === "Delete")
	      return 0xffff;
	    else if (event.key === "Home")
	      return 0xff50;
	    else if (event.key === "End")
	      return 0xff57;
	    else if ( (event.keyCode >= 112) && (event.keyCode <= 123) ) { // F1-F12
	      return event.keyCode+0xffbe-112;
	    }
	    

	    let utf32 = event.key.codePointAt(0);

	    if ((utf32 >= 0x20) && (utf32 <= 0x7f)) // Latin 1
	      return utf32;

	    return utf32 | 0x80000000; // UTF 32 and last bit set to 1
	  };
	  
	  document.addEventListener("keydown",
				    (event) => {

				      event.preventDefault(); // Cancel the native event
				      event.stopPropagation(); // Don't bubble/capture the event any further

				      //console.log(event);

				      if (event.repeat)
					return;

				      const timestamp = new Date().getTime();

				      let mods = Module.computeMods(event.shiftKey,event.altKey, event.ctrlKey);

				      if (mods != Module.mods) {

					Module.mods = mods;

					Module['wayland'].events.push({
					    
					    'type': 6, // mods
					    'mods': mods
					    });
				      }

				      Module['wayland'].events.push({

					'type': 3, // keydown
					'key': Module.computeKey(event)-8, // !! to simulate Linux evdev scancode
					'location': event.location,
					'timestamp': timestamp
					});

				      setTimeout(() => {

					  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

					    // TODO check rw
		      
					    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
					  }
		    
					}, 0);
				    },
				    true);

	  document.addEventListener("keyup",
				    (event) => {

				      //console.log(event);

				      event.preventDefault(); // Cancel the native event
				      event.stopPropagation(); // Don't bubble/capture the event any further

				      if (event.repeat)
					return;

				      let mods = Module.computeMods(event.shiftKey,event.altKey, event.ctrlKey);

				      if (mods != Module.mods) {

					Module.mods = mods;

					Module['wayland'].events.push({

					    'type': 6, // mods
					    'mods': mods
					    });
				      }

				      const timestamp = new Date().getTime();

				      Module['wayland'].events.push({

					'type': 4, // keyup
					'key': Module.computeKey(event)-8, // !! to simulate Linux evdev scancode
					'location': event.location,
					'timestamp': timestamp
					});

				      setTimeout(() => {

					  if ( (Module['fd_table'][0x7e000000].notif_select) && (Module['wayland'].events.length > 0) ) {

					    // TODO check rw
		      
					    Module['fd_table'][0x7e000000].notif_select(0x7e000000, 0);
					  }
		    
					}, 0);
				    },
				    true);
	
      });

      int keymap_fd = open("/dev/shm/keymap", O_RDWR);

      send_event(proxy, "keymap", WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, keymap_fd, 0);

      send_event(proxy, "repeat_info", KEYBOARD_RATE, KEYBOARD_DELAY); 
    }
    else if (strcmp(proxy->interface->name, "wl_pointer") == 0) {

      EM_ASM({
	  Module.pointerListener = true;
	});
    }
  }
  
  proxy->data = data;
  proxy->listeners = implementation;
  
  return 0;
}

int wl_display_dispatch(struct wl_display * display) {

  while (1) {

    int arg1, arg2, arg3;
    
    int event_type = EM_ASM_INT({

	if ( ('wayland' in Module) && (Module['wayland'].events.length > 0) ) {

	  const event = Module['wayland'].events.shift();

	  //console.log("event received "+event.type);

	  if (event.type == 1) { // buffer released

	    Module.HEAPU8[$0] =  event.surface_id & 0xff;
	    Module.HEAPU8[$0+1] = (event.surface_id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.surface_id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.surface_id >> 24) & 0xff;
	    
	    Module.HEAPU8[$1] = event.shm_fd & 0xff;
	    Module.HEAPU8[$1+1] = (event.shm_fd >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (event.shm_fd >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (event.shm_fd >> 24) & 0xff;
	  }
	  else if (event.type == 2) { // frame done

	    Module.HEAPU8[$0] =  event.surface_id & 0xff;
	    Module.HEAPU8[$0+1] = (event.surface_id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.surface_id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.surface_id >> 24) & 0xff;
	    
	    Module.HEAPU8[$1] = event.timestamp & 0xff;
	    Module.HEAPU8[$1+1] = (event.timestamp >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (event.timestamp >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (event.timestamp >> 24) & 0xff;
	  }
	  else if ( (event.type == 3) || (event.type == 4) ) { // key down or up
	    
	    Module.HEAPU8[$0] =  event.key & 0xff;
	    Module.HEAPU8[$0+1] = (event.key >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.key >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.key >> 24) & 0xff;
	    
	    Module.HEAPU8[$1] = event.timestamp & 0xff;
	    Module.HEAPU8[$1+1] = (event.timestamp >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (event.timestamp >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (event.timestamp >> 24) & 0xff;
	  }
	  else if (event.type == 5) { // close button pressed

	    //console.log("Close button pressed: "+event.surface_id);

	    Module.HEAPU8[$0] =  event.surface_id & 0xff;
	    Module.HEAPU8[$0+1] = (event.surface_id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.surface_id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.surface_id >> 24) & 0xff;
	  }
	  else if (event.type == 6) { // mods updated

	    Module.HEAPU8[$0] =  event.mods & 0xff;
	    Module.HEAPU8[$0+1] = (event.mods >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.mods >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.mods >> 24) & 0xff;
	  }
	  else if (event.type == 7) { // wheel

	    Module.HEAPU8[$0] =  event.id & 0xff;
	    Module.HEAPU8[$0+1] = (event.id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.id >> 24) & 0xff;

	    let dx = event.deltaX * 256; // convert to fixed
	    
	    Module.HEAPU8[$1] =  dx & 0xff;
	    Module.HEAPU8[$1+1] = (dx >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (dx >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (dx >> 24) & 0xff;

	    let dy = event.deltaY * 256; // convert to fixed
	    
	    Module.HEAPU8[$2] =  dy & 0xff;
	    Module.HEAPU8[$2+1] = (dy >> 8) & 0xff;
	    Module.HEAPU8[$2+2] = (dy >> 16) & 0xff;
	    Module.HEAPU8[$2+3] = (dy >> 24) & 0xff;
	  }
	  else if (event.type == 8) { // button

	    Module.HEAPU8[$0] =  event.id & 0xff;
	    Module.HEAPU8[$0+1] = (event.id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.id >> 24) & 0xff;

	    Module.HEAPU8[$1] =  event.state & 0xff;
	    Module.HEAPU8[$1+1] = (event.state >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (event.state >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (event.state >> 24) & 0xff;

	    const button = event.button + 0x110;

	    Module.HEAPU8[$2] =  button & 0xff;
	    Module.HEAPU8[$2+1] = (button >> 8) & 0xff;
	    Module.HEAPU8[$2+2] = (button >> 16) & 0xff;
	    Module.HEAPU8[$2+3] = (button >> 24) & 0xff;
	  }
	  else if (event.type == 9) { // move

	    Module.HEAPU8[$0] =  event.id & 0xff;
	    Module.HEAPU8[$0+1] = (event.id >> 8) & 0xff;
	    Module.HEAPU8[$0+2] = (event.id >> 16) & 0xff;
	    Module.HEAPU8[$0+3] = (event.id >> 24) & 0xff;

	    let x = event.x * 256; // convert to fixed
	    
	    Module.HEAPU8[$1] =  x & 0xff;
	    Module.HEAPU8[$1+1] = (x >> 8) & 0xff;
	    Module.HEAPU8[$1+2] = (x >> 16) & 0xff;
	    Module.HEAPU8[$1+3] = (x >> 24) & 0xff;

	    let y = event.y * 256; // convert to fixed
	    
	    Module.HEAPU8[$2] =  y & 0xff;
	    Module.HEAPU8[$2+1] = (y >> 8) & 0xff;
	    Module.HEAPU8[$2+2] = (y >> 16) & 0xff;
	    Module.HEAPU8[$2+3] = (y >> 24) & 0xff;
	  }
	  
	  return event.type;
	}
	
	return 0;
	
      }, &arg1, &arg2, &arg3);

    if (event_type == 1) { // buffer released

      for (int i = 0; i < 16; ++i) {

	if (wl_buffers[i].fd == arg2) {

	  send_event(&wl_buffers[i], "release");
	  break;
	}
      }
    }
    else if (event_type == 2) { // frame done

      for (int i = 0; i < NB_SURFACE_MAX; ++i) {

	if (surfaces[i].id == arg1) {

	  for (int j = 0; j < NB_CALLBACK_MAX; ++j) {

	    if (frame_callbacks[j].wl_surface == &surfaces[i]) {

	      send_event(&frame_callbacks[j], "done", arg2);

	      frame_callbacks[j].wl_surface = NULL;
	  
	      break;
	    }
	  }

	  break;
	}
      }
    }
    else if (event_type == 3) { // key down

      ++keyboard.serial;

      send_event(&keyboard, "key", keyboard.serial, arg2, arg1, WL_KEYBOARD_KEY_STATE_PRESSED);
    }
    else if (event_type == 4) { // key up

      ++keyboard.serial;

      send_event(&keyboard, "key", keyboard.serial, arg2, arg1, WL_KEYBOARD_KEY_STATE_RELEASED);
    }
    else if (event_type == 5) { // close button pressed

      for (int i = 0; i < NB_SURFACE_MAX; ++i) {

	if (surfaces[i].id == arg1) {

	  for (int j = 0; j < NB_SURFACE_MAX; ++j) {

	    if (xdg_surfaces[j].wl_surface == &surfaces[i]) {
	      
	      for (int k = 0; k < NB_SURFACE_MAX; ++k) {

		if (xdg_toplevels[k].xdg_surface == &xdg_surfaces[j]) {

		  send_event(&xdg_toplevels[k], "close");
		  
		  break;
		}
	      }
	      break;
	    }
	  }
	  
	  break;
	}
      }

      //send_event(&keyboard, "key", keyboard.serial, arg2, arg1, WL_KEYBOARD_KEY_STATE_RELEASED);
    }
    else if (event_type == 6) { // mods

      ++keyboard.serial;

      send_event(&keyboard, "modifiers", keyboard.serial, arg1, 0, 0, 0);
    }
    else if (event_type == 7) { // wheel

      if (arg2)
	send_event(&pointer, "axis", 0, WL_POINTER_AXIS_HORIZONTAL_SCROLL, arg2);

      if (arg3)
	send_event(&pointer, "axis", 0, WL_POINTER_AXIS_VERTICAL_SCROLL, arg3);
    }
    else if (event_type == 8) { // button

      ++pointer.serial;

      send_event(&pointer, "button", pointer.serial, 0, arg3, arg2);
    }
    else if (event_type == 9) { // mousemove

      send_event(&pointer, "motion", 0, arg2, arg3);
    }
    else if (event_type == 0) {

      break;
    }
  }

  wl_display_roundtrip(display);

  //usleep(1000);

  return 1;
}

int
wl_display_dispatch_pending(struct wl_display *display) {

  return wl_display_dispatch(display);
}

int
wl_cursor_frame_and_duration(struct wl_cursor *cursor, uint32_t time,
			     uint32_t *duration)
{

  //TODO
  return 0;
}

int
wl_cursor_frame(struct wl_cursor *cursor, uint32_t time)
{
	return wl_cursor_frame_and_duration(cursor, time, NULL);
}

struct wl_buffer *
wl_cursor_image_get_buffer(struct wl_cursor_image *image)
{
  return NULL;
}

static void
wl_cursor_image_destroy(struct wl_cursor_image *image)
{

}

static void
wl_cursor_destroy(struct wl_cursor *cursor)
{

}

static struct wl_cursor *
wl_cursor_create_from_data(struct cursor_metadata *metadata,
			   struct wl_cursor_theme *theme)
{
  return NULL;
}

static void
load_fallback_theme(struct wl_cursor_theme *theme)
{

}

static struct wl_cursor *
wl_cursor_create_from_xcursor_images(struct xcursor_images *images,
				     struct wl_cursor_theme *theme)
{
  return NULL;
}

struct wl_cursor_theme *
wl_cursor_theme_load(const char *name, int size, struct wl_shm *shm)
{
  return NULL;
}

void
wl_cursor_theme_destroy(struct wl_cursor_theme *theme)
{

}

struct wl_cursor *
wl_cursor_theme_get_cursor(struct wl_cursor_theme *theme,
			   const char *name)
{
  return NULL;
}

int
wl_display_flush(struct wl_display *display)
{

  printf("wl_display_flush: head=%d tail=%d\n", display->head, display->tail);

  return 0;
}

int
wl_display_get_fd(struct wl_display *display)
{
	return 0x7e000000;
}

enum xkb_compose_feed_result
xkb_compose_state_feed(struct xkb_compose_state *state,
                       xkb_keysym_t keysym) {

  return XKB_COMPOSE_FEED_IGNORED;
}

xkb_keysym_t
xkb_compose_state_get_one_sym(struct xkb_compose_state *state) {

  return 0;
}

enum xkb_compose_status
xkb_compose_state_get_status(struct xkb_compose_state *state) {

  return XKB_COMPOSE_NOTHING;
}

struct xkb_compose_state *
xkb_compose_state_new(struct xkb_compose_table *table,
                      enum xkb_compose_state_flags flags) {

  return NULL;
}

void
xkb_compose_state_unref(struct xkb_compose_state *state) {

}

struct xkb_compose_table *
xkb_compose_table_new_from_locale(struct xkb_context *context,
                                  const char *locale,
                                  enum xkb_compose_compile_flags flags) {

  return NULL;
}

void
xkb_compose_table_unref(struct xkb_compose_table *table) {

}

struct xkb_context {

};

struct xkb_context *
xkb_context_new(enum xkb_context_flags flags) {

  return (struct xkb_context *)malloc(sizeof(struct xkb_context));
}

void
xkb_context_unref(struct xkb_context *context) {

}

int
xkb_keymap_key_repeats(struct xkb_keymap *keymap, xkb_keycode_t key) {

  if ( (key == 0xffe1) || (key == 0xffe3) || (key == 0xffe9) )
    return 0;
  
  return 1;
}

xkb_mod_index_t
xkb_keymap_mod_get_index(struct xkb_keymap *keymap, const char *name) {

  if (strcmp(name, XKB_MOD_NAME_SHIFT) == 0)
    return MOD_SHIFT_INDEX;
  else if (strcmp(name, XKB_MOD_NAME_ALT) == 0)
    return MOD_ALT_INDEX;
  else if (strcmp(name, XKB_MOD_NAME_CTRL) == 0)
    return MOD_CTRL_INDEX;
  
  return 0;
}

struct xkb_keymap *
xkb_keymap_new_from_string(struct xkb_context *context, const char *string,
                           enum xkb_keymap_format format,
                           enum xkb_keymap_compile_flags flags) {

  return &keymap;
}

void
xkb_keymap_unref(struct xkb_keymap *keymap) {

}

xkb_keysym_t
xkb_keysym_from_name(const char *name, enum xkb_keysym_flags flags) {

  return 0;
}

uint32_t
xkb_keysym_to_utf32(xkb_keysym_t keysym) {

  if (keysym & 0x80000000)
    return keysym & 0x7fffffff;
  else if ( (keysym == 0xff52) || (keysym == 0xff53) || (keysym == 0xff54) || (keysym == 0xff51))
    return 0;
  else if (keysym == 0xff09)
    return 0x09;
  else if (keysym == 0xff08)
    return 0x08;
  else if (keysym == 0xff0d)
    return 0xff0d;
  else if (keysym == 0xff1b)
    return 0xff1b;
  else if (keysym == 0xffff)
    return 0x7f;
  else if ( (keysym >= 0x20) && (keysym <= 0x7f) )
    return keysym;

  return 0;
}

xkb_keysym_t
xkb_utf32_to_keysym(uint32_t ucs) {

  return 0;
}

xkb_keysym_t
xkb_keysym_to_upper(xkb_keysym_t ks) {

  if ( (ks >= 'a') && (ks <= 'z') )
    return ks+'A'-'a';
    
  return ks;
}

xkb_keysym_t
xkb_keysym_to_lower(xkb_keysym_t ks) {

  if ( (ks >= 'A') && (ks <= 'Z') )
    return ks+'a'-'A';
    
  return ks;
}

xkb_keysym_t
xkb_state_key_get_one_sym(struct xkb_state *state, xkb_keycode_t key) {

  return key;
}

int
xkb_state_mod_index_is_active(struct xkb_state *state, xkb_mod_index_t idx,
                              enum xkb_state_component type) {

  if (type == XKB_STATE_MODS_EFFECTIVE) {

    //emscripten_log(EM_LOG_CONSOLE, "--> xkb_state_mod_index_is_active: state->depressed_mods: %d idx=%d", state->depressed_mods, idx);

    return !!(state->depressed_mods & (1<<(idx-1)));
  }
  
  return 0;
}

struct xkb_state *
xkb_state_new(struct xkb_keymap *keymap) {

  kbd_state.depressed_mods = 0;
  kbd_state.latched_mods = 0;
  kbd_state.locked_mods = 0;
  
  return &kbd_state;
}

struct xkb_state *
xkb_state_ref(struct xkb_state *state) {

  return NULL;
}

void
xkb_state_unref(struct xkb_state *state) {

}

enum xkb_state_component
xkb_state_update_key(struct xkb_state *state, xkb_keycode_t key,
                     enum xkb_key_direction direction) {

  return XKB_STATE_MODS_DEPRESSED;
}

enum xkb_state_component
xkb_state_update_mask(struct xkb_state *state,
                      xkb_mod_mask_t depressed_mods,
                      xkb_mod_mask_t latched_mods,
                      xkb_mod_mask_t locked_mods,
                      xkb_layout_index_t depressed_layout,
                      xkb_layout_index_t latched_layout,
                      xkb_layout_index_t locked_layout) {

  state->depressed_mods = depressed_mods;
  state->latched_mods = latched_mods;
  state->locked_mods = locked_mods;
  
  return 0;
}

struct wl_egl_window {

  struct wl_proxy proxy;
  struct wl_surface * surface;
  int width;
  int height;
  
};

struct wl_egl_window wl_egl_window;

struct wl_egl_window *
wl_egl_window_create(struct wl_surface *surface,
		     int width, int height) {

  wl_egl_window.surface = surface;
  wl_egl_window.width = width;
  wl_egl_window.height = height;

  EM_ASM({

      let canvas = Module['surfaces'][$0-1];
      
      canvas.width = $1;
      canvas.height = $2;

      canvas.style.width = $1 / window.devicePixelRatio + "px";
      canvas.style.height = $2 / window.devicePixelRatio + "px";

      canvas.parentElement.style.width = $1 / window.devicePixelRatio + "px";

    }, surface->id, width, height);
    
  return &wl_egl_window;
}

void
wl_egl_window_destroy(struct wl_egl_window *egl_window) {

}

void
wl_egl_window_resize(struct wl_egl_window *egl_window,
		     int width, int height,
		     int dx, int dy) {

}
