// #include <Arduino.h>

// const char index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <title>Zip Line Controls</title>
//   <style>
//     html {font-family: Arial; display: inline-block; text-align: center;}
//     h2 {font-size: 1.4rem;}
//     p {font-size: 1rem;}
//     body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
//     .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #203ac9;
//       outline: none; -webkit-transition: .2s; transition: opacity .2s;}
//     .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #5dfc4e; cursor: pointer;}
//     .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; }
//     .stop_button{background-color: #ff0001; border: none; color: black; padding: 16px 40px;}
//     .direction_button{background-color: #00ff00; border: none; color: black; padding: 16px 40px;}
//     .go_up_button{background-color: #0000ff; border: none; color: yellow; padding: 16px 40px;}
//     .odometer_settings_button{background-color: #c93877; border: none; color: white; padding: 16px 40px;}
//     .log_button{background-color: #c96f10; border: none; color: white; padding: 16px 40px;}
//   </style>
// </head>
// <body>
//   <h2>Zip Line Controls</h2>
//   <p>State: %STATE% Direction: %DIRECTION%</p>
//   <p>Battery Voltage: %VOLTAGE%</p>
//   <p><button id="stop_button" class="button stop_button" onclick="buttonClickSend(this)">Stop Motor</button></p>
//   <p><button id="direction_button" class="button direction_button" onclick="buttonClickSend(this)">Direction</button></p>
//   <p><button id="go_up_button" class="button go_up_button" onclick="buttonClickSend(this)">Go Up</button></p>
//   <p><span>Max Speed: %MAXSPEEDVALUE%</span></p>
//   <p><input type="range" onchange="sliderSend(this)" id="maxSpeedSlider" min="100" max="255" value="%MAXSPEEDVALUE%" step="1" class="slider"></p>
//   <p><span>After Zip Delay (in seconds): %AFTERZIPDELAY%</span></p>
//   <p><input type="range" onchange="sliderSend(this)" id="afterZipDelaySlider" min="0" max="60" value="%AFTERZIPDELAY%" step="1" class="slider"></p>
//   <p><button id="odometer_settings_button" class="button odometer_settings_button" onclick="location.href = '/odometersettingsclick';">Odometer Settings</button></p>
//   <p><button id="log_button" class="button log_button" onclick="location.href = '/logclick';">Log Page</button></p>
// <script>
// function sliderSend(element){
//   var sliderValue = element.value;
//   var xhr = new XMLHttpRequest();
//   if(element.id == "maxSpeedSlider"){xhr.open("GET", "/maxspeed?value="+sliderValue);}
//   else if(element.id == "afterZipDelaySlider"){xhr.open("GET", "/afterzipdelay?value="+sliderValue);}
//   xhr.send();
// }
// function buttonClickSend(element){
//   var xhr = new XMLHttpRequest();
//   if(element.id == "stop_button"){xhr.open("GET", "/stopclick");}
//   else if(element.id == "direction_button"){xhr.open("GET", "/directionclick");}
//   else if(element.id == "go_up_button"){xhr.open("GET", "/goupclick");}
//   xhr.send();
// }
// </script>
// </body>
// </html>
// )rawliteral";


// const char odometer_index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <title>Zip Line Controls</title>
//   <style>
//     html {font-family: Arial; display: inline-block; text-align: center;}
//     h2 {font-size: 1.4rem;}
//     p {font-size: 1rem;}
//     body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
//     .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #203ac9;
//       outline: none; -webkit-transition: .2s; transition: opacity .2s;}
//     .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #5dfc4e; cursor: pointer;}
//     .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer;}
//     .back_button{background-color: #2aa2d1; border: none; color: black; padding: 16px 40px;}
//   </style>
// </head>
// <body>
//   <h2>Odometer Settings</h2>
//   <p><span>Current IR Value: %CURRENTIRVALUE%</span></p>
//   <p><span>Low Threshold: %LOWTHRESHOLD%</span></p>
//   <p><input type="range" onchange="sliderSend(this)" id="lowThresholdSlider" min="1" max="4094" value="%LOWTHRESHOLD%" step="100" class="slider"></p>
//   <p><span>High Threshold: %HIGHTHRESHOLD%</span></p>
//   <p><input type="range" onchange="sliderSend(this)" id="highThresholdSlider" min="1" max="4094" value="%HIGHTHRESHOLD%" step="100" class="slider"></p>
//   <p><button id="back_button" class="button back_button" onclick="location.href = '/back';">Back</button></p>
// <script>
// function sliderSend(element){
//   var sliderValue = element.value;
//   var xhr = new XMLHttpRequest();
//   if(element.id == "lowThresholdSlider"){xhr.open("GET", "/lowthreshold?value="+sliderValue);}
//   else if(element.id == "highThresholdSlider"){xhr.open("GET", "/highthreshold?value="+sliderValue);}
//   xhr.send();
// }
// function buttonClickSend(element){
//   var xhr = new XMLHttpRequest();
//   if(element.id == "back_button"){xhr.open("GET", "/back");}
//   xhr.send();
// }
// </script>
// </body>
// </html>
// )rawliteral";

// const char log_page_index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <title>Zip Line Controls</title>
//   <style>
//     html {font-family: Arial; display: inline-block; text-align: center;}
//     h2 {font-size: 1.4rem;}
//     p {font-size: 1rem;}
//     body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
//     .clear_button{background-color: #c76f12; border: none; color: black; padding: 16px 40px;}
//     .back_button{background-color: #2aa2d1; border: none; color: black; padding: 16px 40px;}
//   </style>
// </head>
// <body>
//   <h2>Log</h2>
//   <p><button id="clear_button" class="button clear_button" onclick="buttonClickSend(this);">Clear Log</button></p>
//   <p><button id="back_button" class="button back_button" onclick="location.href = '/back';">Back</button></p>
//   %LOGSTRING%
// <script>
// function buttonClickSend(element){
//   var xhr = new XMLHttpRequest();
//   if(element.id == "back_button"){xhr.open("GET", "/back");}
//   else if(element.id == "clear_button"){xhr.open("GET", "/clearlog")}
//   xhr.send();
// }
// </script>
// </body>
// </html>
// )rawliteral";
