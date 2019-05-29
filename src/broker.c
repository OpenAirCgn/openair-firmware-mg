#include "broker.h"
#include "mgos.h"

static oa_entry broker_values[NUM_VALUES] = {0};

static void current_values_cb ( struct mg_rpc_request_info *ri,
                                void *cb_arg,
                                struct mg_rpc_frame_info *fi,
                                struct mg_str args) {

  struct mbuf mb;
  struct json_out jbuf = JSON_OUT_MBUF(&mb);
  mbuf_init(&mb, 10); // arbitrary ...?

  uint32_t ts = (uint32_t)(mgos_uptime()+0.5);
  json_printf(&jbuf, "{ts:%u,device_id:%Q,entries:[", ts, mgos_sys_config_get_device_id());
  for (int i = 0; i!= NUM_VALUES; ++i) {
    json_printf(&jbuf, "{ts:%u,tag:%u,value:%u}", broker_values[i].ts, i, broker_values[i].value);
    if (i != NUM_VALUES-1){
      json_printf(&jbuf, ",");
    }
  }
  json_printf(&jbuf, "]}");

  
  LOG(LL_DEBUG, ("json (%u): %.*s", mb.len, mb.len, mb.buf));
  bool ok = mg_rpc_send_responsef(ri, "%.*s", mb.len, mb.buf);
  mbuf_free(&mb);

  if (!ok) {
    LOG(LL_INFO, ("mg_rpc_send_responsef failed: %u", ok));
  }
}

bool oa_broker_init() {
  mg_rpc_add_handler(mgos_rpc_get_global(), "OpenAir.Current", "", current_values_cb, NULL);
  return true;
}

void oa_broker_tick() {
  return ;
}


bool establishing_connection = false;
bool connection_established=false;
struct mg_connection *nc = NULL;

static void ev_handler(struct mg_connection *conn, int ev, void *event_data, void *user_data) {
  int status;
  switch (ev) {
    case MG_EV_CONNECT:
      status = *(int *)event_data;
      LOG(LL_INFO, ("connect event: %d : %s", status, strerror(status)));
      if (status == 0) {
        connection_established = true;
      } 
      establishing_connection = false;
      break;
    case MG_EV_CLOSE:
      connection_established = false;
      nc = NULL;
      break;
    default:
      break;
  }
}

int attempt;
static void establish_connection () {
  if (nc != NULL || establishing_connection) {
    return;
  }
  attempt++;
  if ((attempt % 20)!=0) {
    LOG(LL_INFO, ("Skipping attempt %d", attempt));
    return;
  }
    LOG(LL_INFO, ("attempt %d", attempt));

  establishing_connection = true;
  
  struct mg_mgr * mgr = mgos_get_mgr();
  nc = mg_connect(mgr, mgos_sys_config_get_openair_firehose_addr(), ev_handler, NULL);

  if (nc == NULL) {
    LOG(LL_ERROR, ("Failed to connect!"));
    establishing_connection = false;
    return;
  } else {
    LOG(LL_INFO, ("Worked weirdly."));
  }
}

static void tcp_push (uint32_t ts, oa_tag tag, uint32_t value) {
  // using tcp instead of udp because mongoose is buffering data in a single connection and 
  // can't seem to flush buffer (TODO look into this)
  if (!connection_established) {
    establish_connection();
    return;
  }


  
  // assemble JSON
  struct mbuf mb;
  struct json_out jbuf = JSON_OUT_MBUF(&mb);
  mbuf_init(&mb, 10); // arbitrary ...?

  json_printf(&jbuf, "{ts:%u,device_id:%Q,tag:%u, value:%u}", ts, mgos_sys_config_get_device_id(), tag, value);

  LOG(LL_INFO,("Sending '%.*s'", mb.len, mb.buf));
  mg_send(nc, mb.buf, mb.len); 
  mbuf_free(&mb);
}

void oa_broker_push(oa_tag tag, uint32_t value) {
  uint32_t ts = (uint32_t)(mgos_uptime()+0.5);
  broker_values[tag].ts = ts;
  broker_values[tag].value = value;
  
  // TODO create a dynamic system to register handlers that can aggregate, buffer, whatever.
  // for now it's hard coded.
  if (mgos_sys_config_get_openair_firehose_en()) {
    tcp_push(ts, tag, value);
  }
}


// vim: et:sw=2:ts=2
