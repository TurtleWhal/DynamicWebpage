const char *javascript = R"(
    ws = new WebSocket('ws://' + location.hostname + '/ws');
    ws.onopen = function() {
        console.log('WebSocket connection established');
        ws.send(JSON.stringify({ event: 'load' }));
    };
    ws.onmessage = function(event) {
        console.log('Message from server: ', event.data);
        var data = JSON.parse(event.data);
        var element = document.getElementById(data.id);
        
        if (element) {
            switch(data.type) {
                case 'setText':
                        element.innerText = data.text;
                    break;
                case 'setHtml':
                        element.innerHTML = data.html;
                    break;
                case 'setValue':
                        element.value = data.value;
                    break;
                default:
                    console.warn('Unknown message type:', data.type);
                    break;
            }
        }
    };
    ws.onclose = function() {
        console.log('WebSocket connection closed');
        setTimeout(function() {
            ws = new WebSocket('ws://' + location.hostname + '/ws');
        }, 1000); // Reconnect after 1 second
    };
    ws.onerror = function(error) {
        console.error('WebSocket error: ', error);
    };

    console.log('Hello from DynamicWebServer!');
)";