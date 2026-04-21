#ifndef PARAMETER_HPP
#define PARAMETER_HPP

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#define PARAM_JSON_BUFFER 1024

class Parameter
{
protected:
    const char *name;
    const char *type; // form input type
    const char *description;
    const uint32_t length; // buffer length of Arduino JSON

    const char *_default;     // default value
    const uint32_t maxLength; // max length
    const uint32_t minLength; // min length

    char *additional_arguments; // additional arguments

public:
    // _name - name of the parameter
    // _description - description of the parameter
    Parameter(const char *_name, const char *_type, const char *_description, const char *__default, const uint32_t _maxLength, const uint32_t _minLength = 0, const uint32_t _length = PARAM_JSON_BUFFER);

    void addAdditionalArguments(const char *arguments);

    void getAdditionalArguments(char *argument);
    // update the parameter with new value
    virtual void update(char *_value) = 0;

    // void update(uint32_t _value);

    void update(String _value);

    // void update(bool _value);

    // get parameter's name
    const char *getName();

    // get description's name
    const char *getDescription();

    // get json buffer size
    const uint32_t getBufferLength();

    // get min length size
    const uint32_t getMinLength();

    // get max length size
    const uint32_t getMaxLength();

    // generate HTML element with ESP Async Web Server Response
    void generateResponse(AsyncResponseStream *_response);

    virtual ~Parameter()
    {
    }
};

#endif