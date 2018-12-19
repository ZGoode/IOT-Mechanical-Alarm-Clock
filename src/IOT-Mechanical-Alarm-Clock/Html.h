String parseHomePage();
String parseConfigurePage();
String getAlarmTemplate();
String parseAlarmPage(String alarm);

String footer = "<!-- Footer -->"
                "<footer class='w3-bottom w3-center w3-black w3-padding-small w3-opacity w3-hover-opacity-off'>"
                "<div class='w3-xlarge'>"
                "<a href='https://github.com/ZGoode/IOT-Mechanical-Alarm-Clock'><i class='fa fa-github w3-hover-opacity'></i></a>"
                "<a href='https://twitter.com/FlamingBandaid'><i class='fa fa-twitter w3-hover-opacity'></i></a>"
                "<a href='http://linkedin.com/in/zachary-goode-724441160'><i class='fa fa-linkedin w3-hover-opacity'></i></a>"
                "</div>"
                "</footer>"
                ""
                "</body>"
                "</html>";

String header = "<!DOCTYPE html>"
                "<html>"
                "<title>IOT Mechanical Alarm Clock</title>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>"
                "<link rel='stylesheet' href='https://fonts.googleapis.com/css?family=Lato'>"
                "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>"
                "<style>"
                "body,h1,h2,h3,h4,h5,h6 {font-family: 'Lato', sans-serif;}"
                "body, html {"
                "height: 100%;"
                "color: #777;"
                "line-height: 1.8;"
                "}"
                "/* Create a Parallax Effect */"
                ".bgimg{"
                "background-attachment: fixed;"
                "background-position: center;"
                "background-repeat: no-repeat;"
                "background-size: cover;"
                "}"
                "/* Background Picture */"
                ".bgimg{"
                "background-image: url('%BACKGROUND_IMAGE%');"
                "min-height: 100%;"
                "}"
                ".w3-wide {letter-spacing: 10px;}"
                ".w3-hover-opacity {cursor: pointer;}"
                "</style>"
                "<body>"
                "<!-- Navbar (sit on top) -->"
                "<div class='w3-top'>"
                "<div class='w3-bar' id='myNavbar'>"
                "<a href='Home' class='w3-bar-item w3-button'>HOME</a>"
                "<a href='Configure' class='w3-bar-item w3-button w3-hide-small'><i class='fa fa-cogs'></i> CONFIGURE</a>"
                "<a href='Alarm' class='w3-bar-item w3-button w3-hide-small'><i class='fa fa-clock-o'></i> ALARM</a>"
                "<a href='https://github.com/ZGoode/IOT-Mechanical-Alarm-Clock' class='w3-bar-item w3-button w3-hide-small'><i class='fa fa-th'></i> ABOUT</a>"
                "<a href='/WifiReset' class='w3-bar-item w3-button w3-hide-small w3-right w3-hover-red'>WIFI RESET</a>"
                "<a href='/FactoryReset' class='w3-bar-item w3-button w3-hide-small w3-right w3-hover-red'>FACTORY RESET</a>"
                "</div>"
                "</div>";

String homePage = "<!-- First Parallax Image with Logo Text -->"
                  "<div class='bgimg w3-display-container w3-opacity-min' id='home'>"
                  "<div class='w3-display-middle' style='white-space:nowrap;'>"
                  "<p><span class='w3-center w3-padding-large w3-black w3-xlarge w3-wide w3-animate-opacity'>IOT<span class='w3-hide-small'> MECHANICAL </span> ALARM</span> CLOCK</span></p>"
                  "</div>"
                  "</div>";

String configurePage = "<div class='bgimg w3-display-container w3-opacity-min' id='home'>"
                       "<div class='w3-display-middle' style='white-space:nowrap;'>"
                       "<form class='w3-container' action='/updateConfig' method='get'><h2>Clock Config:</h2>"
                       "<p><label>Time Offset</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='timezone' value='%TIMEZONE%' onkeypress='return isNumberKey(event)'></p>"
                       "<p><input name='twelvehour' class='w3-check w3-margin-top' type='checkbox' %TWELVEHOUR%> Use 12 Hour Time</p>"
                       "<p><input name='isalarmenabled' class='w3-check w3-margin-top' type='checkbox' %ISALARMENABLED%> Enable Alarm Functionality</p>"
                       "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='submit'>Save</button>"
                       "</form>"
                       "</div>"
                       "</div>";

String alarmPage = "<div class='bgimg w3-display-container w3-opacity-min' id='home'>"
                   "<div class='w3-display-middle' style='white-space:nowrap;'>"
                   "<form class='w3-container' action='/updateAlarm' method='get'><h2>Alarms: </h2>"
                   "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='button'>Add Alarm</button>"
                   "<p> </p>"
                   "%ALARM%"
                   "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='submit'>Save</button>"
                   "</form>"
                   "</div>"
                   "</div>";

String alarmTemplate = ""; //add the form for entering an alarm in

String getAlarmTemplate() {
  return alarmTemplate;
}

String parseAlarmPage(String alarm) {
  String form = header + alarmPage + footer;
  form.replace("%ALARM&", alarm);
  return form;
}

String parseConfigurePage() {
  return header + configurePage + footer;
}

String parseHomePage() {
  return header + homePage + footer;
}
