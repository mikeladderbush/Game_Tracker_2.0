#include "control_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>

AppInput appInput;
static WebServer server(80);

static const char* INDEX_HTML = R"HTML(
<html>
<head><title>Scoreboard Control</title></head>
<body style="font-family:sans-serif;text-align:center;margin-top:40px">
<h2>Scoreboard Control</h2>
<button onclick="fetch('/on')">Power On</button>
<button onclick="fetch('/off')">Power Off</button>
<br><br>
<input id="teamBox" placeholder="Enter Team (ex: BOS)">
<button onclick="setTeam()">Set Team</button>
<script>
function setTeam(){
    let t = document.getElementById("teamBox").value;
    fetch("/team?name=" + t);
}
</script>
</body>
</html>
)HTML";

void beginControlServer() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", INDEX_HTML);
    });

    server.on("/on", HTTP_GET, []() {
        appInput.powerOn = true;
        server.send(200, "text/plain", "OK");
    });

    server.on("/off", HTTP_GET, []() {
        appInput.powerOn = false;
        server.send(200, "text/plain", "OK");
    });

    server.on("/team", HTTP_GET, []() {
        String name = server.arg("name");
        name.toUpperCase();
        strncpy(appInput.team, name.c_str(), sizeof(appInput.team) - 1);
        appInput.team[sizeof(appInput.team) - 1] = '\0';
        appInput.teamPending = true;
        server.send(200, "text/plain", "Team set to " + name);
    });

    server.begin();
    Serial.print("Control server running: ");
    Serial.println(WiFi.localIP());
}

void pollControlServer() {
    server.handleClient();
}