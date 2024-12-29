#include <WiFiManager.h>

std::string inputSelect(std::string name, std::string *strings, int stringArraySize, int selected)
{
    std::string returnStr = "<!-- INPUT SELECT --><br/><label for='input_select'>";
    returnStr.append(name.c_str());
    returnStr.append("</label><select name=\"input_select\" id=\"input_select\" class=\"button\">");

    for (int index = 0; index < stringArraySize; index++)
    {
        returnStr.append("<option value=\"");
        returnStr.append(std::to_string(index).c_str());
        returnStr.append("\"");
        if (selected == index)
        {
            returnStr.append(" selected");
        }
        returnStr.append(">");
        returnStr.append(strings[index]);
        returnStr.append("</option>");
    }
    returnStr.append("</select>");
    // Serial.println(returnStr.c_str());
    return returnStr;
}

struct SelectParameter
{
    std::string name;
    std::string selectStrings[10];
    int selectAmount;
    int selected;
    std::string customString = inputSelect(this->name, this->selectStrings, this->selectAmount, this->selected);
    SelectParameter(std::string name, std::initializer_list<std::string> selectStrings, int arraySize, int selected)
    {
        this->name = name;
        for (int index = 0; index < selectStrings.size(); index++)
        {

            this->selectStrings[index] = selectStrings.begin()[index];
        }
        this->selectAmount = arraySize;
        this->selected = selected;
        this->customString = inputSelect(name, this->selectStrings, selectAmount, selected);
    }
};

