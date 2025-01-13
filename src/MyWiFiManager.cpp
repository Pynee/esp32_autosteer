#include "MyWiFiManager.h"

MyWiFiManagerParameter::MyWiFiManagerParameter(const char *id, const char *label, const char *type, const char *defaultValue, int size, int min, int max, const char *custom, int labelPlacement) : WiFiManagerParameter(id, label, defaultValue, size, custom, labelPlacement)
{
    _min = min;
    _max = max;
    _type = type;
};
MyWiFiManagerParameter::MyWiFiManagerParameter(const char *id, const char *label, const char *optionLabels, const char *OptionValues, int length, const char *selectedValue, const char *onChange) : WiFiManagerParameter(id, optionLabels, OptionValues, length, onChange)
{
    _selectedValue = selectedValue;
    _type = label;
}

MyWiFiManagerParameter::MyWiFiManagerParameter(const char *custom) : WiFiManagerParameter(custom) {};

int MyWiFiManagerParameter::getMin() const
{
    return _min;
}
int MyWiFiManagerParameter::getMax() const
{
    return _max;
}
const char *MyWiFiManagerParameter::getOptionLabel(int index)
{
    if (index < _length)
    {
        return (const char *)_label[index];
    }
    return "";
}
const char *MyWiFiManagerParameter::getOptionValue(int index)
{
    if (index < _length)
    {
        return (const char *)_value[index];
    }
    return "";
}
const char *MyWiFiManagerParameter::getSelectedValue()
{
    return _selectedValue;
}
MyWiFiManager::MyWiFiManager() : WiFiManager()
{
    setCustomHeadElement((const char *)FPSTR(HTTP_CUSTOM_HEAD));
    for (int index = 0; index < paramsAmount; index++)
    {
        this->addParameter(&startParameters[index]);
    };
}
String MyWiFiManager::buildSelectOptions(const char *id, const char *label, const char *onChange, String *optionLabels, String *optionValues, int size, String *selectedValue)
{
    String page;
    String pitem = FPSTR(HTTP_FORM_LABEL);
    pitem += FPSTR(HTTP_FORM_SELECT_START);
    if (pitem.indexOf(FPSTR(T_t)) > 0)
        pitem.replace(FPSTR(T_t), label); // T_t title/label
    if (pitem.indexOf(FPSTR(T_i)) > 0)
        pitem.replace(FPSTR(T_i), id); // T_i id and name
    if (pitem.indexOf(FPSTR(T_v)) > 0)
        pitem.replace(FPSTR(T_v), onChange); // T_v value
    page += pitem;
    for (int j = 0; j < size; j++)
    {
        pitem = FPSTR(HTTP_FORM_SELECT_OPTION);
        pitem.replace(FPSTR(T_v), String(optionValues[j]));
        pitem.replace(FPSTR(T_t), String(optionLabels[j]));
        if (optionValues[i] == selectedValue)
        {
            pitem.replace(FPSTR(T_c), F(" selected"));
        }
        page += pitem;
    }
    page += FPSTR(HTTP_FORM_SELECT_END);
    return page;
}
String MyWiFiManager::getParamRangeOut(int start, int stop)
{
    String page;
    if (_paramsCount > stop)
    {

        String HTTP_PARAM_temp = FPSTR(HTTP_FORM_LABEL);
        HTTP_PARAM_temp += FPSTR(HTTP_FORM_CUSTOM_PARAM);
        bool tok_i = HTTP_PARAM_temp.indexOf(FPSTR(T_i)) > 0;
        bool tok_t = HTTP_PARAM_temp.indexOf(FPSTR(T_t)) > 0;
        bool tok_r = HTTP_PARAM_temp.indexOf(FPSTR(T_r)) > 0;
        bool tok_e = HTTP_PARAM_temp.indexOf(FPSTR(T_e)) > 0;
        bool tok_h = HTTP_PARAM_temp.indexOf(FPSTR(T_h)) > 0;
        bool tok_v = HTTP_PARAM_temp.indexOf(FPSTR(T_v)) > 0;
        bool tok_l = HTTP_PARAM_temp.indexOf(FPSTR(T_l)) > 0;

        char valLength[5];

        for (int i = start; i < stop; i++)
        {
            // Serial.println((String)_params[i]->_length);
            if (_params[i] == NULL || _params[i]->_length > 99999)
            {
// try to detect param scope issues, doesnt always catch but works ok
#ifdef WM_DEBUG_LEVEL
                DEBUG_WM(WM_DEBUG_ERROR, F("[ERROR] WiFiManagerParameter is out of scope"));
#endif
                return "";
            }
        }

        // add the extra parameters to the form
        for (int i = start; i < stop; i++)
        {
            // label before or after, @todo this could be done via floats or CSS and eliminated
            String pitem;
            // Input templating
            // "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>";
            // if no ID use customhtml for item, else generate from param string
            if (_params[i]->_selectedValue != NULL)
            {
                if (this->_params[i]->getID() != NULL)
                {
                    switch (_params[i]->getLabelPlacement())
                    {
                    case WFM_LABEL_BEFORE:
                        pitem = FPSTR(HTTP_FORM_LABEL);
                        pitem += FPSTR(HTTP_FORM_PARAM);
                        break;
                    case WFM_LABEL_AFTER:
                        pitem = FPSTR(HTTP_FORM_PARAM);
                        pitem += FPSTR(HTTP_FORM_LABEL);
                        break;
                    default:
                        // WFM_NO_LABEL
                        pitem = FPSTR(HTTP_FORM_PARAM);
                        break;
                    }
                    if (tok_t)
                        pitem.replace(FPSTR(T_t), _params[i]->getLabel()); // T_t title/label
                    if (tok_i)
                        pitem.replace(FPSTR(T_i), this->_params[i]->getID()); // T_i id and name
                    if (tok_r)
                        pitem.replace(FPSTR(T_r), _params[i]->getType()); // T_r input type
                    if (tok_e)
                        pitem.replace(FPSTR(T_e), std::to_string(_params[i]->getMin()).c_str()); // T_p min length
                    if (tok_h)
                        pitem.replace(FPSTR(T_h), std::to_string(_params[i]->getMax()).c_str()); // T_l max length
                    if (tok_v)
                        pitem.replace(FPSTR(T_v), _params[i]->getValue()); // T_v value
                    if (tok_l)
                        pitem.replace(FPSTR(T_l), std::to_string(_params[i]->getValueLength()).c_str()); // T_c text box length
                }
                else
                {
                    pitem = _params[i]->getCustomHTML();
                }
            }
            else
            {
                pitem = FPSTR(HTTP_FORM_LABEL);
                pitem += FPSTR(HTTP_FORM_SELECT_START);
                if (tok_t)
                    pitem.replace(FPSTR(T_t), _params[i]->getType()); // T_t title/label
                if (tok_i)
                    pitem.replace(FPSTR(T_i), this->_params[i]->getID()); // T_i id and name
                if (tok_v)
                    pitem.replace(FPSTR(T_v), _params[i]->getCustomHTML()); // T_v value
                page += pitem;
                for (int j = 0; j < _params[i]->_length; j++)
                {
                    pitem = FPSTR(HTTP_FORM_SELECT_OPTION);
                    pitem.replace(FPSTR(T_v), String(_params[i]->getOptionValue(j)));
                    pitem.replace(FPSTR(T_t), String(_params[i]->getOptionValue(j)));
                    if (_params[i]->getOptionValue(j) == _params[i]->getSelectedValue())
                    {
                        pitem.replace(FPSTR(T_c), F(" selected"));
                    }
                    page += pitem;
                }
                page += FPSTR(HTTP_FORM_SELECT_END);
            }
            page += pitem;
        }
    }
    return page;
};

