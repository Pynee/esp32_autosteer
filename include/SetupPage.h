#include <WiFiManager.h>

std::string inputSelect(std::string name, std::string *strings, int stringArraySize, int selected)
{
    std::string returnStr = "<!-- INPUT SELECT --><br/><label for='input_select'>";
    returnStr.append(name.c_str());
    returnStr.append("</label><select name=\"input_select\" id=\"input_select\" class=\"button\">");

    // String returnString = "<!-- INPUT SELECT --><br/><label for='input_select'>" + name + "IMU connection</label><select name=\"input_select\" id=\"input_select\" class=\"button\">";

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
    Serial.println(returnStr.c_str());
    return returnStr;
}
struct SelectParameter
{
    std::string name;
    std::string selectStrings[10];
    int selectAmount;
    int selected;
    std::string customString = inputSelect(this->name, this->selectStrings, this->selectAmount, this->selected);
    SelectParameter(std::string name, const std::string *selectStrings, int arraySize, int selected)
    {
        this->name = name;
        for (int index = 0; index < arraySize; index++)
        {
            this->selectStrings[index] = selectStrings[index];
        }
        this->selectAmount = arraySize;
        this->selected = selected;
        // this->customString = inputSelect(name,  this->selectStrings, selectAmount, selected);
    }
};

/*struct SiteParameters
{
    // SelectParameter parameters[1] = {{std::string("IMU connection method: "), {std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3}};
    //  imuConnectionSelector.name.append("IMU connection method: ");
    //   WiFiManagerParameter custom_html("<p style=\"font-weight:Bold;\">AgOpenGPS Settings</p>"); // only custom html
    //   WiFiManagerParameter testSelect(testString.c_str());
};*/
// SiteParameters siteParams;
SelectParameter setupParameters[] =
    {{std::string("IMU connection method: "), (std::string[]){std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3},
     {std::string("IMU connection method: "), (std::string[]){std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3},
     {std::string("IMU connection method: "), (std::string[]){std::string("I2C"), std::string("RVC_UART"), std::string("UART"), std::string("SPI")}, 4, 3}};