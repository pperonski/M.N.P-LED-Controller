#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include <string.h>
#include <stdio.h>
#include "Parameter.h"

class TextInput : public Parameter
{
protected:
    char *value;

public:
    // name - name of parameter , description - description of html element, default - a default display value, maxLength - max text length, minLenght - min text length
    TextInput(const char *_name, const char *_description, const char *__default, const uint32_t _maxLength, const uint32_t _minLength = 0);

    void update(char *_value) override;

    void getValue(char *_value);

    void getValue(String &_value);

    ~TextInput();
};

#endif