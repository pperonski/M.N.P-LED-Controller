#include "wifimanager/Parameter.h"

Parameter::Parameter(const char *_name, const char *_type, const char *_description, const char *__default, const uint32_t _maxLength, const uint32_t _minLength, const size_t _length)
    : name(_name), type(_type), description(_description), length(_length), _default(__default), maxLength(_maxLength), minLength(_minLength)
{
    additional_arguments = NULL;
}

const char *Parameter::getName()
{
    return name;
}

const char *Parameter::getDescription()
{
    return description;
}

const uint32_t Parameter::getBufferLength()
{
    return length;
}

const uint32_t Parameter::getMinLength()
{
    return minLength;
}

const uint32_t Parameter::getMaxLength()
{
    return maxLength;
}

void Parameter::update(String _value)
{
    char msg[_value.length()];
    _value.toCharArray(msg, _value.length());

    update(msg);
}

void Parameter::generateResponse(AsyncResponseStream *_response)
{
    DynamicJsonDocument doc(length);

    doc["type"] = type;
    doc["description"] = description;
    doc["default"] = _default;
    doc["minLength"] = minLength;
    doc["maxLength"] = maxLength;
    doc["additional_arguments"] = additional_arguments;

    serializeJson(doc, *_response);
}

void Parameter::addAdditionalArguments(const char *arguments)
{
    if (additional_arguments != NULL)
    {
        free(additional_arguments);
    }

    additional_arguments = (char *)calloc(strlen(arguments) + 1, sizeof(char));

    strcpy(additional_arguments, arguments);
}

void Parameter::getAdditionalArguments(char *arguments)
{
    if (additional_arguments != NULL)
    {
        strcpy(arguments, additional_arguments);
    }
}
