#include "WiFiManager.h"
#include "configuration.h"

const char HTTP_CUSTOM_HEAD[] PROGMEM = "<style>.d-none{ display:none; }.error{background:red;} </style><script>function openPage(e){let a=e.parentNode,r=a.querySelector(\"[tab-name='\"+e.value+\"']\"),l=a.querySelectorAll(\":scope>.tabcontent\");for(var s=0;s<l.length;s++)l[s].classList.add(\"d-none\");null!==r&&r.classList.remove(\"d-none\")}function noDupeValues(e){for(var a=document.querySelectorAll(\"input[type=number]\"),r={},l=0,s=a.length;l<s;++l){var n=a[l].value.trim();if(n&&\"-1\"!=n){if(a[l].className=a[l].className.replace(\"error\",\"\"),r[n]){a[l].className+=\"error\",r[n].className+=\" error\",event.preventDefault();let t=document.querySelector(\"#message\");t.innerHTML=\"Some pins are usedmultiple times please checkvalues\",t.className=t.className.replace(\"d-none\",\"\")}r[n]=a[l]}}return!0}</script>";
const char HTTP_FORM_CUSTOM_PARAM[] PROGMEM = "<br/><input type='{r}' id='{i}' name='{i}' min='{e}' max='{h}' value='{v}' size='{l}'>\n"; // do not remove newline!
const char HTTP_FORM_SELECT_START[] PROGMEM = "<select onchange='{v}' id='{i}' name='{i}'>";
const char HTTP_FORM_SELECT_OPTION[] PROGMEM = "<option value='{v}' {c}>'{t}'</option>";
const char HTTP_FORM_SELECT_END[] PROGMEM = "</select>\n";
const char HTTP_FORM_TAB[] PROGMEM = "<div tab-name={'n'} class=\"tabcontent '{c}'\">";
const char HTTP_DIV_END[] PROGMEM = "</div>";
/*
          <label for="debugPort">Debug UART port:</label>
          <select name="debugPort">
            <option value="-1">None</option>
            <option value="0" selected>Uart0</option>
            <option value="2">Uart2</option></select
          ><br />
*/
class MyWiFiManagerParameter : WiFiManagerParameter
{
private:
    int _min;
    int _max;
    const char *_selectedValue = NULL;
    const char *_type;

protected:
    const char *getType() const;
    int getMin() const;
    int getMax() const;
    const char *getOptionLabel(int index);
    const char *getOptionValue(int index);
    const char *getSelectedValue();
    friend class MyWiFiManager;

public:
    MyWiFiManagerParameter(const char *id, const char *label, const char *type, const char *defaultValue, int size, int min, int max, const char *custom, int labelPlacement);
    MyWiFiManagerParameter(const char *id, const char *label, const char *optionLabels, const char *OptionValues, int length, const char *selectedValue, const char *onChange);
    MyWiFiManagerParameter(const char *custom);
};
struct Tab
{
    String label;
    int paramStart;
    int paramStop;
};

class MyWiFiManager : WiFiManager
{
private:
    const static int paramsAmount = 2;
    MyWiFiManagerParameter startParameters[paramsAmount] = {{"<h3>Tab One</h3>\n<p>11111111111</p>"}, {"<h3>Tab Two</h3>\n<p>22222222222</p>"}};

protected:
    MyWiFiManagerParameter **_params = NULL;
    String buildSelectOptions(const char *id, const char *label, const char *onChange, String *optionLabels, String *optionValues, int size, String *selectedValue);
    String getParamOut();
    String getParamRangeOut(int start, int stop);
    void handleParam();

public:
    MyWiFiManager();
};
