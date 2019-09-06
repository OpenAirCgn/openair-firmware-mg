let L = console.log;
let S = JSON.stringify;


/************************************************************************
 * Interface : the following code dynamically creates the ui.
 ***********************************************************************/

function createNodeFromHTML(html) {
  let template = document.createElement("template");
  template.innerHTML = html.trim();
  return template.content.firstChild;
}
function createStringEntry(name, cfg) {
  return createNodeFromHTML(
    `<div class="cfg_string" id="${name}"><label>${name}</label><input type="text" value="${
      cfg[name]
    }"/></div>`
  );
}
function createBooleanEntry(name, cfg) {
  return createNodeFromHTML(
    `<div class="cfg_boolean" id="${name}"><label>${name}</label><input type="checkbox" value="${name}" ${
      cfg[name] ? "checked" : ""
    }/></div>`
  );
}
function createNumberEntry(name, cfg) {
  return createNodeFromHTML(
    `<div class="cfg_number" id="${name}"><label>${name}</label><input type="number" value="${
      cfg[name]
    }"/></div>`
  );
}

function createObjectEntry(name, cfg, level = 1) {
  let div = createNodeFromHTML(`<div class="cfg_object" id="${name}">`);
  let obj = cfg[name];
  let child_names = Object.keys(obj);
  let bools = [];
  let strings = [];
  let nums = [];
  let objs = [];
  for (let n of child_names) {
    let child = obj[n];
    let childNode;
    // child can be object, string, boolean, number
    switch (typeof child) {
      case "object":
        objs.push(createObjectEntry(n, obj, level + 1));
        // do objects last
        break;
      case "string":
        strings.push(createStringEntry(n, obj));
        break;
      case "boolean":
        bools.push(createBooleanEntry(n, obj));
        break;
      case "number":
        nums.push(createNumberEntry(n, obj));
        break;
      default:
        throw new Error(`unknown type: ${typeof child}`);
    }
  }

  bools.forEach(b => div.appendChild(b));
  nums.forEach(b => div.appendChild(b));
  strings.forEach(b => div.appendChild(b));
  objs.forEach(b => div.appendChild(b));

  let fieldset = createNodeFromHTML(
    `<fieldset><legend>${name}</legend></fieldset>`
  );
  fieldset.appendChild(div);
  return fieldset;
}

// pretty print the configuration to the console
function configToString() {
    for (let section of Object.keys(cfg)) {
      L(section);
      for (let entry of Object.keys(cfg[section])) {
        L(`\t${entry} : ${cfg[section][entry]}`);
      }
    }
}

// this stores the retrieved configuration
// `save` see below only sends diffs back to the
// device
let config = {} 
let filter = null
let defaultFilter = ["openair", "quadsense", "wifi"]


// used mainly for errors, clears the interface
// and replaces it with the passed message.
function clearInterface(msg=null) {
  
  let div = document.querySelector("#main");
  while (div.firstChild) {
    // clear the interface
    div.removeChild(div.firstChild)
  } 

  if (msg !== null) {
    let msgNode = createNodeFromHTML(`<h1>${msg}</h1>`)
    div.appendChild(msgNode)
  }
}

// accessible so it can be called from test, below.
function _populateInterface(cfg) {
  config = cfg
  let div = document.querySelector("#main");
  clearInterface()
  for (let childname of Object.keys(cfg)) {
    if (filter === null || filter.includes(childname)) {
      let child = cfg[childname];
      let ui = createObjectEntry(childname, cfg);
      div.appendChild(ui);
    }
  }
}

// compares the ui to the config values retrieved from the ui
function diff(obj=config, key="") {
  function _diffObject(obj, key) {
    let keys = Object.keys(obj)
    let diffs = {}
    let different = false
    for (let k of keys) {
      let obj2 = obj[k]
      let key2 = `${key}#${k} ` // space is significant
      let d = diff(obj2, key2)
      if (d !== null) {
        different = true
        diffs[k] = d
      }
    }
    return different ? diffs : null
  }

  let differences = {}
  let el = document.querySelector(`${key}input`)
  if (el) {
    let value = el.value
    switch (typeof obj) {
      case "object":
        return _diffObject(obj, key)
        break;
      case "string":
        return value !== obj ? value : null
        break;
      case "number":
        let numValue = parseInt(value)
        if (!isNaN(numValue)) {
          return numValue !== obj ? numValue : null
        } else {
          return null; // throw?
        }
        break;
      case "boolean":
        return el.checked !== obj ? el.checked : null
        break;
      default:
        // err
        return null; // throw ?
    }
  }
  return null // does not exist as a ui element.
}

