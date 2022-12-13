const mqtt = require("mqtt");

const host = "broker.emqx.io";
const mqttPort = "1883"; 
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`;

const connectUrl = `mqtt://${host}:${mqttPort}`;

const client =  mqtt.connect(connectUrl, {
  clientId, 
  clean: true, 
  connectTimeout: 4000, 
  username: "emqx", 
  password: "public", 
  reconnectPeriod: 1000, 
  onSuccess: onConnect
});

function onConnect() {
  console.log("connection successful");
}

const topic1 = "lucas/temperatura";
const topic2 = "lucas/umidade";

let temperatura;
let umidade;

client.on("connect", () => {
  console.log("Connected");
  client.subscribe([topic1, topic2], () => {
    console.log(`Subscribed to topics '${topic1}' and '${topic2}'`);
  });
});

client.on("message", (topic, payload) => {
  if (topic === topic1) {
    temperatura = payload.toString();
  } else if (topic === topic2) {
    umidade = payload.toString();
  }
  console.log(`Received Message: topic = ${topic}, payload = ${payload.toString()}`);
});

const http = require('http');

const hostname = 'localhost';
const port = 3000;

const server = http.createServer((req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/html');
  res.end(`
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
    
        <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    
        <title>Estufa</title>
    
        <style>
            html {
                font-family: Arial;
                display: inline-block;
                margin: 0px auto;
                text-align: center;
            }
    
            h2 { 
                font-size: 3.0rem; 
            }
    
            p { 
                font-size: 3.0rem; 
            }
    
            .units { 
                font-size: 1.2rem; 
            }
    
            .dht-labels {
                font-size: 1.5rem;
                vertical-align:middle;
                padding-bottom: 15px;
            }
        </style>
    
    </head>
    <body>
        <h2>Simulador de Estufa</h2>
    
        <p>
            <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
            <span class="dht-labels">Temperatura: </span> 
            <span id="temperatura">${temperatura}</span>
            <sup class="units">°C</sup>
        </p>
    
        <p>
            <i class="fas fa-tint" style="color:#00add6;"></i> 
            <span class="dht-labels">Umidade: </span>
            <span id="umidade">${umidade}</span>
            <sup class="units">%</sup>
        </p>
    
        <a href="/">Atualizar</a>
    
    </body>
    </html>
  `);
});

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});