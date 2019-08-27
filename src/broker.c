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
  LOG(LL_INFO, ("adding openair current handler"));
  return true;
}

void oa_broker_tick() {
  return ;
}


bool establishing_connection = false;
bool connection_established = false;
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
    LOG(LL_DEBUG, ("already establishing: %d", establishing_connection));
    return;
  }
  attempt++;
  if ((attempt % 200)!=0) {
    return;
  }
  establishing_connection = true;
  LOG(LL_INFO, ("establishing connection, attempt %d", attempt));

  struct mg_mgr * mgr = mgos_get_mgr();
  nc = mg_connect(mgr, mgos_sys_config_get_openair_firehose_addr(), ev_handler, NULL);

  if (nc == NULL) {
    LOG(LL_ERROR, ("Failed to connect!"));
    establishing_connection = false;
    return;
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

  LOG(LL_DEBUG,("Sending '%.*s'", mb.len, mb.buf));
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

// callbacks


void mics_cb(int vred, int vox) {
  oa_broker_push(oa_mics4514_vred, (uint32_t)vred);
  oa_broker_push(oa_mics4514_vox, (uint32_t)vox);
}

void si7006_cb(float celsius, float rh, int temp, int rh_raw) {
  oa_broker_push(oa_si7006_temp, (uint32_t)celsius*1000);
  oa_broker_push(oa_si7006_rh, (uint32_t)rh*100);
  oa_broker_push(oa_si7006_temp_raw, (uint32_t)temp);
  oa_broker_push(oa_si7006_rh_raw, (uint32_t)rh_raw);
}

void sds_cb(uint32_t pm25, uint32_t pm10) {
  oa_broker_push(oa_sds_pm25, pm25);
  oa_broker_push(oa_sds_pm10, pm10);
}
void alpha_cb(
    int alpha1,
    int alpha2,
    int alpha3,
    int alpha4,
    int alpha5,
    int alpha6,
    int alpha7,
    int alpha8,
    int ppb1,
    int ppb2,
    int ppb3,
    int ppb4

    ) {
  oa_broker_push(oa_alpha_1, alpha1);
  oa_broker_push(oa_alpha_2, alpha2);
  oa_broker_push(oa_alpha_3, alpha3);
  oa_broker_push(oa_alpha_4, alpha4);
  oa_broker_push(oa_alpha_5, alpha5);
  oa_broker_push(oa_alpha_6, alpha6);
  oa_broker_push(oa_alpha_7, alpha7);
  oa_broker_push(oa_alpha_8, alpha8);
  oa_broker_push(oa_alpha_1_ppb, ppb1);
  oa_broker_push(oa_alpha_2_ppb, ppb2);
  oa_broker_push(oa_alpha_3_ppb, ppb3);
  oa_broker_push(oa_alpha_4_ppb, ppb4);
}

void bme_cb(uint8_t idx, 
    uint32_t p_raw, float p, 
    uint32_t t_raw, float t, 
    uint32_t h_raw, float h) {
  switch (idx) {
    case 0:
      oa_broker_push(oa_bme0_pressure_raw, p_raw);
      oa_broker_push(oa_bme0_pressure, (uint32_t)(p * 100));
      oa_broker_push(oa_bme0_temp_raw, t_raw);
      oa_broker_push(oa_bme0_temp, (uint32_t)((t + 273.15) * 1000));
      oa_broker_push(oa_bme0_humidity_raw, h_raw);
      oa_broker_push(oa_bme0_humidity, (uint32_t)(h*100));
      break;
    case 1:
      oa_broker_push(oa_bme1_pressure_raw, p_raw);
      oa_broker_push(oa_bme1_pressure, (uint32_t)(p * 100));
      oa_broker_push(oa_bme1_temp_raw, t_raw);
      oa_broker_push(oa_bme1_temp, (uint32_t)((t + 273.15) * 1000));
      oa_broker_push(oa_bme1_humidity_raw, h_raw);
      oa_broker_push(oa_bme1_humidity, (uint32_t)(h*100));
      break;
    default:
      LOG(LL_ERROR, ("Invalid BME idx (%d)", idx));
  }
}

void noisemeter_cb(float dba, float dbc) {
  if (dba < 0) { dba = 0; }
  if (dbc < 0) { dbc = 0; }
  oa_broker_push(oa_dba, (uint32_t)(dba*100));
  oa_broker_push(oa_dbc, (uint32_t)(dbc*100));
}

// vim: et:sw=2:ts=2
