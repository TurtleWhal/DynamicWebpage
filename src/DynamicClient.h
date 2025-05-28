const char *javascript = R"(
    let ws;
let reconnectInterval = 1000; // Initial retry delay
const maxReconnectInterval = 10000; // Max delay between retries

var overlay = document.createElement('div');
var message = document.createElement('div');

function connectWebSocket() {

    message.innerText = 'Connecting to ESP32...';

    ws = new WebSocket('ws://' + location.hostname + '/ws');

    ws.onopen = function () {
        console.log('WebSocket connection established');
        reconnectInterval = 1000; // Reset interval on success

        message.innerText = 'Loading...';

        setTimeout(() => {
            ws.send(JSON.stringify({ event: 'load' }));
        }, 500);
    };

    ws.onmessage = function (event) {
        console.log('Message from server:', event.data);
        let data;
        try {
            data = JSON.parse(event.data);
        } catch (e) {
            console.error('Invalid JSON:', e);
            return;
        }

        const element = document.getElementById(data.id);
        // if (!element) return;

        switch (data.type) {
            case 'loaded':
                console.log('Page loaded, ready to receive events');
                overlay.style.display = 'none';
                break;
            // case 'setText':
            //     element.innerText = data.text;
            //     break;
            case 'setHtml':
                element.innerHTML = data.html;
                break;
            case 'setValue':
                element.value = data.value;
                break;
            case 'getValue':
                ws.send(JSON.stringify({
                    event: 'getValue',
                    id: data.id,
                    value: getValue(element)
                }));
                break;
            case 'addEvent':
                element.addEventListener(data.event, () => {
                    console.log('Event triggered:', data.event);
                    ws.send(JSON.stringify({
                        event: 'event',
                        id: data.id,
                        type: data.event,
                        value: getValue(element)
                    }));
                });
                break;
            default:
                console.warn('Unknown message type:', data.type);
                break;
        }
    };

    ws.onclose = function () {
        console.log('WebSocket connection closed. Attempting to reconnect...');
        retryWebSocketConnection();
    };

    ws.onerror = function (error) {
        console.error('WebSocket error:', error);
        ws.close(); // Trigger reconnect logic
    };
}

function retryWebSocketConnection() {
    setTimeout(() => {
        console.log('Reconnecting WebSocket...');
        connectWebSocket();
        // Increase delay for next retry, up to max
        reconnectInterval = Math.min(reconnectInterval * 2, maxReconnectInterval);
    }, reconnectInterval);
}

function getValue(element) {
    if (element.type === 'checkbox') {
        return element.checked ? 'on' : 'off';
    } else if (element.value !== undefined) {
        return element.value;
    } else {
        return '';
    }
}

overlay.style.position = 'fixed';
overlay.style.top = '0';
overlay.style.left = '0';
overlay.style.bottom = '0';
overlay.style.right = '0';
overlay.style.backgroundColor = 'rgba(0, 0, 0, 0.7)';
overlay.style.zIndex = '9999';
overlay.style.display = 'flex';
overlay.style.justifyContent = 'center';
overlay.style.alignItems = 'center';
overlay.style.color = 'white';

message.style.fontSize = '24px';
message.style.textAlign = 'center';
message.style.fontFamily = "'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif";

overlay.appendChild(message);

document.body.appendChild(overlay);

// Initial connection
console.log('Hello from DynamicWebServer!');
connectWebSocket();

)";