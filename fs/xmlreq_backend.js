

//let L = console.log
const BASE_URL = `http://${window.location.hostname}/rpc`

function send(mg_endpoint, params, cb) {

  let url = `${BASE_URL}/${mg_endpoint}`
  let xhr = new XMLHttpRequest();
  let method = params === null ? 'GET' : 'POST'
  xhr.open(method, url, true)
  xhr.responseType = 'json'

  xhr.onload= () => {
    if (xhr.status === 200) {
      cb(xhr.response) 
    } else {
      clearInterface(`status: ${xhr.status}: ${xhr.statusText}`)
    }
  }
  xhr.onerror = ( err ) => {
      clearInterface(`communication error: ${err}`)
  }

  let payload = params === null ? null : S(params)
  console.log(`sending to ${url} (params: ${payload})`)
  xhr.send(payload)
}

function startCommunication () {
	console.log("Starting (XMLHTTPRequest)")
	return Promise.resolve();

}

function generateVersionInfo() {
  let div = document.querySelector("#version_info")
  if (div) {
    send( "Sys.GetInfo", null, (resp)=> {
      console.log(resp)
      div.textContent = `fw: ${resp.fw_version} (${resp.fw_id})` 
    })
  }
}

// vim: et:sw=2:ts=2