String MyWiFiManager::getParamOut()
{
    String page;
    String pitem = "";

    pitem = FPSTR(HTTP_FORM_SELECT_START);
    pitem.replace(FPSTR(T_v), F("openPage(this)"));
    pitem.replace(FPSTR(T_n), F("board"));
    page = pitem;
    Tab tabs[] = {{"AIO 5 board", 0, 1}, {"ESP32 board", 2, 3}, {"Advanced", 3, 4}};
    String options[] = {"AIO 5 board", "ESP32 board", "Advanced"};
    for (int i = 0; i < sizeof(tabs) / sizeof(Tab); i++)
    {
        pitem = FPSTR(HTTP_FORM_SELECT_OPTION);
        pitem.replace(FPSTR(T_v), String(i));
        pitem.replace(FPSTR(T_t), tabs[i].label);
        if (i == board)
        {
            pitem.replace(FPSTR(T_c), F(" selected"));
        }
    }
    page += FPSTR(HTTP_FORM_SELECT_END);
    for (int i = 0; i < sizeof(tabs) / sizeof(Tab); i++)
    {
        page += pitem;
        pitem = FPSTR(HTTP_FORM_TAB);
        pitem.replace(FPSTR(T_n), String(i));
        if (i != board)
        {
            pitem.replace(FPSTR(T_c), "d-none");
        }
        page += getParamRangeOut(tabs[i].paramStart, tabs[i].paramStop);
        page += FPSTR(HTTP_DIV_END);
    };

    return page;
}
void MyWiFiManager::handleParam()
{

#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(WM_DEBUG_VERBOSE, F("<- HTTP Param"));
#endif
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleparam)); // @token titlewifi

    String pitem = "";

    pitem = FPSTR(HTTP_FORM_START);
    pitem.replace(FPSTR(T_v), F("paramsave"));
    page += pitem;

    page += getParamOut();
    page += FPSTR(HTTP_FORM_END);
    if (_showBack)
        page += FPSTR(HTTP_BACKBTN);
    reportStatus(page);
    page += FPSTR(HTTP_END);

    HTTPSend(page);

#ifdef WM_DEBUG_LEVEL
    DEBUG_WM(WM_DEBUG_DEV, F("Sent param page"));
#endif
}