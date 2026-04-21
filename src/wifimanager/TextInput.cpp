#include "wifimanager/TextInput.h"

TextInput::TextInput(const char *_name, const char *_description, const char *__default, const uint32_t _maxLength, const uint32_t _minLength)
    : Parameter(_name, "text", _description, __default, _maxLength, _minLength, 1024)
{

    value = (char *)calloc(maxLength + 1, sizeof(char));
    strcpy(value, _default);
}

void TextInput::update(char *_value)
{
    strcpy(value, _value);
}

void TextInput::getValue(char *_value)
{
    strcpy(_value, value);
}

void TextInput::getValue(String &_value)
{
    _value = value;
}

TextInput::~TextInput()
{
    free(value);
}