class SetupPage
{

private:
    WiFiManager *wifimanager;
    const char _customHtml_checkbox[16] = "type=\"checkbox\"";
    const static int parameterAmount = 11;
    SelectParameter selectParameters[3] =
        {{std::string("IMU connection method: "), {std::string("BNO08x"), std::string("RVC_UART")}, 2, 0},
         {std::string("IMU connection method: "), {std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3},
         {std::string("IMU connection method: "), {std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3}};

    WiFiManagerParameter parameters[parameterAmount] = {
        {"<p style=\"font-weight:Bold;\">AgOpenGPS Settings</p>"},
        {"<p style=\"font-weight:Bold;\">Send data to:</p>"},
        {"my_serial", "Serial", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER},
        {"my_wifi", "WIFI", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER},
        {"my_ethernet", "Ethernet", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER},
        {"my_can", "CAN", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER},
        {"<p style=\"font-weight:Bold;\">IMU:</p>"},
        {selectParameters[0].customString.c_str()},
        {"<p style=\"font-weight:Bold;\">IMU connection:</p>"},
        {selectParameters[1].customString.c_str()},
        {selectParameters[2].customString.c_str()},
    };

public:
    SetupPage();
    SetupPage(WiFiManager *wifimanager);
    void init();
};

SetupPage::SetupPage(WiFiManager *wifimanager)
{
    this->wifimanager = wifimanager;
}

void SetupPage::init()
{
    for (int index = 0; index < parameterAmount; index++)
    {
        wifimanager->addParameter(&parameters[index]);
    }
}

/*
std::string testStrings[] = {"1", "2"};
  std::string name = "test";
  std::string testString = inputSelect("test", testStrings, 2, 0);
  WiFiManagerParameter custom_html("<p style=\"font-weight:Bold;\">AgOpenGPS Settings</p>"); // only custom html
  WiFiManagerParameter testSelect(testString.c_str());
  // WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
  // WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);
  // WiFiManagerParameter custom_token("api_token", "api token", "", 16);
  // WiFiManagerParameter custom_tokenb("invalid token", "invalid token", "", 0);                                                  // id is invalid, cannot contain spaces
  // WiFiManagerParameter custom_ipaddress("input_ip", "input IP", "", 15, "pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'"); // custom input attrs (ip mask)
  // WiFiManagerParameter custom_input_type("input_pwd", "input pass", "", 15, "type='password'");                                 // custom input attrs (ip mask)
  WiFiManagerParameter custom_send_data("<p style=\"font-weight:Bold;\">Send data to:</p>");
  const char _customHtml_checkbox[] = "type=\"checkbox\"";
  WiFiManagerParameter custom_checkbox_Serial("my_serial", "Serial", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_Wifi("my_wifi", "WIFI", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_Ethernet("my_ethernet", "Ethernet", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  WiFiManagerParameter custom_checkbox_CAN("my_can", "CAN", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);
  Serial.print(testSelect.getCustomHTML());
  setupParameters[0].customString = inputSelect(setupParameters[0].name, setupParameters[0].selectStrings, setupParameters[0].selectAmount, setupParameters[0].selected);
  WiFiManagerParameter test(setupParameters[0].customString.c_str());
  wifiManager.addParameter(&test);
  const char *bufferStr = R"(
  <!-- INPUT CHOICE -->
  <br/>
  <p>Select IMU</p>
  <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  <label for='choice1'>BNO085/BNO055</label><br/>
  <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  <label for='choice2'>???</label><br/>

  <!-- INPUT SELECT -->
  <br/>
  <label for='input_select'>IMU connection</label>
  <select name="input_select" id="input_select" class="button">
  <option value="0">I2C</option>
  <option value="1" selected>I2C RVC</option>
  <option value="2">SPI</option>
  </select>
  )";

  WiFiManagerParameter custom_html_inputs(bufferStr);
  custom_html_inputs.setValue("", 1);
  // add all your parameters here
  wifiManager.addParameter(&custom_html);
  wifiManager.addParameter(&testSelect);
  // wifiManager.addParameter(&custom_mqtt_server);
  // wifiManager.addParameter(&custom_mqtt_port);
  // wifiManager.addParameter(&custom_token);
  // wifiManager.addParameter(&custom_tokenb);
  // wifiManager.addParameter(&custom_ipaddress);
  wifiManager.addParameter(&custom_send_data);
  wifiManager.addParameter(&custom_checkbox_Serial);
  wifiManager.addParameter(&custom_checkbox_Wifi);
  wifiManager.addParameter(&custom_checkbox_Ethernet);
  wifiManager.addParameter(&custom_checkbox_CAN);
  // wifiManager.addParameter(&custom_input_type);

  wifiManager.addParameter(&custom_html_inputs);

  // set values later if you want
  // custom_html.setValue("test", 4);
  // custom_token.setValue("test", 4);

  // const char* icon = "
  // <link rel='icon' type='image/png' sizes='16x16' href='data:image/png;base64,
  // iVBORw0KGgoAAAANSUhEUgAAABAAAAAQBAMAAADt3eJSAAAAMFBMVEU0OkArMjhobHEoPUPFEBIu
  // O0L+AAC2FBZ2JyuNICOfGx7xAwTjCAlCNTvVDA1aLzQ3COjMAAAAVUlEQVQI12NgwAaCDSA0888G
  // CItjn0szWGBJTVoGSCjWs8TleQCQYV95evdxkFT8Kpe0PLDi5WfKd4LUsN5zS1sKFolt8bwAZrCa
  // GqNYJAgFDEpQAAAzmxafI4vZWwAAAABJRU5ErkJggg==' />";

  // set custom html head content , inside <head>
  // examples of favicon, or meta tags etc
  // const char* headhtml = "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAADQElEQVRoQ+2YjW0VQQyE7Q6gAkgFkAogFUAqgFQAVACpAKiAUAFQAaECQgWECggVGH1PPrRvn3dv9/YkFOksoUhhfzwz9ngvKrc89JbnLxuA/63gpsCmwCADWwkNEji8fVNgotDM7osI/x777x5l9F6JyB8R4eeVql4P0y8yNsjM7KGIPBORp558T04A+CwiH1UVUItiUQmZ2XMReSEiAFgjAPBeVS96D+sCYGaUx4cFbLfmhSpnqnrZuqEJgJnd8cQplVLciAgX//Cf0ToIeOB9wpmloLQAwpnVmAXgdf6pwjpJIz+XNoeZQQZlODV9vhc1Tuf6owrAk/8qIhFbJH7eI3eEzsvydQEICqBEkZwiALfF70HyHPpqScPV5HFjeFu476SkRA0AzOfy4hYwstj2ZkDgaphE7m6XqnoS7Q0BOPs/sw0kDROzjdXcCMFCNwzIy0EcRcOvBACfh4k0wgOmBX4xjfmk4DKTS31hgNWIKBCI8gdzogTgjYjQWFMw+o9LzJoZ63GUmjWm2wGDc7EvDDOj/1IVMIyD9SUAL0WEhpriRlXv5je5S+U1i2N88zdPuoVkeB+ls4SyxCoP3kVm9jsjpEsBLoOBNC5U9SwpGdakFkviuFP1keblATkTENTYcxkzgxTKOI3jyDxqLkQT87pMA++H3XvJBYtsNbBN6vuXq5S737WqHkW1VgMQNXJ0RshMqbbT33sJ5kpHWymzcJjNTeJIymJZtSQd9NHQHS1vodoFoTMkfbJzpRnLzB2vi6BZAJxWaCr+62BC+jzAxVJb3dmmiLzLwZhZNPE5e880Suo2AZgB8e8idxherqUPnT3brBDTlPxO3Z66rVwIwySXugdNd+5ejhqp/+NmgIwGX3Py3QBmlEi54KlwmjkOytQ+iJrLJj23S4GkOeecg8G091no737qvRRdzE+HLALQoMTBbJgBsCj5RSWUlUVJiZ4SOljb05eLFWgoJ5oY6yTyJp62D39jDANoKKcSocPJD5dQYzlFAFZJflUArgTPZKZwLXAnHmerfJquUkKZEgyzqOb5TuDt1P3nwxobqwPocZA11m4A1mBx5IxNgRH21ti7KbAGiyNn3HoF/gJ0w05A8xclpwAAAABJRU5ErkJggg==' />";
  // const char* headhtml = "<meta name='color-scheme' content='dark light'><style></style><script></script>";
  // wm.setCustomHeadElement(headhtml);

  // set custom html menu content , inside menu item "custom", see setMenu()
  // const char *menuhtml = "<form action='/custom' method='get'><button>Settings</button></form><br/>\n";
  // wifiManager.setCustomMenuHTML(menuhtml);

  // invert theme, dark
  // show scan RSSI as percentage, instead of signal stength graphic
  // wm.setScanDispPerc(true);

  //  Set cutom menu via menu[] or vector
  //  const char* menu[] = {"wifi","wifinoscan","info","param","close","sep","erase","restart","exit"};
  //  wm.setMenu(menu,9); // custom menu array must provide length

  // wifiManager.resetSettings();
  // wifiManager.setAPCallback();
  // wifiManager.startConfigPortal();
  // wifiManager.setEnableConfigPortal(false);

  */