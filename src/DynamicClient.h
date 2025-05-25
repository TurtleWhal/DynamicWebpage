const char *javascript = R"(
    ws = new WebSocket('ws://' + location.hostname + '/ws');
    ws.onopen = function() {
        console.log('WebSocket connection established');
        setTimeout(function() {
            ws.send(JSON.stringify({ event: 'load' }));
        }, 500); // Delay to ensure the connection is ready
    };
    ws.onmessage = function(event) {
        console.log('Message from server: ', event.data);
        var data = JSON.parse(event.data);
        var element = document.getElementById(data.id);
        
        if (element) {
            switch(data.type) {
                //case 'setText':
                //        element.innerText = data.text;
                //    break;
                case 'setHtml':
                        element.innerHTML = data.html;
                    break;
                case 'setValue':
                        element.value = data.value;
                    break;
                case 'getValue':
                        ws.send(JSON.stringify({ event: 'getValue', id: data.id, value: getValue(element) }));
                    break;
                case 'addEvent':
                        element.addEventListener(data.event, function() {
                            console.log('Event triggered:', data.event);
                            ws.send(JSON.stringify({ event: 'event', id: data.id, type: data.event, value: getValue(element) }));
                        });
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

    function getValue(element) {
        if (element.type === 'checkbox') {
            return element.checked ? 'on' : 'off';
        } else if (element.value !== undefined) {
            return element.value;
        } else {
            return '';
        }
    }

    console.log('Hello from DynamicWebServer!');
)";