function save () {
  let diffs = diff(config)
  if (diffs !== null) {
    let reboot = document.querySelector("#reboot") 
    reboot = reboot ? reboot.checked : false
    send("Config.Set", {config: diffs, save:true, reboot: reboot}, result => {
      if (result.saved) {
        populateInterface()
      } else {
        alert(`Error saving. ${JSON.stringify(result)}`)
      }
    });
    clearInterface("Saving. Please Wait");
  } else {
    alert("Nothing to save!")
  }
}

// resets the interface to the last retrieved values.
function cancel () {
  _populateInterface(config)
}


function populateInterface() {
  L("populate");
  send("Config.Get", null, result => {
    _populateInterface(result);
  });
}


async function _main(cfg_to_show) {
  L("Main");
  filter = cfg_to_show 
  await startCommunication();
  populateInterface();
}

async function main() {
  let filter = window.location.search === "?advanced" ? null : defaultFilter
  _main(filter)
}

async function test() {
  L("Test");
  let response_data =
    '{"id":1556265,"src":"esp32_0AAEAC","result":{"debug":{"udp_log_addr":"","level":2,"file_level":"","stdout_uart":0,"stderr_uart":0,"factory_reset_gpio":-1,"mg_mgr_hexdump_file":"","mbedtls_level":1},"device":{"id":"esp32_0AAEAC","license":""},"sys":{"mount":{"path":"","dev_type":"","dev_opts":"","fs_type":"","fs_opts":""},"tz_spec":"","wdt_timeout":30,"pref_ota_lib":"","atca":{"enable":false,"i2c_bus":0,"i2c_addr":96,"ecdh_slots_mask":12}},"conf_acl":"*","i2c":{"unit_no":0,"enable":true,"freq":100000,"debug":false,"sda_gpio":18,"scl_gpio":19},"i2c1":{"unit_no":1,"enable":false,"freq":100000,"debug":false,"sda_gpio":22,"scl_gpio":23},"http":{"enable":true,"listen_addr":"80","document_root":"/","ssl_cert":"","ssl_key":"","ssl_ca_cert":"","upload_acl":"*","hidden_files":"","auth_domain":"","auth_file":""},"rpc":{"enable":true,"max_frame_size":4096,"max_queue_length":25,"default_out_channel_idle_close_timeout":10,"acl_file":"","auth_domain":"","auth_file":"","uart":{"uart_no":0,"baud_rate":115200,"fc_type":2,"dst":""},"ws":{"enable":true,"server_address":"","reconnect_interval_min":1,"reconnect_interval_max":60,"ssl_server_name":"","ssl_ca_file":"","ssl_client_cert_file":""}},"wifi":{"ap":{"enable":false,"ssid":"Mongoose_??????","pass":"Mongoose","hidden":false,"channel":6,"max_connections":10,"ip":"192.168.4.1","netmask":"255.255.255.0","gw":"192.168.4.1","dhcp_start":"192.168.4.2","dhcp_end":"192.168.4.100","trigger_on_gpio":-1,"disable_after":0,"hostname":"","keep_enabled":true},"sta":{"enable":true,"ssid":"marienUniversum","pass":"asdfghjkllkjhgfdsa","user":"","anon_identity":"","cert":"","key":"","ca_cert":"","ip":"","netmask":"","gw":"","nameserver":"","dhcp_hostname":""},"sta1":{"enable":false,"ssid":"","pass":"","user":"","anon_identity":"","cert":"","key":"","ca_cert":"","ip":"","netmask":"","gw":"","nameserver":"","dhcp_hostname":""},"sta2":{"enable":false,"ssid":"","pass":"","user":"","anon_identity":"","cert":"","key":"","ca_cert":"","ip":"","netmask":"","gw":"","nameserver":"","dhcp_hostname":""},"sta_cfg_idx":0,"sta_connect_timeout":30},"board":{"led1":{"pin":13,"active_high":true},"led2":{"pin":-1,"active_high":true},"led3":{"pin":-1,"active_high":true},"btn1":{"pin":0,"pull_up":true},"btn2":{"pin":-1,"pull_up":false},"btn3":{"pin":-1,"pull_up":false}},"openair":{}}}';
  let result = JSON.parse(response_data).result;
  _populateInterface(result);
}

// vim: et:sw=2:ts=2
