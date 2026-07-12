#include "control_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include "nba_teams.h"

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
<select id="sportBox">
  <option value="NBA">NBA</option>
</select>
<select id="teamBox">
  <option value="ATL">Atlanta Hawks</option>
  <option value="BOS">Boston Celtics</option>
  <option value="BKN">Brooklyn Nets</option>
  <option value="CHA">Charlotte Hornets</option>
  <option value="CHI">Chicago Bulls</option>
  <option value="CLE">Cleveland Cavaliers</option>
  <option value="DAL">Dallas Mavericks</option>
  <option value="DEN">Denver Nuggets</option>
  <option value="DET">Detroit Pistons</option>
  <option value="GSW">Golden State Warriors</option>
  <option value="HOU">Houston Rockets</option>
  <option value="IND">Indiana Pacers</option>
  <option value="LAC">LA Clippers</option>
  <option value="LAL">Los Angeles Lakers</option>
  <option value="MEM">Memphis Grizzlies</option>
  <option value="MIA">Miami Heat</option>
  <option value="MIL">Milwaukee Bucks</option>
  <option value="MIN">Minnesota Timberwolves</option>
  <option value="NOP">New Orleans Pelicans</option>
  <option value="NYK">New York Knicks</option>
  <option value="OKC">Oklahoma City Thunder</option>
  <option value="ORL">Orlando Magic</option>
  <option value="PHI">Philadelphia 76ers</option>
  <option value="PHX">Phoenix Suns</option>
  <option value="POR">Portland Trail Blazers</option>
  <option value="SAC">Sacramento Kings</option>
  <option value="SAS">San Antonio Spurs</option>
  <option value="TOR">Toronto Raptors</option>
  <option value="UTA">Utah Jazz</option>
  <option value="WAS">Washington Wizards</option>
</select>
<button onclick="setTeam()">Set Team</button>
<script>
function setTeam(){
    let t = document.getElementById("teamBox").value;
    fetch("/team?name=" + t)
        .then(r => r.text())
        .then(msg => alert(msg));
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

        char buf[4];
        strncpy(buf, name.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        if (!teamFromAbbr(buf)) {
            // Shouldn't happen through the dropdown UI, but the server has no
            // auth, so guard against a bad value hitting this endpoint directly.
            server.send(400, "text/plain", "Invalid selection - please try again.");
            return;
        }

        strncpy(appInput.team, buf, sizeof(appInput.team) - 1);
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