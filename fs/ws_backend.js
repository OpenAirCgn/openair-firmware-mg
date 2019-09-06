
/************************************************************************
 * Communication
 ***********************************************************************/

let ws; // websocket
let wsHandler = {};

function send(method, params, cb) {
  let id = parseInt(Date.now() / 1000000, 10);
  let msg = {
    id,
    method,
    params
  };
  payload = S(msg);
  L(`sending: ${payload})`);
  if (ws) {
    ws.send(payload);
    wsHandler[id] = cb;
  } else {
    clearInterface("no websocket, sorry")
  }
}

function onmessageCallback(msg) {
  L(msg);
  let data = JSON.parse(msg.data);
  //lookup, retrieve, delete handler
  let handler = wsHandler[data.id];
  if (!handler) {
    L(`Could not retrieve handler for: ${data.id}. Discarding message.`);
    return;
  }
  delete wsHandler[data.id];
  handler(data.result);
  // call handle
}

function startCommunication() {
  L("Start");
  return new Promise(resolve => {
    const url = `ws://${window.location.hostname}/rpc`;
    ws = new WebSocket(url);
    ws.onopen = () => {
      resolve();
    };
    ws.onmessage = onmessageCallback;
    ws.onerror = e => {
      clearInterface(`websocket error: ${S(e)}`);
    };
    ws.onclose = () => {
      clearInterface(`websocket closed`)
      ws = null
    }
  });
}

// vim: et:sw=2:ts=2
