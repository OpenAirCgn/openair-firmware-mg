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


void oa_broker_push(oa_tag tag, uint32_t value) {
  uint32_t ts = (uint32_t)(mgos_uptime()+0.5);
  broker_values[tag].ts = ts;
  broker_values[tag].value = value;
}


// vim: et:sw=2:ts=2
